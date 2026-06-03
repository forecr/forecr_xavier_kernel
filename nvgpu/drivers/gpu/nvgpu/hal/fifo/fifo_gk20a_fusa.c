// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/log.h>
#include <nvgpu/soc.h>
#include <nvgpu/io.h>
#include <nvgpu/fifo.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/power_features/cg.h>

#include "hal/fifo/fifo_gk20a.h"

#include <nvgpu/hw/gk20a/hw_fifo_gk20a.h>

bool gk20a_fifo_find_pbdma_for_runlist(struct gk20a *g,
				       u32 runlist_id, u32 *pbdma_mask)
{
	u32 runlist_bit = BIT32(runlist_id);
	u32 num_pbdmas = nvgpu_get_litter_value(g, GPU_LIT_HOST_NUM_PBDMA);
	u32 id;

	*pbdma_mask = 0U;

	for (id = 0U; id < num_pbdmas; id++) {
		u32 pbdma_map = nvgpu_readl(g, fifo_pbdma_map_r(id));

		if ((pbdma_map & runlist_bit) != 0U) {
			*pbdma_mask |= BIT32(id);
		}
	}

	return *pbdma_mask != 0U;
}

u32 gk20a_fifo_get_runlist_timeslice(struct gk20a *g)
{
	(void)g;
	return fifo_runlist_timeslice_timeout_128_f() |
			fifo_runlist_timeslice_timescale_3_f() |
			fifo_runlist_timeslice_enable_true_f();
}

u32 gk20a_fifo_get_pb_timeslice(struct gk20a *g)
{
	(void)g;
	return fifo_pb_timeslice_timeout_16_f() |
			fifo_pb_timeslice_timescale_0_f() |
			fifo_pb_timeslice_enable_true_f();
}
