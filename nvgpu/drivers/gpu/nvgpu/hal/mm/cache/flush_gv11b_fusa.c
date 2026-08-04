// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>

#include <nvgpu/hw/gv11b/hw_flush_gv11b.h>

#include "flush_gk20a.h"
#include "flush_gv11b.h"

int gv11b_mm_l2_flush(struct gk20a *g, bool invalidate)
{
	int err = 0;

	nvgpu_log(g, gpu_dbg_mm, "gv11b_mm_l2_flush");

	err = g->ops.mm.cache.fb_flush(g);
	if (err != 0) {
		nvgpu_err(g, "mm.cache.fb_flush()[1] failed err=%d", err);
		return err;
	}
	err = gk20a_mm_l2_flush(g, invalidate);
	if (err != 0) {
		nvgpu_err(g, "gk20a_mm_l2_flush failed");
		return err;
	}
	err = g->ops.mm.cache.fb_flush(g);
	if (err != 0) {
		nvgpu_err(g, "mm.cache.fb_flush()[2] failed err=%d",
			  err);
		return err;
	}

	return err;
}
