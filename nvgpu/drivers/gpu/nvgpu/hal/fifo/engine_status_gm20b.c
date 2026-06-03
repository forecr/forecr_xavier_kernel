// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/debug.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/engine_status.h>
#include <nvgpu/engines.h>
#include <nvgpu/fifo.h>

#include <nvgpu/hw/gm20b/hw_fifo_gm20b.h>

#include "engine_status_gm20b.h"

void gm20b_dump_engine_status(struct gk20a *g, struct nvgpu_debug_context *o)
{
	u32 i, host_num_engines;
	struct nvgpu_engine_status_info engine_status;

	host_num_engines = nvgpu_get_litter_value(g, GPU_LIT_HOST_NUM_ENGINES);

	gk20a_debug_output(o, "Engine status - chip %-5s", g->name);
	gk20a_debug_output(o, "--------------------------");

	for (i = 0; i < host_num_engines; i++) {
		if (!nvgpu_engine_check_valid_id(g, i)) {
			/* Skip invalid engines */
			continue;
		}

		g->ops.engine_status.read_engine_status_info(g, i, &engine_status);

		gk20a_debug_output(o,
			"Engine %d | "
			"ID: %d - %-9s next_id: %d %-9s | status: %s",
			i,
			engine_status.ctx_id,
			nvgpu_engine_status_is_ctx_type_tsg(
				&engine_status) ?
				"[tsg]" : "[channel]",
			engine_status.ctx_next_id,
			nvgpu_engine_status_is_next_ctx_type_tsg(
				&engine_status) ?
				"[tsg]" : "[channel]",
			nvgpu_fifo_decode_pbdma_ch_eng_status(
				engine_status.ctxsw_state));

		if (engine_status.is_faulted) {
			gk20a_debug_output(o, "  State: faulted");
		}
		if (engine_status.is_busy) {
			gk20a_debug_output(o, "  State: busy");
		}
	}
	gk20a_debug_output(o, " ");
}
