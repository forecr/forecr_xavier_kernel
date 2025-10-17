// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/fuse.h>
#include <nvgpu/enabled.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>

#include "fuse_gp10b.h"

#include <nvgpu/hw/gp10b/hw_fuse_gp10b.h>

int gp10b_fuse_check_priv_security(struct gk20a *g)
{
	u32 gcplex_config;
	bool is_wpr_enabled = false;
	bool is_auto_fetch_disable = false;

	if (g->ops.fuse.read_gcplex_config_fuse(g, &gcplex_config) != 0) {
		nvgpu_err(g, "err reading gcplex config fuse, check fuse clk");
		return -EINVAL;
	}

	if (g->ops.fuse.fuse_opt_priv_sec_en(g) != 0U) {
		/*
		 * all falcons have to boot in LS mode and this needs
		 * wpr_enabled set to 1 and vpr_auto_fetch_disable
		 * set to 0. In this case gmmu tries to pull wpr
		 * and vpr settings from tegra mc
		 */
		nvgpu_set_enabled(g, NVGPU_SEC_PRIVSECURITY, true);
		nvgpu_set_enabled(g, NVGPU_SEC_SECUREGPCCS, true);
#ifdef CONFIG_NVGPU_SIM
		if (nvgpu_is_enabled(g, NVGPU_IS_FMODEL)) {
			/*
			 * Do not check other fuses as they are not yet modeled
                         * on FMODEL.
			 */
			return 0;
		}
#endif
		is_wpr_enabled =
			(gcplex_config & GCPLEX_CONFIG_WPR_ENABLED_MASK) != 0U;
		is_auto_fetch_disable =
			(gcplex_config & GCPLEX_CONFIG_VPR_AUTO_FETCH_DISABLE_MASK) != 0U;
		if (is_wpr_enabled && !is_auto_fetch_disable) {
			if (g->ops.fuse.fuse_opt_sec_debug_en(g) != 0U) {
				nvgpu_log(g, gpu_dbg_info,
						"gcplex_config = 0x%08x, "
						"secure mode: ACR debug",
						gcplex_config);
			} else {
				nvgpu_log(g, gpu_dbg_info,
						"gcplex_config = 0x%08x, "
						"secure mode: ACR non debug",
						gcplex_config);
			}

		} else {
			nvgpu_err(g, "gcplex_config = 0x%08x "
				"invalid wpr_enabled/vpr_auto_fetch_disable "
				"with priv_sec_en", gcplex_config);
			/* do not try to boot GPU */
			return -EINVAL;
		}
	} else {
		nvgpu_set_enabled(g, NVGPU_SEC_PRIVSECURITY, false);
		nvgpu_set_enabled(g, NVGPU_SEC_SECUREGPCCS, false);
		nvgpu_log(g, gpu_dbg_info,
				"gcplex_config = 0x%08x, non secure mode",
				gcplex_config);
	}

	return 0;
}

bool gp10b_fuse_is_opt_ecc_enable(struct gk20a *g)
{
	return nvgpu_readl(g, fuse_opt_ecc_en_r()) != 0U;
}

bool gp10b_fuse_is_opt_feature_override_disable(struct gk20a *g)
{
	return nvgpu_readl(g,
			fuse_opt_feature_fuses_override_disable_r()) != 0U;
}
