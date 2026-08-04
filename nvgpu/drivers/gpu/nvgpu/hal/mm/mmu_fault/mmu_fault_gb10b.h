/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_MMU_FAULT_MMU_FAULT_GB10B_H
#define HAL_MM_MMU_FAULT_MMU_FAULT_GB10B_H

struct mmu_fault_info;

void gb10b_mm_mmu_fault_parse_mmu_fault_info(struct mmu_fault_info *mmufault);
u32 gb10b_gmmu_fault_mmu_eng_id_ce0_val(void);

#endif /* HAL_MM_MMU_FAULT_MMU_FAULT_GB10B_H */
