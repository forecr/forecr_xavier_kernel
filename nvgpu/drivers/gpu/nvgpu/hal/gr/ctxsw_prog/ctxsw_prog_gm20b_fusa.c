// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/utils.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/static_analysis.h>

#include "ctxsw_prog_gm20b.h"

#include <nvgpu/hw/gm20b/hw_ctxsw_prog_gm20b.h>

u32 gm20b_ctxsw_prog_hw_get_fecs_header_size(void)
{
	return ctxsw_prog_fecs_header_v();
}

u32 gm20b_ctxsw_prog_get_patch_count(struct gk20a *g, struct nvgpu_mem *ctx_mem)
{
	return nvgpu_mem_rd(g, ctx_mem, ctxsw_prog_main_image_patch_count_o());
}

void gm20b_ctxsw_prog_set_patch_count(struct gk20a *g,
	struct nvgpu_mem *ctx_mem, u32 count)
{
	nvgpu_mem_wr(g, ctx_mem, ctxsw_prog_main_image_patch_count_o(), count);
}

void gm20b_ctxsw_prog_set_patch_addr(struct gk20a *g,
	struct nvgpu_mem *ctx_mem, u64 addr)
{
	nvgpu_mem_wr(g, ctx_mem,
		ctxsw_prog_main_image_patch_adr_lo_o(), u64_lo32(addr));
	nvgpu_mem_wr(g, ctx_mem,
		ctxsw_prog_main_image_patch_adr_hi_o(), u64_hi32(addr));
}

u32 gm20b_ctxsw_prog_get_main_image_ctx_id(struct gk20a *g,
	struct nvgpu_mem *ctx_mem)
{
	return nvgpu_mem_rd(g, ctx_mem, ctxsw_prog_main_image_context_id_o());
}

#if defined(CONFIG_NVGPU_GRAPHICS) && defined(CONFIG_NVGPU_ZCULL)
void gm20b_ctxsw_prog_set_zcull_ptr(struct gk20a *g, struct nvgpu_mem *ctx_mem,
	u64 addr)
{
	addr = addr >> 8;
	nvgpu_mem_wr(g, ctx_mem, ctxsw_prog_main_image_zcull_ptr_o(),
		u64_lo32(addr));
}

void gm20b_ctxsw_prog_set_zcull(struct gk20a *g, struct nvgpu_mem *ctx_mem,
	u32 mode)
{
	nvgpu_mem_wr(g, ctx_mem, ctxsw_prog_main_image_zcull_o(), mode);
}

void gm20b_ctxsw_prog_set_zcull_mode_no_ctxsw(struct gk20a *g,
	struct nvgpu_mem *ctx_mem)
{
	nvgpu_mem_wr(g, ctx_mem, ctxsw_prog_main_image_zcull_o(),
			ctxsw_prog_main_image_zcull_mode_no_ctxsw_v());
}

bool gm20b_ctxsw_prog_is_zcull_mode_separate_buffer(u32 mode)
{
	return mode == ctxsw_prog_main_image_zcull_mode_separate_buffer_v();
}
#endif
