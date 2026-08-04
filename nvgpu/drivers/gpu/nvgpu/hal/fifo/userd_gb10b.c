// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bug.h>
#include <nvgpu/channel.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/channel_sync_semaphore.h>

#include "userd_gb10b.h"

#include <nvgpu/hw/gb10b/hw_ram_gb10b.h>

#ifdef CONFIG_NVGPU_USERD
void gb10b_userd_init_mem(struct gk20a *g, struct nvgpu_channel *c)
{
	struct nvgpu_mem *mem = c->userd_mem;
	u32 offset = c->userd_offset / U32(sizeof(u32));

	nvgpu_log_fn(g, " ");

	nvgpu_mem_wr32(g, mem, offset + ram_userd_gp_put_w(), 0);
}

u32 gb10b_sema_based_gp_get(struct gk20a *g, struct nvgpu_channel *ch)
{
	nvgpu_assert(nvgpu_is_enabled(g, NVGPU_SUPPORT_SEMA_BASED_GPFIFO_GET));
	nvgpu_channel_update_gpfifo_get(ch);

	return ch->gpfifo.get;
}

#endif /* CONFIG_NVGPU_USERD */
