// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>

#include "hal/fifo/ramin_ga10b.h"

#include <nvgpu/hw/ga10b/hw_ram_ga10b.h>

void ga10b_ramin_init_pdb(struct gk20a *g, struct nvgpu_mem *inst_block,
		u64 pdb_addr, struct nvgpu_mem *pdb_mem)
{
	u32 pdb_addr_lo = u64_lo32(pdb_addr >> ram_in_base_shift_v());
	u32 pdb_addr_hi = u64_hi32(pdb_addr);

	nvgpu_log_info(g, "pde pa=0x%llx", pdb_addr);

	nvgpu_mem_wr32(g, inst_block, ram_in_page_dir_base_lo_w(),
		nvgpu_aperture_mask(g, pdb_mem,
				ram_in_page_dir_base_target_sys_mem_ncoh_f(),
				ram_in_page_dir_base_target_sys_mem_coh_f(),
				ram_in_page_dir_base_target_vid_mem_f()) |
		ram_in_page_dir_base_vol_true_f() |
		ram_in_big_page_size_64kb_f() |
		ram_in_page_dir_base_lo_f(pdb_addr_lo) |
		ram_in_use_ver2_pt_format_true_f());

	nvgpu_mem_wr32(g, inst_block, ram_in_page_dir_base_hi_w(),
		ram_in_page_dir_base_hi_f(pdb_addr_hi));
}

void ga10b_ramin_set_magic_value(struct gk20a *g, struct nvgpu_mem *inst_block)
{
	nvgpu_mem_wr32(g, inst_block, ram_in_engine_fw_magic_value_w(),
			ram_in_engine_fw_magic_value_engine_fw_magic_value_v());
}
