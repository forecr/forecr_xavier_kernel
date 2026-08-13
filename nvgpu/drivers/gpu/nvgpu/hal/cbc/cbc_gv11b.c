// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.


#include <nvgpu/cbc.h>
#include <nvgpu/log.h>
#include <nvgpu/gk20a.h>

#include "cbc_gv11b.h"

void gv11b_cbc_init(struct gk20a *g, struct nvgpu_cbc *cbc, bool is_resume)
{
	s32 err;
	enum nvgpu_cbc_op cbc_op = is_resume ? nvgpu_cbc_op_invalidate
					     : nvgpu_cbc_op_clear;

	nvgpu_log_fn(g, " ");

	g->ops.fb.cbc_configure(g, cbc);
	/*
	 * The cbc_op_invalidate command marks all CBC lines as invalid, this
	 * causes all comptag lines to be fetched from the backing store.
	 * Whereas, the cbc_op_clear goes a step further and clears the contents
	 * of the backing store as well, because of this, cbc_op_clear should
	 * only be called during the first power-on and not on suspend/resume
	 * cycle, as the backing store might contain valid compression metadata
	 * for already allocated surfaces and clearing it will corrupt those
	 * surfaces.
	 */

	err = g->ops.cbc.ctrl(g, cbc_op, 0,
		nvgpu_safe_sub_u32(cbc->max_comptag_lines, 1U));
	if (err != 0) {
		nvgpu_err(g, "Comptags clear failed: %d", err);
	}
}
