/*
 * Copyright (c) 2015-2022, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>

#include <nvgpu/hw/gv11b/hw_ram_gv11b.h>

#include "hal/fifo/ramin_gv11b.h"

void gv11b_ramin_set_gr_ptr(struct gk20a *g,
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

void gv11b_ramin_set_subctx_pdb_info(struct gk20a *g,
		u32 subctx_id, struct nvgpu_mem *pdb_mem,
		bool replayable, bool add, u32 *subctx_pdb_map)
{
	u32 format_word = 0;
	u32 pdb_addr_lo = 0;
	u32 pdb_addr_hi = 0;
	u64 pdb_addr;
	u32 aperture;

	if (add) {
		aperture = nvgpu_aperture_mask(g, pdb_mem,
				ram_in_sc_page_dir_base_target_sys_mem_ncoh_v(),
				ram_in_sc_page_dir_base_target_sys_mem_coh_v(),
				ram_in_sc_page_dir_base_target_vid_mem_v());

		pdb_addr = nvgpu_mem_get_addr(g, pdb_mem);
		pdb_addr_lo = u64_lo32(pdb_addr >> ram_in_base_shift_v());
		pdb_addr_hi = u64_hi32(pdb_addr);
		format_word = ram_in_sc_page_dir_base_target_f(aperture, 0U) |
			ram_in_sc_page_dir_base_vol_f(
			ram_in_sc_page_dir_base_vol_true_v(), 0U) |
			ram_in_sc_use_ver2_pt_format_f(1U, 0U) |
			ram_in_sc_big_page_size_f(1U, 0U) |
			ram_in_sc_page_dir_base_lo_0_f(pdb_addr_lo);

		if (replayable) {
			format_word |=
				ram_in_sc_page_dir_base_fault_replay_tex_f(1U, 0U) |
				ram_in_sc_page_dir_base_fault_replay_gcc_f(1U, 0U);
		}
	} else {
		aperture = ram_in_sc_page_dir_base_target_invalid_v();
		format_word = ram_in_sc_page_dir_base_target_f(aperture, 0U);
	}

	nvgpu_log(g, gpu_dbg_info, "%s subctx[%u] pdb info lo %x hi %x",
			add ? "add" : "remove", subctx_id,
			format_word, pdb_addr_hi);

	subctx_pdb_map[subctx_id * 4U] = format_word;
	subctx_pdb_map[(subctx_id * 4U) + 1U] = pdb_addr_hi;
}

void gv11b_ramin_init_subctx_pdb_map(struct gk20a *g,
		u32 *subctx_pdb_map)
{
	u32 max_subctx_count = g->ops.gr.init.get_max_subctx_count();
	u32 i;

	/* Initially, all subcontexts are invalid in the TSG. */
	for (i = 0; i < max_subctx_count; i++) {
		gv11b_ramin_set_subctx_pdb_info(g, i, NULL, false, false,
						subctx_pdb_map);
	}
}

void gv11b_ramin_init_subctx_valid_mask(struct gk20a *g,
		struct nvgpu_mem *inst_block, unsigned long *valid_subctx_mask)
{
	u32 max_subctx_count = g->ops.gr.init.get_max_subctx_count();
	u32 id;

	for (id = 0U; id < max_subctx_count; id += 32U) {
		u32 subctx_mask = ((u32 *)valid_subctx_mask)[id / 32U];

		nvgpu_mem_wr32(g, inst_block,
				ram_in_sc_pdb_valid_long_w(id), subctx_mask);

		nvgpu_log(g, gpu_dbg_info | gpu_dbg_mig,
			"id[%d] max_subctx_count[%u] subctx_mask[%x] ",
			id, max_subctx_count, subctx_mask);
	}
}

void gv11b_ramin_init_subctx_pdb(struct gk20a *g,
		struct nvgpu_mem *inst_block, u32 *subctx_pdb_map)
{
	u32 max_subctx_count = g->ops.gr.init.get_max_subctx_count();
	u32 size = max_subctx_count * 4U * 4U;

	nvgpu_mem_wr_n(g, inst_block, ram_in_sc_page_dir_base_vol_w(0) * 4U,
		subctx_pdb_map, size);
}

void gv11b_ramin_set_eng_method_buffer(struct gk20a *g,
		struct nvgpu_mem *inst_block, u64 gpu_va)
{
	u32 addr_lo = u64_lo32(gpu_va);
	u32 addr_hi = u64_hi32(gpu_va);

	nvgpu_mem_wr32(g, inst_block, ram_in_eng_method_buffer_addr_lo_w(),
			addr_lo);
	nvgpu_mem_wr32(g, inst_block, ram_in_eng_method_buffer_addr_hi_w(),
			addr_hi);
}

void gv11b_ramin_init_pdb(struct gk20a *g, struct nvgpu_mem *inst_block,
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
