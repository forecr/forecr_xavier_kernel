// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/ltc.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/static_analysis.h>

#include "ltc_intr_gm20b.h"

#include <nvgpu/hw/gm20b/hw_ltc_gm20b.h>


void gm20b_ltc_intr_configure(struct gk20a *g)
{
	u32 reg;

	/* Disable interrupts to reduce noise and increase perf */
	reg = nvgpu_readl(g, ltc_ltcs_ltss_intr_r());
	reg &= ~ltc_ltcs_ltss_intr_en_evicted_cb_m();
	reg &= ~ltc_ltcs_ltss_intr_en_illegal_compstat_access_m();
	reg &= ~ltc_ltcs_ltss_intr_en_illegal_compstat_m();
	nvgpu_writel(g, ltc_ltcs_ltss_intr_r(), reg);
}

static void gm20b_ltc_intr_handle_lts_interrupts(struct gk20a *g,
							u32 ltc, u32 slice)
{
	u32 ltc_intr;
	u32 ltc_stride = nvgpu_get_litter_value(g, GPU_LIT_LTC_STRIDE);
	u32 lts_stride = nvgpu_get_litter_value(g, GPU_LIT_LTS_STRIDE);

	ltc_intr = nvgpu_readl(g, nvgpu_safe_add_u32(ltc_ltc0_lts0_intr_r(),
			nvgpu_safe_add_u32(nvgpu_safe_mult_u32(ltc_stride, ltc),
				nvgpu_safe_mult_u32(lts_stride, slice))));
	nvgpu_log(g, gpu_dbg_intr, "ltc%d, slice %d: %08x",
		  ltc, slice, ltc_intr);
	nvgpu_writel(g, nvgpu_safe_add_u32(ltc_ltc0_lts0_intr_r(),
			nvgpu_safe_add_u32(nvgpu_safe_mult_u32(ltc_stride, ltc),
			   nvgpu_safe_mult_u32(lts_stride, slice))), ltc_intr);
}

int gm20b_ltc_intr_isr(struct gk20a *g, u32 ltc)
{
	u32 slice;

	if (ltc >= nvgpu_ltc_get_ltc_count(g)) {
		return -ENODEV;
	}

	for (slice = 0U; slice < g->ltc->slices_per_ltc; slice =
					nvgpu_safe_add_u32(slice, 1U)) {
		gm20b_ltc_intr_handle_lts_interrupts(g, ltc, slice);
	}

	return 0;
}
