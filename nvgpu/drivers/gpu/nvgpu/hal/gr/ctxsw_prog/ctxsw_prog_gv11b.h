/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CTXSW_PROG_GV11B_H
#define NVGPU_CTXSW_PROG_GV11B_H

#include <nvgpu/types.h>

void gv11b_ctxsw_prog_set_context_buffer_ptr(struct gk20a *g,
	struct nvgpu_mem *ctx_mem, u64 addr);
void gv11b_ctxsw_prog_set_type_per_veid_header(struct gk20a *g,
	struct nvgpu_mem *ctx_mem);
#if defined(CONFIG_NVGPU_GRAPHICS) && defined(CONFIG_NVGPU_ZCULL)
void gv11b_ctxsw_prog_set_zcull_ptr(struct gk20a *g, struct nvgpu_mem *ctx_mem,
	u64 addr);
#endif /* CONFIG_NVGPU_GRAPHICS and CONFIG_NVGPU_ZCULL */
#ifdef CONFIG_NVGPU_GFXP
void gv11b_ctxsw_prog_set_full_preemption_ptr(struct gk20a *g,
	struct nvgpu_mem *ctx_mem, u64 addr);
void gv11b_ctxsw_prog_set_full_preemption_ptr_veid0(struct gk20a *g,
	struct nvgpu_mem *ctx_mem, u64 addr);
#endif /* CONFIG_NVGPU_GFXP */
#ifdef CONFIG_NVGPU_DEBUGGER
void gv11b_ctxsw_prog_set_pm_ptr(struct gk20a *g, struct nvgpu_mem *ctx_mem,
	u64 addr);
u32 gv11b_ctxsw_prog_hw_get_pm_mode_stream_out_ctxsw(void);
u32 gv11b_ctxsw_prog_hw_get_perf_counter_register_stride(void);
#endif /* CONFIG_NVGPU_DEBUGGER */

#endif /* NVGPU_CTXSW_PROG_GV11B_H */
