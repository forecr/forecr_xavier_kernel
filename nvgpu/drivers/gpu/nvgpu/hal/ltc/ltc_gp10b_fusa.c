// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/ltc.h>
#include <nvgpu/log.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/static_analysis.h>

#include <nvgpu/hw/gp10b/hw_ltc_gp10b.h>

#include "ltc_gm20b.h"
#include "ltc_gp10b.h"

u64 gp10b_determine_L2_size_bytes(struct gk20a *g)
{
	u32 reg_val;
	u32 slice_size;
	u32 slices_per_l2;
	u64 ret;

	nvgpu_log_fn(g, " ");

	reg_val = gk20a_readl(g, ltc_ltc0_lts0_tstg_info_1_r());
	slice_size = ltc_ltc0_lts0_tstg_info_1_slice_size_in_kb_v(reg_val);
	slices_per_l2 = ltc_ltc0_lts0_tstg_info_1_slices_per_l2_v(reg_val);

	ret = nvgpu_safe_mult_u64(g->ltc->ltc_count,
			nvgpu_safe_mult_u64(
				nvgpu_safe_mult_u64(U64(slice_size), 1024ULL),
				U64(slices_per_l2)));

	nvgpu_log(g, gpu_dbg_info, "L2 size: %llu\n", ret);

	nvgpu_log_fn(g, "done");

	return ret;
}

#if defined(CONFIG_NVGPU_NON_FUSA) || defined(CONFIG_NVGPU_KERNEL_MODE_SUBMIT)
void gp10b_ltc_set_enabled(struct gk20a *g, bool enabled)
{
	u32 reg_f = ltc_ltcs_ltss_tstg_set_mgmt_2_l2_bypass_mode_enabled_f();
	u32 reg = gk20a_readl(g, ltc_ltcs_ltss_tstg_set_mgmt_2_r());

	if (enabled) {
		/* bypass disabled (normal caching ops) */
		reg &= ~reg_f;
	} else {
		/* bypass enabled (no caching) */
		reg |= reg_f;
	}

	nvgpu_writel(g, ltc_ltcs_ltss_tstg_set_mgmt_2_r(), reg);
}
#endif
