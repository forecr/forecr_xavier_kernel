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

#include "lrc_gb10b.h"

#include <nvgpu/hw/gb10b/hw_lrc_gb10b.h>

static void gb10b_handle_each_lrc_parity(struct gk20a *g, u32 offset,
				u32 lrc_ecc_intr_status, u32 lrcc, u32 lrc)
{
	u32 lrc_ecc_status = 0U;
	u32 lrc_ecc_addr = 0U;
	u32 uncorrected_cnt = 0U;
	u32 uncorrected_delta = 0U;
	u32 uncorrected_overflow = 0U;

	if ((lrc_ecc_intr_status & ~(lrc_ecc_intr_status_uncorrected_err_m())) != 0U) {
		nvgpu_err(g, "Unsupported LRC ECC error detected in LRCC%d, LRC%d! "
				"Treating this as a fatal error: lrc_ecc_intr_status 0x%08x",
				lrcc, lrc, lrc_ecc_intr_status);
		nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_LRC,
				GPU_LRC_SRAM_ECC_UNSUPPORTED);
		BUG();
	}

	/* Handle ECC uncorrected errors. */

	lrc_ecc_status = nvgpu_readl(g, nvgpu_safe_add_u32(lrc_ecc_status_r(), offset));
	lrc_ecc_addr = nvgpu_readl(g, nvgpu_safe_add_u32(lrc_ecc_address_r(), offset));

	uncorrected_cnt = nvgpu_readl(g, nvgpu_safe_add_u32(lrc_ecc_uncorrected_err_count_r(), offset));
	uncorrected_delta = lrc_ecc_uncorrected_err_count_total_v(uncorrected_cnt);
	uncorrected_overflow = lrc_ecc_status &
			lrc_ecc_status_uncorrected_err_total_counter_overflow_m();

	/* No need to clear count and status registers since GPU is reset for fatal errors. */

	if (uncorrected_overflow != 0U) {
		nvgpu_info(g, "uncorrected lrc ecc counter overflow!");
		uncorrected_delta += BIT32(lrc_ecc_uncorrected_err_count_total_s());
	}
	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_LRC,
			GPU_LRC_SRAM_ECC_UNCORRECTED);
	nvgpu_err(g, "Uncorrected ECC error detected in LRC SRAM! "
			"lrcc: %d, lrc: %d lrc_ecc_status: 0x%08x, "
			"ecc error address: 0x%08x, uncorrected err count: 0x%08x, "
			"lrc_ecc_intr_status: 0x%08x",
			lrcc, lrc, lrc_ecc_status, lrc_ecc_addr, uncorrected_delta,
			lrc_ecc_intr_status);

	nvgpu_sw_quiesce(g);
}

static void gb10b_lrc_parity_ue_handler(struct gk20a *g, u64 cookie)
{
	u32 lrcc_count = 0U;
	u32 lrc_count = 0U;
	u32 lrcc, lrc = 0U;
	u32 lrcc_stride = 0U;
	u32 lrc_stride = 0U;

	(void)cookie;

	lrcc_stride = nvgpu_get_litter_value(g, GPU_LIT_LRCC_STRIDE);
	lrc_stride = nvgpu_get_litter_value(g, GPU_LIT_LRC_STRIDE);

	/* Initialise the LRCC, LRC count with LTC and LTS (slice) count respectively. */
	lrcc_count = g->ltc->ltc_count;
	lrc_count = g->ltc->slices_per_ltc;
	/*
	 * Go through all LRCCs explicitly.
	 */
	for (lrcc = 0; lrcc < lrcc_count; lrcc++) {
		/* LRCC is a cluster of 4 LRCs. Go through all LRCs explicitly. */
		for (lrc = 0; lrc < lrc_count; lrc++) {
			u32 offset, lrc_ecc_intr_status = 0U;

			offset = nvgpu_safe_add_u32(nvgpu_safe_mult_u32(lrcc_stride, lrcc),
									nvgpu_safe_mult_u32(lrc_stride, lrc));
			lrc_ecc_intr_status = nvgpu_readl(g, nvgpu_safe_add_u32(
							lrc_ecc_intr_status_r(), offset));
			if (lrc_ecc_intr_status != 0U) {
				gb10b_handle_each_lrc_parity(g, offset, lrc_ecc_intr_status,
								lrcc, lrc);
			}
		}
	}
}

int gb10b_lrc_init_hw(struct gk20a *g)
{
	u32 ue_vector = 0;
	u32 intr_ctrl_msg_ue = 0;
	u32 lrcc = 0;
	u32 lrcc_count = 0U;
	u32 lrcc_stride = 0U;

    /*
     * Parity diagonostic bits are enabled by default in HW,
     * so no need of INTR_EN and INTR_MASK programming.
     */

	ue_vector = nvgpu_gin_get_unit_stall_vector(g, NVGPU_GIN_INTR_UNIT_LRC,
				NVGPU_LRC_UE_VECTOR_OFFSET);
	intr_ctrl_msg_ue = nvgpu_gin_get_intr_ctrl_msg(g, ue_vector,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);

	/* LRCC count is equal to LTC count */
	lrcc_count = g->ops.priv_ring.enum_ltc(g);
	lrcc_stride = nvgpu_get_litter_value(g, GPU_LIT_LRCC_STRIDE);

	/*
	 * All 4 LRCCs have different intr_ctrl registers. Program all
	 * intr_ctrl registers with a common vector and set a
	 * common handler for them.
	 */

	for (lrcc = 0; lrcc < lrcc_count; lrcc++) {
		nvgpu_writel(g, nvgpu_safe_add_u32(lrc_lrcc0_intr_ctrl_r(),
						nvgpu_safe_mult_u32(lrcc_stride, lrcc)), intr_ctrl_msg_ue);
	}
	nvgpu_gin_set_stall_handler(g, ue_vector, &gb10b_lrc_parity_ue_handler, 0);
	nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_LRC,
			NVGPU_LRC_UE_VECTOR_OFFSET,
			NVGPU_GIN_INTR_ENABLE);

	return 0;
}