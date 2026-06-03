/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FIFO_MMU_FAULT_GP10B_H
#define NVGPU_FIFO_MMU_FAULT_GP10B_H

struct gk20a;
struct mmu_fault_info;

void gp10b_fifo_get_mmu_fault_info(struct gk20a *g, u32 mmu_fault_id,
	struct mmu_fault_info *mmufault);
void gp10b_fifo_get_mmu_fault_desc(struct mmu_fault_info *mmufault);
void gp10b_fifo_get_mmu_fault_client_desc(struct mmu_fault_info *mmufault);

#endif /* NVGPU_FIFO_MMU_FAULT_GP10B_H */
