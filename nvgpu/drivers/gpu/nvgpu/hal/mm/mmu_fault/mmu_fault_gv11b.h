/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_MMU_FAULT_MMU_FAULT_GV11B_H
#define HAL_MM_MMU_FAULT_MMU_FAULT_GV11B_H

struct gk20a;
struct mmu_fault_info;

void gv11b_mm_mmu_fault_info_mem_destroy(struct gk20a *g);
void gv11b_mm_mmu_fault_disable_hw(struct gk20a *g);

void gv11b_mm_mmu_fault_setup_hw(struct gk20a *g);
int  gv11b_mm_mmu_fault_setup_sw(struct gk20a *g);

void gv11b_mm_mmu_fault_handle_nonreplay_replay_fault(struct gk20a *g,
		 u32 fault_status, u32 index);
void gv11b_mm_mmu_fault_handle_mmu_fault_common(struct gk20a *g,
		 struct mmu_fault_info *mmufault, u32 *invalidate_replay_val);
void gv11b_mm_mmu_fault_handle_other_fault_notify(struct gk20a *g,
						  u32 fault_status);

void gv11b_mm_mmu_fault_parse_mmu_fault_info(struct mmu_fault_info *mmufault);
u32 gv11b_gmmu_fault_mmu_eng_id_ce0_val(void);
void gv11b_fb_copy_from_hw_fault_buf(struct gk20a *g,
	 struct nvgpu_mem *mem, u32 offset, struct mmu_fault_info *mmufault);
bool gv11b_mm_mmu_fault_handle_mmu_fault_refch(struct gk20a *g,
		struct mmu_fault_info *mmufault, u32 *id_ptr,
		unsigned int *id_type_ptr, unsigned int *rc_type_ptr);

#endif
