/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_MMU_FAULT_MMU_FAULT_GA10B_H
#define HAL_MM_MMU_FAULT_MMU_FAULT_GA10B_H

struct mmu_fault_info;

void ga10b_mm_mmu_fault_parse_mmu_fault_info(struct mmu_fault_info *mmufault);

#endif /* HAL_MM_MMU_FAULT_MMU_FAULT_GA10B_H */
