// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/ltc.h>
#include <nvgpu/log.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/static_analysis.h>

#include <nvgpu/hw/gp10b/hw_ltc_gp10b.h>

#include "ltc_intr_gp10b.h"
#include "ltc_intr_gm20b.h"

int gp10b_ltc_intr_isr(struct gk20a *g, u32 ltc)
{
	u32 slice;

	if (ltc >= nvgpu_ltc_get_ltc_count(g)) {
		return -ENODEV;
	}

	for (slice = 0U; slice < g->ltc->slices_per_ltc; slice =
				nvgpu_safe_add_u32(slice, 1U)) {
		gp10b_ltc_intr_handle_lts_interrupts(g, ltc, slice);
	}

	return 0;
}

void gp10b_ltc_intr_configure(struct gk20a *g)
{
	u32 ltc_intr;

	gm20b_ltc_intr_configure(g);

	/* Enable ECC interrupts */
	ltc_intr = nvgpu_readl(g, ltc_ltcs_ltss_intr_r());
	ltc_intr |= ltc_ltcs_ltss_intr_en_ecc_sec_error_enabled_f() |
			ltc_ltcs_ltss_intr_en_ecc_ded_error_enabled_f();
	nvgpu_writel(g, ltc_ltcs_ltss_intr_r(), ltc_intr);
}
