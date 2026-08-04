// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/nvgpu_err_info.h>

#include "cic_mon_priv.h"

int nvgpu_cic_mon_bound_check_hw_unit_id(struct gk20a *g, u32 hw_unit_id)
{
	if (g->cic_mon == NULL) {
		nvgpu_err(g, "CIC is not initialized");
		return -EINVAL;
	}

	if (g->cic_mon->num_hw_modules == 0U) {
		cic_dbg(g, "LUT not initialized.");
		return -EINVAL;
	}

	if (hw_unit_id >= g->cic_mon->num_hw_modules) {
		cic_dbg(g, "Invalid input HW unit ID.");
		return -EINVAL;
	}

	return 0;
}

int nvgpu_cic_mon_bound_check_err_id(struct gk20a *g, u32 hw_unit_id,
		u32 err_id)
{
	int err = 0;

	if ((g->cic_mon == NULL) || (g->cic_mon->err_lut == NULL)) {
		cic_dbg(g, "CIC/LUT not initialized.");
		return -EINVAL;
	}

	err = nvgpu_cic_mon_bound_check_hw_unit_id(g, hw_unit_id);
	if (err != 0) {
		return err;
	}

	if (err_id >= g->cic_mon->err_lut[hw_unit_id].num_errs) {
		err = -EINVAL;
	}

	return err;
}

#ifdef CONFIG_NVGPU_INTR_DEBUG
int nvgpu_cic_mon_get_err_desc(struct gk20a *g, u32 hw_unit_id,
		u32 err_id, struct nvgpu_err_desc **err_desc)
{
	int err = 0;

	/* if (g->cic_mon != NULL) and (g->cic_mon->err_lut != NULL) check
	 * can be skipped here as it checked as part of
	 * nvgpu_cic_mon_bound_check_err_id() called below.
	 */

	err = nvgpu_cic_mon_bound_check_err_id(g, hw_unit_id, err_id);
	if (err != 0) {
		return err;
	}

	*err_desc = &(g->cic_mon->err_lut[hw_unit_id].errs[err_id]);

	return err;
}
#endif

#ifndef CONFIG_NVGPU_MON_PRESENT
int nvgpu_cic_mon_get_num_hw_modules(struct gk20a *g)
{
	if (g->cic_mon == NULL) {
		nvgpu_err(g, "CIC is not initialized");
		return -EINVAL;
	}

	return nvgpu_safe_cast_u32_to_s32(g->cic_mon->num_hw_modules);
}
#endif
