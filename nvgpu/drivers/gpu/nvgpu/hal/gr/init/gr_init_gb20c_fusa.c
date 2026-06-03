// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/hw/gb20c/hw_gr_gb20c.h>

#include "gr_init_gb20c.h"

u32 gb20c_gr_init_get_attrib_cb_default_size(struct gk20a *g)
{
	(void)g;
	return gr_gpc0_ppc0_cbm_beta_cb_size_v_default_v();
}

u32 gb20c_gr_init_get_min_gpm_fifo_depth(struct gk20a *g)
{
	(void)g;
	return gr_pd_ab_dist_cfg2_state_limit_min_gpm_fifo_depths_v();
}

u32 gb20c_gr_init_get_bundle_cb_token_limit(struct gk20a *g)
{
	(void)g;
	return gr_pd_ab_dist_cfg2_token_limit_init_v();
}

bool gb20c_is_cwd_gpc_tpc_id_removed(void)
{
	return true;
}