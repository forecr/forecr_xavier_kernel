// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>

#include "hal/vgpu/fb/intr/fb_intr_gb10b_vf.h"
#include "fb_gb10b_vf.h"

void vf_gb10b_fb_init_hw(struct gk20a *g)
{
	vf_gb10b_fb_intr_vectorid_init(g);

	if (g->ops.fb.intr.enable != NULL) {
		g->ops.fb.intr.enable(g);
	}
}
