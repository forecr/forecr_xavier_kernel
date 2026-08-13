// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bug.h>
#include <nvgpu/channel.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/nvgpu_mem.h>

#include "userd_ga10b.h"

#include <nvgpu/hw/ga10b/hw_ram_ga10b.h>

#ifdef CONFIG_NVGPU_USERD
void ga10b_userd_init_mem(struct gk20a *g, struct nvgpu_channel *c)
{
	struct nvgpu_mem *mem = c->userd_mem;
	u32 offset = c->userd_offset / U32(sizeof(u32));

	nvgpu_log_fn(g, " ");

	nvgpu_mem_wr32(g, mem, offset + ram_userd_put_w(), 0);
	nvgpu_mem_wr32(g, mem, offset + ram_userd_get_w(), 0);
	nvgpu_mem_wr32(g, mem, offset + ram_userd_ref_w(), 0);
	nvgpu_mem_wr32(g, mem, offset + ram_userd_put_hi_w(), 0);
	nvgpu_mem_wr32(g, mem, offset + ram_userd_top_level_get_w(), 0);
	nvgpu_mem_wr32(g, mem, offset + ram_userd_top_level_get_hi_w(), 0);
	nvgpu_mem_wr32(g, mem, offset + ram_userd_get_hi_w(), 0);
	nvgpu_mem_wr32(g, mem, offset + ram_userd_gp_get_w(), 0);
	nvgpu_mem_wr32(g, mem, offset + ram_userd_gp_put_w(), 0);
}
#endif /* CONFIG_NVGPU_USERD */
