// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/debug.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/pbdma_status.h>

#include <nvgpu/hw/gb10b/hw_pbdma_gb10b.h>

#include "pbdma_gb10b.h"

void gb10b_pbdma_dump_status(struct gk20a *g, struct nvgpu_debug_context *o)
{
	u32 i, host_num_pbdma;
	struct nvgpu_pbdma_status_info pbdma_status;

	host_num_pbdma = nvgpu_get_litter_value(g, GPU_LIT_HOST_NUM_PBDMA);

	gk20a_debug_output(o, "PBDMA Status - chip %-5s", g->name);
	gk20a_debug_output(o, "-------------------------");

	for (i = 0U; i < host_num_pbdma; i++) {
		g->ops.pbdma_status.read_pbdma_status_info(g, i,
			&pbdma_status);

		gk20a_debug_output(o, "pbdma %d:", i);
		gk20a_debug_output(o,
			"  id: %d - %-9s next_id: - %d %-9s | status: %s",
			pbdma_status.id,
			nvgpu_pbdma_status_is_id_type_tsg(&pbdma_status) ?
				   "[tsg]" : "[channel]",
			pbdma_status.next_id,
			nvgpu_pbdma_status_is_next_id_type_tsg(
				&pbdma_status) ?
				   "[tsg]" : "[channel]",
			nvgpu_fifo_decode_pbdma_ch_eng_status(
				pbdma_status.pbdma_channel_status));
		gk20a_debug_output(o,
			"  PBDMA_PUT %016llx PBDMA_GET %016llx",
			(u64)nvgpu_readl(g, pbdma_put_r(i)) +
			((u64)nvgpu_readl(g, pbdma_put_hi_r(i)) << 32ULL),
			(u64)nvgpu_readl(g, pbdma_get_r(i)) +
			((u64)nvgpu_readl(g, pbdma_get_hi_r(i)) << 32ULL));
		gk20a_debug_output(o,
			"  GP_PUT    %08x  GP_GET  %08x  "
			"FETCH   %08x HEADER %08x",
			nvgpu_readl(g, pbdma_gp_put_r(i)),
			nvgpu_readl(g, pbdma_gp_get_r(i)),
			nvgpu_readl(g, pbdma_gp_fetch_r(i)),
			nvgpu_readl(g, pbdma_pb_header_r(i)));
		gk20a_debug_output(o,
			"  HDR       %08x  SHADOW0 %08x  SHADOW1 %08x",
			g->ops.pbdma.read_data(g, i),
			nvgpu_readl(g, pbdma_gp_shadow_0_r(i)),
			nvgpu_readl(g, pbdma_gp_shadow_1_r(i)));
	}

	gk20a_debug_output(o, " ");
}
