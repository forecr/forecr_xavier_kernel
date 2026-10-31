/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GM20B_PERF
#define NVGPU_GM20B_PERF

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_mem;

bool gm20b_perf_get_membuf_overflow_status(struct gk20a *g, u32 pma_channel_id);
u32 gm20b_perf_get_membuf_pending_bytes(struct gk20a *g, u32 pma_channel_id);
void gm20b_perf_set_membuf_handled_bytes(struct gk20a *g, u32 pma_channel_id,
					 u32 entries, u32 entry_size);

void gm20b_perf_membuf_reset_streaming(struct gk20a *g, u32 pma_channel_id);

void gm20b_perf_enable_membuf(struct gk20a *g, u32 pma_channel_id, u32 size,
			      u64 buf_addr);
void gm20b_perf_disable_membuf(struct gk20a *g, u32 pma_channel_id);

int gm20b_perf_init_inst_block(struct gk20a *g, u32 pma_channel_id,
			       struct nvgpu_mem *inst_block);
void gm20b_perf_deinit_inst_block(struct gk20a *g, u32 pma_channel_id);

u32 gm20b_perf_get_pmmsys_per_chiplet_offset(void);
u32 gm20b_perf_get_pmmgpc_per_chiplet_offset(void);
u32 gm20b_perf_get_pmmfbp_per_chiplet_offset(void);

#endif /* CONFIG_NVGPU_DEBUGGER */
#endif
