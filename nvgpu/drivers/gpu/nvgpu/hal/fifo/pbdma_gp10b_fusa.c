// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>

#include <nvgpu/hw/gp10b/hw_pbdma_gp10b.h>

#include "pbdma_gp10b.h"

u32 gp10b_pbdma_get_signature(struct gk20a *g)
{
	return g->ops.get_litter_value(g, GPU_LIT_GPFIFO_CLASS)
		| pbdma_signature_sw_zero_f();
}

u32 gp10b_pbdma_get_fc_runlist_timeslice(void)
{
	return (pbdma_runlist_timeslice_timeout_128_f() |
		pbdma_runlist_timeslice_timescale_3_f() |
		pbdma_runlist_timeslice_enable_true_f());
}

u32 gp10b_pbdma_get_config_auth_level_privileged(void)
{
	return pbdma_config_auth_level_privileged_f();
}
