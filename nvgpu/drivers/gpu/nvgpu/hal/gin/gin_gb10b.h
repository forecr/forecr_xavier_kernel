/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GIN_GB10B_H
#define NVGPU_GIN_GB10B_H

#include <nvgpu/types.h>

struct gk20a;

void gb10b_gin_init(struct gk20a *g);
u32 gb10b_gin_get_intr_ctrl_msg(struct gk20a *g, u32 vector,
        bool cpu_enable, bool gsp_enable);
u32 gb10b_gin_get_vector_from_intr_ctrl_msg(struct gk20a *g, u32 intr_ctrl_msg);
u64 gb10b_gin_read_intr_top(struct gk20a *g, u64 intr_top_mask);
void gb10b_gin_write_intr_top_en_clear(struct gk20a *g, u64 val);
void gb10b_gin_write_intr_top_en_set(struct gk20a *g, u64 val);
u64 gb10b_gin_subtree_leaf_status(struct gk20a *g, u32 subtree);
void gb10b_gin_subtree_leaf_clear(struct gk20a *g, u32 subtree, u64 leaf);
void gb10b_gin_subtree_leaf_enable(struct gk20a *g, u32 subtree, u64 leaf);
void gb10b_gin_subtree_leaf_disable(struct gk20a *g, u32 subtree, u64 leaf);
u32 gb10b_gin_get_num_vectors(struct gk20a *g);
bool gb10b_gin_is_mmu_fault_pending(struct gk20a *g);
bool gb10b_gin_is_stall_and_eng_intr_pending(struct gk20a *g, u32 engine_id,
		u32 *eng_intr_pending);

#endif /* NVGPU_GIN_GB10B_H */
