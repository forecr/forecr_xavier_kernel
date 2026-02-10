/*
 * Copyright (c) 2019-2022, NVIDIA CORPORATION.  All rights reserved.
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

#include <nvgpu/gk20a.h>
#include <nvgpu/fifo.h>
#include <nvgpu/gmmu.h>
#include <nvgpu/mm.h>

#include "mm_gv11b.h"

void gv11b_mm_init_inst_block(struct nvgpu_mem *inst_block,
		struct vm_gk20a *vm, u32 big_page_size)
{
	struct gk20a *g = gk20a_from_vm(vm);
	u64 pdb_addr = nvgpu_pd_gpu_addr(g, &vm->pdb);

	nvgpu_log_info(g, "inst block phys = 0x%llx, kv = 0x%p",
		nvgpu_inst_block_addr(g, inst_block), inst_block->cpu_va);

	g->ops.ramin.init_pdb(g, inst_block, pdb_addr, vm->pdb.mem);

	if ((big_page_size != 0U) && (g->ops.ramin.set_big_page_size != NULL)) {
		g->ops.ramin.set_big_page_size(g, inst_block, big_page_size);
	}
}

int gv11b_mm_init_inst_block_core(struct nvgpu_mem *inst_block,
		struct vm_gk20a *vm, u32 big_page_size)
{
	struct gk20a *g = gk20a_from_vm(vm);
	u32 max_subctx_count = g->ops.gr.init.get_max_subctx_count();
	unsigned long *valid_subctxs;
	u32 *subctx_pdb_map;

	subctx_pdb_map = nvgpu_kzalloc(g, max_subctx_count * sizeof(u32) * 4U);
	if (subctx_pdb_map == NULL) {
		nvgpu_err(g, "subctx_pdb_map alloc failed");
		return -ENOMEM;
	}

	valid_subctxs = nvgpu_kzalloc(g,
				BITS_TO_LONGS(max_subctx_count) *
				sizeof(unsigned long));
	if (valid_subctxs == NULL) {
		nvgpu_err(g, "valid_subctxs bitmask alloc failed");
		nvgpu_kfree(g, subctx_pdb_map);
		return -ENOMEM;
	}

	gv11b_mm_init_inst_block(inst_block, vm, big_page_size);

	/* Program subctx pdb info in the instance block */
	g->ops.ramin.init_subctx_pdb_map(g, subctx_pdb_map);
	g->ops.ramin.set_subctx_pdb_info(g, CHANNEL_INFO_VEID0, vm->pdb.mem,
					 false, true, subctx_pdb_map);
	g->ops.ramin.init_subctx_pdb(g, inst_block, subctx_pdb_map);

	/*
	 * Program subctx pdb valid mask in the instance block.
	 * Only subctx 0 is valid here.
	 */
	nvgpu_set_bit(CHANNEL_INFO_VEID0, valid_subctxs);
	g->ops.ramin.init_subctx_mask(g, inst_block, valid_subctxs);

	nvgpu_kfree(g, valid_subctxs);
	nvgpu_kfree(g, subctx_pdb_map);

	return 0;
}

bool gv11b_mm_is_bar1_supported(struct gk20a *g)
{
	(void)g;
	return false;
}
