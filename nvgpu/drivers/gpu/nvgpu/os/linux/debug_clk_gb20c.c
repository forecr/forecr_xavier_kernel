// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/debug.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/power_features/pg.h>
#include <nvgpu/pmu.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/pmu/volt.h>
#include <nvgpu/pmu/perf.h>
#include <nvgpu/pmu/clk/clk.h>

#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/ktime.h>

#include "os_linux.h"
#include "platform_gk20a.h"
#include "debug_clk_gb20c.h"
#include <nvgpu/pmu/clk/clk.h>
#include "common/pmu/clk/ucode_clk_inf.h"
#include "common/pmu/volt/volt_rail.h"
#include "common/pmu/volt/volt.h"
#include "common/pmu/clk/clk.h"
#include "common/pmu/clk/clk_domain.h"
#include "common/pmu/clk/clk_vf_rel.h"
#include "common/pmu/perf/ucode_perf_change_seq_inf.h"

#ifdef CONFIG_NVGPU_GR_VIRTUALIZATION
#include <nvgpu/vgpu/tegra_vgpu.h>
#include "common/vgpu/clk_vgpu.h"
#include "common/vgpu/pmu_vgpu.h"
#endif

#define MHZ_TO_HZ(a) ((unsigned long)(a) * 1000000UL)

static ssize_t gpu_gpc_rate_read(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos);
static ssize_t gpu_gpc_rate_write(struct file *file, const char __user *user_buf,
				  size_t count, loff_t *ppos);
static ssize_t gpu_sys_rate_read(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos);
static ssize_t gpu_sys_rate_write(struct file *file, const char __user *user_buf,
				  size_t count, loff_t *ppos);
static ssize_t gpu_nvd_rate_read(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos);
static ssize_t gpu_nvd_rate_write(struct file *file, const char __user *user_buf,
				  size_t count, loff_t *ppos);
static ssize_t gpu_uproc_rate_read(struct file *file, char __user *user_buf,
				   size_t count, loff_t *ppos);
static ssize_t gpu_uproc_rate_write(struct file *file, const char __user *user_buf,
				    size_t count, loff_t *ppos);
static ssize_t gpu_gpc_allowed_rates_read(struct file *file, char __user *user_buf,
					   size_t count, loff_t *ppos);
static ssize_t gpu_sys_allowed_rates_read(struct file *file, char __user *user_buf,
					   size_t count, loff_t *ppos);
static ssize_t gpu_nvd_allowed_rates_read(struct file *file, char __user *user_buf,
					   size_t count, loff_t *ppos);
static ssize_t gpu_uproc_allowed_rates_read(struct file *file, char __user *user_buf,
					     size_t count, loff_t *ppos);
static ssize_t vdd_gpc_voltage_read(struct file *file, char __user *user_buf,
				     size_t count, loff_t *ppos);
static ssize_t vdd_gpc_voltage_write(struct file *file, const char __user *user_buf,
				      size_t count, loff_t *ppos);
static ssize_t vdd_gpm_voltage_read(struct file *file, char __user *user_buf,
				     size_t count, loff_t *ppos);
static ssize_t vdd_gpm_voltage_write(struct file *file, const char __user *user_buf,
				      size_t count, loff_t *ppos);
static ssize_t vdd_gpc_allowed_voltages_read(struct file *file, char __user *user_buf,
					      size_t count, loff_t *ppos);
static ssize_t vdd_gpm_allowed_voltages_read(struct file *file, char __user *user_buf,
					      size_t count, loff_t *ppos);

/* Power management function declarations */
int nvgpu_power_mgmt_data_init(struct gk20a *g);
void nvgpu_power_mgmt_data_deinit(struct gk20a *g);

/* File operations structure declarations */
static const struct file_operations gpu_gpc_rate_fops;
static const struct file_operations gpu_sys_rate_fops;
static const struct file_operations gpu_nvd_rate_fops;
static const struct file_operations gpu_uproc_rate_fops;
static const struct file_operations gpu_gpc_allowed_rates_fops;
static const struct file_operations gpu_sys_allowed_rates_fops;
static const struct file_operations gpu_nvd_allowed_rates_fops;
static const struct file_operations gpu_uproc_allowed_rates_fops;
static const struct file_operations vf_table_fops;
static const struct file_operations vdd_gpc_voltage_fops;
static const struct file_operations vdd_gpm_voltage_fops;
static const struct file_operations vdd_gpc_allowed_voltages_fops;
static const struct file_operations vdd_gpm_allowed_voltages_fops;
static const struct file_operations vdd_gpc_rail_limit_offset_vmin_fops;
static const struct file_operations vdd_gpc_rail_limit_offset_vmax_fops;
static const struct file_operations vdd_gpm_rail_limit_offset_vmin_fops;
static const struct file_operations vdd_gpm_rail_limit_offset_vmax_fops;
static const struct file_operations voltage_limits_fops;
static const struct file_operations gpc_voltage_limits_fops;
static const struct file_operations gpm_voltage_limits_fops;
static const struct file_operations gpu_gpc_freq_offset_khz_fops;
static const struct file_operations gpu_sys_freq_offset_khz_fops;
static const struct file_operations gpu_nvd_freq_offset_khz_fops;
static const struct file_operations gpu_uproc_freq_offset_khz_fops;
static const struct file_operations gpu_gpc_drive_freq_mhz_fops;
static const struct file_operations gpu_gpc_sentry_freq_mhz_fops;

/* Function declarations for mode and regime control */
static ssize_t clock_mode_read(struct file *file, char __user *user_buf,
			      size_t count, loff_t *ppos);
static ssize_t clock_mode_write(struct file *file, const char __user *user_buf,
			       size_t count, loff_t *ppos);
static ssize_t gpc_voltage_limits_read(struct file *file, char __user *user_buf,
				      size_t count, loff_t *ppos);
static ssize_t gpm_voltage_limits_read(struct file *file, char __user *user_buf,
				      size_t count, loff_t *ppos);

/* Per-domain regime control functions */
static ssize_t gpc_regime_read(struct file *file, char __user *user_buf,
			      size_t count, loff_t *ppos);
static ssize_t gpc_regime_write(struct file *file, const char __user *user_buf,
			       size_t count, loff_t *ppos);
static ssize_t sys_regime_read(struct file *file, char __user *user_buf,
			      size_t count, loff_t *ppos);
static ssize_t sys_regime_write(struct file *file, const char __user *user_buf,
			       size_t count, loff_t *ppos);
static ssize_t nvd_regime_read(struct file *file, char __user *user_buf,
			      size_t count, loff_t *ppos);
static ssize_t nvd_regime_write(struct file *file, const char __user *user_buf,
			       size_t count, loff_t *ppos);
static ssize_t uproc_regime_read(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos);
static ssize_t uproc_regime_write(struct file *file, const char __user *user_buf,
				 size_t count, loff_t *ppos);

/* Helper function to check if rate write is allowed */
static bool is_rate_write_allowed(struct gk20a *g, u32 clk_domain);

/* File operations for mode and per-domain regime control */
static const struct file_operations clock_mode_fops;
static const struct file_operations gpc_regime_fops;
static const struct file_operations sys_regime_fops;
static const struct file_operations nvd_regime_fops;
static const struct file_operations uproc_regime_fops;

/* Clock control modes */
#define CLOCK_MODE_AUTO		0
#define CLOCK_MODE_MANUAL	1

static const struct pto_counter_regs pto_gpc_regs = {
	.cfg  = 0x00132a74,
	.cnt0 = 0x00132a78,
	.cnt1 = 0x00132a7c,
};

static const struct pto_counter_regs pto_nvd_regs = {
	.cfg  = 0x00137574,
	.cnt0 = 0x00137578,
	.cnt1 = 0x0013757c,
};

static const struct pto_counter_regs pto_uproc_regs = {
	.cfg  = 0x00136878,
	.cnt0 = 0x0013687c,
	.cnt1 = 0x00136880,
};

static const struct pto_counter_regs pto_sys_regs = {
	.cfg  = 0x00136e74,
	.cnt0 = 0x00136e78,
	.cnt1 = 0x00136e7c,
};

struct pto_counter_ctx {
	struct gk20a *g;
	const struct pto_counter_regs *regs;
};

static struct pto_counter_ctx gpc_ctx;
static struct pto_counter_ctx sys_ctx;
static struct pto_counter_ctx nvd_ctx;
static struct pto_counter_ctx uproc_ctx;

/* GPU Power Management Data Structure */
struct gpu_power_mgmt_data {
	/* Mutex to serialize all power management sysfs access */
	struct nvgpu_mutex sysfs_lock;

	/* Clock control mode: 0 = auto, 1 = manual */
	u32 clock_mode;

	/* Per-domain clock regimes */
	u32 gpc_regime;
	u32 sys_regime;
	u32 nvd_regime;
	u32 uproc_regime;

	/* Allowed VDD GPC voltages (parsed from VF table during init) */
	u32 *vdd_gpc_allowed_voltages;
	u32 vdd_gpc_num_allowed_voltages;

	/* Allowed VDD GPM voltages (parsed from VF table during init) */
	u32 *vdd_gpm_allowed_voltages;
	u32 vdd_gpm_num_allowed_voltages;

	/* Allowed GPC clock rates (parsed from VF table during init) */
	u32 *allowed_gpc_rates;
	u32 num_allowed_gpc_rates;

	/* Allowed SYS clock rates (parsed from VF table during init) */
	u32 *allowed_sys_rates;
	u32 num_allowed_sys_rates;

	/* Allowed NVD clock rates (parsed from VF table during init) */
	u32 *allowed_nvd_rates;
	u32 num_allowed_nvd_rates;

	/* Allowed UPROC clock rates (parsed from VF table during init) */
	u32 *allowed_uproc_rates;
	u32 num_allowed_uproc_rates;
};

static ssize_t pto_counter_generic_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
	struct pto_counter_ctx *ctx = file->private_data;
	struct gk20a *g = ctx->g;
	const struct pto_counter_regs *regs = ctx->regs;
	char buf[64];
	int len;
	u32 cfg_val, cnt0_val, cnt1_val;
	u64 start_count, end_count, freq;
	ktime_t t_start, t_end;
	s64 elapsed_ns;
	int err;
	bool elcg_was_enabled = false;
	bool gpc_rg_was_enabled = false;
	bool vid_elpg_was_enabled = false;

	if (!g)
		return -EINVAL;

	/* Only perform measurement on first read (ppos == 0) */
	if (*ppos != 0)
		return 0;

	if (g->is_virtual) {
#ifdef CONFIG_NVGPU_GR_VIRTUALIZATION
		/* Use IVC call for virtualized environment */
		err = vgpu_pto_counter_get_freq(g, regs, &freq);
		if (err) {
			nvgpu_err(g, "vgpu_pto_counter_get_freq failed: %d", err);
			return err;
		}
#else
		nvgpu_err(g, "vgpu_pto_counter_get_freq not supported");
		return -ENOSYS;
#endif
	} else {
		/* Original implementation for non-virtualized environment */
		err = gk20a_busy(g);
		if (err)
			return err;

		/* Temporarily disable GPC_RG to get gpc clock measurements */
		if (ctx == &gpc_ctx && g->gpc_rg_enabled) {
			gpc_rg_was_enabled = true;
			nvgpu_pg_gpc_rg_disable(g);
			/* Wait for clocks to stabilize after disabling GPC_RG */
			msleep(1);
		}
		/* Temporarily disable NVDEC_ELPG to get nvd clock measurements
		 * if all Video engines ELPG are currently enabled.
		 */
		if (ctx == &nvd_ctx &&
			g->nvdec_elpg_enabled) {
			vid_elpg_was_enabled = true;
			nvgpu_pg_vid_elpg_disable(g, PMU_PG_ELPG_ENGINE_ID_VID_NVDEC);
			/* Wait for clocks to stabilize after disabling NVDEC_ELPG */
			msleep(1);
		}
		/* Temporarily disable ELCG to get accurate clock measurements */
		if (g->elcg_enabled) {
			elcg_was_enabled = true;
			nvgpu_cg_elcg_set_elcg_enabled(g, false);
			/* Wait for clocks to stabilize after disabling ELCG */
			msleep(1);
		}

		gk20a_writel(g, regs->cfg, 0x29000000);
		usleep_range(1000, 2000);
		gk20a_writel(g, regs->cfg, 0x28000000);
		gk20a_writel(g, regs->cfg, 0x2A000000);

		cfg_val = 0x22000000;
		gk20a_writel(g, regs->cfg, cfg_val);

		cnt0_val = gk20a_readl(g, regs->cnt0);
		cnt1_val = gk20a_readl(g, regs->cnt1);
		start_count = ((u64)(cnt1_val & 0xF) << 32) | cnt0_val;

		cfg_val = 0x2A000000;
		gk20a_writel(g, regs->cfg, cfg_val);

		t_start = ktime_get();
		usleep_range(1000, 2000);
		t_end = ktime_get();
		elapsed_ns = ktime_to_ns(ktime_sub(t_end, t_start));

		cfg_val = 0x22000000;
		gk20a_writel(g, regs->cfg, cfg_val);

		cnt0_val = gk20a_readl(g, regs->cnt0);
		cnt1_val = gk20a_readl(g, regs->cnt1);
		end_count = ((u64)(cnt1_val & 0xF) << 32) | cnt0_val;

		cfg_val = 0x2A000000;
		gk20a_writel(g, regs->cfg, cfg_val);

		/* Restore ELCG state if we disabled it */
		if (elcg_was_enabled) {
			nvgpu_cg_elcg_set_elcg_enabled(g, true);
		}
		/* Restore NVDEC_ELPG state if we disabled it */
		if (vid_elpg_was_enabled) {
			nvgpu_pg_vid_elpg_enable(g, PMU_PG_ELPG_ENGINE_ID_VID_NVDEC);
		}
		/* Restore GPC_RG state if we disabled it */
		if (gpc_rg_was_enabled) {
			nvgpu_pg_gpc_rg_enable(g);
		}

		gk20a_idle(g);

		freq = div64_u64((end_count - start_count) * 1000000000ULL, elapsed_ns);
	}

	len = snprintf(buf, sizeof(buf), "%llu\n", freq);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations pto_counter_generic_fops = {
	.open = simple_open,
	.read = pto_counter_generic_read,
	.llseek = default_llseek,
};

/* Initialize power management data structure */
int nvgpu_power_mgmt_data_init(struct gk20a *g)
{
	struct gpu_power_mgmt_data *pm_data;

	pm_data = nvgpu_kzalloc(g, sizeof(*pm_data));
	if (!pm_data) {
		return -ENOMEM;
	}

	nvgpu_mutex_init(&pm_data->sysfs_lock);

	/* Initialize mode with default values */
	pm_data->clock_mode = CLOCK_MODE_AUTO;      /* Default to auto mode */

	/* Initialize regimes to 0 - actual values will be set when sending RPCs */
	pm_data->gpc_regime = 0;
	pm_data->sys_regime = 0;
	pm_data->nvd_regime = 0;
	pm_data->uproc_regime = 0;

	/* TODO: Parse VF table to populate allowed values
	 * For now, initialize with zero counts and NULL pointers
	 */

	/* Initialize VDD GPC voltages */
	pm_data->vdd_gpc_num_allowed_voltages = 0;
	pm_data->vdd_gpc_allowed_voltages = NULL;

	/* Initialize VDD GPM voltages */
	pm_data->vdd_gpm_num_allowed_voltages = 0;
	pm_data->vdd_gpm_allowed_voltages = NULL;

	/* Initialize GPC clock rates */
	pm_data->num_allowed_gpc_rates = 0;
	pm_data->allowed_gpc_rates = NULL;

	/* Initialize SYS clock rates */
	pm_data->num_allowed_sys_rates = 0;
	pm_data->allowed_sys_rates = NULL;

	/* Initialize NVD clock rates */
	pm_data->num_allowed_nvd_rates = 0;
	pm_data->allowed_nvd_rates = NULL;

	/* Initialize UPROC clock rates */
	pm_data->num_allowed_uproc_rates = 0;
	pm_data->allowed_uproc_rates = NULL;

	g->power_mgmt_data = pm_data;

	nvgpu_info(g, "Clock management initialized - Mode: AUTO");
	return 0;
}

/* Cleanup power management data structure */
void nvgpu_power_mgmt_data_deinit(struct gk20a *g)
{
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;

	if (!pm_data) {
		return;
	}

	if (pm_data->allowed_uproc_rates) {
		nvgpu_kfree(g, pm_data->allowed_uproc_rates);
	}
	if (pm_data->allowed_nvd_rates) {
		nvgpu_kfree(g, pm_data->allowed_nvd_rates);
	}
	if (pm_data->allowed_sys_rates) {
		nvgpu_kfree(g, pm_data->allowed_sys_rates);
	}
	if (pm_data->allowed_gpc_rates) {
		nvgpu_kfree(g, pm_data->allowed_gpc_rates);
	}
	if (pm_data->vdd_gpm_allowed_voltages) {
		nvgpu_kfree(g, pm_data->vdd_gpm_allowed_voltages);
	}
	if (pm_data->vdd_gpc_allowed_voltages) {
		nvgpu_kfree(g, pm_data->vdd_gpc_allowed_voltages);
	}
	nvgpu_mutex_destroy(&pm_data->sysfs_lock);
	nvgpu_kfree(g, pm_data);
	g->power_mgmt_data = NULL;
}

/* VF Table Functions */
static ssize_t vf_table_read(struct file *file, char __user *user_buf,
			     size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	char *buf = NULL;
	int len = 0;
	int err = 0;
	struct nvgpu_clk_vf_points_status *vf_points_status = NULL;
	u32 num_vf_points = 0;
	u32 i = 0;
	ssize_t ret = 0;
	char error_buf[64] = {};

	if (!g) {
		len = snprintf(error_buf, sizeof(error_buf), "Invalid GPU instance\n");
		return simple_read_from_buffer(user_buf, count, ppos, error_buf, len);
	}

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		len = snprintf(error_buf, sizeof(error_buf), "Not Supported in virtual mode\n");
		return simple_read_from_buffer(user_buf, count, ppos, error_buf, len);
	}

	if (nvgpu_is_powered_off(g)) {
		len = snprintf(error_buf, sizeof(error_buf), "GPU is powered off\n");
		return simple_read_from_buffer(user_buf, count, ppos, error_buf, len);
	}

	buf = nvgpu_kzalloc(g, 65536);
	if (!buf)
		return -ENOMEM;
	/* Allocate memory for VF points status to avoid stack overflow */
	vf_points_status = nvgpu_kzalloc(g, sizeof(*vf_points_status));
	if (!vf_points_status) {
		ret = -ENOMEM;
		goto cleanup_buf;
	}

	if (g->is_virtual) {
#ifdef CONFIG_NVGPU_GR_VIRTUALIZATION
		err = vgpu_get_vf_points_status(g, vf_points_status, &num_vf_points, TEGRA_VGPU_CLK_ALL);
		if (err) {
			nvgpu_err(g, "vgpu_get_vf_points_status failed: %d", err);
			goto cleanup_vf_status;
		}
#else
		nvgpu_err(g, "%s not supported", __func__);
		ret = -EOPNOTSUPP;
		goto cleanup_vf_status;
#endif
	} else {
		/* Get VF points status using NVGPU structures */
		err = nvgpu_clk_vf_points_get_status(g, vf_points_status);
		if (err != 0) {
			len = snprintf(buf, 65536, "Error getting VF points status: %d\n", err);
			goto cleanup_vf_status;
		}
		num_vf_points = vf_points_status->super.objCount;
	}

	len = snprintf(buf, 65536, "VF Table Status:\n");
	len += snprintf(buf + len, 65536 - len,
			"Number of VF Points: %u\n", num_vf_points);
	len += snprintf(buf + len, 65536 - len, "VF Points:\n");
	len += snprintf(buf + len, 65536 - len, "Index\tFreq(MHz)\tVoltage(uV)\n");

	for (i = 0; i < num_vf_points && i < NVGPU_CLK_VF_POINTS_MAX_OBJECTS; i++) {
		struct nvgpu_clk_vf_point_status *vf_point = &vf_points_status->vf_points[i];

		/* Skip invalid VF points (zero freq and zero voltage are invalid) */
		if (vf_point->freq_mhz == 0 && vf_point->voltage_uv == 0) {
			continue;
		}

		if (len >= 65536 - 128) {
			len += snprintf(buf + len, 65536 - len, "... (truncated)\n");
			break;
		}

		len += snprintf(buf + len, 65536 - len, "%u\t%u\t\t%u\n",
			i, vf_point->freq_mhz, vf_point->voltage_uv);
	}

cleanup_vf_status:
	nvgpu_kfree(g, vf_points_status);
cleanup_buf:
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, len);
	nvgpu_kfree(g, buf);
	return ret;
}

/* Voltage Functions */
static ssize_t vdd_gpc_voltage_read(struct file *file, char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[64];
	int len;
	int err;
	void *pmu_status_buf = NULL;
	u32 gpc_voltage_uv = 0;

	if (g->is_virtual) {
#ifdef CONFIG_NVGPU_GR_VIRTUALIZATION
		err = vgpu_get_gpu_voltage(g, TEGRA_VGPU_GPU_VOLTAGE_TYPE_GPC, &gpc_voltage_uv);
		if (err) {
			nvgpu_err(g, "vgpu_get_gpu_voltage failed: %d", err);
			return err;
		}
#else
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
#endif
	} else {
		if (!pm_data) {
			len = snprintf(buf, sizeof(buf), "0\n");
			return simple_read_from_buffer(user_buf, count, ppos, buf, len);
		}

		if (nvgpu_is_powered_off(g))
			return -EAGAIN;

		err = gk20a_busy(g);
		if (err != 0)
			return -EAGAIN;

		err = nvgpu_pmu_wait_fw_ready(g, g->pmu);
		if (err != 0) {
			nvgpu_err(g, "PMU not ready to process requests");
			gk20a_idle(g);
			return -EAGAIN;
		}

		nvgpu_mutex_acquire(&pm_data->sysfs_lock);

		err = nvgpu_volt_rail_get_status(g, &pmu_status_buf);
		if (err != 0) {
			nvgpu_err(g, "Failed to get PMU voltage status");
			nvgpu_mutex_release(&pm_data->sysfs_lock);
			gk20a_idle(g);
			return -EAGAIN;
		}

		err = nvgpu_volt_rail_get_gpc_voltage(g, pmu_status_buf, &gpc_voltage_uv);
		if (err != 0) {
			len = snprintf(buf, sizeof(buf),
				"GPC voltage unavailable: failed to extract GPC voltage (%d)\n", err);
			nvgpu_mutex_release(&pm_data->sysfs_lock);
			gk20a_idle(g);
			return simple_read_from_buffer(user_buf, count, ppos, buf, len);
		}


		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
	}

	len = snprintf(buf, sizeof(buf), "%d\n", gpc_voltage_uv);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t vdd_gpc_voltage_write(struct file *file, const char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	u64 voltage;
	int ret;
	int err;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (!pm_data) {
		return -ENODEV;
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;


	if (count >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;

	buf[count] = '\0';

	ret = kstrtoull(buf, 0, &voltage);
	if (ret)
		return ret;

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	/* Set GPC clock domain to VR regime before setting voltage */
	err = nvgpu_pmu_clk_domains_fll_set_regime_50(g, CTRL_CLK_FLL_REGIME_ID_VR,
						       NV2080_CTRL_CLK_DOMAIN_LEGACY_GPCCLK);
	if (err != 0) {
		nvgpu_err(g, "Failed to set GPC domain to VR regime: %d", err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	/* Update stored regime value */
	pm_data->gpc_regime = CTRL_CLK_FLL_REGIME_ID_VR;

	/* Automatically switch to manual mode when VR regime is set */
	if (pm_data->clock_mode != CLOCK_MODE_MANUAL) {
		pm_data->clock_mode = CLOCK_MODE_MANUAL;
		nvgpu_info(g, "Switched to MANUAL mode for VR regime");
	}

	/* Execute voltage-only change via PMU change sequencer */
	err = nvgpu_pmu_perf_changeseq_set_voltage_only(g, GB20C_VOLT_RAIL_GPC_IDX, (u32)voltage);
	if (err != 0) {
		nvgpu_err(g, "Failed to set GPC voltage via change sequencer: %d", err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	nvgpu_info(g, "Setting VDD GPC voltage to %llu uV with VR regime", voltage);

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return count;
}

static ssize_t vdd_gpm_voltage_read(struct file *file, char __user *user_buf,
				     size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[64];
	int len;
	int err;
	void *pmu_status_buf = NULL;
	u32 gpm_voltage_uv = 0;

	if (g->is_virtual) {
#ifdef CONFIG_NVGPU_GR_VIRTUALIZATION
		err = vgpu_get_gpu_voltage(g, TEGRA_VGPU_GPU_VOLTAGE_TYPE_GPM, &gpm_voltage_uv);
		if (err) {
			nvgpu_err(g, "vgpu_get_gpu_voltage failed: %d", err);
			return err;
		}
#else
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
#endif
	} else {
		if (!pm_data) {
			len = snprintf(buf, sizeof(buf), "0\n");
			return simple_read_from_buffer(user_buf, count, ppos, buf, len);
		}

		if (nvgpu_is_powered_off(g))
			return -EAGAIN;

		err = gk20a_busy(g);
		if (err != 0)
			return -EAGAIN;

		err = nvgpu_pmu_wait_fw_ready(g, g->pmu);
		if (err != 0) {
			nvgpu_err(g, "PMU not ready to process requests");
			gk20a_idle(g);
			return -EAGAIN;
		}

		nvgpu_mutex_acquire(&pm_data->sysfs_lock);

		err = nvgpu_volt_rail_get_status(g, &pmu_status_buf);
		if (err != 0) {
			nvgpu_err(g, "Failed to get PMU voltage status");
			nvgpu_mutex_release(&pm_data->sysfs_lock);
			gk20a_idle(g);
			return -EAGAIN;
		}

		err = nvgpu_volt_rail_get_gpm_voltage(g, pmu_status_buf, &gpm_voltage_uv);
		if (err != 0) {
			len = snprintf(buf, sizeof(buf),
				"GPM voltage unavailable: failed to extract GPM voltage (%d)\n", err);
			nvgpu_mutex_release(&pm_data->sysfs_lock);
			gk20a_idle(g);
			return simple_read_from_buffer(user_buf, count, ppos, buf, len);
		}

		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
	}

	len = snprintf(buf, sizeof(buf), "%d\n", gpm_voltage_uv);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t vdd_gpm_voltage_write(struct file *file, const char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	u64 voltage;
	int ret;
	int err;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (!pm_data) {
		return -ENODEV;
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;

	if (count >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;

	buf[count] = '\0';

	ret = kstrtoull(buf, 0, &voltage);
	if (ret)
		return ret;

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	/* Set SYS clock domain to VR regime before setting voltage */
	err = nvgpu_pmu_clk_domains_fll_set_regime_50(g, CTRL_CLK_FLL_REGIME_ID_VR,
						       NV2080_CTRL_CLK_DOMAIN_LEGACY_XBARCLK);
	if (err != 0) {
		nvgpu_err(g, "Failed to set SYS domain to VR regime: %d", err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	pm_data->sys_regime = CTRL_CLK_FLL_REGIME_ID_VR;

	/* Set NVD clock domain to VR regime */
	err = nvgpu_pmu_clk_domains_fll_set_regime_50(g, CTRL_CLK_FLL_REGIME_ID_VR,
						       NV2080_CTRL_CLK_DOMAIN_LEGACY_NVDCLK);
	if (err != 0) {
		nvgpu_err(g, "Failed to set NVD domain to VR regime: %d", err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	pm_data->nvd_regime = CTRL_CLK_FLL_REGIME_ID_VR;

	/* Set UPROC clock domain to VR regime */
	err = nvgpu_pmu_clk_domains_fll_set_regime_50(g, CTRL_CLK_FLL_REGIME_ID_VR,
						       NV2080_CTRL_CLK_DOMAIN_LEGACY_UPROCCLK);
	if (err != 0) {
		nvgpu_err(g, "Failed to set UPROC domain to VR regime: %d", err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	pm_data->uproc_regime = CTRL_CLK_FLL_REGIME_ID_VR;

	/* Automatically switch to manual mode when VR regime is set */
	if (pm_data->clock_mode != CLOCK_MODE_MANUAL) {
		pm_data->clock_mode = CLOCK_MODE_MANUAL;
		nvgpu_info(g, "Switched to MANUAL mode for VR regime");
	}

	/* Execute voltage-only change via PMU change sequencer */
	err = nvgpu_pmu_perf_changeseq_set_voltage_only(g, GB20C_VOLT_RAIL_GPM_IDX, (u32)voltage);
	if (err != 0) {
		nvgpu_err(g, "Failed to set GPM voltage via change sequencer: %d", err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	nvgpu_info(g, "Setting VDD GPM voltage to %llu uV with VR regime", voltage);

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return count;
}

static ssize_t vdd_gpc_rail_limit_offset_vmin_read(struct file *file,
	char __user *user_buf,
	size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[64];
	int len;
	int err;
	u32 gpc_voltage_mv = 0;

	if (g->is_virtual) {
		nvgpu_err(g, "rail_limit_offset_vmin_mv read not supported in non-virtual mode");
		return -ENOSYS;
	} else {
		if (!pm_data) {
			len = snprintf(buf, sizeof(buf), "0\n");
			return simple_read_from_buffer(user_buf, count, ppos, buf, len);
		}

		if (nvgpu_is_powered_off(g))
			return -EAGAIN;


		err = gk20a_busy(g);
		if (err != 0)
			return -EAGAIN;

		err = nvgpu_pmu_wait_fw_ready(g, g->pmu);
		if (err != 0) {
			nvgpu_err(g, "PMU not ready to process requests");
			gk20a_idle(g);
		return -EAGAIN;
		}

		nvgpu_mutex_acquire(&pm_data->sysfs_lock);

		// TODO: Implement actual rail_limit_offset reading from board obj
		len = snprintf(buf, sizeof(buf), "0\n");

		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
	}

	len = snprintf(buf, sizeof(buf), "%d\n", gpc_voltage_mv);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t vdd_gpc_rail_limit_offset_vmin_write(struct file *file,
	const char __user *user_buf,
	size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	s32 voltage;
	int ret;
	int err;

	if (!pm_data) {
		return -ENODEV;
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;


	if (count >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;

	buf[count] = '\0';

	ret = kstrtoint(buf, 0, &voltage);
	if (ret)
		return ret;

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	// Send VDD GPC rail_limit_offset setting to PMU in units of uV
	nvgpu_info(g, "Setting VDD GPC rail_limit_offset_vmin_mv to %d mV", voltage);
	err = nvgpu_volt_rail_set_gpc_limit(g, voltage * 1000U, CTRL_VOLT_RAIL_VOLT_DELTA_VMIN_LIM_IDX);
	if (err) {
		nvgpu_err(g, "Failed to set GPC rail_limit_offset_vmin_mv to %d mV - %d",
			voltage, err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return count;
}

static ssize_t vdd_gpc_rail_limit_offset_vmax_read(struct file *file,
	char __user *user_buf,
	size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[64];
	int len;
	int err;
	u32 gpc_voltage_mv = 0;

	if (g->is_virtual) {
		nvgpu_err(g, "rail_limit_offset_vmax_mv read not supported in non-virtual mode");
		return -ENOSYS;
	} else {
		if (!pm_data) {
			len = snprintf(buf, sizeof(buf), "0\n");
			return simple_read_from_buffer(user_buf, count, ppos, buf, len);
		}

		if (nvgpu_is_powered_off(g))
			return -EAGAIN;


		err = gk20a_busy(g);
		if (err != 0)
			return -EAGAIN;

		err = nvgpu_pmu_wait_fw_ready(g, g->pmu);
		if (err != 0) {
			nvgpu_err(g, "PMU not ready to process requests");
			gk20a_idle(g);
		return -EAGAIN;
		}

		nvgpu_mutex_acquire(&pm_data->sysfs_lock);

		// TODO: Implement actual rail_limit_offset reading from board obj
		len = snprintf(buf, sizeof(buf), "0\n");

		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
	}

	len = snprintf(buf, sizeof(buf), "%d\n", gpc_voltage_mv);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t vdd_gpc_rail_limit_offset_vmax_write(struct file *file,
	const char __user *user_buf,
	size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	s32 voltage;
	int ret;
	int err;

	if (!pm_data) {
		return -ENODEV;
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;


	if (count >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;

	buf[count] = '\0';

	ret = kstrtoint(buf, 0, &voltage);
	if (ret)
		return ret;

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	// Send VDD GPC rail_limit_offset setting to PMU in units of uV
	nvgpu_info(g, "Setting VDD GPC rail_limit_offset_vmax_mv to %d mV", voltage);
	err = nvgpu_volt_rail_set_gpc_limit(g, voltage * 1000U, CTRL_VOLT_RAIL_VOLT_DELTA_REL_LIM_IDX);
	if (err) {
		nvgpu_err(g, "Failed to set GPC rail_limit_offset_vmax_mv to %d mV - %d",
			voltage, err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return count;
}

static ssize_t vdd_gpm_rail_limit_offset_vmin_read(struct file *file,
	char __user *user_buf,
	size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[64];
	int len;
	int err;
	u32 gpm_voltage_uv = 0;

	if (g->is_virtual) {
		nvgpu_err(g, "rail_limit_offset_vmin_mv read not supported in non-virtual mode");
		return -ENOSYS;
	} else {
		if (!pm_data) {
			len = snprintf(buf, sizeof(buf), "0\n");
			return simple_read_from_buffer(user_buf, count, ppos, buf, len);
		}

		if (nvgpu_is_powered_off(g))
			return -EAGAIN;


		err = gk20a_busy(g);
		if (err != 0)
			return -EAGAIN;

		err = nvgpu_pmu_wait_fw_ready(g, g->pmu);
		if (err != 0) {
			nvgpu_err(g, "PMU not ready to process requests");
			gk20a_idle(g);
		return -EAGAIN;
		}

		nvgpu_mutex_acquire(&pm_data->sysfs_lock);

		// TODO: Implement actual rail_limit_offset reading from board obj
		len = snprintf(buf, sizeof(buf), "0\n");

		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
	}

	len = snprintf(buf, sizeof(buf), "%d\n", gpm_voltage_uv);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t vdd_gpm_rail_limit_offset_vmin_write(struct file *file,
	const char __user *user_buf,
	size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	s32 voltage;
	int ret;
	int err;

	if (!pm_data) {
		return -ENODEV;
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;


	if (count >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;

	buf[count] = '\0';

	ret = kstrtoint(buf, 0, &voltage);
	if (ret)
		return ret;

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	// Send VDD GPC rail_limit_offset setting to PMU in units of uV
	nvgpu_info(g, "Setting VDD GPM rail_limit_offset_vmin_mv to %d mV", voltage);
	err = nvgpu_volt_rail_set_gpm_limit(g, voltage * 1000U, CTRL_VOLT_RAIL_VOLT_DELTA_VMIN_LIM_IDX);
	if (err) {
		nvgpu_err(g, "Failed to set GPM rail_limit_offset_vmin_mv to %d mV - %d",
			voltage, err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return count;
}

static ssize_t vdd_gpm_rail_limit_offset_vmax_read(struct file *file,
	char __user *user_buf,
	size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[64];
	int len;
	int err;
	u32 gpm_voltage_uv = 0;

	if (g->is_virtual) {
		nvgpu_err(g, "rail_limit_offset_vmax_mv read not supported in non-virtual mode");
		return -ENOSYS;
	} else {
		if (!pm_data) {
			len = snprintf(buf, sizeof(buf), "0\n");
			return simple_read_from_buffer(user_buf, count, ppos, buf, len);
		}

		if (nvgpu_is_powered_off(g))
			return -EAGAIN;


		err = gk20a_busy(g);
		if (err != 0)
			return -EAGAIN;

		err = nvgpu_pmu_wait_fw_ready(g, g->pmu);
		if (err != 0) {
			nvgpu_err(g, "PMU not ready to process requests");
			gk20a_idle(g);
		return -EAGAIN;
		}

		nvgpu_mutex_acquire(&pm_data->sysfs_lock);

		// TODO: Implement actual rail_limit_offset reading from board obj
		len = snprintf(buf, sizeof(buf), "0\n");

		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
	}

	len = snprintf(buf, sizeof(buf), "%d\n", gpm_voltage_uv);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t vdd_gpm_rail_limit_offset_vmax_write(struct file *file,
	const char __user *user_buf,
	size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	s32 voltage;
	int ret;
	int err;

	if (!pm_data) {
		return -ENODEV;
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;


	if (count >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;

	buf[count] = '\0';

	ret = kstrtoint(buf, 0, &voltage);
	if (ret)
		return ret;

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	// Send VDD GPC rail_limit_offset setting to PMU in units of uV
	nvgpu_info(g, "Setting VDD GPM rail_limit_offset_vmax_mv to %d mV", voltage);
	err = nvgpu_volt_rail_set_gpm_limit(g, voltage * 1000U, CTRL_VOLT_RAIL_VOLT_DELTA_REL_LIM_IDX);
	if (err) {
		nvgpu_err(g, "Failed to set GPM rail_limit_offset_vmax_mv to %d mV - %d",
			voltage, err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return count;
}

/* Power Management Initialization */
static void gk20a_debug_clk_power_mgmt_init(struct gk20a *g)
{
	int ret;

	ret = nvgpu_power_mgmt_data_init(g);
	if (ret) {
		nvgpu_err(g, "Failed to initialize power management data: %d", ret);
	}
}

void gk20a_debug_clk_init(struct gk20a *g)
{
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct dentry *gpu_root;
	struct dentry *clk_dir;
	struct dentry *regulator_dir;
	struct dentry *gpu_gpc_dir, *gpu_sys_dir, *gpu_nvd_dir, *gpu_uproc_dir;
	struct dentry *vdd_gpc_dir, *vdd_gpm_dir;
	struct gk20a_platform *platform = gk20a_get_platform(dev_from_gk20a(g));
	bool chip_id = false;

	/* Initialize power management data */
	gk20a_debug_clk_power_mgmt_init(g);

	gpu_root = l->debugfs;

	/* Clock root directory */
	clk_dir = debugfs_create_dir("clk", gpu_root);
	if (IS_ERR_OR_NULL(clk_dir)) {
		nvgpu_err(g, "Failed to create clk debugfs directory");
		return;
	}

	/* Clock mode control file */
	debugfs_create_file("mode", 0644, clk_dir, g, &clock_mode_fops);

	/* Global VF Table - accessible at clk/vf_table */
	debugfs_create_file("vf_table", 0444, clk_dir, g, &vf_table_fops);

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB20C)
	if ((platform->platform_chip_id == TEGRA_256) ||
		(platform->platform_chip_id == TEGRA_256_VF))
		chip_id = true;
#endif
	/* GPU GPC clock subdirectory */
	gpu_gpc_dir = debugfs_create_dir("gpu_gpc", clk_dir);
	if (!IS_ERR_OR_NULL(gpu_gpc_dir)) {
		if (!chip_id) {
			debugfs_create_file("rate", 0644, gpu_gpc_dir, g, &gpu_gpc_rate_fops);
			debugfs_create_file("freq_offset_khz", 0644, gpu_gpc_dir, g,
					&gpu_gpc_freq_offset_khz_fops);
		}
		debugfs_create_file("regime", 0644, gpu_gpc_dir, g, &gpc_regime_fops);
		debugfs_create_file("pto_counter", 0444, gpu_gpc_dir, &gpc_ctx, &pto_counter_generic_fops);
		debugfs_create_file("allowed_rates", 0444, gpu_gpc_dir, g, &gpu_gpc_allowed_rates_fops);
		debugfs_create_file("drive_rate", 0444, gpu_gpc_dir, g, &gpu_gpc_drive_freq_mhz_fops);
		debugfs_create_file("sentry_rate", 0444, gpu_gpc_dir, g, &gpu_gpc_sentry_freq_mhz_fops);
	}

	/* GPU SYS clock subdirectory */
	gpu_sys_dir = debugfs_create_dir("gpu_sys", clk_dir);
	if (!IS_ERR_OR_NULL(gpu_sys_dir)) {
		if (!chip_id) {
			debugfs_create_file("rate", 0644, gpu_sys_dir, g, &gpu_sys_rate_fops);
			debugfs_create_file("freq_offset_khz", 0644, gpu_sys_dir, g,
					&gpu_sys_freq_offset_khz_fops);
		}
		debugfs_create_file("regime", 0644, gpu_sys_dir, g, &sys_regime_fops);
		debugfs_create_file("pto_counter", 0444, gpu_sys_dir, &sys_ctx, &pto_counter_generic_fops);
		debugfs_create_file("allowed_rates", 0444, gpu_sys_dir, g, &gpu_sys_allowed_rates_fops);
	}

	/* GPU NVD clock subdirectory */
	gpu_nvd_dir = debugfs_create_dir("gpu_nvd", clk_dir);
	if (!IS_ERR_OR_NULL(gpu_nvd_dir)) {
		if (!chip_id) {
			debugfs_create_file("rate", 0644, gpu_nvd_dir, g, &gpu_nvd_rate_fops);
			debugfs_create_file("freq_offset_khz", 0644, gpu_nvd_dir, g,
					&gpu_nvd_freq_offset_khz_fops);
		}
		debugfs_create_file("regime", 0644, gpu_nvd_dir, g, &nvd_regime_fops);
		debugfs_create_file("pto_counter", 0444, gpu_nvd_dir, &nvd_ctx, &pto_counter_generic_fops);
		debugfs_create_file("allowed_rates", 0444, gpu_nvd_dir, g, &gpu_nvd_allowed_rates_fops);
	}

	/* GPU UPROC clock subdirectory */
	gpu_uproc_dir = debugfs_create_dir("gpu_uproc", clk_dir);
	if (!IS_ERR_OR_NULL(gpu_uproc_dir)) {
		if (!chip_id) {
			debugfs_create_file("rate", 0644, gpu_uproc_dir, g, &gpu_uproc_rate_fops);
			debugfs_create_file("freq_offset_khz", 0644, gpu_uproc_dir, g,
					&gpu_uproc_freq_offset_khz_fops);
		}
		debugfs_create_file("regime", 0644, gpu_uproc_dir, g, &uproc_regime_fops);
		debugfs_create_file("pto_counter", 0444, gpu_uproc_dir, &uproc_ctx, &pto_counter_generic_fops);
		debugfs_create_file("allowed_rates", 0444, gpu_uproc_dir, g, &gpu_uproc_allowed_rates_fops);
	}

	/* Regulator root directory */
	regulator_dir = debugfs_create_dir("regulator", gpu_root);
	if (IS_ERR_OR_NULL(regulator_dir)) {
		nvgpu_err(g, "Failed to create regulator debugfs directory");
		return;
	}

	/* VDD GPC regulator subdirectory */
	vdd_gpc_dir = debugfs_create_dir("vdd_gpc", regulator_dir);
	if (!IS_ERR_OR_NULL(vdd_gpc_dir)) {
		debugfs_create_file("voltage", 0644, vdd_gpc_dir, g, &vdd_gpc_voltage_fops);
		debugfs_create_file("voltage_limits", 0444, vdd_gpc_dir, g, &gpc_voltage_limits_fops);
		debugfs_create_file("rail_limit_offset_vmin_mv", 0644, vdd_gpc_dir, g,
			 &vdd_gpc_rail_limit_offset_vmin_fops);
		debugfs_create_file("rail_limit_offset_vmax_mv", 0644, vdd_gpc_dir, g,
			 &vdd_gpc_rail_limit_offset_vmax_fops);
	}

	/* VDD GPM regulator subdirectory */
	vdd_gpm_dir = debugfs_create_dir("vdd_gpm", regulator_dir);
	if (!IS_ERR_OR_NULL(vdd_gpm_dir)) {
		debugfs_create_file("voltage", 0644, vdd_gpm_dir, g, &vdd_gpm_voltage_fops);
		debugfs_create_file("voltage_limits", 0444, vdd_gpm_dir, g, &gpm_voltage_limits_fops);
		debugfs_create_file("rail_limit_offset_vmin_mv", 0644, vdd_gpm_dir, g,
			&vdd_gpm_rail_limit_offset_vmin_fops);
		debugfs_create_file("rail_limit_offset_vmax_mv", 0644, vdd_gpm_dir, g,
			&vdd_gpm_rail_limit_offset_vmax_fops);
	}

	/* Voltage limits debugfs entry */

	/* Initialize PTO counter contexts */
	gpc_ctx.g = g;
	gpc_ctx.regs = &pto_gpc_regs;
	sys_ctx.g = g;
	sys_ctx.regs = &pto_sys_regs;
	nvd_ctx.g = g;
	nvd_ctx.regs = &pto_nvd_regs;
	uproc_ctx.g = g;
	uproc_ctx.regs = &pto_uproc_regs;
}

/* Clock Drive & Sentry frequency Functions */
static ssize_t gpu_gpc_drive_freq_mhz_read(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	int len;
	u32 freq = 0;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (!pm_data) {
		len = snprintf(buf, sizeof(buf), "0\n");
		return simple_read_from_buffer(user_buf, count, ppos, buf, len);
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	// Get GPU GPC Drive frequency value from vbios
	(void) clk_domain_get_gpc_drive_mode_freq(g, &freq);
	len = snprintf(buf, sizeof(buf), "%u\n", freq);

	nvgpu_mutex_release(&pm_data->sysfs_lock);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t gpu_gpc_sentry_freq_mhz_read(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	int len;
	u32 freq = 0;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (!pm_data) {
		len = snprintf(buf, sizeof(buf), "0\n");
		return simple_read_from_buffer(user_buf, count, ppos, buf, len);
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	// Get GPU GPC Sentry frequency value from vbios
	(void) clk_domain_get_gpc_sentry_mode_freq(g, &freq);
	len = snprintf(buf, sizeof(buf), "%u\n", freq);

	nvgpu_mutex_release(&pm_data->sysfs_lock);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

/* Clock offset Functions */
static ssize_t gpu_gpc_freq_offset_khz_read(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = NULL;
	char buf[32] = {};
	int len = 0;
	int err = 0;

	if (!g)
		return -EINVAL;

	pm_data = g->power_mgmt_data;
	if (!pm_data) {
		nvgpu_err(g, "pm data is NULL\n");
		len = snprintf(buf, sizeof(buf), "0\n");
		return simple_read_from_buffer(user_buf, count, ppos, buf, len);
	}

	if (nvgpu_is_powered_off(g)) {
		nvgpu_info(g, "nvgpu is powered off\n");
		return -EAGAIN;
	}


	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	// TODO: Implement actual GPU GPC ofset_khz reading from board obj
	len = snprintf(buf, sizeof(buf), "0\n");

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t gpu_nvd_freq_offset_khz_read(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = NULL;
	char buf[32] = {};
	int len = 0;
	int err = 0;

	if (!g)
		return -EINVAL;

	pm_data = g->power_mgmt_data;
	if (!pm_data) {
		nvgpu_err(g, "pm data is NULL\n");
		len = snprintf(buf, sizeof(buf), "0\n");
		return simple_read_from_buffer(user_buf, count, ppos, buf, len);
	}

	if (nvgpu_is_powered_off(g)) {
		nvgpu_info(g, "nvgpu is powered off\n");
		return -EAGAIN;
	}

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	// TODO: Implement actual GPU NVD ofset_khz reading from board obj
	len = snprintf(buf, sizeof(buf), "0\n");

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t gpu_sys_freq_offset_khz_read(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = NULL;
	char buf[32] = {};
	int len = 0;
	int err = 0;

	if (!g)
		return -EINVAL;

	pm_data = g->power_mgmt_data;
	if (!pm_data) {
		nvgpu_err(g, "pm data is NULL\n");
		len = snprintf(buf, sizeof(buf), "0\n");
		return simple_read_from_buffer(user_buf, count, ppos, buf, len);
	}

	if (nvgpu_is_powered_off(g)) {
		nvgpu_info(g, "nvgpu is powered off\n");
		return -EAGAIN;
	}


	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	// TODO: Implement actual GPU SYS ofset_khz reading from board obj
	len = snprintf(buf, sizeof(buf), "0\n");

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t gpu_uproc_freq_offset_khz_read(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = NULL;
	char buf[32] = {};
	int len = 0;
	int err = 0;

	if (!g)
		return -EINVAL;

	pm_data = g->power_mgmt_data;
	if (!pm_data) {
		nvgpu_err(g, "pm data is NULL\n");
		len = snprintf(buf, sizeof(buf), "0\n");
		return simple_read_from_buffer(user_buf, count, ppos, buf, len);
	}

	if (nvgpu_is_powered_off(g)) {
		nvgpu_info(g, "nvgpu is powered off\n");
		return -EAGAIN;
	}

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	// TODO: Implement actual GPU UPROC ofset_khz reading from board obj
	len = snprintf(buf, sizeof(buf), "0\n");

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t gpu_gpc_freq_offset_khz_write(struct file *file, const char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	s32 offset_khz;
	int buf_size;
	int err;

	if (!pm_data) {
		return -ENODEV;
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;

	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;

	buf[buf_size] = '\0';

	if (kstrtos32(buf, 0, &offset_khz) != 0)
		return -EINVAL;

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	/* Set the actual clock offset using PMU RPC */
	err = clk_domains_set_offset_50(g, offset_khz, CTRL_CLK_DOMAIN_GPCCLK);
	if (err) {
		nvgpu_err(g, "Failed to set GPC offset to %d KHz: %d", offset_khz, err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return count;
}

static ssize_t gpu_sys_freq_offset_khz_write(struct file *file, const char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	s32 offset_khz;
	int buf_size;
	int err;

	if (!pm_data) {
		return -ENODEV;
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;

	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;

	buf[buf_size] = '\0';

	if (kstrtos32(buf, 0, &offset_khz) != 0)
		return -EINVAL;

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	/* Set the actual clock offset using PMU RPC */
	err = clk_domains_set_offset_50(g, offset_khz, CTRL_CLK_DOMAIN_XBARCLK);
	if (err) {
		nvgpu_err(g, "Failed to set SYS offset to %d KHz: %d", offset_khz, err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return count;
}

static ssize_t gpu_nvd_freq_offset_khz_write(struct file *file, const char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	s32 offset_khz;
	int buf_size;
	int err;

	if (!pm_data) {
		return -ENODEV;
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;


	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;

	buf[buf_size] = '\0';

	if (kstrtos32(buf, 0, &offset_khz) != 0)
		return -EINVAL;

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	/* Set the actual clock offset using PMU RPC */
	err = clk_domains_set_offset_50(g, offset_khz, CTRL_CLK_DOMAIN_NVDCLK);
	if (err) {
		nvgpu_err(g, "Failed to set NVD offset to %d KHz: %d", offset_khz, err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return count;
}

static ssize_t gpu_uproc_freq_offset_khz_write(struct file *file, const char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	s32 offset_khz;
	int buf_size;
	int err;

	if (!pm_data) {
		return -ENODEV;
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;


	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;

	buf[buf_size] = '\0';

	if (kstrtos32(buf, 0, &offset_khz) != 0)
		return -EINVAL;

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	/* Set the actual clock offset using PMU RPC */
	err = clk_domains_set_offset_50(g, offset_khz, CTRL_CLK_DOMAIN_UPROCCLK);
	if (err) {
		nvgpu_err(g, "Failed to set UPROC offset to %d KHz: %d", offset_khz, err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return count;
}

/* Clock Rate Functions */
static ssize_t gpu_gpc_rate_read(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = NULL;
	char buf[32] = {};
	int len = 0;
	int err = 0;

	if (!g)
		return -EINVAL;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	pm_data = g->power_mgmt_data;
	if (!pm_data) {
		nvgpu_err(g, "pm data is NULL\n");
		len = snprintf(buf, sizeof(buf), "0\n");
		return simple_read_from_buffer(user_buf, count, ppos, buf, len);
	}

	if (nvgpu_is_powered_off(g)) {
		nvgpu_info(g, "nvgpu is powered off\n");
		return -EAGAIN;
	}

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	// TODO: Implement actual GPU GPC rate reading from hardware
	len = snprintf(buf, sizeof(buf), "0\n");

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t gpu_gpc_rate_write(struct file *file, const char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	u32 rate_mhz;
	int buf_size;
	int err;

	if (!pm_data) {
		return -ENODEV;
	}

	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;

	buf[buf_size] = '\0';

	if (kstrtou32(buf, 0, &rate_mhz) != 0)
		return -EINVAL;

	/* Input is already in MHz - no conversion needed */
	if (rate_mhz == 0) {
		nvgpu_err(g, "Invalid clock rate: %u MHz", rate_mhz);
		return -EINVAL;
	}

	if (g->is_virtual) {
#ifdef CONFIG_NVGPU_GR_VIRTUALIZATION
		/* Use IVC call for virtualized environment - convert MHz to Hz */
		unsigned long rate_hz;

		rate_hz = MHZ_TO_HZ(rate_mhz);
		err = g->ops.clk.set_rate(g, CTRL_CLK_DOMAIN_GPCCLK, rate_hz);
		if (err) {
			nvgpu_err(g, "vgpu_set_clk_rate failed: %d", err);
			return err;
		}
#else
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
#endif
	} else {
		if (nvgpu_is_powered_off(g))
			return -EAGAIN;

		err = gk20a_busy(g);
		if (err != 0)
			return -EAGAIN;

		err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
		if (err != 0) {
			nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
			gk20a_idle(g);
			return -EAGAIN;
		}

		nvgpu_mutex_acquire(&pm_data->sysfs_lock);

		/* Set the actual GPC clock rate using PMU RPC */
		err = set_clock_rate_via_pmu_rpc(g, rate_mhz, CTRL_CLK_DOMAIN_GPCCLK,
					pm_data->clock_mode);
		if (err) {
			nvgpu_err(g, "Failed to set GPC rate to %u MHz: %d", rate_mhz, err);
			nvgpu_mutex_release(&pm_data->sysfs_lock);
			gk20a_idle(g);
			return err;
		}

		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
	}

	return count;
}

static ssize_t gpu_sys_rate_read(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = NULL;
	char buf[32] =  {};
	int len = 0;
	int err = 0;

	if (!g)
		return -EINVAL;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	pm_data = g->power_mgmt_data;
	if (!pm_data) {
		nvgpu_err(g, "pm data is NULL\n");
		len = snprintf(buf, sizeof(buf), "0\n");
		return simple_read_from_buffer(user_buf, count, ppos, buf, len);
	}

	if (nvgpu_is_powered_off(g)) {
		nvgpu_info(g, "nvgpu is powered off\n");
		return -EAGAIN;
	}


	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);
	// TODO: Implement actual GPU SYS rate reading from hardware
	len = snprintf(buf, sizeof(buf), "0\n");

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t gpu_sys_rate_write(struct file *file, const char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[64];
	int buf_size;
	u32 rate_mhz;
	int err;

	if (!g)
		return -EINVAL;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (!pm_data) {
		return -ENODEV;
	}

	/* Check if write is allowed based on current mode */
	if (!is_rate_write_allowed(g, CTRL_CLK_DOMAIN_XBARCLK)) {
		nvgpu_err(g, "SYS rate write not allowed in auto mode. Only GPC rate is writable in auto mode.");
		return -EPERM;
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;


	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;

	buf[buf_size] = '\0';

	if (kstrtou32(buf, 10, &rate_mhz) != 0)
		return -EINVAL;

	/* Input is already in MHz - no conversion needed */
	if (rate_mhz == 0) {
		nvgpu_err(g, "Invalid clock rate: %u MHz", rate_mhz);
		return -EINVAL;
	}

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	/* Set the actual SYS clock rate using PMU RPC */
	err = set_clock_rate_via_pmu_rpc(g, rate_mhz, CTRL_CLK_DOMAIN_XBARCLK,
				CLOCK_MODE_MANUAL);
	if (err) {
		nvgpu_err(g, "Failed to set SYS rate to %u MHz: %d", rate_mhz, err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return count;
}

static ssize_t gpu_nvd_rate_read(struct file *file, char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = NULL;
	char buf[32] = {};
	int len = 0;
	int err = 0;

	if (!g)
		return -EINVAL;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	pm_data = g->power_mgmt_data;
	if (!pm_data) {
		nvgpu_err(g, "pm data is NULL\n");
		len = snprintf(buf, sizeof(buf), "0\n");
		return simple_read_from_buffer(user_buf, count, ppos, buf, len);
	}

	if (nvgpu_is_powered_off(g)) {
		nvgpu_info(g, "nvgpu is powered off\n");
		return -EAGAIN;
	}

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	// TODO: Implement actual GPU NVD rate reading from hardware
	len = snprintf(buf, sizeof(buf), "0\n");

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t gpu_nvd_rate_write(struct file *file, const char __user *user_buf,
				  size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[64];
	int buf_size;
	u32 rate_mhz;
	int err;

	if (!g)
		return -EINVAL;

	if (!pm_data) {
		return -ENODEV;
	}

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	/* Check if write is allowed based on current mode */
	if (!is_rate_write_allowed(g, CTRL_CLK_DOMAIN_NVDCLK)) {
		nvgpu_err(g, "NVD rate write not allowed in auto mode. Only GPC rate is writable in auto mode.");
		return -EPERM;
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;


	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;

	buf[buf_size] = '\0';

	if (kstrtou32(buf, 0, &rate_mhz) != 0)
		return -EINVAL;

	/* Input is already in MHz - no conversion needed */
	if (rate_mhz == 0) {
		nvgpu_err(g, "Invalid clock rate: %u MHz", rate_mhz);
		return -EINVAL;
	}

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	/* Use NVD domain to trigger a VF point change that includes NVD */
	err = set_clock_rate_via_pmu_rpc(g, rate_mhz, CTRL_CLK_DOMAIN_NVDCLK,
				CLOCK_MODE_MANUAL);
	if (err) {
		nvgpu_err(g, "Failed to set NVD rate to %u MHz: %d", rate_mhz, err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return count;
}

static ssize_t gpu_uproc_rate_read(struct file *file, char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = NULL;
	char buf[32] = {};
	int len = 0;
	int err = 0;

	if (!g)
		return -EINVAL;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	pm_data = g->power_mgmt_data;
	if (!pm_data) {
		nvgpu_err(g, "pm data is NULL\n");
		len = snprintf(buf, sizeof(buf), "0\n");
		return simple_read_from_buffer(user_buf, count, ppos, buf, len);
	}

	if (nvgpu_is_powered_off(g)) {
		nvgpu_info(g, "nvgpu is powered off\n");
		return -EAGAIN;
	}

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	// TODO: Implement actual GPU UPROC rate reading from hardware
	len = snprintf(buf, sizeof(buf), "0\n");

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t gpu_uproc_rate_write(struct file *file, const char __user *user_buf,
				    size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[64];
	int buf_size;
	u32 rate_mhz;
	int err;

	if (!g)
		return -EINVAL;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (!pm_data) {
		return -ENODEV;
	}

	/* Check if write is allowed based on current mode */
	if (!is_rate_write_allowed(g, CTRL_CLK_DOMAIN_UPROCCLK)) {
		nvgpu_err(g, "UPROC rate write not allowed in auto mode. Only GPC rate is writable in auto mode.");
		return -EPERM;
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;


	buf_size = min(count, (sizeof(buf)-1));
	if (copy_from_user(buf, user_buf, buf_size))
		return -EFAULT;

	buf[buf_size] = '\0';

	if (kstrtou32(buf, 0, &rate_mhz) != 0)
		return -EINVAL;

	/* Input is already in MHz - no conversion needed */
	if (rate_mhz == 0) {
		nvgpu_err(g, "Invalid clock rate: %u MHz", rate_mhz);
		return -EINVAL;
	}

	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	err = nvgpu_pmu_wait_pg_ready(g, g->pmu);
	if (err != 0) {
		nvgpu_err(g, "PMU PG not booted yet, can't process RPCs");
		gk20a_idle(g);
		return -EAGAIN;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	/* Use UPROC domain to trigger a VF point change that includes UPROC */
	err = set_clock_rate_via_pmu_rpc(g, rate_mhz, CTRL_CLK_DOMAIN_UPROCCLK,
				CLOCK_MODE_MANUAL);
	if (err) {
		nvgpu_err(g, "Failed to set UPROC rate to %u MHz: %d", rate_mhz, err);
		nvgpu_mutex_release(&pm_data->sysfs_lock);
		gk20a_idle(g);
		return err;
	}

	nvgpu_mutex_release(&pm_data->sysfs_lock);
	gk20a_idle(g);

	return count;
}

/* Allowed Rates Functions */
static ssize_t gpu_gpc_allowed_rates_read(struct file *file, char __user *user_buf,
					   size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	char *buf = NULL;
	int len = 0;
	int err = 0;
	struct nvgpu_clk_vf_points_status *vf_points_status = NULL;
	u32 i = 0;
	ssize_t ret = 0;
	u32 gpc_vmin_uv = 0, gpc_vmax_uv = 0;
	u16 gpc_domain_idx = 0;
	struct boardobjgrp_e32 *psuper_domain = NULL;
	struct pmu_board_obj *domain_obj = NULL;
	struct clk_domain_50_prog_physical *pDomain50ProgPhysical = NULL;
	struct clk_domain_50_physical_rail_vf_primary *pVfPrimary = NULL;
	struct nv_pmu_clk_clk_vf_rel_vf_entry_pri *pVfEntryPri = NULL;
	struct clk_vf_rel *pVfRel = NULL;
	u8 railidx = 0;
	u16 vf_start_idx = 0, vf_end_idx = 0;

	if (!g)
		return -EINVAL;

	buf = nvgpu_kzalloc(g, 65536);
	if (!buf) {
		return -ENOMEM;
	}

	/* Allocate memory for VF points status */
	vf_points_status = nvgpu_kzalloc(g, sizeof(*vf_points_status));
	if (!vf_points_status) {
		ret = -ENOMEM;
		goto cleanup_buf;
	}

	if (g->is_virtual) {
#ifdef CONFIG_NVGPU_GR_VIRTUALIZATION
		u32 num_vf_points = NVGPU_CLK_VF_POINTS_MAX_OBJECTS;
		/* Use IVC call for virtualized environment */
		err = vgpu_get_vf_points_status(g, vf_points_status, &num_vf_points, TEGRA_VGPU_CLK_GPC);
		if (err) {
			nvgpu_err(g, "vgpu_get_vf_points_status failed: %d", err);
			goto cleanup_vf_status;
		}
		vf_start_idx = 0;
		vf_end_idx = num_vf_points - 1;

		err = vgpu_get_gpu_volt_rail_limits(g, TEGRA_VGPU_GPU_VOLTAGE_TYPE_GPC,
				&gpc_vmin_uv, &gpc_vmax_uv);
		if (err) {
			nvgpu_err(g, "vgpu_get_gpu_volt_rail_limits failed: %d", err);
			goto cleanup_vf_status;

		}
#else
		nvgpu_err(g, "%s not supported", __func__);
		err = -EOPNOTSUPP;
		goto cleanup_vf_status;
#endif
	} else {
		if (nvgpu_is_powered_off(g)) {
			nvgpu_info(g, "nvgpu is powered off\n");
			ret = -EAGAIN;
			goto cleanup_vf_status;
		}

		err = gk20a_busy(g);
		if (err != 0) {
			ret = -EAGAIN;
			goto cleanup_vf_status;
		}

		/* Get GPC domain index */
		err = nvgpu_pmu_clk_domain_get_index_by_domain(g, CLKWHICH_GPCCLK,
			(u32 *)&gpc_domain_idx);
		if (err != 0) {
			len = snprintf(buf, 65536, "Error: Failed to get GPC domain index\n");
			goto cleanup_idle;
		}

		/* Get GPC voltage limits */
		err = nvgpu_volt_rail_get_limits(g, 0, &gpc_vmin_uv, &gpc_vmax_uv);
		if (err != 0) {
			len += snprintf(buf + len, 65536 - len, "Error reading GPC voltage limits\n");
			goto cleanup_idle;
		}

		/* Get GPC domain VF curve range */

		psuper_domain = &g->pmu->clk_pmu->clk_domainobjs_50->super.super;
		domain_obj = BOARDOBJGRP_OBJ_GET_BY_IDX(&(psuper_domain->super), gpc_domain_idx);
		if (domain_obj == NULL) {
			len = snprintf(buf, 65536, "Error: No clkDomain Obj for GPC index: %d\n",
				       gpc_domain_idx);
			goto cleanup_idle;
		}

		pDomain50ProgPhysical = (struct clk_domain_50_prog_physical *) domain_obj;
		railidx = clkDomainProgVoltRailIdxGet_50_PROG_PHYSICAL(g, (void *)pDomain50ProgPhysical);
		pVfPrimary = &pDomain50ProgPhysical->rail_vf_item[railidx].data.primary;
		pVfRel = CLK_CLK_VF_REL_GET(g->pmu->clk_pmu, pVfPrimary->clk_vf_rel_idx_first);
		pVfEntryPri = &pVfRel->vf_entry_pri;
		vf_start_idx = pVfEntryPri->vfPointIdxFirst;
		vf_end_idx = pVfEntryPri->vfPointIdxLast;

		/* Get VF points status using NVGPU structures */
		err = nvgpu_clk_vf_points_get_status(g, vf_points_status);
		if (err != 0) {
			len = snprintf(buf, 65536, "Error getting VF points status: %d\n", err);
			goto cleanup_idle;
		}
	}

	len = snprintf(buf, 65536, "GPC Allowed Rates:\n");
	len += snprintf(buf + len, 65536 - len,	"GPC Voltage Limits: vmin=%u uV, vmax=%u uV\n",
			gpc_vmin_uv, gpc_vmax_uv);
	len += snprintf(buf + len, 65536 - len, "VF Curve Range: %d-%d\n", vf_start_idx, vf_end_idx);
	len += snprintf(buf + len, 65536 - len, "Freq(MHz)\tVoltage(uV)\tVF_Index\n");

	/* Iterate only through GPC domain's VF curve range */
	for (i = vf_start_idx; i <= vf_end_idx && i < NVGPU_CLK_VF_POINTS_MAX_OBJECTS; i++) {
		struct nvgpu_clk_vf_point_status *vf_point = &vf_points_status->vf_points[i];

		if (len >= 65536 - 128) {
			len += snprintf(buf + len, 65536 - len, "... (truncated)\n");
			break;
		}

		/* Filter VF points within GPC voltage limits and reasonable frequency range */
		if (vf_point->voltage_uv >= gpc_vmin_uv && vf_point->voltage_uv <= gpc_vmax_uv &&
		    vf_point->freq_mhz > 0 && vf_point->freq_mhz < 10000) {
			len += snprintf(buf + len, 65536 - len, "%u\t\t%u\t\t%u\n",
				vf_point->freq_mhz, vf_point->voltage_uv, i);
		}
	}

cleanup_idle:
	if (!g->is_virtual)
		gk20a_idle(g);
cleanup_vf_status:
	nvgpu_kfree(g, vf_points_status);
cleanup_buf:
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, len);
	nvgpu_kfree(g, buf);
	return ret;
}

static ssize_t gpu_sys_allowed_rates_read(struct file *file, char __user *user_buf,
					   size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	char *buf = NULL;
	int len = 0;
	int err = 0;
	struct nvgpu_clk_vf_points_status *vf_points_status = NULL;
	u32 i = 0;
	ssize_t ret = 0;
	u32 gpm_vmin_uv = 0, gpm_vmax_uv = 0;
	u16 sys_domain_idx = 0;
	struct boardobjgrp_e32 *psuper_domain = NULL;
	struct pmu_board_obj *domain_obj = NULL;
	struct clk_domain_50_prog_physical *pDomain50ProgPhysical = NULL;
	struct clk_domain_50_physical_rail_vf_primary *pVfPrimary = NULL;
	struct nv_pmu_clk_clk_vf_rel_vf_entry_pri *pVfEntryPri = NULL;
	struct clk_vf_rel *pVfRel = NULL;
	u8 railidx = 0;
	u16 vf_start_idx = 0, vf_end_idx = 0;

	if (!g)
		return -EINVAL;

	buf = nvgpu_kzalloc(g, 65536);
	if (!buf) {
		return -ENOMEM;
	}

	/* Allocate memory for VF points status */
	vf_points_status = nvgpu_kzalloc(g, sizeof(*vf_points_status));
	if (!vf_points_status) {
		ret = -ENOMEM;
		goto cleanup_buf;
	}

	if (g->is_virtual) {
#ifdef CONFIG_NVGPU_GR_VIRTUALIZATION
		u32 num_vf_points = NVGPU_CLK_VF_POINTS_MAX_OBJECTS;
		/* Use IVC call for virtualized environment */
		err = vgpu_get_vf_points_status(g, vf_points_status, &num_vf_points, TEGRA_VGPU_CLK_SYS);
		if (err) {
			nvgpu_err(g, "vgpu_get_vf_points_status failed: %d", err);
			goto cleanup_vf_status;
		}
		vf_start_idx = 0;
		vf_end_idx = num_vf_points - 1;

		err = vgpu_get_gpu_volt_rail_limits(g, TEGRA_VGPU_GPU_VOLTAGE_TYPE_GPM,
				&gpm_vmin_uv, &gpm_vmax_uv);
		if (err) {
			nvgpu_err(g, "vgpu_get_gpu_volt_rail_limits failed: %d", err);
			goto cleanup_vf_status;

		}
#else
		nvgpu_err(g, "%s not supported", __func__);
		err = -EOPNOTSUPP;
		goto cleanup_vf_status;
#endif
	} else {
		if (nvgpu_is_powered_off(g)) {
			nvgpu_info(g, "nvgpu is powered off\n");
			ret = -EAGAIN;
			goto cleanup_vf_status;
		}

		err = gk20a_busy(g);
		if (err != 0) {
			ret = -EAGAIN;
			goto cleanup_vf_status;
		}

		/* Get SYS/XBAR domain index */
		err = nvgpu_pmu_clk_domain_get_index_by_domain(g, CLKWHICH_XBARCLK,
			(u32 *)&sys_domain_idx);
		if (err != 0) {
			len = snprintf(buf, 65536, "Error: Failed to get SYS domain index\n");
			goto cleanup_idle;
		}

		/* Get GPM voltage limits */
		err = nvgpu_volt_rail_get_limits(g, 1, &gpm_vmin_uv, &gpm_vmax_uv);
		if (err != 0) {
			len += snprintf(buf + len, 65536 - len, "Error reading GPM voltage limits\n");
			goto cleanup_idle;
		}

		/* Get SYS domain VF curve range */

		psuper_domain = &g->pmu->clk_pmu->clk_domainobjs_50->super.super;
		domain_obj = BOARDOBJGRP_OBJ_GET_BY_IDX(&(psuper_domain->super), sys_domain_idx);
		if (domain_obj == NULL) {
			len = snprintf(buf, 65536, "Error: No clkDomain Obj for SYS index: %d\n",
				       sys_domain_idx);
			goto cleanup_idle;
		}

		pDomain50ProgPhysical = (struct clk_domain_50_prog_physical *) domain_obj;
		railidx = clkDomainProgVoltRailIdxGet_50_PROG_PHYSICAL(g, (void *)pDomain50ProgPhysical);
		pVfPrimary = &pDomain50ProgPhysical->rail_vf_item[railidx].data.primary;
		pVfRel = CLK_CLK_VF_REL_GET(g->pmu->clk_pmu, pVfPrimary->clk_vf_rel_idx_first);
		pVfEntryPri = &pVfRel->vf_entry_pri;
		vf_start_idx = pVfEntryPri->vfPointIdxFirst;
		vf_end_idx = pVfEntryPri->vfPointIdxLast;

		/* Get VF points status using NVGPU structures */
		err = nvgpu_clk_vf_points_get_status(g, vf_points_status);
		if (err != 0) {
			len = snprintf(buf, 65536, "Error getting VF points status: %d\n", err);
			goto cleanup_idle;
		}
	}

	len = snprintf(buf, 65536, "SYS Allowed Rates:\n");
	len += snprintf(buf + len, 65536 - len, "GPM Voltage Limits: vmin=%u uV, vmax=%u uV\n",
			gpm_vmin_uv, gpm_vmax_uv);
	len += snprintf(buf + len, 65536 - len, "VF Curve Range: %d-%d\n", vf_start_idx, vf_end_idx);
	len += snprintf(buf + len, 65536 - len, "Freq(MHz)\tVoltage(uV)\tVF_Index\n");

	/* Iterate only through SYS domain's VF curve range */
	for (i = vf_start_idx; i <= vf_end_idx && i < NVGPU_CLK_VF_POINTS_MAX_OBJECTS; i++) {
		struct nvgpu_clk_vf_point_status *vf_point = &vf_points_status->vf_points[i];

		if (len >= 65536 - 128) {
			len += snprintf(buf + len, 65536 - len, "... (truncated)\n");
			break;
		}

		/* Filter VF points within GPM voltage limits and reasonable frequency range */
		if (vf_point->voltage_uv >= gpm_vmin_uv && vf_point->voltage_uv <= gpm_vmax_uv &&
		    vf_point->freq_mhz > 0 && vf_point->freq_mhz < 10000) {
			len += snprintf(buf + len, 65536 - len, "%u\t\t%u\t\t%u\n",
				vf_point->freq_mhz, vf_point->voltage_uv, i);
		}
	}

cleanup_idle:
	if (!g->is_virtual)
		gk20a_idle(g);
cleanup_vf_status:
	nvgpu_kfree(g, vf_points_status);
cleanup_buf:
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, len);
	nvgpu_kfree(g, buf);
	return ret;
}

static ssize_t gpu_nvd_allowed_rates_read(struct file *file, char __user *user_buf,
					   size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	char *buf = NULL;
	int len = 0;
	int err = 0;
	struct nvgpu_clk_vf_points_status *vf_points_status = NULL;
	u32 i = 0;
	ssize_t ret = 0;
	u32 gpm_vmin_uv = 0, gpm_vmax_uv = 0;
	u16 nvd_domain_idx = 0;
	struct boardobjgrp_e32 *psuper_domain = 0;
	struct pmu_board_obj *domain_obj = 0;
	struct clk_domain_50_prog_physical *pDomain50ProgPhysical = NULL;
	struct clk_domain_50_physical_rail_vf_primary *pVfPrimary = NULL;
	struct nv_pmu_clk_clk_vf_rel_vf_entry_pri *pVfEntryPri = NULL;
	struct clk_vf_rel *pVfRel = NULL;
	u8 railidx = 0;
	u16 vf_start_idx = 0, vf_end_idx = 0;

	if (!g)
		return -EINVAL;

	buf = nvgpu_kzalloc(g, 65536);
	if (!buf) {
		return -ENOMEM;
	}

	/* Allocate memory for VF points status */
	vf_points_status = nvgpu_kzalloc(g, sizeof(*vf_points_status));
	if (!vf_points_status) {
		ret = -ENOMEM;
		goto cleanup_buf;
	}

	if (g->is_virtual) {
#ifdef CONFIG_NVGPU_GR_VIRTUALIZATION
		u32 num_vf_points = NVGPU_CLK_VF_POINTS_MAX_OBJECTS;
		/* Use IVC call for virtualized environment */
		err = vgpu_get_vf_points_status(g, vf_points_status, &num_vf_points, TEGRA_VGPU_CLK_NVD);
		if (err) {
			nvgpu_err(g, "vgpu_get_vf_points_status failed: %d", err);
			goto cleanup_vf_status;
		}
		vf_start_idx = 0;
		vf_end_idx = num_vf_points - 1;

		err = vgpu_get_gpu_volt_rail_limits(g, TEGRA_VGPU_GPU_VOLTAGE_TYPE_GPM,
				&gpm_vmin_uv, &gpm_vmax_uv);
		if (err) {
			nvgpu_err(g, "vgpu_get_gpu_volt_rail_limits failed: %d", err);
			goto cleanup_vf_status;

		}
#else
		nvgpu_err(g, "%s not supported", __func__);
		err = -EOPNOTSUPP;
		goto cleanup_vf_status;
#endif
	} else {
		if (nvgpu_is_powered_off(g)) {
			nvgpu_info(g, "nvgpu is powered off\n");
			ret = -EAGAIN;
			goto cleanup_vf_status;
		}

		err = gk20a_busy(g);
		if (err != 0) {
			ret = -EAGAIN;
			goto cleanup_vf_status;
		}

		/* Get NVD domain index */
		err = nvgpu_pmu_clk_domain_get_index_by_domain(g, CLKWHICH_NVDCLK,
			(u32 *)&nvd_domain_idx);
		if (err != 0) {
			len = snprintf(buf, 65536, "Error: Failed to get NVD domain index\n");
			goto cleanup_idle;
		}

		/* Get GPM voltage limits */
		err = nvgpu_volt_rail_get_limits(g, 1, &gpm_vmin_uv, &gpm_vmax_uv);
		if (err != 0) {
			len += snprintf(buf + len, 65536 - len, "Error reading GPM voltage limits\n");
			goto cleanup_idle;
		}

		/* Get NVD domain VF curve range */

		psuper_domain = &g->pmu->clk_pmu->clk_domainobjs_50->super.super;
		domain_obj = BOARDOBJGRP_OBJ_GET_BY_IDX(&(psuper_domain->super), nvd_domain_idx);
		if (domain_obj == NULL) {
			len = snprintf(buf, 65536, "Error: No clkDomain Obj for NVD index: %d\n",
				       nvd_domain_idx);
			goto cleanup_idle;
		}

		pDomain50ProgPhysical = (struct clk_domain_50_prog_physical *) domain_obj;
		railidx = clkDomainProgVoltRailIdxGet_50_PROG_PHYSICAL(g, (void *)pDomain50ProgPhysical);
		pVfPrimary = &pDomain50ProgPhysical->rail_vf_item[railidx].data.primary;
		pVfRel = CLK_CLK_VF_REL_GET(g->pmu->clk_pmu, pVfPrimary->clk_vf_rel_idx_first);
		pVfEntryPri = &pVfRel->vf_entry_pri;
		vf_start_idx = pVfEntryPri->vfPointIdxFirst;
		vf_end_idx = pVfEntryPri->vfPointIdxLast;

		/* Get VF points status using NVGPU structures */
		err = nvgpu_clk_vf_points_get_status(g, vf_points_status);
		if (err != 0) {
			len = snprintf(buf, 65536, "Error getting VF points status: %d\n", err);
			goto cleanup_idle;
		}
	}

	len = snprintf(buf, 65536, "NVD Allowed Rates:\n");
	len += snprintf(buf + len, 65536 - len, "GPM Voltage Limits: vmin=%u uV, vmax=%u uV\n",
			gpm_vmin_uv, gpm_vmax_uv);
	len += snprintf(buf + len, 65536 - len, "VF Curve Range: %d-%d\n", vf_start_idx, vf_end_idx);
	len += snprintf(buf + len, 65536 - len, "Freq(MHz)\tVoltage(uV)\tVF_Index\n");

	/* Iterate only through NVD domain's VF curve range */
	for (i = vf_start_idx; i <= vf_end_idx && i < NVGPU_CLK_VF_POINTS_MAX_OBJECTS; i++) {
		struct nvgpu_clk_vf_point_status *vf_point = &vf_points_status->vf_points[i];

		if (len >= 65536 - 128) {
			len += snprintf(buf + len, 65536 - len, "... (truncated)\n");
			break;
		}

		/* Filter VF points within GPM voltage limits and reasonable frequency range */
		if (vf_point->voltage_uv >= gpm_vmin_uv && vf_point->voltage_uv <= gpm_vmax_uv &&
		    vf_point->freq_mhz > 0 && vf_point->freq_mhz < 10000) {
			len += snprintf(buf + len, 65536 - len, "%u\t\t%u\t\t%u\n",
				vf_point->freq_mhz, vf_point->voltage_uv, i);
		}
	}

cleanup_idle:
	if (!g->is_virtual)
		gk20a_idle(g);
cleanup_vf_status:
	nvgpu_kfree(g, vf_points_status);
cleanup_buf:
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, len);
	nvgpu_kfree(g, buf);
	return ret;
}

static ssize_t gpu_uproc_allowed_rates_read(struct file *file, char __user *user_buf,
					     size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	char *buf = NULL;
	int len = 0;
	int err = 0;
	struct nvgpu_clk_vf_points_status *vf_points_status  = NULL;
	u32 i = 0;
	ssize_t ret = 0;
	u32 gpm_vmin_uv = 0, gpm_vmax_uv = 0;
	u16 uproc_domain_idx = 0;
	struct boardobjgrp_e32 *psuper_domain = NULL;
	struct pmu_board_obj *domain_obj = NULL;
	struct clk_domain_50_prog_physical *pDomain50ProgPhysical = NULL;
	struct clk_domain_50_physical_rail_vf_primary *pVfPrimary = NULL;
	struct nv_pmu_clk_clk_vf_rel_vf_entry_pri *pVfEntryPri = NULL;
	struct clk_vf_rel *pVfRel = NULL;
	u8 railidx = 0;
	u16 vf_start_idx = 0, vf_end_idx = 0;

	if (!g)
		return -EINVAL;

	buf = nvgpu_kzalloc(g, 65536);
	if (!buf) {
		return -ENOMEM;
	}

	/* Allocate memory for VF points status */
	vf_points_status = nvgpu_kzalloc(g, sizeof(*vf_points_status));
	if (!vf_points_status) {
		ret = -ENOMEM;
		goto cleanup_buf;
	}

	if (g->is_virtual) {
#ifdef CONFIG_NVGPU_GR_VIRTUALIZATION
		u32 num_vf_points = NVGPU_CLK_VF_POINTS_MAX_OBJECTS;
		/* Use IVC call for virtualized environment */
		err = vgpu_get_vf_points_status(g,
			vf_points_status, &num_vf_points, TEGRA_VGPU_CLK_UPROC);
		if (err) {
			nvgpu_err(g, "vgpu_get_vf_points_status failed: %d", err);
			goto cleanup_vf_status;
		}
		vf_start_idx = 0;
		vf_end_idx = num_vf_points - 1;

		err = vgpu_get_gpu_volt_rail_limits(g, TEGRA_VGPU_GPU_VOLTAGE_TYPE_GPM,
				&gpm_vmin_uv, &gpm_vmax_uv);
		if (err) {
			nvgpu_err(g, "vgpu_get_gpu_volt_rail_limits failed: %d", err);
			goto cleanup_vf_status;

		}
#else
		nvgpu_err(g, "%s not supported", __func__);
		err = -EOPNOTSUPP;
		goto cleanup_vf_status;
#endif
	} else {
		if (nvgpu_is_powered_off(g)) {
			nvgpu_info(g, "nvgpu is powered off\n");
			ret = -EAGAIN;
			goto cleanup_vf_status;
		}

		err = gk20a_busy(g);
		if (err != 0) {
			ret = -EAGAIN;
			goto cleanup_vf_status;
		}

		/* Get UPROC domain index */
		err = nvgpu_pmu_clk_domain_get_index_by_domain(g, CLKWHICH_UPROCCLK,
			(u32 *)&uproc_domain_idx);
		if (err != 0) {
			len = snprintf(buf, 65536, "Error: Failed to get UPROC domain index\n");
			goto cleanup_idle;
		}

		/* Get GPM voltage limits */
		err = nvgpu_volt_rail_get_limits(g, 1, &gpm_vmin_uv, &gpm_vmax_uv);
		if (err != 0) {
			len += snprintf(buf + len, 65536 - len, "Error reading GPM voltage limits\n");
			goto cleanup_idle;
		}

		/* Get UPROC domain VF curve range */

		psuper_domain = &g->pmu->clk_pmu->clk_domainobjs_50->super.super;
		domain_obj = BOARDOBJGRP_OBJ_GET_BY_IDX(&(psuper_domain->super), uproc_domain_idx);
		if (domain_obj == NULL) {
			len = snprintf(buf, 65536, "Error: No clkDomain Obj for UPROC index: %d\n",
			       uproc_domain_idx);
			goto cleanup_idle;
		}

		pDomain50ProgPhysical = (struct clk_domain_50_prog_physical *) domain_obj;
		railidx = clkDomainProgVoltRailIdxGet_50_PROG_PHYSICAL(g, (void *)pDomain50ProgPhysical);
		pVfPrimary = &pDomain50ProgPhysical->rail_vf_item[railidx].data.primary;
		pVfRel = CLK_CLK_VF_REL_GET(g->pmu->clk_pmu, pVfPrimary->clk_vf_rel_idx_first);
		pVfEntryPri = &pVfRel->vf_entry_pri;
		vf_start_idx = pVfEntryPri->vfPointIdxFirst;
		vf_end_idx = pVfEntryPri->vfPointIdxLast;

		/* Get VF points status using NVGPU structures */
		err = nvgpu_clk_vf_points_get_status(g, vf_points_status);
		if (err != 0) {
			len = snprintf(buf, 65536, "Error getting VF points status: %d\n", err);
			goto cleanup_idle;
		}
	}

	len = snprintf(buf, 65536, "UPROC Allowed Rates:\n");
	len += snprintf(buf + len, 65536 - len, "GPM Voltage Limits: vmin=%u uV, vmax=%u uV\n",
			gpm_vmin_uv, gpm_vmax_uv);
	len += snprintf(buf + len, 65536 - len, "VF Curve Range: %d-%d\n", vf_start_idx, vf_end_idx);
	len += snprintf(buf + len, 65536 - len, "Freq(MHz)\tVoltage(uV)\tVF_Index\n");

	/* Iterate only through UPROC domain's VF curve range */
	for (i = vf_start_idx; i <= vf_end_idx && i < NVGPU_CLK_VF_POINTS_MAX_OBJECTS; i++) {
		struct nvgpu_clk_vf_point_status *vf_point = &vf_points_status->vf_points[i];

		if (len >= 65536 - 128) {
			len += snprintf(buf + len, 65536 - len, "... (truncated)\n");
			break;
		}

		/* Filter VF points within GPM voltage limits and reasonable frequency range */
		if (vf_point->voltage_uv >= gpm_vmin_uv && vf_point->voltage_uv <= gpm_vmax_uv &&
		    vf_point->freq_mhz > 0 && vf_point->freq_mhz < 10000) {
			len += snprintf(buf + len, 65536 - len, "%u\t\t%u\t\t%u\n",
				vf_point->freq_mhz, vf_point->voltage_uv, i);
		}
	}

cleanup_idle:
	if (!g->is_virtual)
		gk20a_idle(g);
cleanup_vf_status:
	nvgpu_kfree(g, vf_points_status);
cleanup_buf:
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, len);
	nvgpu_kfree(g, buf);
	return ret;
}

/* File Operations */
static const struct file_operations gpu_gpc_rate_fops = {
	.open = simple_open,
	.read = gpu_gpc_rate_read,
	.write = gpu_gpc_rate_write,
	.llseek = default_llseek,
};

static const struct file_operations gpu_sys_rate_fops = {
	.open = simple_open,
	.read = gpu_sys_rate_read,
	.write = gpu_sys_rate_write,
	.llseek = default_llseek,
};

static const struct file_operations gpu_nvd_rate_fops = {
	.open = simple_open,
	.read = gpu_nvd_rate_read,
	.write = gpu_nvd_rate_write,
	.llseek = default_llseek,
};

static const struct file_operations gpu_uproc_rate_fops = {
	.open = simple_open,
	.read = gpu_uproc_rate_read,
	.write = gpu_uproc_rate_write,
	.llseek = default_llseek,
};

static const struct file_operations gpu_gpc_allowed_rates_fops = {
	.open = simple_open,
	.read = gpu_gpc_allowed_rates_read,
	.llseek = default_llseek,
};

static const struct file_operations gpu_sys_allowed_rates_fops = {
	.open = simple_open,
	.read = gpu_sys_allowed_rates_read,
	.llseek = default_llseek,
};

static const struct file_operations gpu_nvd_allowed_rates_fops = {
	.open = simple_open,
	.read = gpu_nvd_allowed_rates_read,
	.llseek = default_llseek,
};

static const struct file_operations gpu_uproc_allowed_rates_fops = {
	.open = simple_open,
	.read = gpu_uproc_allowed_rates_read,
	.llseek = default_llseek,
};

/* Drive and Sentry frequency operations */
static const struct file_operations gpu_gpc_drive_freq_mhz_fops = {
	.open = simple_open,
	.read = gpu_gpc_drive_freq_mhz_read,
};

static const struct file_operations gpu_gpc_sentry_freq_mhz_fops = {
	.open = simple_open,
	.read = gpu_gpc_sentry_freq_mhz_read,
};

/* VF Table File Operations */
static const struct file_operations vf_table_fops = {
	.open = simple_open,
	.read = vf_table_read,
};

/* Voltage File Operations */
static const struct file_operations vdd_gpc_voltage_fops = {
	.open = simple_open,
	.read = vdd_gpc_voltage_read,
	.write = vdd_gpc_voltage_write,
};

static const struct file_operations vdd_gpm_voltage_fops = {
	.open = simple_open,
	.read = vdd_gpm_voltage_read,
	.write = vdd_gpm_voltage_write,
};

static const struct file_operations vdd_gpc_rail_limit_offset_vmin_fops = {
	.open = simple_open,
	.read = vdd_gpc_rail_limit_offset_vmin_read,
	.write = vdd_gpc_rail_limit_offset_vmin_write,
};

static const struct file_operations vdd_gpc_rail_limit_offset_vmax_fops = {
	.open = simple_open,
	.read = vdd_gpc_rail_limit_offset_vmax_read,
	.write = vdd_gpc_rail_limit_offset_vmax_write,
};

static const struct file_operations vdd_gpm_rail_limit_offset_vmin_fops = {
	.open = simple_open,
	.read = vdd_gpm_rail_limit_offset_vmin_read,
	.write = vdd_gpm_rail_limit_offset_vmin_write,
};

static const struct file_operations vdd_gpm_rail_limit_offset_vmax_fops = {
	.open = simple_open,
	.read = vdd_gpm_rail_limit_offset_vmax_read,
	.write = vdd_gpm_rail_limit_offset_vmax_write,
};

static const struct file_operations gpu_gpc_freq_offset_khz_fops = {
	.open = simple_open,
	.read = gpu_gpc_freq_offset_khz_read,
	.write = gpu_gpc_freq_offset_khz_write,
};

static const struct file_operations gpu_nvd_freq_offset_khz_fops = {
	.open = simple_open,
	.read = gpu_nvd_freq_offset_khz_read,
	.write = gpu_nvd_freq_offset_khz_write,
};

static const struct file_operations gpu_sys_freq_offset_khz_fops = {
	.open = simple_open,
	.read = gpu_sys_freq_offset_khz_read,
	.write = gpu_sys_freq_offset_khz_write,
};

static const struct file_operations gpu_uproc_freq_offset_khz_fops = {
	.open = simple_open,
	.read = gpu_uproc_freq_offset_khz_read,
	.write = gpu_uproc_freq_offset_khz_write,
};

/* Allowed Voltages Functions */
static ssize_t vdd_gpc_allowed_voltages_read(struct file *file, char __user *user_buf,
					      size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char *buf;
	int len = 0;
	ssize_t ret;
	u32 i;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (!pm_data) {
		return -ENODEV;
	}

	buf = nvgpu_kzalloc(g, 1024);
	if (!buf) {
		return -ENOMEM;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	for (i = 0; i < pm_data->vdd_gpc_num_allowed_voltages; i++) {
		len += snprintf(buf + len, 1024 - len, "%u\n",
			pm_data->vdd_gpc_allowed_voltages[i]);
	}

	nvgpu_mutex_release(&pm_data->sysfs_lock);

	ret = simple_read_from_buffer(user_buf, count, ppos, buf, len);
	nvgpu_kfree(g, buf);

	return ret;
}

static ssize_t vdd_gpm_allowed_voltages_read(struct file *file, char __user *user_buf,
					      size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char *buf;
	int len = 0;
	ssize_t ret;
	u32 i;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (!pm_data) {
		return -ENODEV;
	}

	buf = nvgpu_kzalloc(g, 1024);
	if (!buf) {
		return -ENOMEM;
	}

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	for (i = 0; i < pm_data->vdd_gpm_num_allowed_voltages; i++) {
		len += snprintf(buf + len, 1024 - len, "%u\n",
			pm_data->vdd_gpm_allowed_voltages[i]);
	}

	nvgpu_mutex_release(&pm_data->sysfs_lock);

	ret = simple_read_from_buffer(user_buf, count, ppos, buf, len);
	nvgpu_kfree(g, buf);

	return ret;
}

/* Allowed Voltages File Operations */
static const struct file_operations vdd_gpc_allowed_voltages_fops = {
	.open = simple_open,
	.read = vdd_gpc_allowed_voltages_read,
	.llseek = default_llseek,
};

static const struct file_operations vdd_gpm_allowed_voltages_fops = {
	.open = simple_open,
	.read = vdd_gpm_allowed_voltages_read,
	.llseek = default_llseek,
};
static ssize_t voltage_limits_read(struct file *file, char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	char buf[1024];
	u32 vmin_uv = 0, vmax_uv = 0;
	int len = 0;
	int err;
	struct boardobjgrp *pboardobjgrp;
	struct pmu_board_obj *obj = NULL;
	u16 index;
	u32 rail_version = 0;
	void *pmu_status_buf = NULL;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;


	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;
	/* Check if volt subsystem is initialized */
#ifdef CONFIG_NVGPU_LS_PMU
	if (!g->pmu || !g->pmu->volt || !g->pmu->volt->volt_metadata) {
		len += snprintf(buf + len, sizeof(buf) - len,
				"Error: PMU volt subsystem not initialized\n");
		goto done;
	}
#else
			len += snprintf(buf + len, sizeof(buf) - len,
			"Error: PMU LS not enabled in build\n");
	goto done;
#endif

	/* Call status update to refresh data from PMU FIRST */
	err = nvgpu_volt_rail_get_status(g, &pmu_status_buf);

	/* Call nvgpu_pmu_volt_get_vmin_vmax_ps35 to process voltage data */
	err = nvgpu_pmu_volt_get_vmin_vmax_ps35(g, &vmin_uv, &vmax_uv);
	if (err != 0) {
		len += snprintf(buf + len, sizeof(buf) - len, "Error reading voltage data\n");
		goto done;
	}

	rail_version = g->pmu->volt->volt_metadata->volt_rail_metadata.version;
	pboardobjgrp = &g->pmu->volt->volt_metadata->volt_rail_metadata.volt_rails.super;

	/* Display only essential rail information */
	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj*, obj, index) {
		if (rail_version == 0x10U) { /* CTRL_VOLT_RAIL_VERSION_10 */
			struct voltage_rail *volt_rail = (struct voltage_rail *)(void *)obj;

			switch (index) {
			case 0:
		len += snprintf(buf + len, sizeof(buf) - len,
						"GPC rail:\nvmin=%u, vmax=%u\n",
						volt_rail->vmin_limitu_v, volt_rail->max_limitu_v);
				break;
			case 1:
		len += snprintf(buf + len, sizeof(buf) - len,
						"GPM rail:\nvmin=%u, vmax=%u\n",
						volt_rail->vmin_limitu_v, volt_rail->max_limitu_v);
				break;
			}
		} else if (rail_version == 0x30U) { /* CTRL_VOLT_RAIL_VERSION_30 */
			struct voltage_rail_3x *volt_rail_3x;


			volt_rail_3x = (struct voltage_rail_3x *)(void *)obj;
			switch (index) {
			case 0:
		len += snprintf(buf + len, sizeof(buf) - len,
						"GPC rail:\nvmin=%u, vmax=%u\n",
						volt_rail_3x->vmin_limitu_v, volt_rail_3x->max_limitu_v);
				break;
			case 1:
		len += snprintf(buf + len, sizeof(buf) - len,
						"GPM rail:\nvmin=%u, vmax=%u\n",
						volt_rail_3x->vmin_limitu_v, volt_rail_3x->max_limitu_v);
				break;
			}
		}
	}

done:
	gk20a_idle(g);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations voltage_limits_fops = {
	.open = simple_open,
	.read = voltage_limits_read,
	.llseek = default_llseek,
};

static const struct file_operations gpc_voltage_limits_fops = {
	.open = simple_open,
	.read = gpc_voltage_limits_read,
	.llseek = default_llseek,
};

static const struct file_operations gpm_voltage_limits_fops = {
	.open = simple_open,
	.read = gpm_voltage_limits_read,
	.llseek = default_llseek,
};

/* PTO Counter context for each clock domain */
struct gpu_pto_context {
	struct nvgpu_ref refcount;
	struct nvgpu_cond cond;
	struct nvgpu_mutex mutex;
	u32 pto_counter;
};

/* Helper function to check if rate write is allowed based on current mode */
static bool is_rate_write_allowed(struct gk20a *g, u32 clk_domain)
{
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;

	if (!pm_data)
		return false;

	/* In manual mode, all clock domains are writable */
	if (pm_data->clock_mode == CLOCK_MODE_MANUAL)
		return true;

	/* In auto mode, only GPC is writable */
	if (pm_data->clock_mode == CLOCK_MODE_AUTO)
		return (clk_domain == CTRL_CLK_DOMAIN_GPCCLK);

	return false;
}

/* Clock mode and regime control functions */
static ssize_t clock_mode_read(struct file *file, char __user *user_buf,
			      size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	int len;


	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (!pm_data)
		return -ENODEV;

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);
	len = snprintf(buf, sizeof(buf), "%u\n", pm_data->clock_mode);
	nvgpu_mutex_release(&pm_data->sysfs_lock);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t clock_mode_write(struct file *file, const char __user *user_buf,
			       size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	u32 mode;
	int ret;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (!pm_data)
		return -ENODEV;

	if (count >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;
	buf[count] = '\0';

	ret = kstrtou32(buf, 0, &mode);
	if (ret)
		return ret;

	if (mode != CLOCK_MODE_AUTO && mode != CLOCK_MODE_MANUAL)
		return -EINVAL;

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);
	pm_data->clock_mode = mode;
	nvgpu_mutex_release(&pm_data->sysfs_lock);

	nvgpu_info(g, "Clock mode set to %s",
		  (mode == CLOCK_MODE_AUTO) ? "AUTO" : "MANUAL");

	return count;
}

/* Per-domain regime control functions */
static ssize_t gpc_regime_read(struct file *file, char __user *user_buf,
			      size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = NULL;
	char buf[32] = {};
	int len = 0;

	if (!g)
		return -EINVAL;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	pm_data = g->power_mgmt_data;
	if (!pm_data)
		return -ENODEV;

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);
	len = snprintf(buf, sizeof(buf), "0x%02x\n", pm_data->gpc_regime);
	nvgpu_mutex_release(&pm_data->sysfs_lock);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t gpc_regime_write(struct file *file, const char __user *user_buf,
			       size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	u32 regime;
	int ret;


	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (!pm_data)
		return -ENODEV;

	if (count >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;
	buf[count] = '\0';

	ret = kstrtou32(buf, 0, &regime);
	if (ret)
		return ret;

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	ret = nvgpu_pmu_clk_domains_fll_set_regime_50(g, regime, NV2080_CTRL_CLK_DOMAIN_LEGACY_GPCCLK);
	if (ret == 0) {
		pm_data->gpc_regime = regime;

		/* Automatically switch to manual mode when FR, FFR, or VR regimes are set */
		if (regime == CTRL_CLK_FLL_REGIME_ID_FR ||
		    regime == CTRL_CLK_FLL_REGIME_ID_FFR ||
		    regime == CTRL_CLK_FLL_REGIME_ID_VR) {
			if (pm_data->clock_mode != CLOCK_MODE_MANUAL) {
				pm_data->clock_mode = CLOCK_MODE_MANUAL;
				nvgpu_info(g, "Switched to MANUAL mode for regime 0x%02x", regime);
			}
		}
		nvgpu_info(g, "GPC clock regime set to 0x%02x", regime);
	} else {
		nvgpu_err(g, "Failed to set GPC domain regime");
		count = ret;
	}
	nvgpu_mutex_release(&pm_data->sysfs_lock);

	return count;
}

static ssize_t sys_regime_read(struct file *file, char __user *user_buf,
			      size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = NULL;
	char buf[32] = {};
	int len = 0;

	if (!g)
		return -EINVAL;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	pm_data = g->power_mgmt_data;
	if (!pm_data)
		return -ENODEV;

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);
	len = snprintf(buf, sizeof(buf), "0x%02x\n", pm_data->sys_regime);
	nvgpu_mutex_release(&pm_data->sysfs_lock);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t sys_regime_write(struct file *file, const char __user *user_buf,
			       size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	u32 regime;
	int ret;


	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (!pm_data)
		return -ENODEV;

	if (count >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;
	buf[count] = '\0';

	ret = kstrtou32(buf, 0, &regime);
	if (ret)
		return ret;

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	ret = nvgpu_pmu_clk_domains_fll_set_regime_50(g, regime, NV2080_CTRL_CLK_DOMAIN_LEGACY_XBARCLK);
	if (ret == 0) {
		pm_data->sys_regime = regime;
		nvgpu_info(g, "SYS clock regime set to 0x%02x", regime);
		if (regime == CTRL_CLK_FLL_REGIME_ID_FR ||
		    regime == CTRL_CLK_FLL_REGIME_ID_FFR ||
		    regime == CTRL_CLK_FLL_REGIME_ID_VR) {
			if (pm_data->clock_mode != CLOCK_MODE_MANUAL) {
				pm_data->clock_mode = CLOCK_MODE_MANUAL;
				nvgpu_info(g, "Switched to MANUAL mode for regime 0x%02x", regime);
			}
		}
	} else {
		nvgpu_err(g, "Failed to set SYS domain regime");
		count = ret;
	}
	nvgpu_mutex_release(&pm_data->sysfs_lock);

	return count;
}

static ssize_t nvd_regime_read(struct file *file, char __user *user_buf,
			      size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = NULL;
	char buf[32] = {};
	int len = 0;

	if (!g)
		return -EINVAL;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	pm_data = g->power_mgmt_data;
	if (!pm_data)
		return -ENODEV;

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);
	len = snprintf(buf, sizeof(buf), "0x%02x\n", pm_data->nvd_regime);
	nvgpu_mutex_release(&pm_data->sysfs_lock);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t nvd_regime_write(struct file *file, const char __user *user_buf,
			       size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	u32 regime;
	int ret;


	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (!pm_data)
		return -ENODEV;

	if (count >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;
	buf[count] = '\0';

	ret = kstrtou32(buf, 0, &regime);
	if (ret)
		return ret;

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	ret = nvgpu_pmu_clk_domains_fll_set_regime_50(g, regime, NV2080_CTRL_CLK_DOMAIN_LEGACY_NVDCLK);
	if (ret == 0) {
		pm_data->nvd_regime = regime;
		nvgpu_info(g, "NVD clock regime set to 0x%02x", regime);
		if (regime == CTRL_CLK_FLL_REGIME_ID_FR ||
		    regime == CTRL_CLK_FLL_REGIME_ID_FFR ||
		    regime == CTRL_CLK_FLL_REGIME_ID_VR) {
			if (pm_data->clock_mode != CLOCK_MODE_MANUAL) {
				pm_data->clock_mode = CLOCK_MODE_MANUAL;
				nvgpu_info(g, "Switched to MANUAL mode for regime 0x%02x", regime);
			}
		}
	} else {
		nvgpu_err(g, "Failed to set NVD domain regime");
		count = ret;
	}
	nvgpu_mutex_release(&pm_data->sysfs_lock);

	return count;
}

static ssize_t uproc_regime_read(struct file *file, char __user *user_buf,
				size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = NULL;
	char buf[32] = {};
	int len = 0;

	if (!g)
		return -EINVAL;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	pm_data = g->power_mgmt_data;
	if (!pm_data)
		return -ENODEV;

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);
	len = snprintf(buf, sizeof(buf), "0x%02x\n", pm_data->uproc_regime);
	nvgpu_mutex_release(&pm_data->sysfs_lock);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t uproc_regime_write(struct file *file, const char __user *user_buf,
				 size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	struct gpu_power_mgmt_data *pm_data = g->power_mgmt_data;
	char buf[32];
	u32 regime;
	int ret;


	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (!pm_data)
		return -ENODEV;

	if (count >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(buf, user_buf, count))
		return -EFAULT;
	buf[count] = '\0';

	ret = kstrtou32(buf, 0, &regime);
	if (ret)
		return ret;

	nvgpu_mutex_acquire(&pm_data->sysfs_lock);

	ret = nvgpu_pmu_clk_domains_fll_set_regime_50(g, regime, NV2080_CTRL_CLK_DOMAIN_LEGACY_UPROCCLK);
	if (ret == 0) {
		pm_data->uproc_regime = regime;
		nvgpu_info(g, "UPROC clock regime set to 0x%02x", regime);
		if (regime == CTRL_CLK_FLL_REGIME_ID_FR ||
		    regime == CTRL_CLK_FLL_REGIME_ID_FFR ||
		    regime == CTRL_CLK_FLL_REGIME_ID_VR) {
			if (pm_data->clock_mode != CLOCK_MODE_MANUAL) {
				pm_data->clock_mode = CLOCK_MODE_MANUAL;
				nvgpu_info(g, "Switched to MANUAL mode for regime 0x%02x", regime);
			}
		}
	} else {
		nvgpu_err(g, "Failed to set UPROC domain regime");
		count = ret;
	}
	nvgpu_mutex_release(&pm_data->sysfs_lock);

	return count;
}

/* File operations for mode and per-domain regime control */
static const struct file_operations clock_mode_fops = {
	.open = simple_open,
	.read = clock_mode_read,
	.write = clock_mode_write,
	.llseek = default_llseek,
};
static const struct file_operations gpc_regime_fops = {
	.open = simple_open,
	.read = gpc_regime_read,
	.write = gpc_regime_write,
	.llseek = default_llseek,
};

static const struct file_operations sys_regime_fops = {
	.open = simple_open,
	.read = sys_regime_read,
	.write = sys_regime_write,
	.llseek = default_llseek,
};

static const struct file_operations nvd_regime_fops = {
	.open = simple_open,
	.read = nvd_regime_read,
	.write = nvd_regime_write,
	.llseek = default_llseek,
};

static const struct file_operations uproc_regime_fops = {
	.open = simple_open,
	.read = uproc_regime_read,
	.write = uproc_regime_write,
	.llseek = default_llseek,
};

static ssize_t gpc_voltage_limits_read(struct file *file, char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	char buf[256];
	u32 vmin_uv = 0, vmax_uv = 0;
	int len = 0;
	int err;
	struct boardobjgrp *pboardobjgrp;
	struct pmu_board_obj *obj = NULL;
	u16 index;
	u32 rail_version = 0;
	void *pmu_status_buf = NULL;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;


	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	/* Check if volt subsystem is initialized */
#ifdef CONFIG_NVGPU_LS_PMU
	if (!g->pmu || !g->pmu->volt || !g->pmu->volt->volt_metadata) {
		len += snprintf(buf + len, sizeof(buf) - len,
				"Error: PMU volt subsystem not initialized\n");
		goto done;
	}
#else
			len += snprintf(buf + len, sizeof(buf) - len,
			"Error: PMU LS not enabled in build\n");
	goto done;
#endif

	/* Call status update to refresh data from PMU FIRST */
	err = nvgpu_volt_rail_get_status(g, &pmu_status_buf);

	/* Call nvgpu_pmu_volt_get_vmin_vmax_ps35 to process voltage data */
	err = nvgpu_pmu_volt_get_vmin_vmax_ps35(g, &vmin_uv, &vmax_uv);
	if (err != 0) {
		len += snprintf(buf + len, sizeof(buf) - len, "Error reading voltage data\n");
		goto done;
	}

	rail_version = g->pmu->volt->volt_metadata->volt_rail_metadata.version;
	pboardobjgrp = &g->pmu->volt->volt_metadata->volt_rail_metadata.volt_rails.super;

	/* Display only GPC rail (index 0) information */
	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj*, obj, index) {
		if (index == 0) { /* GPC rail only */
			if (rail_version == 0x10U) { /* CTRL_VOLT_RAIL_VERSION_10 */
				struct voltage_rail *volt_rail = (struct voltage_rail *)(void *)obj;


		len += snprintf(buf + len, sizeof(buf) - len,
						"vmin=%u, vmax=%u\n",
						volt_rail->vmin_limitu_v, volt_rail->max_limitu_v);
			} else if (rail_version == 0x30U) { /* CTRL_VOLT_RAIL_VERSION_30 */
				struct voltage_rail_3x *volt_rail_3x;


			volt_rail_3x = (struct voltage_rail_3x *)(void *)obj;

		len += snprintf(buf + len, sizeof(buf) - len,
						"vmin=%u, vmax=%u\n",
						volt_rail_3x->vmin_limitu_v, volt_rail_3x->max_limitu_v);
			}
			break;
		}
	}

done:
	gk20a_idle(g);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t gpm_voltage_limits_read(struct file *file, char __user *user_buf,
				       size_t count, loff_t *ppos)
{
	struct gk20a *g = file->private_data;
	char buf[256];
	u32 vmin_uv = 0, vmax_uv = 0;
	int len = 0;
	int err;
	struct boardobjgrp *pboardobjgrp;
	struct pmu_board_obj *obj = NULL;
	u16 index;
	u32 rail_version = 0;
	void *pmu_status_buf = NULL;

	if (g->is_virtual) {
		nvgpu_err(g, "%s not supported", __func__);
		return -EOPNOTSUPP;
	}

	if (nvgpu_is_powered_off(g))
		return -EAGAIN;


	err = gk20a_busy(g);
	if (err != 0)
		return -EAGAIN;

	/* Check if volt subsystem is initialized */
#ifdef CONFIG_NVGPU_LS_PMU
	if (!g->pmu || !g->pmu->volt || !g->pmu->volt->volt_metadata) {
		len += snprintf(buf + len, sizeof(buf) - len,
				"Error: PMU volt subsystem not initialized\n");
		goto done;
	}
#else
			len += snprintf(buf + len, sizeof(buf) - len,
			"Error: PMU LS not enabled in build\n");
	goto done;
#endif

	/* Call status update to refresh data from PMU FIRST */
	err = nvgpu_volt_rail_get_status(g, &pmu_status_buf);

	/* Call nvgpu_pmu_volt_get_vmin_vmax_ps35 to process voltage data */
	err = nvgpu_pmu_volt_get_vmin_vmax_ps35(g, &vmin_uv, &vmax_uv);
	if (err != 0) {
		len += snprintf(buf + len, sizeof(buf) - len, "Error reading voltage data\n");
		goto done;
	}

	rail_version = g->pmu->volt->volt_metadata->volt_rail_metadata.version;
	pboardobjgrp = &g->pmu->volt->volt_metadata->volt_rail_metadata.volt_rails.super;

	/* Display only GPM rail (index 1) information */
	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj*, obj, index) {
		if (index == 1) { /* GPM rail only */
			if (rail_version == 0x10U) { /* CTRL_VOLT_RAIL_VERSION_10 */
				struct voltage_rail *volt_rail = (struct voltage_rail *)(void *)obj;


		len += snprintf(buf + len, sizeof(buf) - len,
						"vmin=%u, vmax=%u\n",
						volt_rail->vmin_limitu_v, volt_rail->max_limitu_v);
			} else if (rail_version == 0x30U) { /* CTRL_VOLT_RAIL_VERSION_30 */
				struct voltage_rail_3x *volt_rail_3x;


			volt_rail_3x = (struct voltage_rail_3x *)(void *)obj;

		len += snprintf(buf + len, sizeof(buf) - len,
						"vmin=%u, vmax=%u\n",
						volt_rail_3x->vmin_limitu_v, volt_rail_3x->max_limitu_v);
			}
			break;
		}
	}

done:
	gk20a_idle(g);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}
