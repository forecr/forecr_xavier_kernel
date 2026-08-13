// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2013-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <linux/devfreq.h>
#ifdef CONFIG_DEVFREQ_THERMAL
#include <linux/devfreq_cooling.h>
#endif
#include <linux/export.h>
#ifdef CONFIG_GK20A_PM_QOS
#include <linux/pm_qos.h>
#endif
#include <linux/pm_opp.h>
#include <linux/of.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 14, 0)
#include <governor.h>
#endif

#include <nvgpu/kmem.h>
#include <nvgpu/log.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/pmu/clk/clk.h>
#include <nvgpu/clk_arb.h>
#include <nvgpu/pmu/pmu_perfmon.h>

#include "platform_gk20a.h"
#include "scale.h"
#include "os_linux.h"
#include "driver_common.h"

/*
 * gk20a_scale_qos_notify()
 *
 * This function is called when the minimum QoS requirement for the device
 * has changed. The function calls postscaling callback if it is defined.
 */

#if defined(CONFIG_GK20A_PM_QOS) && defined(CONFIG_COMMON_CLK)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
int gk20a_scale_qos_min_notify(struct notifier_block *nb,
			  unsigned long n, void *p)
{
	struct gk20a_scale_profile *profile =
			container_of(nb, struct gk20a_scale_profile,
			qos_min_notify_block);
	struct gk20a *g = get_gk20a(profile->dev);
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct devfreq *devfreq = l->devfreq;

	if (!devfreq)
		return NOTIFY_OK;

	nvgpu_mutex_acquire(&profile->lock);

	profile->qos_min_freq = (unsigned long)n * 1000UL;

	nvgpu_mutex_release(&profile->lock);

	return NOTIFY_OK;
}

int gk20a_scale_qos_max_notify(struct notifier_block *nb,
			  unsigned long n, void *p)
{
	struct gk20a_scale_profile *profile =
			container_of(nb, struct gk20a_scale_profile,
			qos_max_notify_block);
	struct gk20a *g = get_gk20a(profile->dev);
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct devfreq *devfreq = l->devfreq;

	if (!devfreq)
		return NOTIFY_OK;

	nvgpu_mutex_acquire(&profile->lock);

	profile->qos_max_freq = (unsigned long)n * 1000UL;

	nvgpu_mutex_release(&profile->lock);

	return NOTIFY_OK;
}

u16 gk20a_scale_clamp_clk_target(struct gk20a *g,
				 u16 gpc2clk_target)
{
	struct gk20a_scale_profile *profile = g->scale_profile;
	u16 min_freq_mhz, max_freq_mhz;

	if (!profile)
		return gpc2clk_target;

	nvgpu_mutex_acquire(&profile->lock);

	min_freq_mhz = (u16) (profile->qos_min_freq / 1000000UL);
	max_freq_mhz = (u16) (profile->qos_max_freq / 1000000UL);

	nvgpu_log_info(g, "target %u qos_min %u qos_max %u", gpc2clk_target,
		       min_freq_mhz, max_freq_mhz);

	if (gpc2clk_target < min_freq_mhz) {
		gpc2clk_target = min_freq_mhz;
	}

	if (gpc2clk_target > max_freq_mhz) {
		gpc2clk_target = max_freq_mhz;
	}

	nvgpu_mutex_release(&profile->lock);

	return gpc2clk_target;
}
#else
int gk20a_scale_qos_notify(struct notifier_block *nb,
			  unsigned long n, void *p)
{
	struct gk20a_scale_profile *profile =
			container_of(nb, struct gk20a_scale_profile,
			qos_notify_block);
	struct gk20a *g = get_gk20a(profile->dev);
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct devfreq *devfreq = l->devfreq;

	if (!devfreq)
		return NOTIFY_OK;

	mutex_lock(&devfreq->lock);
	/* check for pm_qos min and max frequency requirement */
	profile->qos_min_freq =
	  (unsigned long)pm_qos_read_min_bound(PM_QOS_GPU_FREQ_BOUNDS) * 1000UL;
	profile->qos_max_freq =
	  (unsigned long)pm_qos_read_max_bound(PM_QOS_GPU_FREQ_BOUNDS) * 1000UL;

	if (profile->qos_min_freq > profile->qos_max_freq) {
		nvgpu_err(g,
			"QoS: setting invalid limit, min_freq=%lu max_freq=%lu",
			profile->qos_min_freq, profile->qos_max_freq);
		profile->qos_min_freq = profile->qos_max_freq;
	}

	update_devfreq(devfreq);
	mutex_unlock(&devfreq->lock);

	return NOTIFY_OK;
}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0) */
#elif defined(CONFIG_GK20A_PM_QOS)
int gk20a_scale_qos_notify(struct notifier_block *nb,
			  unsigned long n, void *p)
{
	struct gk20a_scale_profile *profile =
		container_of(nb, struct gk20a_scale_profile,
			     qos_notify_block);
	struct gk20a_platform *platform = dev_get_drvdata(profile->dev);
	struct gk20a *g = get_gk20a(profile->dev);
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	unsigned long freq;

	if (!platform->postscale)
		return NOTIFY_OK;

	/* get the frequency requirement. if devfreq is enabled, check if it
	 * has higher demand than qos */
	freq = platform->clk_round_rate(profile->dev,
			(u32)pm_qos_read_min_bound(PM_QOS_GPU_FREQ_BOUNDS));
	if (l->devfreq)
		freq = max(l->devfreq->previous_freq, freq);

	/* Update gpu load because we may scale the emc target
	 * if the gpu load changed. */
	nvgpu_pmu_sysfs_load_refresh(g, NULL, NVGPU_PMU_PERFMON_CLASS_GR);
	platform->postscale(profile->dev, freq);

	return NOTIFY_OK;
}
#else
int gk20a_scale_qos_notify(struct notifier_block *nb,
			  unsigned long n, void *p)
{
	return 0;
}
#endif

/*
 * gk20a_scale_make_freq_table(profile)
 *
 * This function initialises the frequency table for the given device profile
 */

static int gk20a_scale_make_freq_table(struct gk20a_scale_profile *profile)
{
	struct gk20a_platform *platform = dev_get_drvdata(profile->dev);
	int num_freqs, err;
	unsigned long *freqs;

	if (platform->get_clk_freqs) {
		/* get gpu frequency table */
		err = platform->get_clk_freqs(profile->dev, &freqs,
					&num_freqs);

		if (err)
			return -ENOSYS;
	} else
		return -ENOSYS;

	profile->devfreq_profile.freq_table = (unsigned long *)freqs;
	profile->devfreq_profile.max_state = num_freqs;

	return 0;
}

/*
 * gk20a_scale_target(dev, *freq, flags)
 *
 * This function scales the clock
 */

static int gk20a_scale_target(struct device *dev, unsigned long *freq,
			      u32 flags)
{
	struct gk20a_platform *platform = dev_get_drvdata(dev);
	struct gk20a *g = get_gk20a(dev);

	g->ops.clk.set_rate(g, CTRL_CLK_DOMAIN_GPCCLK, *freq);
	*freq = g->ops.clk.get_rate(g, CTRL_CLK_DOMAIN_GPCCLK);

	g->last_freq = *freq;

	/* postscale will only scale emc (dram clock) if evaluating
	 * gk20a_tegra_get_emc_rate() produces a new or different emc
	 * target because the load or_and gpufreq has changed */
	if (platform->postscale)
		platform->postscale(dev, *freq);

	return 0;
}

/*
 * update_load_estimate_busy_cycles(dev)
 *
 * Update load estimate using pmu idle counters. Result is normalised
 * based on the time it was asked last time.
 */

static void update_load_estimate_busy_cycles(struct device *dev)
{
	struct gk20a *g = get_gk20a(dev);
	struct gk20a_scale_profile *profile = g->scale_profile;
	unsigned long dt;
	u32 busy_cycles_norm;
	ktime_t t;

	t = ktime_get();
	dt = ktime_us_delta(t, profile->last_event_time);

	profile->dev_stat.total_time = dt;
	profile->last_event_time = t;
	nvgpu_pmu_load_query(g, &busy_cycles_norm, NVGPU_PMU_PERFMON_CLASS_GR);
	profile->dev_stat.busy_time =
		(busy_cycles_norm * dt) / PMU_BUSY_CYCLES_NORM_MAX;
}

/*
 * gk20a_scale_suspend(dev)
 *
 * This function informs devfreq of suspend
 */

void gk20a_scale_suspend(struct device *dev)
{
	struct gk20a *g = get_gk20a(dev);
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct devfreq *devfreq = l->devfreq;

	if (!devfreq)
		return;

	devfreq_suspend_device(devfreq);
}

/*
 * gk20a_scale_resume(dev)
 *
 * This functions informs devfreq of resume
 */

void gk20a_scale_resume(struct device *dev)
{
	struct gk20a *g = get_gk20a(dev);
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct devfreq *devfreq = l->devfreq;

	if (!devfreq)
		return;

	g->last_freq = 0;
	devfreq_resume_device(devfreq);
}

/*
 * gk20a_scale_get_dev_status(dev, *stat)
 *
 * This function queries the current device status.
 */

static int gk20a_scale_get_dev_status(struct device *dev,
				      struct devfreq_dev_status *stat)
{
	struct gk20a *g = get_gk20a(dev);
	struct gk20a_scale_profile *profile = g->scale_profile;
	struct gk20a_platform *platform = dev_get_drvdata(dev);

	/* inform edp about new constraint */
	if (platform->prescale)
		platform->prescale(dev);

	/* Make sure there are correct values for the current frequency */
	profile->dev_stat.current_frequency =
				g->ops.clk.get_rate(g, CTRL_CLK_DOMAIN_GPCCLK);

	/* Update load estimate */
	update_load_estimate_busy_cycles(dev);

	/* Copy the contents of the current device status */
	*stat = profile->dev_stat;

	/* Finally, clear out the local values */
	profile->dev_stat.total_time = 0;
	profile->dev_stat.busy_time = 0;

	return 0;
}

/*
 * get_cur_freq(struct device *dev, unsigned long *freq)
 *
 * This function gets the current GPU clock rate.
 */

static int get_cur_freq(struct device *dev, unsigned long *freq)
{
	struct gk20a *g = get_gk20a(dev);
	*freq = g->ops.clk.get_rate(g, CTRL_CLK_DOMAIN_GPCCLK);
	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0)
static int register_gpu_opp(struct device *dev)
{
	return 0;
}

static void unregister_gpu_opp(struct device *dev)
{
}
#else
static void unregister_gpu_opp(struct device *dev)
{
	/* Remove device tree OPP table if it exists */
	if (dev->of_node &&
	    of_find_property(dev->of_node, "operating-points-v2", NULL))
		dev_pm_opp_of_remove_table(dev);

	/* Remove any dynamic OPP entries */
	dev_pm_opp_remove_all_dynamic(dev);
}

/*
 * Helper function to read OPP frequencies and replace freq table
 * Returns 0 on success, negative error code on failure
 */
static int read_opp_freqs_and_replace(struct device *dev,
				      struct gk20a *g,
				      struct gk20a_scale_profile *profile,
				      int opp_count)
{
	unsigned long *opp_freqs;
	struct dev_pm_opp *opp;
	unsigned long freq = 0;
	int opp_idx;

	opp_freqs = nvgpu_kzalloc(g, sizeof(*opp_freqs) * opp_count);
	if (!opp_freqs)
		return -ENOMEM;

	/* Read frequencies from OPP table in descending order */
	for (opp_idx = 0; opp_idx < opp_count; opp_idx++) {
		opp = dev_pm_opp_find_freq_ceil(dev, &freq);
		if (IS_ERR(opp)) {
			nvgpu_err(g, "OPP %d failed", opp_idx);
			nvgpu_kfree(g, opp_freqs);
			return PTR_ERR(opp);
		}

		opp_freqs[opp_idx] = freq;
		nvgpu_info(g, "Freq %lu Hz idx %d", freq, opp_idx);
		dev_pm_opp_put(opp);
		freq++; /* Increment for next search */
	}

	/* Successfully read all frequencies, replace the table */
	profile->devfreq_profile.freq_table = opp_freqs;
	profile->devfreq_profile.max_state = opp_count;

	return 0;
}

static int register_gpu_opp(struct device *dev)
{
	struct gk20a_platform *platform = dev_get_drvdata(dev);
	struct gk20a *g = platform->g;
	struct gk20a_scale_profile *profile = g->scale_profile;
	unsigned long *freq_table = profile->devfreq_profile.freq_table;
	int max_states = profile->devfreq_profile.max_state;
	int opp_count;
	int i;
	int err = 0;

	/* Try to initialize OPP table from device tree first */
	if (dev->of_node &&
	    of_find_property(dev->of_node, "operating-points-v2", NULL)) {
		nvgpu_info(g,
			"Found OPP property, initializing from DT");

		err = dev_pm_opp_of_add_table(dev);
		if (err == 0) {
			opp_count = dev_pm_opp_get_opp_count(dev);
			if (opp_count > 0) {
				err = read_opp_freqs_and_replace(dev, g,
								  profile,
								  opp_count);
				if (err == 0)
					return 0; /* Success */

				nvgpu_err(g, "Failed read OPPs: %d", err);
			} else {
				nvgpu_err(g, "OPP init but count=%d",
					opp_count);
			}
			dev_pm_opp_of_remove_table(dev);
		} else {
			nvgpu_err(g, "Failed to init OPP table: %d", err);
		}
	}

	/* Check if OPP table already exists (fallback check) */
	opp_count = dev_pm_opp_get_opp_count(dev);
	if (opp_count > 0) {
		nvgpu_info(g,
			"OPP table has %d entries, decoupled from PMU VF",
			opp_count);
		return 0;  /* Success - using existing OPP table */
	}


	for (i = 0; i < max_states; ++i) {
		err = dev_pm_opp_add(dev, freq_table[i], 0);
		if (err) {
			nvgpu_err(g,
				"Failed to add OPP %lu: %d\n",
				freq_table[i],
				err);
			unregister_gpu_opp(dev);
			break;
		}
	}


	return err;
}
#endif

/*
 * OPP functions for device tree-based frequency management
 * These functions handle OPP table management for devices with OPP configuration
 */

/**
 * gk20a_tegra_init_opp_table() - Initialize OPP table from device tree
 * @dev: Device pointer
 *
 * This function initializes the OPP (Operating Performance Points) table
 * from the device tree. This allows devfreq frequencies to be configured
 * independently from PMU-provided frequencies.
 *
 * Returns: 0 on success, -ENODEV if no OPP table found (not an error),
 *          or error code on failure
 */
int gk20a_tegra_init_opp_table(struct device *dev)
{
	int err;

	/* Initialize OPP table from device tree using "operating-points-v2" */
	err = dev_pm_opp_of_add_table(dev);
	if (err) {
		/*
		 * If OPP table not found in DT, it's not an error -
		 * driver will use PMU-provided frequencies as fallback
		 */
		if (err == -ENODEV) {
			dev_info(dev,
				"No OPP in DT, will use PMU freqs\n");
			return 0;
		}
		dev_err(dev, "Failed to init OPP from DT: %d\n", err);
		return err;
	}

	dev_info(dev, "OPP table initialized from device tree\n");
	return 0;
}

/**
 * gk20a_tegra_cleanup_opp_table() - Cleanup OPP table
 * @dev: Device pointer
 *
 * Removes the OPP table that was initialized from device tree
 */
void gk20a_tegra_cleanup_opp_table(struct device *dev)
{
	dev_pm_opp_of_remove_table(dev);
}

/**
 * gk20a_tegra_get_clk_freqs_from_opp() - Get frequency table from OPP
 * @dev: Device pointer
 * @freqs: Pointer to store allocated frequency array
 * @num_freqs: Pointer to store number of frequencies
 *
 * This function retrieves the frequency table from device tree OPP entries,
 * allowing devfreq to be decoupled from the PMU frequency table.
 *
 * If no OPP table exists in device tree, returns -ENODEV and the caller
 * should fall back to using PMU-provided frequencies.
 *
 * Returns: 0 on success, error code on failure
 */
int gk20a_tegra_get_clk_freqs_from_opp(struct device *dev,
				       unsigned long **freqs,
				       int *num_freqs)
{
	struct dev_pm_opp *opp;
	unsigned long *freq_table;
	unsigned long freq = 0;
	int count, i = 0;

	/* Count number of OPP entries in device tree */
	count = dev_pm_opp_get_opp_count(dev);
	if (count <= 0) {
		dev_info(dev, "No OPP, use PMU frequencies\n");
		return -ENODEV;
	}

	/* Allocate frequency table */
	freq_table = devm_kcalloc(dev, count, sizeof(*freq_table),
				  GFP_KERNEL);
	if (!freq_table)
		return -ENOMEM;

	/* Iterate through all OPP entries and extract frequencies */
	for (i = 0; i < count; i++) {
		opp = dev_pm_opp_find_freq_ceil(dev, &freq);
		if (IS_ERR(opp)) {
			dev_err(dev, "Failed to find OPP %lu: %ld\n",
				freq, PTR_ERR(opp));
			devm_kfree(dev, freq_table);
			return PTR_ERR(opp);
		}

		freq_table[i] = freq;
		dev_dbg(dev, "OPP[%d]: %lu Hz\n", i, freq);

		dev_pm_opp_put(opp);

		/* Move to next frequency */
		freq++;
	}

	*freqs = freq_table;
	*num_freqs = count;

	dev_info(dev, "Loaded %d devfreq OPPs from device tree\n", count);
	return 0;
}

/*
 * gk20a_scale_init(dev)
 */

void gk20a_scale_init(struct device *dev)
{
	struct gk20a_platform *platform = dev_get_drvdata(dev);
	struct gk20a *g = platform->g;
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct gk20a_scale_profile *profile;
#ifdef CONFIG_DEVFREQ_THERMAL
	struct thermal_cooling_device *cooling;
#endif
	struct devfreq *devfreq;
	int err;

	if (g->scale_profile)
		return;

	if (!platform->devfreq_governor)
		return;

	/* For virtual devices, initialize OPP table from device tree first */
	if (platform->virtual_dev) {
		err = gk20a_tegra_init_opp_table(dev);
		if (err && err != -ENODEV) {
			nvgpu_err(g, "Failed to init OPP table: %d", err);
			/*
			 * Continue - will fall back to GPU server freq_table
			 * if no OPP table found
			 */
		}
	}

	profile = nvgpu_kzalloc(g, sizeof(*profile));
	if (!profile)
		return;

	profile->dev = dev;
#ifdef CONFIG_GK20A_PM_QOS
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
	profile->dev_stat.busy = false;
#endif
#endif

	/* Create frequency table */
	err = gk20a_scale_make_freq_table(profile);
	if (err || !profile->devfreq_profile.max_state) {
		nvgpu_err(g, "Failed to create frequency table, err=%d, max_state=%u",
			  err, profile->devfreq_profile.max_state);
		goto err_get_freqs;
	}

	/* Try to read frequencies from device tree OPP table */
	if (profile->dev->of_node &&
	    of_find_property(profile->dev->of_node,
			     "operating-points-v2", NULL)) {
		nvgpu_info(g,
			"DT OPP property found - will read from OPP");
	} else {
		nvgpu_info(g, "No specific device tree OPP table, using default frequency table");
	}

	profile->qos_min_freq = 0;
	profile->qos_max_freq = UINT_MAX;

	/* Store device profile so we can access it if devfreq governor
	 * init needs that */
	g->scale_profile = profile;

	if (platform->devfreq_governor) {
		int error = 0;

		register_gpu_opp(dev);
		nvgpu_devfreq_init(g);

		profile->devfreq_profile.initial_freq =
			profile->devfreq_profile.freq_table[0];
		profile->devfreq_profile.target = gk20a_scale_target;
		profile->devfreq_profile.get_dev_status =
			gk20a_scale_get_dev_status;
		profile->devfreq_profile.get_cur_freq = get_cur_freq;
		profile->devfreq_profile.polling_ms = 25;

		devfreq = devfreq_add_device(dev,
					&profile->devfreq_profile,
					platform->devfreq_governor, NULL);

		if (IS_ERR(devfreq)) {
			nvgpu_err(g, "devfreq_add_device failed, error=%ld", PTR_ERR(devfreq));
			devfreq = NULL;
		} else {
			nvgpu_info(g, "enabled scaling for GPU\n");
		}

		l->devfreq = devfreq;

		if (devfreq != NULL) {
#ifdef CONFIG_DEVFREQ_THERMAL
			cooling = of_devfreq_cooling_register(dev->of_node, devfreq);
			if (IS_ERR(cooling)) {
				nvgpu_err(g, "Failed to register DevFreq cooling device: %ld",
					  PTR_ERR(cooling));
				l->cooling = NULL;
			} else {
				l->cooling = cooling;
				nvgpu_info(g, "DevFreq cooling device registered successfully");
			}
#endif
			/* create symlink /sys/devices/gpu.0/devfreq_dev */
			error = sysfs_create_link(&dev->kobj,
				&devfreq->dev.kobj, "devfreq_dev");

			if (error) {
				nvgpu_err(g,
					"Failed to create devfreq_dev: %d",
					error);
			}
		}
	}

#ifdef CONFIG_GK20A_PM_QOS
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	nvgpu_mutex_init(&profile->lock);

	/* Should we register min frequency QoS callback for this device? */
	if (devfreq && platform->qos_min_notify) {
		profile->qos_min_notify_block.notifier_call =
					platform->qos_min_notify;

		err = dev_pm_qos_add_notifier(devfreq->dev.parent,
					      &profile->qos_min_notify_block,
					      DEV_PM_QOS_MIN_FREQUENCY);
		if (err) {
			nvgpu_err(g, "failed to add min freq notifier %d", err);
		}
	}

	/* Should we register max frequency QoS callback for this device? */
	if (devfreq && platform->qos_max_notify) {
		profile->qos_max_notify_block.notifier_call =
					platform->qos_max_notify;

		err = dev_pm_qos_add_notifier(devfreq->dev.parent,
					      &profile->qos_max_notify_block,
					      DEV_PM_QOS_MAX_FREQUENCY);
		if (err) {
			nvgpu_err(g, "failed to add max freq notifier %d", err);
		}
	}
#else
	/* Should we register QoS callback for this device? */
	if (platform->qos_notify) {
		profile->qos_notify_block.notifier_call =
					platform->qos_notify;

		pm_qos_add_min_notifier(PM_QOS_GPU_FREQ_BOUNDS,
					&profile->qos_notify_block);
		pm_qos_add_max_notifier(PM_QOS_GPU_FREQ_BOUNDS,
					&profile->qos_notify_block);
	}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0) */
#endif

	return;

err_get_freqs:
	nvgpu_kfree(g, profile);
}

void gk20a_scale_exit(struct device *dev)
{
	struct gk20a_platform *platform = dev_get_drvdata(dev);
	struct gk20a *g = platform->g;
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	struct devfreq *devfreq = l->devfreq;
	struct gk20a_scale_profile *profile = NULL;
#endif
	int err;

	if (!platform->devfreq_governor)
		return;

#ifdef CONFIG_GK20A_PM_QOS
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	if (devfreq && platform->qos_min_notify) {
		profile = g->scale_profile;

		err = dev_pm_qos_remove_notifier(devfreq->dev.parent,
						 &profile->qos_min_notify_block,
						 DEV_PM_QOS_MIN_FREQUENCY);
		if (err) {
			nvgpu_err(g, "failed to remove min freq notifier %d", err);
		}
	}

	if (devfreq && platform->qos_max_notify) {
		if (!profile)
			profile = g->scale_profile;

		err = dev_pm_qos_remove_notifier(devfreq->dev.parent,
						 &profile->qos_max_notify_block,
						 DEV_PM_QOS_MAX_FREQUENCY);
		if (err) {
			nvgpu_err(g, "failed to remove max freq notifier %d", err);
		}
	}

	if (devfreq && (platform->qos_min_notify || platform->qos_max_notify)) {
		if (!profile)
			profile = g->scale_profile;
		nvgpu_mutex_destroy(&profile->lock);
	}
#else
	if (platform->qos_notify) {
		pm_qos_remove_min_notifier(PM_QOS_GPU_FREQ_BOUNDS,
				&g->scale_profile->qos_notify_block);
		pm_qos_remove_max_notifier(PM_QOS_GPU_FREQ_BOUNDS,
				&g->scale_profile->qos_notify_block);
	}
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0) */
#endif

#ifdef CONFIG_DEVFREQ_THERMAL
	if (l->cooling) {
		devfreq_cooling_unregister(l->cooling);
		l->cooling = NULL;
	}
#endif

	if (platform->devfreq_governor) {
		sysfs_remove_link(&dev->kobj, "devfreq_dev");

		err = devfreq_remove_device(l->devfreq);
		l->devfreq = NULL;

		unregister_gpu_opp(dev);
	}

	nvgpu_kfree(g, g->scale_profile);
	g->scale_profile = NULL;
}

/*
 * gk20a_scale_hw_init(dev)
 *
 * Initialize hardware portion of the device
 */

void gk20a_scale_hw_init(struct device *dev)
{
	struct gk20a_platform *platform = dev_get_drvdata(dev);
	struct gk20a_scale_profile *profile = platform->g->scale_profile;

	/* make sure that scaling has bee initialised */
	if (!profile)
		return;

	profile->dev_stat.total_time = 0;
	profile->last_event_time = ktime_get();
}
