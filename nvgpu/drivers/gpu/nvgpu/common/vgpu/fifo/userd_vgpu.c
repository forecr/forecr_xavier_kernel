// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/trace.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/fifo/userd.h>

#include "userd_vgpu.h"

int vgpu_userd_setup_sw(struct gk20a *g)
{
	return nvgpu_userd_init_slabs(g);
}

void vgpu_userd_cleanup_sw(struct gk20a *g)
{
	nvgpu_userd_free_slabs(g);
}
