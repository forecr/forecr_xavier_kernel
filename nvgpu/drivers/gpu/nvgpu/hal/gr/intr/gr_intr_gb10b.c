// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/class.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/string.h>
#include <nvgpu/errata.h>
#include <nvgpu/grmgr.h>

#include <nvgpu/gr/config.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/gr/gr_instances.h>
#include <nvgpu/gr/gr_intr.h>
#include <nvgpu/gr/gr_falcon.h>
#include <nvgpu/gr/gr_utils.h>

#include "common/gr/gr_priv.h"
#include "common/gr/gr_config_priv.h"
#include "common/gr/gr_intr_priv.h"
#include "gr_intr_gb10b.h"

#include <nvgpu/hw/gb10b/hw_gr_gb10b.h>

void gb10b_gr_intr_tpc_exception_sm_enable(struct gk20a *g)
{
	u32 tpc_exception_en = nvgpu_readl(g,
				gr_gpc0_tpc0_tpccs_tpc_exception_en_r());

	tpc_exception_en &=
			~gr_gpc0_tpc0_tpccs_tpc_exception_en_sm_enabled_f();
	tpc_exception_en |= gr_gpc0_tpc0_tpccs_tpc_exception_en_sm_enabled_f();
	nvgpu_writel(g,
		     gr_gpcs_tpcs_tpccs_tpc_exception_en_r(),
		     tpc_exception_en);
}

#ifdef CONFIG_NVGPU_DEBUGGER
u64 gb10b_gr_intr_tpc_enabled_exceptions(struct gk20a *g)
{
	u32 sm_id;
	u64 tpc_exception_en = 0;
	u32 sm_bit_in_tpc = 0U;
	u32 offset, regval, tpc_offset, gpc_offset;
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 tpc_in_gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_TPC_IN_GPC_STRIDE);
	struct nvgpu_gr_config *config = nvgpu_gr_get_config_ptr(g);

	for (sm_id = 0; sm_id < config->no_of_sm; sm_id++) {
		struct nvgpu_sm_info *sm_info =
			nvgpu_gr_config_get_sm_info(config, sm_id);
		nvgpu_assert(sm_info != NULL);

		tpc_offset = nvgpu_safe_mult_u32(tpc_in_gpc_stride,
			nvgpu_gr_config_get_sm_info_tpc_index(sm_info));
		gpc_offset = nvgpu_safe_mult_u32(gpc_stride,
			nvgpu_gr_config_get_sm_info_gpc_index(sm_info));
		offset = nvgpu_safe_add_u32(tpc_offset, gpc_offset);

		regval = gk20a_readl(g,	nvgpu_safe_add_u32(
			      gr_gpc0_tpc0_tpccs_tpc_exception_en_r(), offset));
		/*
		 * Each bit represents corresponding enablement state, bit 0
		 * corrsponds to SM0.
		 */
		sm_bit_in_tpc =
			gr_gpc0_tpc0_tpccs_tpc_exception_en_sm_v(regval);
		tpc_exception_en |= (u64)sm_bit_in_tpc << sm_id;
	}

	return tpc_exception_en;
}
#endif
