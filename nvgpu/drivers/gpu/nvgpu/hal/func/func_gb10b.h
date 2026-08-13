/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef FUNC_GB10B_H
#define FUNC_GB10B_H

int gb10b_func_tlb_invalidate(struct gk20a *g, struct nvgpu_mem *pdb);
int gb10b_func_tlb_flush(struct gk20a *g);
void gb10b_func_l2_invalidate(struct gk20a *g);
void gb10b_func_l2_invalidate_locked(struct gk20a *g);
int gb10b_func_bar1_bind(struct gk20a *g, struct nvgpu_mem *bar1_inst);
int gb10b_func_bar2_bind(struct gk20a *g, struct nvgpu_mem *bar2_inst);
void gb10b_func_write_mmu_fault_buffer_lo_hi(struct gk20a *g, u32 index,
		u32 addr_lo, u32 addr_hi);
u32 gb10b_func_read_mmu_fault_buffer_get(struct gk20a *g, u32 index);
void gb10b_func_write_mmu_fault_buffer_get(struct gk20a *g, u32 index,
		u32 reg_val);
u32 gb10b_func_read_mmu_fault_buffer_put(struct gk20a *g, u32 index);
u32 gb10b_func_read_mmu_fault_buffer_size(struct gk20a *g, u32 index);
void gb10b_func_write_mmu_fault_buffer_size(struct gk20a *g, u32 index,
		u32 reg_val);
void gb10b_func_read_mmu_fault_addr_lo_hi(struct gk20a *g,
		u32 *addr_lo, u32 *addr_hi);
void gb10b_func_read_mmu_fault_inst_lo_hi(struct gk20a *g,
		u32 *inst_lo, u32 *inst_hi);
u32 gb10b_func_read_mmu_fault_info(struct gk20a *g);
u32 gb10b_func_read_mmu_fault_status(struct gk20a *g);
void gb10b_func_write_mmu_fault_status(struct gk20a *g, u32 reg_val);

#endif
