// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/channel.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>

#include "hal/fifo/ramin_gm20b.h"

#include <nvgpu/hw/gm20b/hw_ram_gm20b.h>

void gm20b_ramin_set_big_page_size(struct gk20a *g,
		struct nvgpu_mem *mem, u32 size)
{
	u32 val;

	nvgpu_log_fn(g, " ");

	nvgpu_log_info(g, "big page size %u", size);
	val = nvgpu_mem_rd32(g, mem, ram_in_big_page_size_w());
	val &= ~ram_in_big_page_size_m();

	if (size == SZ_64K) {
		val |= ram_in_big_page_size_64kb_f();
	} else {
#ifndef CONFIG_NVGPU_HAL_NON_FUSA
		nvgpu_err(g, "only SZ_64K is allowed");
		return;
#else
		val |= ram_in_big_page_size_128kb_f();
#endif
	}

	nvgpu_mem_wr32(g, mem, ram_in_big_page_size_w(), val);
	nvgpu_log_fn(g, "done");
}
