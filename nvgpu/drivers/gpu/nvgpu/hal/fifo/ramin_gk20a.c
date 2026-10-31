// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/channel.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>

#include "hal/fifo/ramin_gk20a.h"

#include <nvgpu/hw/gk20a/hw_ram_gk20a.h>

void gk20a_ramin_set_gr_ptr(struct gk20a *g,
		struct nvgpu_mem *inst_block, u64 gpu_va)
{
	u32 addr_lo = u64_lo32(gpu_va) >> ram_in_base_shift_v();
	u32 addr_hi = u64_hi32(gpu_va);

	nvgpu_mem_wr32(g, inst_block, ram_in_gr_wfi_target_w(),
		 ram_in_gr_cs_wfi_f() | ram_in_gr_wfi_mode_virtual_f() |
		 ram_in_gr_wfi_ptr_lo_f(addr_lo));

	nvgpu_mem_wr32(g, inst_block, ram_in_gr_wfi_ptr_hi_w(),
		 ram_in_gr_wfi_ptr_hi_f(addr_hi));
}

void gk20a_ramin_init_pdb(struct gk20a *g, struct nvgpu_mem *inst_block,
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
		       ram_in_page_dir_base_lo_f(pdb_addr_lo));

	nvgpu_mem_wr32(g, inst_block, ram_in_page_dir_base_hi_w(),
		ram_in_page_dir_base_hi_f(pdb_addr_hi));
}

void gk20a_ramin_set_adr_limit(struct gk20a *g,
		struct nvgpu_mem *inst_block, u64 va_limit)
{
	nvgpu_mem_wr32(g, inst_block, ram_in_adr_limit_lo_w(),
		u64_lo32(va_limit - 1U) & ~0xfffU);

	nvgpu_mem_wr32(g, inst_block, ram_in_adr_limit_hi_w(),
		ram_in_adr_limit_hi_f(u64_hi32(va_limit - 1U)));
}
