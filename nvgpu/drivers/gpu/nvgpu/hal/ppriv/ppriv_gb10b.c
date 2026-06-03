// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bug.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/fbp.h>
#include <nvgpu/io.h>

#include <nvgpu/hw/gb10b/hw_ppriv_gb10b.h>

#include "ppriv_gb10b.h"

void gb10b_init_fbp_physical_logical_id_map(struct gk20a *g, u32 *fbp_physical_id_map)
{
	u32 val, i, phy_id, log_id;
	u32 num_fbps = nvgpu_fbp_get_num_fbps(g->fbp);
	u32 stride = nvgpu_safe_sub_u32(ppriv_rs_ctrl_fbp_fbp1_rs_id_r(),
				ppriv_rs_ctrl_fbp_fbp0_rs_id_r());

	for (i = 0U; i < num_fbps; i++) {
		val = nvgpu_readl(g, nvgpu_safe_add_u32(ppriv_rs_ctrl_fbp_fbp0_rs_id_r(),
							nvgpu_safe_mult_u32(stride, i)));
		phy_id = ppriv_rs_ctrl_fbp_fbp0_rs_id_physical_v(val);
		log_id = ppriv_rs_ctrl_fbp_fbp0_rs_id_logical_v(val);
		fbp_physical_id_map[phy_id] = log_id;
		nvgpu_log(g, gpu_dbg_prof, "Fbp mapping init: physical(%u)->logical(%u)", phy_id, log_id);
	}
}
