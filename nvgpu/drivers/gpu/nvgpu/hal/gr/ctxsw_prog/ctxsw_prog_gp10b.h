/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CTXSW_PROG_GP10B_H
#define NVGPU_CTXSW_PROG_GP10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_mem;

void gp10b_ctxsw_prog_set_compute_preemption_mode_cta(struct gk20a *g,
	struct nvgpu_mem *ctx_mem);
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gp10b_ctxsw_prog_init_ctxsw_hdr_data(struct gk20a *g,
	struct nvgpu_mem *ctx_mem);
#endif
#ifdef CONFIG_NVGPU_GFXP
void gp10b_ctxsw_prog_set_graphics_preemption_mode_gfxp(struct gk20a *g,
	struct nvgpu_mem *ctx_mem);
void gp10b_ctxsw_prog_set_full_preemption_ptr(struct gk20a *g,
	struct nvgpu_mem *ctx_mem, u64 addr);
#endif /* CONFIG_NVGPU_GFXP */
#ifdef CONFIG_NVGPU_CILP
void gp10b_ctxsw_prog_set_compute_preemption_mode_cilp(struct gk20a *g,
	struct nvgpu_mem *ctx_mem);
#endif
#ifdef CONFIG_NVGPU_DEBUGGER
void gp10b_ctxsw_prog_set_pmu_options_boost_clock_frequencies(struct gk20a *g,
	struct nvgpu_mem *ctx_mem, u32 boosted_ctx);
#endif /* CONFIG_NVGPU_DEBUGGER */
#ifdef CONFIG_DEBUG_FS
void gp10b_ctxsw_prog_dump_ctxsw_stats(struct gk20a *g,
	struct nvgpu_mem *ctx_mem);
#endif

#endif /* NVGPU_CTXSW_PROG_GP10B_H */
