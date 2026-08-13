/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RAMIN_GV11B_H
#define NVGPU_RAMIN_GV11B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_mem;

void gv11b_ramin_set_gr_ptr(struct gk20a *g,
		struct nvgpu_mem *inst_block, u64 gpu_va);
void gv11b_ramin_set_subctx_pdb_info(struct gk20a *g,
		u32 subctx_id, struct nvgpu_mem *pdb_mem,
		bool replayable, bool add, u32 *subctx_pdb_map);
void gv11b_ramin_init_subctx_pdb_map(struct gk20a *g,
		u32 *subctx_pdb_map);
void gv11b_ramin_init_subctx_valid_mask(struct gk20a *g,
		struct nvgpu_mem *inst_block, unsigned long *valid_subctx_mask);
void gv11b_ramin_init_subctx_pdb(struct gk20a *g,
		struct nvgpu_mem *inst_block, u32 *subctx_pdb_map);
void gv11b_ramin_set_eng_method_buffer(struct gk20a *g,
		struct nvgpu_mem *inst_block, u64 gpu_va);
void gv11b_ramin_init_pdb(struct gk20a *g, struct nvgpu_mem *inst_block,
		u64 pdb_addr, struct nvgpu_mem *pdb_mem);

#endif /* NVGPU_RAMIN_GV11B_H */
