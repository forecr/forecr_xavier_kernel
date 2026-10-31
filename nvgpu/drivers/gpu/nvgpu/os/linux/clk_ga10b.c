// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

/*
 * Linux clock support for ga10b
 */

#if defined(CONFIG_NVIDIA_CONFTEST)
#include <nvidia/conftest.h>
#endif

#include <linux/clk.h>
#include <linux/devfreq.h>

#include "clk.h"
#include "clk_ga10b.h"
#include "os_linux.h"
#include "platform_gk20a.h"

#include <nvgpu/gk20a.h>
#include <nvgpu/clk_arb.h>
#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/gr/config.h>
#include <nvgpu/pmu/clk/clk.h>

/*
 * GA10B clock list:
 * platform->clk[0]- sysclk
 * For GPU Full config:
 * platform->clk[1] - gpc0 clk
 * platform->clk[2] - gpc1 clk
 * platform->clk[3] - fuse clk
 * For GPU GPC Floor-swept config:
 * platform->clk[1] - Active gpc(gpc0/gpc1) clk
 * platform->clk[2] - fuse clk
 */

/*
 * GPU clock policy for ga10b:
 * All sys, gpc0 and gpc1 clk are at same rate.
 * So, for clock set_rate, change all clocks for
 * any clock rate change request.
 */

/*
 * PWRCLK is used for pmu runs at fixed rate 204MHZ in ga10b
 * PWRCLK is enabled once gpu out of reset. CCF is not
 * supporting any clock set/get calls for PWRCLK. To support
 * legacy code, nvgpu driver only supporting clk_get_rate by
 * returning fixed 204MHz rate
 */
#define NVGPU_GA10B_PWRCLK_RATE 204000000UL;

unsigned long nvgpu_ga10b_linux_clk_get_rate(
					struct gk20a *g, u32 api_domain)
{
	struct gk20a_platform *platform = gk20a_get_platform(dev_from_gk20a(g));
	unsigned long ret;

	switch (api_domain) {
	case CTRL_CLK_DOMAIN_SYSCLK:
	case CTRL_CLK_DOMAIN_GPCCLK:
		ret = clk_get_rate(platform->clk[0]);
		break;
	case CTRL_CLK_DOMAIN_PWRCLK:
		/* power domain is at fixed clock */
		ret = NVGPU_GA10B_PWRCLK_RATE;
		break;
	default:
		nvgpu_err(g, "unknown clock: %u", api_domain);
		ret = 0;
		break;
	}

	return ret;
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

	if (!df)
		return;

#if defined(NV_DEVFREQ_HAS_FREQ_TABLE)
	*min_freq = df->freq_table[0];
	*max_freq = df->freq_table[df->max_state - 1];
#else
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

int nvgpu_ga10b_linux_clk_set_rate(struct gk20a *g,
				     u32 api_domain, unsigned long rate)
{
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct gk20a_platform *platform = gk20a_get_platform(dev_from_gk20a(g));
	struct nvgpu_gr_config *gr_config = nvgpu_gr_get_config_ptr(g);
	u32 gpc_count = nvgpu_gr_config_get_gpc_count(gr_config);
	unsigned long min_rate = 0, max_rate = ULONG_MAX;
	int ret;

	switch (api_domain) {
	case CTRL_CLK_DOMAIN_GPCCLK:
	case CTRL_CLK_DOMAIN_SYSCLK:
		devfreq_get_freq_range(l->devfreq, &min_rate, &max_rate);
		rate = max(rate, min_rate);
		rate = min(rate, max_rate);
		ret = clk_set_rate(platform->clk[0], rate);
		ret = clk_set_rate(platform->clk[1], rate);
		/* Set second gpcclk for full-config */
		if (gpc_count == 2U)
			ret = clk_set_rate(platform->clk[2], rate);
		break;
	case CTRL_CLK_DOMAIN_PWRCLK:
		nvgpu_err(g, "unsupported operation: %u", api_domain);
		ret = -EINVAL;
		break;
	default:
		nvgpu_err(g, "unknown clock: %u", api_domain);
		ret = -EINVAL;
		break;
	}

	return ret;
}
