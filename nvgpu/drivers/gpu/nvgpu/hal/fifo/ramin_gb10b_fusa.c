// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>

#include <nvgpu/hw/gb10b/hw_ram_gb10b.h>

#include "ramin_gb10b.h"

void gb10b_ramin_set_gr_ptr(struct gk20a *g,
		struct nvgpu_mem *inst_block, u64 gpu_va)
{
	u32 addr_lo = u64_lo32(gpu_va) >> ram_in_base_shift_v();
	u32 addr_hi = u64_hi32(gpu_va);

	/* point this address to engine_wfi_ptr */
	nvgpu_mem_wr32(g, inst_block, ram_in_engine_wfi_target_w(),
		ram_in_engine_cs_wfi_v() |
		ram_in_engine_wfi_mode_f(ram_in_engine_wfi_mode_virtual_v()) |
		ram_in_engine_wfi_ptr_lo_f(addr_lo));

	nvgpu_mem_wr32(g, inst_block, ram_in_engine_wfi_ptr_hi_w(),
		ram_in_engine_wfi_ptr_hi_f(addr_hi));
}

