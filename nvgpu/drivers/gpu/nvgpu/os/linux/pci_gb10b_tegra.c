// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

/* GA10B platform data has been used as a baseline for GB10B */

#if defined (CONFIG_NVIDIA_CONFTEST)
#include <nvidia/conftest.h>
#endif

#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/devfreq.h>
#include <linux/device.h>
#include <linux/dma-buf.h>
#include <linux/fuse.h>
#include <linux/hashtable.h>
#include <linux/iommu.h>
#include <linux/list.h>
#include <linux/of_platform.h>
#include <linux/pci.h>
#include <linux/pm_opp.h>
#include <linux/pm_runtime.h>
#include <linux/reset.h>
#include <linux/sysfs.h>
#include <linux/thermal.h>
#include <linux/version.h>
#if defined(CONFIG_INTERCONNECT)
#include <linux/platform/tegra/mc_utils.h>
#include <linux/interconnect.h>
#endif

#include <nvgpu/gk20a.h>
#include <nvgpu/nvhost.h>
#include <nvgpu/soc.h>
#include <nvgpu/fuse.h>
#include <nvgpu/nvgpu_init.h>

#ifdef CONFIG_NV_TEGRA_BPMP
#include <soc/tegra/tegra-bpmp-dvfs.h>
#endif /* CONFIG_NV_TEGRA_BPMP */

#ifdef CONFIG_TEGRA_BPMP
#include <soc/tegra/bpmp.h>
#endif /* CONFIG_TEGRA_BPMP */

#include <uapi/linux/nvgpu.h>

#include "os/linux/clk.h"
#include "os/linux/module.h"
#include "os/linux/os_linux.h"
#include "os/linux/platform_gk20a.h"
#include "os/linux/platform_gk20a_tegra.h"
#include "os/linux/platform_gp10b.h"
#include "os/linux/scale.h"

#include <nvgpu/io.h>
#include "hal/fuse/fuse_gb10b.h"
#include <nvgpu/hw/gb10b/hw_fuse_gb10b.h>
#include <nvgpu/pmu/clk/clk.h>
#include <nvgpu/pmu/pmu_perfmon.h>
#include <nvgpu/power_features/pg.h>

/*
 * GB10B platform clock list:
 * platform->clk[0]- sysclk
 * platform->clk[1]- gpcclk
 * platform->clk[2]- nvdclk
 * platform->clk[3]- uprocclk
 */
#define NVGPU_CLK_SYS 0
#define NVGPU_CLK_GPC 1
#define NVGPU_CLK_NVD 2
#define NVGPU_CLK_UPROC 3

/*
 * TODO: clean these defines and include BPMP header.
 * For GB10B: JIRA NVGPU-9261
 */
#define TEGRA264_STRAP_NV_FUSE_CTRL_OPT_GPU		1U
#define TEGRA264_STRAP_NV_FUSE_CTRL_OPT_NVDEC	2U
#define TEGRA264_STRAP_NV_FUSE_CTRL_OPT_NVENC	3U
#define TEGRA264_STRAP_NV_FUSE_CTRL_OPT_OFA		4U
#define TEGRA264_STRAP_NV_FUSE_CTRL_OPT_NVJPG	5U
#define TEGRA264_STRAP_MAX						5U

#define GB10B_GPU_PG_MASK_NUM_BITS				17U
#define GB10B_NVDEC_PG_MASK_NUM_BITS			2U
#define GB10B_NVENC_PG_MASK_NUM_BITS			2U
#define GB10B_OFA_PG_MASK_NUM_BITS				1U
#define GB10B_NVJPG_PG_MASK_NUM_BITS			2U

#define GB10B_MAX_FS_GPC	3U
/*
 * there are 4 FBPs in GB10B, but FBP3 can never be floorswept
 * Hence, maximum floorsweepable FBP is 3
 */
#define GB10B_MAX_FS_FBP	3U

#define GB10B_MAX_FS_TPC_GPC0_1	4U
#define GB10B_MAX_FS_TPC_GPC2	3U

#define MASK_BIT_FIELD(val, _hi, _lo) \
	(((val) & ((1UL << ((_hi) + 1UL)) - 1UL)) >> (_lo))

#define NVGPU_GPC0_DISABLE  BIT(0)
#define NVGPU_GPC1_DISABLE  BIT(1)
#define NVGPU_GPC2_DISABLE  BIT(2)

#define NUM_MSS_PORTS	8
#define BUS_WIDTH	32
#define GPU_BANDWIDTH	(NUM_MSS_PORTS * BUS_WIDTH)

#define to_tegra_devfreq(x) \
	container_of(x, struct gb10b_tegra_devfreq_dev, dev)

struct gb10b_tegra_thermal_dev {
	struct list_head devices;
	const char *tzname;
#if defined(NV_THERMAL_ZONE_FOR_EACH_TRIP_PRESENT)
	const struct thermal_trip *trip;
#else
	int trip;
#endif
};

struct gb10b_tegra_devfreq_dev {
	struct device dev;
	const char *clk_name;
	const char *dev_name;
	const char *icc_name;
	unsigned int bw_util;
	struct list_head masters;
	struct list_head gpu_slaves;
	struct list_head video_slaves;
	struct clk *clk;
	struct devfreq *devfreq;
	struct list_head thermals;
#if defined(CONFIG_INTERCONNECT)
	struct icc_path *icc_path;
#endif
	ktime_t last_update;
	enum nvgpu_pmu_perfmon_class perfmon_class;
};

enum gb10b_devfreq_id {
	GB10B_DEVFREQ_GPU = 0,
	GB10B_DEVFREQ_VIDEO,
	GB10B_DEVFREQ_GPC0,
	GB10B_DEVFREQ_GPC1,
	GB10B_DEVFREQ_GPC2,
	GB10B_DEVFREQ_SYS,
};

static struct gb10b_tegra_devfreq_dev gb10b_devfreq_arr[] = {
	{
		.dev_name = "gpu-gpc",
		.icc_name = "gpu-write",
		.bw_util = 400,
		.perfmon_class = NVGPU_PMU_PERFMON_CLASS_GR,
	},
	{
		.clk_name = "nvdclk",
		.dev_name = "gpu-nvd",
		.icc_name = "video-write",
		.bw_util = 400,
		.perfmon_class = NVGPU_PMU_PERFMON_CLASS_NVD,
	},
	{ .clk_name = "gpc0clk", .dev_name = "gpc0clk" },
	{ .clk_name = "gpc1clk", .dev_name = "gpc1clk" },
	{ .clk_name = "gpc2clk", .dev_name = "gpc2clk" },
	{ .clk_name = "sysclk", .dev_name = "sysclk" },
};

static struct list_head gb10b_devfreq_masters;

static ssize_t bw_util_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf,
			     size_t count)
{
	struct gb10b_tegra_devfreq_dev *tdev = to_tegra_devfreq(dev);
	unsigned long rate;
	unsigned int util;
	u32 kBps;
	int err;

	err = kstrtouint(buf, 0, &util);
	if (err)
		return err;

	tdev->bw_util = min_t(unsigned int, util, 1000);

	if (tdev->clk && tdev->icc_path) {
		rate = clk_get_rate(tdev->clk);
		kBps = Bps_to_icc(rate * GPU_BANDWIDTH * tdev->bw_util / 1000);
		err = icc_set_bw(tdev->icc_path, 0, kBps);
		if (err) {
			dev_err(dev,
				"failed to set icc bw request for %s: %d\n",
				tdev->dev_name, err);
		}
	}

	return count;
}

static ssize_t bw_util_show(struct device *dev,
			    struct device_attribute *attr,
			    char *buf)
{
	struct gb10b_tegra_devfreq_dev *tdev = to_tegra_devfreq(dev);

	return sprintf(buf, "%u\n", tdev->bw_util);
}

static DEVICE_ATTR_RW(bw_util);

static struct attribute *dev_entries[] = {
	&dev_attr_bw_util.attr,
	NULL,
};

static struct attribute_group dev_attr_group = {
	.name = "tunable",
	.attrs = dev_entries,
};

static void gb10b_tegra_clk_disable_all(struct device *dev)
{
	struct gk20a *g = get_gk20a(dev);
	struct gb10b_tegra_devfreq_dev *tdev;
	struct gk20a_platform *platform = dev_get_drvdata(dev);
	unsigned int i;

	/*
	 * Early stop since clock management is not supported on simluation
	 * or FPGA platforms during early bringup time.
	 */
	if (nvgpu_platform_is_simulation(g) || nvgpu_platform_is_fpga(g))
		return;

	for (i = 0; i < ARRAY_SIZE(gb10b_devfreq_arr); i++) {
		tdev = gb10b_devfreq_arr + i;
		if (tdev->clk) {
			clk_disable_unprepare(tdev->clk);
		} else {
			nvgpu_log_info(g, "Clock data not available: %s", tdev->clk_name);
		}
	}

	/* disable uproc clock.  uproc clock not part of devfreq array */
	if (platform->clk[NVGPU_CLK_UPROC]) {
		clk_disable_unprepare(platform->clk[NVGPU_CLK_UPROC]);
	} else {
		nvgpu_log_info(g, "Clock data not available: %s", "uprocclk");
	}
}

static int gb10b_tegra_clk_enable_all(struct device *dev)
{
	struct gk20a *g = get_gk20a(dev);
	struct gb10b_tegra_devfreq_dev *tdev;
	struct gk20a_platform *platform = dev_get_drvdata(dev);
	unsigned int i;
	int ret;

	/*
	 * Early stop since clock management is not supported on simluation
	 * or FPGA platforms during early bringup time.
	 */
	if (nvgpu_platform_is_simulation(g) || nvgpu_platform_is_fpga(g))
		return 0;

	for (i = 0; i < ARRAY_SIZE(gb10b_devfreq_arr); i++) {
		tdev = gb10b_devfreq_arr + i;
		if (tdev->clk) {
			ret = clk_prepare_enable(tdev->clk);
			if(ret != 0) {
				nvgpu_err(g, "Clock enable not successful: %s", tdev->clk_name);
			}
			ret = clk_set_rate(tdev->clk,UINT_MAX);
			if(ret != 0) {
				nvgpu_err(g, "Clock rate set not successful: %s", tdev->clk_name);
			}
		} else {
			nvgpu_log_info(g, "Clock data not available: %d", i);
		}
	}

	/* enable uproc clock.  uproc clock not part of devfreq array */
	if (platform->clk[NVGPU_CLK_UPROC]) {
		ret = clk_prepare_enable(platform->clk[NVGPU_CLK_UPROC]);
		if(ret != 0) {
			nvgpu_err(g, "Clock enable not successful: %s", "uprocclk");
		}
		ret = clk_set_rate(platform->clk[NVGPU_CLK_UPROC],UINT_MAX);
		if(ret != 0) {
			nvgpu_err(g, "Clock rate set not successful: %s", "uprocclk");
		}
	} else {
		nvgpu_log_info(g, "Clock data not available: %s", "uprocclk");
	}

	return 0;
}

static bool gb10b_tegra_is_clock_available(struct device *dev, const char *clk_name)
{
#ifdef CONFIG_NVGPU_STATIC_POWERGATE
	struct gk20a *g = get_gk20a(dev);
	u32 gpc_disable, fbp_disable, tpc_disable[GB10B_MAX_FS_GPC],
		nvdec_disable, nvenc_disable, ofa_disable, nvjpg_disable;
	u32 i;

	/* sync hw fuse_status_opt_xxx_r on current floor-sweeping config */
	gpc_disable = nvgpu_readl(g, fuse_status_opt_gpc_r());
	fbp_disable = nvgpu_readl(g, fuse_status_opt_fbp_r());
	for (i = 0U; i < GB10B_MAX_FS_GPC; i++) {
		tpc_disable[i] = nvgpu_readl(g, fuse_status_opt_tpc_gpc_r(i));
	}
	nvdec_disable = nvgpu_readl(g, fuse_status_opt_nvdec_r());
	nvenc_disable = nvgpu_readl(g, fuse_status_opt_nvenc_r());
	ofa_disable = nvgpu_readl(g, fuse_status_opt_ofa_r());
	nvjpg_disable = nvgpu_readl(g, fuse_status_opt_nvjpg_r());

	g->gpu_pg_mask = gpc_disable;
	g->gpu_pg_mask |= (fbp_disable & 0x7U) << 3U;
	g->gpu_pg_mask |= tpc_disable[0] << 6U;
	g->gpu_pg_mask |= tpc_disable[1] << 10U;
	g->gpu_pg_mask |= (tpc_disable[2] & 0x7U) << 14U;
	g->nvdec_pg_mask = nvdec_disable;
	g->nvenc_pg_mask = nvenc_disable;
	g->ofa_pg_mask = ofa_disable;
	g->nvjpg_pg_mask = nvjpg_disable;

	/* Check for gpc/nvd floor-sweeping and report availability of gpcclks/nvdclk */
	if ((strcmp(clk_name, "gpc0clk") == 0) &&
		(gpc_disable & NVGPU_GPC0_DISABLE)) {
		nvgpu_log_info(g, "GPC0 is floor-swept");
		return false;
	}

	if ((strcmp(clk_name, "gpc1clk") == 0) &&
		(gpc_disable & NVGPU_GPC1_DISABLE)) {
		nvgpu_log_info(g, "GPC1 is floor-swept");
		return false;
	}

	if ((strcmp(clk_name, "gpc2clk") == 0) &&
		(gpc_disable & NVGPU_GPC2_DISABLE)) {
		nvgpu_log_info(g, "GPC2 is floor-swept");
		return false;
	}

	if ((strcmp(clk_name, "nvdclk") == 0) &&
		(nvdec_disable == ((1U << GB10B_NVDEC_PG_MASK_NUM_BITS) - 1U)) &&
		(nvenc_disable == ((1U << GB10B_NVENC_PG_MASK_NUM_BITS) - 1U)) &&
		(ofa_disable == ((1U << GB10B_OFA_PG_MASK_NUM_BITS) - 1U)) &&
		(nvjpg_disable == ((1U << GB10B_NVJPG_PG_MASK_NUM_BITS) - 1U))) {
		nvgpu_log_info(g, "NVD is floor-swept");
		return false;
	}
#endif

	return true;
}

static int gb10b_tegra_devfreq_target(struct device *dev,
				      unsigned long *freq,
				      u32 flags)
{
	struct gb10b_tegra_devfreq_dev *tdev = to_tegra_devfreq(dev);
	struct gb10b_tegra_devfreq_dev *slave, *master;
	unsigned long master_freq = 0;
	u32 kBps;
	int err;

	/*
	 * Access GPU clocks when rail gated would make
	 * BPMP-FW directly poweron GPU power domain, which
	 * leads to PCIe error.
	 */
	if (pm_runtime_suspended(dev->parent))
		return 0;


	clk_set_rate(tdev->clk, *freq);
	*freq = clk_get_rate(tdev->clk);

	/* Update memory bandwidth QoS request based on current frequency */
	if (tdev->icc_path) {
		kBps = Bps_to_icc(*freq * GPU_BANDWIDTH * tdev->bw_util / 1000);
		err = icc_set_bw(tdev->icc_path, 0, kBps);
		if (err) {
			dev_err(dev,
				"failed to set icc bw request for %s: %d\n",
				tdev->dev_name, err);
		}
	}

	/*
	 * Scale frequency of each slave device to the same frequency as its
	 * master. There is a special case that sysclk needs to be scaled to
	 * the max frequency value out of GPU and VIDEO devfreq devices to
	 * prevent performance bottleneck.
	 */
	if (tdev == &gb10b_devfreq_arr[GB10B_DEVFREQ_GPU]) {
		list_for_each_entry(slave, &tdev->gpu_slaves, gpu_slaves) {
			if (slave == &gb10b_devfreq_arr[GB10B_DEVFREQ_SYS]) {
				master = &gb10b_devfreq_arr[GB10B_DEVFREQ_VIDEO];

				if (master->clk) {
					master_freq = clk_get_rate(master->clk);
					if (master_freq > *freq)
						clk_set_rate(slave->clk, master_freq);
					else
						clk_set_rate(slave->clk, *freq);
				}
			} else {
				clk_set_rate(slave->clk, *freq);
			}
		}
	} else if (tdev == &gb10b_devfreq_arr[GB10B_DEVFREQ_VIDEO]) {
		list_for_each_entry(slave, &tdev->video_slaves, video_slaves) {
			if (slave == &gb10b_devfreq_arr[GB10B_DEVFREQ_SYS]) {
				master = &gb10b_devfreq_arr[GB10B_DEVFREQ_GPU];

				if (master->clk) {
					master_freq = clk_get_rate(master->clk);
					if (master_freq > *freq)
						clk_set_rate(slave->clk, master_freq);
					else
						clk_set_rate(slave->clk, *freq);
				}
			} else {
				clk_set_rate(slave->clk, *freq);
			}
		}
	}

	return 0;
}

static int gb10b_tegra_devfreq_get_cur_freq(struct device *dev,
					    unsigned long *freq)
{
	struct gb10b_tegra_devfreq_dev *tdev = to_tegra_devfreq(dev);

	*freq = clk_get_rate(tdev->clk);

	return 0;
}

static int gb10b_tegra_devfreq_get_dev_status(struct device *dev,
					      struct devfreq_dev_status *stat)
{
	struct gb10b_tegra_devfreq_dev *tdev = to_tegra_devfreq(dev);
	struct gk20a *g = get_gk20a(dev->parent);
	ktime_t t = ktime_get();
	unsigned long dt;
	u64 result;
	u32 load;

	dt = ktime_us_delta(t, tdev->last_update);

	stat->total_time = dt;

	/* Load is normalized so that value range is from 0 to 1000 */
	nvgpu_pmu_load_query(g, &load, tdev->perfmon_class);

	if (!nvgpu_safe_mult_u64_return((u64) load, (u64) dt, &result)) {
		dev_err(dev, "load multiplication overflow\n");
		return -EOVERFLOW;
	}

	stat->busy_time = result / PMU_BUSY_CYCLES_NORM_MAX;

	stat->current_frequency = clk_get_rate(tdev->clk);

	tdev->last_update = t;

	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 12, 0) \
	&& defined(NV_THERMAL_ZONE_FOR_EACH_TRIP_PRESENT) \
	&& defined(NV_DEVFREQ_DEV_PROFILE_HAS_IS_COOLING_DEVICE) \
	&& defined(CONFIG_DEVFREQ_THERMAL)
static int gb10b_tegra_thermal_get_passive_trip_cb(struct thermal_trip *trip,
						   void *arg)
{
	const struct thermal_trip **trip_ret = arg;

	/* Return zero to continue the search */
	if (trip->type != THERMAL_TRIP_PASSIVE)
		return 0;

	/* Return nonzero to terminate the search */
	*trip_ret = trip;
	return 1;
}

#endif
static int gb10b_tegra_devfreq_register(struct device *dev)
{
	struct gb10b_tegra_devfreq_dev *tdev = to_tegra_devfreq(dev);
	struct devfreq_dev_profile *profile;
	struct devfreq *devfreq;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 12, 0) \
	&& defined(NV_DEVFREQ_DEV_PROFILE_HAS_IS_COOLING_DEVICE) \
	&& defined(CONFIG_DEVFREQ_THERMAL)
	struct gb10b_tegra_thermal_dev *thdev;
	struct thermal_zone_device *tzdev;
#if defined(NV_THERMAL_ZONE_FOR_EACH_TRIP_PRESENT)
	const struct thermal_trip *trip = NULL;
#else
	enum thermal_trip_type type;
	int j;
#endif
	const char *tzname;
	int i, err, count;
	u32 thmin, thmax;
#endif
	unsigned long max_rate, min_rate, margin, rate;
	long val;

	/* Get max frequency step */
	val = clk_round_rate(tdev->clk, ULONG_MAX);
	max_rate = (val < 0) ? ULONG_MAX : (unsigned long)val;

	/* Get min frequency step */
	val = clk_round_rate(tdev->clk, 0);
	min_rate = (val < 0) ? ULONG_MAX : (unsigned long)val;

	/* Get frequency margin size */
	val = clk_round_rate(tdev->clk,
			     (ULONG_MAX - min_rate) ? (min_rate + 1) : ULONG_MAX);
	margin = (val < 0) ? 0 : (unsigned long)val - min_rate;

	rate = min_rate;
	do {
		dev_pm_opp_add(&tdev->dev, rate, 0);

		if ((ULONG_MAX - rate) < margin)
			break;

		rate += margin;
	} while (rate <= max_rate && margin);

	/* Prepare devfreq profile with driver-specific callbacks */
	profile = devm_kzalloc(dev, sizeof(*profile), GFP_KERNEL);
	if (!profile)
		return -ENOMEM;

	profile->target = gb10b_tegra_devfreq_target;
	profile->get_cur_freq = gb10b_tegra_devfreq_get_cur_freq;
	profile->get_dev_status = gb10b_tegra_devfreq_get_dev_status;
	profile->initial_freq = clk_get_rate(tdev->clk);
	profile->polling_ms = 25;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 12, 0) \
	&& defined(NV_DEVFREQ_DEV_PROFILE_HAS_IS_COOLING_DEVICE) \
	&& defined(CONFIG_DEVFREQ_THERMAL)
	profile->is_cooling_device = true;
#endif

	/* Register the device into devfreq framework with the defined profile */
	devfreq = devm_devfreq_add_device(dev,
					  profile,
					  DEVFREQ_GOV_PERFORMANCE,
					  NULL);
	if (IS_ERR(devfreq)) {
		dev_warn(dev->parent,
			 "failed to expose %s under with performance governor",
			 dev_name(dev));
		return PTR_ERR(devfreq);
	}

	tdev->devfreq = devfreq;

#if defined(NV_DEVFREQ_HAS_SUSPEND_FREQ)
    tdev->devfreq->suspend_freq = tdev->devfreq->scaling_max_freq;
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 12, 0) \
	&& defined(NV_DEVFREQ_DEV_PROFILE_HAS_IS_COOLING_DEVICE) \
	&& defined(CONFIG_DEVFREQ_THERMAL)
	/*
	 * Early stop the cooling device registration if there is no cooling
	 * device for the devfreq device
	 */
	if (!devfreq->cdev) {
		dev_err(dev->parent,
			"%s devfreq device don't have cooling device\n",
			dev_name(dev));
		err = -ENODEV;
		goto err_thermal_cdev;
	}

	/*
	 * Early stop the cooling device registration if the required DT
	 * properties are not specified
	 */
	count = of_property_count_strings(dev->parent->of_node,
					  "nvidia,thermal-zones");
	if (count == -EINVAL) {
		return 0;
	} else if (count < 0) {
		err = count;
		goto err_thermal_cdev;
	}

	err = of_property_count_u32_elems(dev->parent->of_node,
					  "nvidia,cooling-device");
	if (err == -EINVAL) {
		dev_err(dev->parent,
			"'nvidia,thermal-zones' and 'nvidia,cooling-device' "
			"properties should coexist\n");
		goto err_thermal_cdev;
	} else if (err < 0) {
		goto err_thermal_cdev;
	}

	/* Sanity check of the format of nvidia,thermal-zones property */
	if ((err / 2) != count) {
		dev_err(dev->parent,
			"wrong format of nvidia,thermal-zones property\n");
		err = -EINVAL;
		goto err_thermal_cdev;
	}

	/* Sanity check of the format of nvidia,cooling-device property */
	if (!(err / 2) || err % 2) {
		dev_err(dev->parent,
			"wrong format of nvidia,cooling-device property\n");
		err = -EINVAL;
		goto err_thermal_cdev;
	}

	/*
	 * Register devfreq cooling device to the specified thermal zones with
	 * passive SW throttling trip points. Instead of hardcoding the names,
	 * fetching thermal zone names from the device tree so that different
	 * GPU entities on different chips can register themselves into proper
	 * thermal zones based on chip location.
	 */
	for (i = 0; i < count; i++) {
		of_property_read_string_index(dev->parent->of_node,
					      "nvidia,thermal-zones", i, &tzname);
		of_property_read_u32_index(dev->parent->of_node,
					   "nvidia,cooling-device", i*2+0, &thmin);
		of_property_read_u32_index(dev->parent->of_node,
					   "nvidia,cooling-device", i*2+1, &thmax);

		tzdev = thermal_zone_get_zone_by_name(tzname);
		if (!IS_ERR(tzdev)) {
#if defined(NV_THERMAL_ZONE_FOR_EACH_TRIP_PRESENT)
			thermal_zone_for_each_trip(tzdev,
						   gb10b_tegra_thermal_get_passive_trip_cb,
						   &trip);
			if (!trip) {
				dev_err(dev->parent,
					"fail to find passive trip in %s thermal zone",
					tzname);
				goto err_thermal;
			}

			err = thermal_bind_cdev_to_trip(tzdev, trip, devfreq->cdev,
							thmax, thmin, THERMAL_WEIGHT_DEFAULT);
			if (err) {
				dev_err(dev->parent,
					"fail to bind cdev to %s thermal zone\n",
					tzname);
				goto err_thermal;
			}

			thdev = devm_kzalloc(dev, sizeof(*thdev), GFP_KERNEL);
			if (!thdev) {
				err = -ENOMEM;
				goto err_thermal;
			}

			thdev->tzname = tzname;
			thdev->trip = trip;
			list_add_tail(&thdev->devices,
				      &tdev->thermals);
#else
			for (j = 0; j < tzdev->num_trips; j++) {
#if defined(NV_THERMAL_ZONE_DEVICE_OPS_STRUCT_HAS_GET_TRIP_TYPE)
				err = tzdev->ops->get_trip_type(tzdev, j, &type);
#else
				type = tzdev->trips[j].type;
#endif
				if (err || type != THERMAL_TRIP_PASSIVE)
					continue;

				err = thermal_zone_bind_cooling_device(tzdev, j,
								       devfreq->cdev,
								       thmin,
								       thmax,
								       THERMAL_WEIGHT_DEFAULT);
				if (!err) {
					thdev = devm_kzalloc(dev, sizeof(*thdev), GFP_KERNEL);
					if (!thdev) {
						err = -ENOMEM;
						goto err_thermal;
					}

					thdev->tzname = tzname;
					thdev->trip = j;
					list_add_tail(&thdev->devices,
						      &tdev->thermals);
				} else {
					dev_err(dev->parent,
						"fail to bind to %s thermal zone\n",
						tzname);
					goto err_thermal;
				}
			}
#endif
		} else {
			dev_err(dev->parent,
				"cannot find %s thermal zone device\n", tzname);
			err = -ENODEV;
			goto err_thermal;
		}
	}
#endif
	return 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 12, 0) \
	&& defined(NV_DEVFREQ_DEV_PROFILE_HAS_IS_COOLING_DEVICE) \
	&& defined(CONFIG_DEVFREQ_THERMAL)
err_thermal:
	list_for_each_entry(thdev, &tdev->thermals, devices) {
		tzdev = thermal_zone_get_zone_by_name(thdev->tzname);
		if (IS_ERR(tzdev)) {
			dev_warn(dev->parent,
				 "cannot find %s thermal zone device\n",
				 thdev->tzname);
			continue;
		}

#if defined(NV_THERMAL_ZONE_FOR_EACH_TRIP_PRESENT)
		thermal_unbind_cdev_from_trip(tzdev,
					      thdev->trip,
					      tdev->devfreq->cdev);
#else
		thermal_zone_unbind_cooling_device(tzdev,
						   thdev->trip,
						   tdev->devfreq->cdev);
#endif
	}
err_thermal_cdev:
	devm_devfreq_remove_device(dev, tdev->devfreq);
	dev_pm_opp_remove_all_dynamic(dev);
	tdev->devfreq = NULL;
	return err;
#endif
}

static void gb10b_tegra_devfreq_unregister(struct device *dev)
{
	struct gb10b_tegra_devfreq_dev *tdev = to_tegra_devfreq(dev);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 12, 0) \
	&& defined(NV_DEVFREQ_DEV_PROFILE_HAS_IS_COOLING_DEVICE) \
	&& defined(CONFIG_DEVFREQ_THERMAL)
	struct gb10b_tegra_thermal_dev *thdev;
	struct thermal_zone_device *tzdev;
#endif

	if (tdev->devfreq) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 12, 0) \
	&& defined(NV_DEVFREQ_DEV_PROFILE_HAS_IS_COOLING_DEVICE) \
	&& defined(CONFIG_DEVFREQ_THERMAL)
		list_for_each_entry(thdev, &tdev->thermals, devices) {
			tzdev = thermal_zone_get_zone_by_name(thdev->tzname);
			if (IS_ERR(tzdev)) {
				dev_warn(dev->parent,
					 "cannot find %s thermal zone device\n",
					 thdev->tzname);
				continue;
			}

#if defined(NV_THERMAL_ZONE_FOR_EACH_TRIP_PRESENT)
			thermal_unbind_cdev_from_trip(tzdev,
						      thdev->trip,
						      tdev->devfreq->cdev);
#else
			thermal_zone_unbind_cooling_device(tzdev,
							   thdev->trip,
							   tdev->devfreq->cdev);
#endif
		}
#endif
		devm_devfreq_remove_device(&tdev->dev, tdev->devfreq);
		dev_pm_opp_remove_all_dynamic(&tdev->dev);
		tdev->devfreq = NULL;
	}
}

static void gb10b_tegra_device_release(struct device *dev)
{
	;
}

static unsigned long gb10b_linux_clk_get_rate(
					struct gk20a *g, u32 api_domain)
{
	struct gk20a_platform *platform = gk20a_get_platform(dev_from_gk20a(g));
	unsigned long rate = 0;

	/*
	 * Early stop since clock management is not supported on simluation
	 * or FPGA platforms during early bringup time.
	 */
	if (nvgpu_platform_is_simulation(g) || nvgpu_platform_is_fpga(g))
		return rate;

	switch (api_domain) {
	case CTRL_CLK_DOMAIN_SYSCLK:
		rate = clk_get_rate(platform->clk[NVGPU_CLK_SYS]);
		break;
	case CTRL_CLK_DOMAIN_GPCCLK:
		rate = clk_get_rate(platform->clk[NVGPU_CLK_GPC]);
		break;
	case CTRL_CLK_DOMAIN_NVDCLK:
		rate = clk_get_rate(platform->clk[NVGPU_CLK_NVD]);
		break;
	default:
		nvgpu_err(g, "unknown clock: 0x%x", api_domain);
		break;
	}

	return rate;
}

static s32 devfreq_pm_qos_read_value(struct devfreq *df, enum dev_pm_qos_req_type type)
{
        struct device *dev = df->dev.parent;
        struct dev_pm_qos *pm_qos = dev->power.qos;
        s32 ret = 0;
        unsigned long flags;

        spin_lock_irqsave(&dev->power.lock, flags);

        switch (type) {
        case DEV_PM_QOS_MIN_FREQUENCY:
                ret = IS_ERR_OR_NULL(pm_qos) ? PM_QOS_MIN_FREQUENCY_DEFAULT_VALUE
                        : READ_ONCE(pm_qos->freq.min_freq.target_value);
                break;
        case DEV_PM_QOS_MAX_FREQUENCY:
                ret = IS_ERR_OR_NULL(pm_qos) ? PM_QOS_MAX_FREQUENCY_DEFAULT_VALUE
                        : READ_ONCE(pm_qos->freq.max_freq.target_value);
                break;
        default:
                break;
        }

        spin_unlock_irqrestore(&dev->power.lock, flags);

        return ret;
}

static void devfreq_get_freq_range(struct devfreq *df,
                                   unsigned long *min_freq,
                                   unsigned long *max_freq)
{
        s32 qos_min_freq, qos_max_freq;

#if defined(NV_DEVFREQ_HAS_FREQ_TABLE)
	if (df->max_state == 0U) {
		*min_freq = ULONG_MAX;
		*max_freq = ULONG_MAX;
		return;
	}

        *min_freq = df->freq_table[0];
        *max_freq = df->freq_table[df->max_state - 1];
#else
	if (df->profile->max_state == 0U) {
		*min_freq = ULONG_MAX;
		*max_freq = ULONG_MAX;
		return;
	}

        *min_freq = df->profile->freq_table[0];
        *max_freq = df->profile->freq_table[df->profile->max_state - 1];
#endif

        qos_min_freq = devfreq_pm_qos_read_value(df, DEV_PM_QOS_MIN_FREQUENCY);
        qos_max_freq = devfreq_pm_qos_read_value(df, DEV_PM_QOS_MAX_FREQUENCY);

        /* Apply constraints from PM QoS */
        *min_freq = max(*min_freq, (unsigned long)qos_min_freq * 1000);
        if (qos_max_freq != PM_QOS_MAX_FREQUENCY_DEFAULT_VALUE)
                *max_freq = min(*max_freq, (unsigned long)qos_max_freq * 1000);

        /* Apply constraints from OPP framework */
        *min_freq = max(*min_freq, df->scaling_min_freq);
        *max_freq = min(*max_freq, df->scaling_max_freq);
}

static int gb10b_linux_clk_set_rate(struct gk20a *g,
				     u32 api_domain, unsigned long rate)
{
	struct gb10b_tegra_devfreq_dev *tdev;
	unsigned long min_rate = 0, max_rate = ULONG_MAX;
	int ret = 0;

	/*
	 * Early stop since clock management is not supported on simluation
	 * or FPGA platforms during early bringup time.
	 */
	if (nvgpu_platform_is_simulation(g) || nvgpu_platform_is_fpga(g))
		return 0;

	switch (api_domain) {
	case CTRL_CLK_DOMAIN_SYSCLK:
		/*
		 * Use raw clk_set_rate API since GB10B_DEVFREQ_SYS is not a
		 * master devfreq device.
		 */
		tdev = &gb10b_devfreq_arr[GB10B_DEVFREQ_SYS];
		if (!tdev->devfreq) {
			nvgpu_err(g, "no backed devfreq device");
			return -ENODEV;
		}

		devfreq_get_freq_range(tdev->devfreq, &min_rate, &max_rate);
		rate = max(rate, min_rate);
		rate = min(rate, max_rate);

		if (nvgpu_is_powered_on(g)) {
			ret = gk20a_busy(g);
			if (ret)
				return ret;
			ret = nvgpu_pg_elpg_protected_call(g, clk_set_rate(tdev->clk, rate));
			gk20a_idle(g);
		}
		if (ret != 0) {
			nvgpu_err(g,
				  "Setting %s frequency failed!",
				  tdev->clk_name);
		}
		return ret;
	case CTRL_CLK_DOMAIN_GPCCLK:
		tdev = &gb10b_devfreq_arr[GB10B_DEVFREQ_GPU];
		break;
	case CTRL_CLK_DOMAIN_NVDCLK:
		tdev = &gb10b_devfreq_arr[GB10B_DEVFREQ_VIDEO];
		break;
	default:
		nvgpu_err(g, "unknown clock domain: 0x%x", api_domain);
		return -EINVAL;
	}

	if (!tdev->devfreq) {
		nvgpu_err(g, "no backed devfreq device");
		return -ENODEV;
	}

	devfreq_get_freq_range(tdev->devfreq, &min_rate, &max_rate);
	rate = max(rate, min_rate);
	rate = min(rate, max_rate);

	if (nvgpu_is_powered_on(g)) {
		ret = gk20a_busy(g);
		if (ret)
			return ret;
		ret = nvgpu_pg_elpg_protected_call(g, \
			gb10b_tegra_devfreq_target(&tdev->dev, &rate, 0u));
		gk20a_idle(g);
	}

	if (ret != 0) {
		nvgpu_err(g, "Setting %s frequency failed!", tdev->clk_name);
	}

	return ret;
}

static int gb10b_tegra_device_register(struct device *dev,
				       struct gb10b_tegra_devfreq_dev *tdev,
				       int node)
{
	int err = 0;

	err = dev_set_name(&tdev->dev, "%s-%d", tdev->dev_name, node);
	if (err) {
		dev_err(dev,
			"failed to set devfreq name as %s\n",
			tdev->dev_name);
		tdev->clk = NULL;
		return err;
	}

	tdev->dev.parent = dev;
	tdev->dev.release = gb10b_tegra_device_release;
	err = device_register(&tdev->dev);
	if (err) {
		dev_err(dev,
			"failed to register %s dev under %s dev\n",
			tdev->dev_name, dev_name(dev));
		put_device(&tdev->dev);
		tdev->clk = NULL;
		return err;
	}

	err = sysfs_create_group(&tdev->dev.kobj, &dev_attr_group);
	if (err) {
		dev_err(dev,
			"failed to create device attributes for %s\n",
			tdev->dev_name);
		device_unregister(&tdev->dev);
		tdev->clk = NULL;
		return err;
	}

	return err;
}

static int gb10b_tegra_probe(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct pci_bus *pbus = pdev->bus;
	struct gk20a_platform *platform = dev_get_drvdata(dev);
	struct gk20a *g = get_gk20a(dev);
	struct gb10b_tegra_devfreq_dev *tdev, *gpu, *video, *slave;
	struct pci_host_bridge *bridge;
	struct device *ctrl;
	unsigned int i;
	int node = 0, err = 0;

	platform->disable_bigpage = !iommu_get_domain_for_dev(dev) &&
		(NVGPU_CPU_PAGE_SIZE < SZ_64K);

	/*
	 * Early stop since clock management is supported only on Si.
	 */
	if (!nvgpu_platform_is_silicon(g))
		return 0;

	/*
	 * Get NUMA node id to differentiate different GPUs on socket0 or
	 * socket1. NUMA information is expected to be saved in the DT node
	 * of the upstream connected PCIe host controller.
	 */
	while (pbus->parent)
		pbus = pbus->parent;

	bridge = to_pci_host_bridge(pbus->bridge);
	ctrl = bridge->dev.parent;

#ifdef CONFIG_NUMA
	node = dev_to_node(ctrl);
	if (node < 0) {
		dev_dbg(dev, "No numa-node-id for %s\n", dev_name(ctrl));
		node = 0;
	}
#endif

	/*
	 * Initialize tegra devfreq device info but skip the initialization
	 * of GB10B_DEVFREQ_GPU device in this block. The primary clock used
	 * for GB10B_DEVFREQ_GPU device will be one of these clocks [gpc0clk,
	 * gpc1clk, gpc2clk] based on their availability.
	 */
	for (i = GB10B_DEVFREQ_GPU + 1; i < ARRAY_SIZE(gb10b_devfreq_arr); i++) {
		tdev = gb10b_devfreq_arr + i;

		INIT_LIST_HEAD(&tdev->masters);
		INIT_LIST_HEAD(&tdev->gpu_slaves);
		INIT_LIST_HEAD(&tdev->video_slaves);
		INIT_LIST_HEAD(&tdev->thermals);

		/*
		 * Skip initialization if the component driven by this clock
		 * is floorswept or static powergated at boot time
		 */
		if (!gb10b_tegra_is_clock_available(dev, tdev->clk_name))
			continue;

		tdev->clk = devm_clk_get(dev, tdev->clk_name);
		if (IS_ERR_OR_NULL(tdev->clk)) {
			dev_warn(dev,
				 "failed to get %s clock: %ld\n",
				 tdev->clk_name, PTR_ERR(tdev->clk));
			tdev->clk = NULL;
			continue;
		}
		clk_set_rate(tdev->clk, UINT_MAX);

#if defined(CONFIG_INTERCONNECT)
		if (tdev->icc_name) {
			tdev->icc_path = devm_of_icc_get(dev, tdev->icc_name);
			if (IS_ERR_OR_NULL(tdev->icc_path)) {
				dev_warn(dev,
					 "fail to get icc path %ld\n",
					 PTR_ERR(tdev->icc_path));
				tdev->icc_path = NULL;
			}
		}
#endif

		err = gb10b_tegra_device_register(dev, tdev, node);
		if (err)
			goto err_probe;
	}

	/*
	 * Initialize GB10B_DEVFREQ_GPU tegra devfreq device info based on
	 * the availability of these clocks [gpc0clk, gpc1clk,gpc2clk].
	 */
	tdev = &gb10b_devfreq_arr[GB10B_DEVFREQ_GPU];

	INIT_LIST_HEAD(&tdev->masters);
	INIT_LIST_HEAD(&tdev->gpu_slaves);
	INIT_LIST_HEAD(&tdev->video_slaves);
	INIT_LIST_HEAD(&tdev->thermals);

	if (gb10b_devfreq_arr[GB10B_DEVFREQ_GPC0].clk) {
		tdev->clk_name = gb10b_devfreq_arr[GB10B_DEVFREQ_GPC0].clk_name;
		tdev->clk = devm_clk_get(dev, tdev->clk_name);
	} else if (gb10b_devfreq_arr[GB10B_DEVFREQ_GPC1].clk) {
		tdev->clk_name = gb10b_devfreq_arr[GB10B_DEVFREQ_GPC1].clk_name;
		tdev->clk = devm_clk_get(dev, tdev->clk_name);
	} else if (gb10b_devfreq_arr[GB10B_DEVFREQ_GPC2].clk) {
		tdev->clk_name = gb10b_devfreq_arr[GB10B_DEVFREQ_GPC2].clk_name;
		tdev->clk = devm_clk_get(dev, tdev->clk_name);
	} else {
		dev_err(dev,
			"no gpc clock is available for gpu devfreq device\n");
		err = -ENODEV;
		goto err_probe;
	}

#if defined(CONFIG_INTERCONNECT)
	if (tdev->icc_name) {
		tdev->icc_path = devm_of_icc_get(dev, tdev->icc_name);
		if (IS_ERR_OR_NULL(tdev->icc_path)) {
			dev_warn(dev,
				 "fail to get icc path %ld\n",
				 PTR_ERR(tdev->icc_path));
			tdev->icc_path = NULL;
		}
	}
#endif

	err = gb10b_tegra_device_register(dev, tdev, node);
	if (err)
		goto err_probe;

	/*
	 * Construct the devfreq topology. Only devfreq devices in the
	 * gb10b_devfreq_masters could be registered into the devfreq
	 * framework. Here is the topology:
	 *
	 * masters
	 * ├── gpu([gpc0clk,gpc1clk,gpc2clk])
	 * │   ├── gpc0clk (if gpu->clk != gpc0clk)
	 * │   ├── gpc1clk (if gpu->clk != gpc1clk)
	 * │   ├── gpc2clk (if gpu->clk != gpc2clk)
	 * │   └── sysclk
	 * └── video(nvdclk)
	 *     └── sysclk
	 */
	INIT_LIST_HEAD(&gb10b_devfreq_masters);

	/* Wiring the gpu clock */
	gpu = &gb10b_devfreq_arr[GB10B_DEVFREQ_GPU];
	if (gpu->clk) {
		slave = &gb10b_devfreq_arr[GB10B_DEVFREQ_GPC0];
		if (slave->clk && slave->clk != gpu->clk)
			list_add_tail(&slave->gpu_slaves, &gpu->gpu_slaves);

		slave = &gb10b_devfreq_arr[GB10B_DEVFREQ_GPC1];
		if (slave->clk && slave->clk != gpu->clk)
			list_add_tail(&slave->gpu_slaves, &gpu->gpu_slaves);

		slave = &gb10b_devfreq_arr[GB10B_DEVFREQ_GPC2];
		if (slave->clk && slave->clk != gpu->clk)
			list_add_tail(&slave->gpu_slaves, &gpu->gpu_slaves);

		slave = &gb10b_devfreq_arr[GB10B_DEVFREQ_SYS];
		if (slave->clk)
			list_add_tail(&slave->gpu_slaves, &gpu->gpu_slaves);

		list_add_tail(&gpu->masters, &gb10b_devfreq_masters);
	}

	/* Wiring the video clock */
	video = &gb10b_devfreq_arr[GB10B_DEVFREQ_VIDEO];
	if (video->clk) {
		slave = &gb10b_devfreq_arr[GB10B_DEVFREQ_SYS];
		if (slave->clk)
			list_add_tail(&slave->video_slaves, &video->video_slaves);

		list_add_tail(&video->masters, &gb10b_devfreq_masters);
	}

	/*
	 * Hook clocks to platform data to support clk_arb fot devtool test.
	 * Ideally, we should deprecate the usage of clk_arb and expose clock
	 * control only through linux devfreq framework to avoid ambiguity.
	 */
	platform->clk[NVGPU_CLK_SYS] = gb10b_devfreq_arr[GB10B_DEVFREQ_SYS].clk;
	platform->clk[NVGPU_CLK_GPC] = gb10b_devfreq_arr[GB10B_DEVFREQ_GPU].clk;
	platform->clk[NVGPU_CLK_NVD] = gb10b_devfreq_arr[GB10B_DEVFREQ_VIDEO].clk;

	/* Handle uproc clock outside of devfreq framework.
	 * Todo: Does uproc clock need to be part of devfreq? */
	platform->clk[NVGPU_CLK_UPROC] = devm_clk_get(dev, "uprocclk");
	if (IS_ERR_OR_NULL(platform->clk[NVGPU_CLK_UPROC])) {
		dev_warn(dev,
			 "failed to get %s clock: %ld\n",
			 "uprocclk", PTR_ERR(platform->clk[NVGPU_CLK_UPROC]));
		platform->clk[NVGPU_CLK_UPROC] = NULL;
	}

	nvgpu_linux_init_clk_support(platform->g);
	g->ops.clk.get_rate = gb10b_linux_clk_get_rate;
	g->ops.clk.set_rate = gb10b_linux_clk_set_rate;

	/* Register devfreq devices into devfreq framework */
	list_for_each_entry(tdev, &gb10b_devfreq_masters, masters) {
		err = gb10b_tegra_devfreq_register(&tdev->dev);
		if (err) {
			dev_warn(dev,
				 "failed to register %s into devfreq framework: %d\n",
				 tdev->dev_name, err);
		}
	}

	return 0;

err_probe:
	for (i = 0; i < ARRAY_SIZE(gb10b_devfreq_arr); i++) {
		tdev = gb10b_devfreq_arr + i;
		if (!tdev->clk)
			continue;
		devm_clk_put(dev, tdev->clk);
		tdev->clk = NULL;
		sysfs_remove_group(&tdev->dev.kobj, &dev_attr_group);
		device_unregister(&tdev->dev);
	}
	return err;
}

static int gb10b_tegra_late_probe(struct device *dev)
{
	struct gk20a_platform *platform = gk20a_get_platform(dev);
	int err;

	err = gk20a_tegra_init_secure_alloc(platform);
	if (err)
		return err;

	/* for bringup, enable the clocks */
	err = gb10b_tegra_clk_enable_all(dev);
	if (err)
		return err;

	return 0;
}

static int gb10b_tegra_remove(struct device *dev)
{
	struct gb10b_tegra_devfreq_dev *tdev;
	unsigned int i;

	if (!pm_runtime_status_suspended(dev)) {
		gb10b_tegra_clk_disable_all(dev);
	}

#ifdef CONFIG_TEGRA_GK20A_NVHOST
	nvgpu_free_nvhost_dev(get_gk20a(dev));
#endif

	for (i = 0; i < ARRAY_SIZE(gb10b_devfreq_arr); i++) {
		tdev = gb10b_devfreq_arr + i;

#if defined(CONFIG_INTERCONNECT)
		if (tdev->icc_path) {
			icc_set_bw(tdev->icc_path, 0, 0);
			tdev->icc_path = NULL;
		}
#endif

		gb10b_tegra_devfreq_unregister(&tdev->dev);

		if (tdev->clk) {
			devm_clk_put(dev, tdev->clk);
			tdev->clk = NULL;
			sysfs_remove_group(&tdev->dev.kobj, &dev_attr_group);
			device_unregister(&tdev->dev);
		}

		INIT_LIST_HEAD(&tdev->masters);
		INIT_LIST_HEAD(&tdev->gpu_slaves);
		INIT_LIST_HEAD(&tdev->video_slaves);
		INIT_LIST_HEAD(&tdev->thermals);
	}

	INIT_LIST_HEAD(&gb10b_devfreq_masters);

	return 0;
}

static int gb10b_tegra_static_pg_control(struct device *dev, struct tegra_bpmp *bpmp,
		struct mrq_strap_request *req)
{
#if defined(CONFIG_TEGRA_BPMP) && defined(CONFIG_NVGPU_STATIC_POWERGATE)
	struct tegra_bpmp_message msg;
	int err;

	memset(&msg, 0, sizeof(msg));
	msg.mrq = MRQ_STRAP;
	msg.tx.data = req;
	msg.tx.size = sizeof(*req);

	err = tegra_bpmp_transfer(bpmp, &msg);
	return err;
#endif
	return 0;
}

static bool gb10b_tegra_is_railgated(struct device *dev)
{
	struct gk20a *g = get_gk20a(dev);
	bool ret = false;

	if (pm_runtime_status_suspended(dev)) {
		ret = true;
	}

	nvgpu_log(g, gpu_dbg_info, "railgated? %s", ret ? "yes" : "no");

	return ret;
}

static int gb10b_tegra_bpmp_mrq_set(struct device *dev, u32 req_id, u32 val)
{
#if defined(CONFIG_TEGRA_BPMP) && defined(CONFIG_NVGPU_STATIC_POWERGATE)
	struct tegra_bpmp *bpmp;
	struct mrq_strap_request req;
	struct gk20a *g = get_gk20a(dev);
	int ret = 0;

	/*
	 * Silicon - Static pg feature fuse settings will done in BPMP
	 * Pre- Silicon - Static pg feature fuse settings will be done
	 * in NvGPU driver during gpu poweron
	 * Send computed PG masks to BPMP per req_id
	 */
	if (nvgpu_platform_is_silicon(g)) {
		bpmp = tegra_bpmp_get(dev);
		if (!IS_ERR(bpmp)) {
			/* send PG mask */
			memset(&req, 0, sizeof(req));
			req.cmd = STRAP_SET;
			req.id = req_id;
			req.value = val;
			ret = gb10b_tegra_static_pg_control(dev, bpmp, &req);
			if (ret != 0) {
				nvgpu_err(g, "PG mask send failed, req_id %d", req_id);
				return ret;
			}
		}
	}
#endif
	return 0;
}

static int gb10b_tegra_bpmp_mrq_set_all(struct device *dev)
{
	int err = 0;

#if defined(CONFIG_TEGRA_BPMP) && defined(CONFIG_NVGPU_STATIC_POWERGATE)
	struct gk20a *g = get_gk20a(dev);

	err = gb10b_tegra_bpmp_mrq_set(dev,
			TEGRA264_STRAP_NV_FUSE_CTRL_OPT_GPU, g->gpu_pg_mask);
	if (err) {
		goto exit;
	}
	err = gb10b_tegra_bpmp_mrq_set(dev,
			TEGRA264_STRAP_NV_FUSE_CTRL_OPT_NVDEC, g->nvdec_pg_mask);
	if (err) {
		goto exit;
	}
	err = gb10b_tegra_bpmp_mrq_set(dev,
			TEGRA264_STRAP_NV_FUSE_CTRL_OPT_NVENC, g->nvenc_pg_mask);
	if (err) {
		goto exit;
	}
	err = gb10b_tegra_bpmp_mrq_set(dev,
			TEGRA264_STRAP_NV_FUSE_CTRL_OPT_OFA, g->ofa_pg_mask);
	if (err) {
		goto exit;
	}
	err = gb10b_tegra_bpmp_mrq_set(dev,
			TEGRA264_STRAP_NV_FUSE_CTRL_OPT_NVJPG, g->nvjpg_pg_mask);
	if (err) {
		goto exit;
	}
#endif

exit:
	return err;
}

static int gb10b_tegra_railgate(struct device *dev)
{
	struct gb10b_tegra_devfreq_dev *tdev;
	int err = 0;

	list_for_each_entry(tdev, &gb10b_devfreq_masters, masters) {
		/* Scale device to suspend frequency */
		if (tdev->devfreq) {
			err = devfreq_suspend_device(tdev->devfreq);
			if (err) {
				dev_err(dev,
					"failed to suspend device %s: %d\n",
					tdev->dev_name, err);
				return err;
			}
		}

#if defined(CONFIG_INTERCONNECT)
		/* Reset icc bw request to 0 */
		if (tdev->icc_path) {
			err = icc_set_bw(tdev->icc_path, 0, 0);
			if (err) {
				dev_err(dev,
					"failed to reset icc bw request for %s: %d\n",
					tdev->dev_name, err);
				return err;
			}
		}
#endif

	}

	/*
	* BPMP-FW will disable clocks on BPMP side, but kernel
	* should disable clocks as well to maintain reference
	* count in the kernel for common clock framework.
	*/
	gb10b_tegra_clk_disable_all(dev);

	return 0;
}

static int gb10b_tegra_unrailgate(struct device *dev)
{
	struct gb10b_tegra_devfreq_dev *tdev;
	int err = 0;

	/*
	* BPMP-FW will enable clocks on BPMP side, but kernel
	* should enable clocks as well to maintain reference
	* count in the kernel for common clock framework.
	*/
	gb10b_tegra_clk_enable_all(dev);

	list_for_each_entry(tdev, &gb10b_devfreq_masters, masters) {

		/* Scale device to resume frequency */
		if (tdev->devfreq) {
			err = devfreq_resume_device(tdev->devfreq);
			if (err) {
				dev_err(dev,
					"failed to resume device %s: %d\n",
					tdev->dev_name, err);
				return err;
			}
		}
	}

	err = gb10b_tegra_bpmp_mrq_set_all(dev);

	return err;
}

#ifdef CONFIG_NVGPU_STATIC_POWERGATE
static int gb10b_tegra_set_gpu_pg_mask(struct device *dev, u32 gpu_pg_mask)
{
	struct gk20a *g = get_gk20a(dev);
	u32 gpc_mask, fbp_mask, tpc_mask[GB10B_MAX_FS_GPC];
	u32 max_gpc_pg_config = (1U << GB10B_MAX_FS_GPC) - 1U;
	u32 max_fbp_pg_config = (1U << GB10B_MAX_FS_FBP) - 1U;
	u32 tmp_gpc_mask = 0U;
	u32 tmp_tpc_mask = 0U;
	u32 i = 0U;

	if ((gpu_pg_mask & ((1U << GB10B_GPU_PG_MASK_NUM_BITS) - 1U))
			== g->gpu_pg_mask) {
		nvgpu_info(g, "no value change, same gpu_pg_mask already set");
		return 0;
	}

	gpc_mask = MASK_BIT_FIELD(gpu_pg_mask, 2U, 0U);
	fbp_mask = MASK_BIT_FIELD(gpu_pg_mask, 5U, 3U);
	tpc_mask[0] = MASK_BIT_FIELD(gpu_pg_mask, 9U, 6U);
	tpc_mask[1] = MASK_BIT_FIELD(gpu_pg_mask, 13U, 10U);
	tpc_mask[2] = MASK_BIT_FIELD(gpu_pg_mask, 16U, 14U);


	if (gpc_mask == max_gpc_pg_config) {
		nvgpu_err(g, "Invalid GPC-PG mask: 0x%x", gpc_mask);
		return -EINVAL;
	}

	if (fbp_mask == max_fbp_pg_config){
		nvgpu_err(g, "Invalid FBP-PG mask: 0x%x", fbp_mask);
		return -EINVAL;
	}

	if ((tpc_mask[0] == 0xFU) && (tpc_mask[1] == 0xFU) &&
		(tpc_mask[2] == 0x7U)) {
		nvgpu_err(g, "Invalid TPC-PG mask, all TPCs can't be FSed");
		return -EINVAL;
	}

	for (i = 0U; i < GB10B_MAX_FS_GPC; i++) {
		/*
		 * extract the ith bit from gpc_mask
		 * to know if GPC(i) is floorswept or not
		 */
		tmp_gpc_mask = gpc_mask & (1U << i);
		tmp_tpc_mask = tpc_mask[i];
		if (i == 2) {
			/* GPC2 contains only 3 TPCs. so
			 * mark the 4th bit as 1(fused) out
			 */
			tmp_tpc_mask = tpc_mask[i] | 0x8U;
		}

		/*
		 * If GPC(i) is floorswept without all TPCs being
		 * floorswept is invalid
		 */
		if ((tmp_gpc_mask) && (tmp_tpc_mask != 0xFU)) {
			nvgpu_err(g, "GPC(%d) is FSed, so all TPCs must be FSed", i);
			return -EINVAL;
		}

		/*
		 * having all TPCs being floorswept without GPC(i) being
		 * floorswept is invalid
		 */

		if ((!tmp_gpc_mask) && (tmp_tpc_mask == 0xFU)) {
			nvgpu_err(g, "GPC(%d) must be FSed, if all TPCs are FSed", i);
			return -EINVAL;
		}
	}

	/* store this gpu_pg_mask */
	g->gpu_pg_mask = gpu_pg_mask;

	return gb10b_tegra_bpmp_mrq_set(dev,
			TEGRA264_STRAP_NV_FUSE_CTRL_OPT_GPU, g->gpu_pg_mask);
}

static int gb10b_tegra_set_nvdec_pg_mask(struct device *dev, u32 nvdec_pg_mask)
{
	struct gk20a *g = get_gk20a(dev);
	u32 nvdec_mask;

	if ((nvdec_pg_mask & ((1U << GB10B_NVDEC_PG_MASK_NUM_BITS) - 1U))
			== g->nvdec_pg_mask) {
		nvgpu_info(g, "no value change, same nvdec_pg_mask already set");
		return 0;
	}

	nvdec_mask = MASK_BIT_FIELD(nvdec_pg_mask, 1U, 0U);

	/* all possible value are valid, store this nvdec_pg_mask */
	g->nvdec_pg_mask = nvdec_mask;

	return gb10b_tegra_bpmp_mrq_set(dev,
			TEGRA264_STRAP_NV_FUSE_CTRL_OPT_NVDEC, g->nvdec_pg_mask);
}

static int gb10b_tegra_set_nvenc_pg_mask(struct device *dev, u32 nvenc_pg_mask)
{
	struct gk20a *g = get_gk20a(dev);
	u32 nvenc_mask;

	if ((nvenc_pg_mask & ((1U << GB10B_NVENC_PG_MASK_NUM_BITS) - 1U))
			== g->nvenc_pg_mask) {
		nvgpu_info(g, "no value change, same nvenc_pg_mask already set");
		return 0;
	}

	nvenc_mask = MASK_BIT_FIELD(nvenc_pg_mask, 1U, 0U);

	/* all possible value are valid, store this nvenc_pg_mask */
	g->nvenc_pg_mask = nvenc_mask;

	return gb10b_tegra_bpmp_mrq_set(dev,
			TEGRA264_STRAP_NV_FUSE_CTRL_OPT_NVENC, g->nvenc_pg_mask);
}

static int gb10b_tegra_set_ofa_pg_mask(struct device *dev, u32 ofa_pg_mask)
{
	struct gk20a *g = get_gk20a(dev);
	u32 ofa_mask;

	if ((ofa_pg_mask & ((1U << GB10B_OFA_PG_MASK_NUM_BITS) - 1U))
			== g->ofa_pg_mask) {
		nvgpu_info(g, "no value change, same ofa_pg_mask already set");
		return 0;
	}

	ofa_mask = MASK_BIT_FIELD(ofa_pg_mask, 0U, 0U);

	/* all possible value are valid, store this ofa_pg_mask */
	g->ofa_pg_mask = ofa_mask;

	return gb10b_tegra_bpmp_mrq_set(dev,
			TEGRA264_STRAP_NV_FUSE_CTRL_OPT_OFA, g->ofa_pg_mask);
}

static int gb10b_tegra_set_nvjpg_pg_mask(struct device *dev, u32 nvjpg_pg_mask)
{
	struct gk20a *g = get_gk20a(dev);
	u32 nvjpg_mask;

	if ((nvjpg_pg_mask & ((1U << GB10B_NVJPG_PG_MASK_NUM_BITS) - 1U))
			== g->nvjpg_pg_mask) {
		nvgpu_info(g, "no value change, same nvjpg_pg_mask already set");
		return 0;
	}

	nvjpg_mask = MASK_BIT_FIELD(nvjpg_pg_mask, 1U, 0U);

	/* all possible value are valid, store this nvjpg_pg_mask */
	g->nvjpg_pg_mask = nvjpg_mask;

	return gb10b_tegra_bpmp_mrq_set(dev,
			TEGRA264_STRAP_NV_FUSE_CTRL_OPT_NVJPG, g->nvjpg_pg_mask);
}
#endif

/*
 * Get freq table for GPCCLK domain.
 */
static int gb10b_clk_get_freqs(struct device *dev,
				unsigned long **freqs, int *num_freqs)
{
	struct gk20a_platform *platform = gk20a_get_platform(dev);
	struct gk20a *g = platform->g;
	struct gb10b_tegra_devfreq_dev *gpu;

	/* make sure the devfreq GPU clk data is available */
	gpu = &gb10b_devfreq_arr[GB10B_DEVFREQ_GPU];
	if (gpu && gpu->devfreq) {
#if defined(NV_DEVFREQ_HAS_FREQ_TABLE)
		*num_freqs = (int) gpu->devfreq->max_state;
		*freqs = gpu->devfreq->freq_table;
#else
		*num_freqs = (int) gpu->devfreq->profile->max_state;
		*freqs = gpu->devfreq->profile->freq_table;
#endif
	} else {
		nvgpu_err(g, "Failed to get devfreq GPU frequency table");
		return -ENOSYS;
	}

	return 0;
}

struct gk20a_platform gb10b_pci_tegra_platform = {
	.get_clk_freqs = gb10b_clk_get_freqs,

#ifdef CONFIG_TEGRA_GK20A_NVHOST
	.has_syncpoints = true,
#endif

	/* ptimer src frequency in hz*/
	.ptimer_src_freq = 31250000,

	.ch_wdt_init_limit_ms = 5000,

	.probe = gb10b_tegra_probe,
	.late_probe = gb10b_tegra_late_probe,
	.remove = gb10b_tegra_remove,
	.railgate_delay_init    = 500,
	.can_railgate_init      = true,

#ifdef CONFIG_NVGPU_STATIC_POWERGATE
	/* Disable power features to support GPU_BRINGUP */
	/* Enabling tracked in JIRA NVGPU-9261 */
	.can_tpc_pg             = false,
	.can_gpc_pg             = false,
	.can_fbp_pg             = false,

	.disable_aspm = true,
	.disable_nvlink = true,

	.set_tpc_pg_mask        = NULL,
	.set_gpc_pg_mask        = NULL,
	.set_fbp_pg_mask        = NULL,
	.set_gpu_pg_mask        = gb10b_tegra_set_gpu_pg_mask,
	.set_nvdec_pg_mask      = gb10b_tegra_set_nvdec_pg_mask,
	.set_nvenc_pg_mask      = gb10b_tegra_set_nvenc_pg_mask,
	.set_ofa_pg_mask        = gb10b_tegra_set_ofa_pg_mask,
	.set_nvjpg_pg_mask      = gb10b_tegra_set_nvjpg_pg_mask,
#endif
	/* power management features */
	/* Enable power management features: JIRA NVGPU-9261 */
	/* Disable power features to support GPU_BRINGUP */
	.can_slcg               = true,
	.enable_slcg            = true,
	.can_blcg               = true,
	.enable_blcg            = true,
	.can_elcg               = true,
	.enable_elcg            = true,
	.can_flcg               = true,
	.enable_flcg            = false,
	.enable_gpcclk_flcg     = false,
	.enable_nvdclk_flcg     = true,
	.enable_sysclk_slowdown = true,
	.enable_fgpg            = true,
	.enable_perfmon         = true,

	.enable_elpg            = true,
	.enable_elpg_ms         = true,
	.can_elpg_init          = true,
	.enable_aelpg           = true,

	/* power management callbacks */
	.unrailgate = gb10b_tegra_unrailgate,
	.railgate = gb10b_tegra_railgate,
	.is_railgated = gb10b_tegra_is_railgated,

	.busy = gk20a_tegra_busy,
	.idle = gk20a_tegra_idle,

	.dump_platform_dependencies = gk20a_tegra_debug_dump,

	.platform_chip_id = TEGRA_264,
	.soc_name = "tegra264",

	.honors_aperture = true,
	.unified_memory = true,

	/*
	 * This specifies the maximum contiguous size of a DMA mapping to Linux
	 * kernel's DMA framework.
	 * The IOMMU is capable of mapping all of physical memory and hence
	 * dma_mask is set to memory size (512GB in this case).
	 * For iGPU, nvgpu executes own dma allocs (e.g. alloc_page()) and
	 * sg_table construction. No IOMMU mapping is required and so dma_mask
	 * value is not important.
	 * However, for dGPU connected over PCIe through an IOMMU, dma_mask is
	 * significant. In this case, IOMMU bit in GPU physical address is not
	 * relevant.
	 */
	.dma_mask = DMA_BIT_MASK(39),

	.reset_assert = gp10b_tegra_reset_assert,
	.reset_deassert = gp10b_tegra_reset_deassert,

	/*
	 * Size includes total size of ctxsw VPR buffers.
	 * The size can vary for different chips as attribute ctx buffer
	 * size depends on max number of tpcs supported on the chip.
	 */
	.secure_buffer_size = 0x8e1000, /* ~8.9 MiB */
	.is_pci_igpu = true,

#ifdef CONFIG_NVGPU_SIM
	/* Fake rpc details might change with SIM NET releases */
	.fake_rpc_base = 0x03b50000,
	.fake_rpc_size = 0x00010000,
#endif
};
