// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/utils.h>
#include <nvgpu/nvgpu_mem.h>

#include "ctxsw_prog_gm20b.h"
#include "ctxsw_prog_gv11b.h"

#include <nvgpu/hw/gv11b/hw_ctxsw_prog_gv11b.h>

void gv11b_ctxsw_prog_set_context_buffer_ptr(struct gk20a *g,
	struct nvgpu_mem *ctx_mem, u64 addr)
{
	nvgpu_mem_wr(g, ctx_mem,
		ctxsw_prog_main_image_context_buffer_ptr_hi_o(),
		u64_hi32(addr));
	nvgpu_mem_wr(g, ctx_mem,
		ctxsw_prog_main_image_context_buffer_ptr_o(),
		u64_lo32(addr));
}

void gv11b_ctxsw_prog_set_type_per_veid_header(struct gk20a *g,
	struct nvgpu_mem *ctx_mem)
{
	nvgpu_mem_wr(g, ctx_mem,
		ctxsw_prog_main_image_ctl_o(),
		ctxsw_prog_main_image_ctl_type_per_veid_header_v());
}

#if defined(CONFIG_NVGPU_GRAPHICS) && defined(CONFIG_NVGPU_ZCULL)
void gv11b_ctxsw_prog_set_zcull_ptr(struct gk20a *g, struct nvgpu_mem *ctx_mem,
	u64 addr)
{
	addr = addr >> 8;
	nvgpu_mem_wr(g, ctx_mem, ctxsw_prog_main_image_zcull_ptr_o(),
		u64_lo32(addr));
	nvgpu_mem_wr(g, ctx_mem, ctxsw_prog_main_image_zcull_ptr_hi_o(),
		u64_hi32(addr));
}
#endif

#ifdef CONFIG_NVGPU_GFXP
void gv11b_ctxsw_prog_set_full_preemption_ptr(struct gk20a *g,
	struct nvgpu_mem *ctx_mem, u64 addr)
{
	addr = addr >> 8;
	nvgpu_mem_wr(g, ctx_mem,
		ctxsw_prog_main_image_full_preemption_ptr_o(),
		u64_lo32(addr));
	nvgpu_mem_wr(g, ctx_mem,
		ctxsw_prog_main_image_full_preemption_ptr_hi_o(),
		u64_hi32(addr));
}

void gv11b_ctxsw_prog_set_full_preemption_ptr_veid0(struct gk20a *g,
	struct nvgpu_mem *ctx_mem, u64 addr)
{
	addr = addr >> 8;
	nvgpu_mem_wr(g, ctx_mem,
		ctxsw_prog_main_image_full_preemption_ptr_veid0_o(),
		u64_lo32(addr));
	nvgpu_mem_wr(g, ctx_mem,
		ctxsw_prog_main_image_full_preemption_ptr_veid0_hi_o(),
		u64_hi32(addr));
}
#endif /* CONFIG_NVGPU_GFXP */
