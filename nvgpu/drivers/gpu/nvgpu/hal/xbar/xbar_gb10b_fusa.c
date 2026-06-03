// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/io.h>
#include <nvgpu/soc.h>
#include <nvgpu/bug.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/ltc.h>
#include <nvgpu/gin.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/nvgpu_err.h>

#include "xbar_gb10b.h"

#include <nvgpu/hw/gb10b/hw_xbar_gb10b.h>

static void gb10b_handle_each_gnic_xbar_read_ecc_error(struct gk20a *g,
	u32 offset, u32 gpc, u32 gnic, u32 xbar_ecc_status)
{
	u32 xbar_ecc_address, xbar_ecc_uncorr_count = 0U;
	u32 xbar_ecc_uncorr_count_delta = 0U;
	bool is_xbar_ecc_uncorr_err_overflow = false;

	if ((xbar_ecc_status &
			xbar_gpc0_gnic0_preg_ecc_status_corrected_err_e2e_m()) != 0U) {
		nvgpu_err(g, "Unsupported XBAR ECC corrected error. "
			"Treating this as a fatal error!");
		BUG();
	}

	xbar_ecc_address = nvgpu_readl(g, xbar_gpc0_gnic0_preg_ecc_address_r());
	xbar_ecc_uncorr_count = nvgpu_readl(g,
		xbar_gpc0_gnic0_preg_ecc_uncorrected_err_count_r());
	xbar_ecc_uncorr_count_delta =
		xbar_gpc0_gnic0_preg_ecc_uncorrected_err_count_total_v(xbar_ecc_uncorr_count);
	is_xbar_ecc_uncorr_err_overflow = xbar_ecc_status &
		xbar_gpc0_gnic0_preg_ecc_status_uncorrected_err_total_counter_overflow_m();

	if (is_xbar_ecc_uncorr_err_overflow != 0U) {
		nvgpu_info(g, "xbar ecc uncorrected err counter overflow!");
		xbar_ecc_uncorr_count_delta +=
			BIT32(xbar_gpc0_gnic0_preg_ecc_uncorrected_err_count_total_s());
	}

	nvgpu_err(g, "gpc%dgnic%d_preg_xbar_ecc_status: 0x%08x, "
		"xbar_ecc_uncorr_count: 0x%08x, xbar_ecc_address: 0x%08x",
		gpc, gnic, xbar_ecc_status, xbar_ecc_uncorr_count_delta,
		xbar_ecc_address);

	/* Clear the interrupt */
	nvgpu_writel(g, nvgpu_safe_add_u32(
		xbar_gpc0_gnic0_preg_ecc_status_r(), offset),
		xbar_gpc0_gnic0_preg_ecc_status_reset_clear_f());
}

void gb10b_handle_xbar_read_ecc_err(struct gk20a *g)
{
	u32 num_gnics_per_gpc, gpc_count = 0U;
	u32 gnic_stride, gpc_stride = 0U;
	u32 gpc, gnic = 0U;

	/* Get number non-FSed GPCs using priv_ring HAL */
	gpc_count = g->ops.priv_ring.get_gpc_count(g);

	gpc_stride = nvgpu_get_litter_value(g,
		GPU_LIT_PRI_GNIC_PRI_STRIDE);

	gnic_stride = nvgpu_get_litter_value(g,
		GPU_LIT_PRI_STRIDE_ACROSS_GNICS);
	num_gnics_per_gpc = nvgpu_get_litter_value(g,
		GPU_LIT_NUM_GPCARB_PRI_TARGETS_PER_GPC);

	/* Go through all GPCs and GNICs explicitly */
	for (gpc = 0; gpc < gpc_count; gpc++) {
		for (gnic = 0; gnic < num_gnics_per_gpc; num_gnics_per_gpc++) {
			u32 offset, xbar_ecc_status = 0U;
			offset = nvgpu_safe_add_u32(nvgpu_safe_mult_u32(gpc_stride, gpc),
						nvgpu_safe_mult_u32(gnic_stride, gnic));
			xbar_ecc_status = nvgpu_readl(g, nvgpu_safe_add_u32(
				xbar_gpc0_gnic0_preg_ecc_status_r(), offset));
			if (xbar_ecc_status != 0U) {
				gb10b_handle_each_gnic_xbar_read_ecc_error(g, offset,
					gpc, gnic, xbar_ecc_status);
			}
		}
	}
}

