// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/nvgpu_err_info.h>
#include <nvgpu/cic_mon.h>

#include "cic_mon_priv.h"

/*
 * LUT is currently indexed using hw_unit_id. However for newly added units,
 * hw_unit_id falls out of the size of LUT. For eg: for NVENC, hw_unit_id is
 * 0x1000. This function calculates the correct index in the LUT for these new
 * units using the hw_unit_id. The calculated index can then be used to access
 * the unit's structure in the LUT.
 */

static u32 get_lut_index_from_hw_unit_id(u32 hw_unit_id)
{
	u32 lut_index = 0U;

	switch (hw_unit_id) {
	case 0x100000:
		lut_index = 16U;
		break;
	case 0x200000:
		lut_index = 17U;
		break;
	case 0x300000:
		lut_index = 18U;
		break;
	case 0x400000:
		lut_index = 19U;
		break;
	case 0x500000:
		lut_index = 20U;
		break;
	case 0x600000:
		lut_index = 21U;
		break;
	case 0x700000:
		lut_index = 22U;
		break;
	case 0x800000:
		lut_index = 23U;
		break;
	default:
		lut_index = hw_unit_id;
		break;
	}
	return lut_index;
}
void nvgpu_report_err_to_sdl(struct gk20a *g, u32 hw_unit_id, u32 err_id)
{
	int32_t err = 0;
	u32 ss_err_id = 0U;
	u32 lut_index = 0U;

	if (g->ops.cic_mon.report_err == NULL) {
		return;
	}

	lut_index = get_lut_index_from_hw_unit_id(hw_unit_id);
	err = nvgpu_cic_mon_bound_check_err_id(g, lut_index, err_id);
	if (err != 0) {
		nvgpu_err(g, "Invalid hw_unit_id/err_id"
				"hw_unit_id = 0x%x, err_id=0x%x",
				hw_unit_id, err_id);
		goto handle_report_failure;
	}

	/**
	 * Prepare error ID that will be reported to Safety_Services.
	 * Format of the error ID:
	 * - HW_unit_id (8-bits: bit-0 to 3 and bit-20 to 23),
	 * - Error_id (5-bits: bit-4 to 8),
	 * - Corrected/Uncorrected error (1-bit: bit-9),
	 * - Recovery triggered or nor (1 bit: bit 10),
	 * - Quiesce triggered or nor (1 bit: bit 11),
	 * - Reserved for future use to suggest error handling to user (8 bits: bit-12 to 19),
	 * - Remaining (8 bits: bit-24 to 31) are unused.
	 */

	/*
	 * For newly added units with hw_unit_id beyond 0xF,
	 * bits 20-23 are used to capture the hw unit ID while bits
	 * 0-3 remain 0. For units with hw_unit_ids below 0xF,
	 * bits 20-23 remain 0 and hw unit ID is captured in bits 0-3.
	 */
	hw_unit_id = hw_unit_id & HW_UNIT_ID_MASK;
	err_id = err_id & ERR_ID_MASK;
	ss_err_id =  ((err_id) << (ERR_ID_FIELD_SHIFT)) | hw_unit_id;
	if (g->cic_mon->err_lut[lut_index].errs[err_id].is_critical) {
		ss_err_id =  ss_err_id |
			(U32(1) << U32(CORRECTED_BIT_FIELD_SHIFT));
	}
	if (g->cic_mon->err_lut[lut_index].errs[err_id].is_rc_triggered) {
		ss_err_id =  ss_err_id |
			(U32(1) << U32(RC_BIT_FIELD_SHIFT));
	}
	if (g->cic_mon->err_lut[lut_index].errs[err_id].is_quiesce_triggered) {
		ss_err_id =  ss_err_id |
			(U32(1) << U32(QUIESCE_BIT_FIELD_SHIFT));
	}

	if (g->ops.cic_mon.report_err(g, ss_err_id) != 0) {
		nvgpu_err(g, "Failed to report an error: "
				"hw_unit_id = 0x%x, err_id=0x%x, "
				"ss_err_id = 0x%x",
				hw_unit_id, err_id, ss_err_id);
		goto handle_report_failure;
	}

#ifndef CONFIG_NVGPU_RECOVERY
	/*
	 * Trigger SW quiesce, in case of an uncorrected error is reported
	 * to Safety_Services, in safety build.
	 */
	if (g->cic_mon->err_lut[lut_index].errs[err_id].is_critical) {
		nvgpu_sw_quiesce(g);
	}
#endif

	return;

handle_report_failure:
#ifdef CONFIG_NVGPU_BUILD_CONFIGURATION_IS_SAFETY
	/*
	 * Trigger SW quiesce, in case of a SW error is encountered during
	 * error reporting to Safety_Services, in safety build.
	 */
	nvgpu_sw_quiesce(g);
#endif
	return;
}

