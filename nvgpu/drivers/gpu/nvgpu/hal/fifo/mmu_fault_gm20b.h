/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FIFO_MMU_FAULT_GM20B_H
#define NVGPU_FIFO_MMU_FAULT_GM20B_H

struct gk20a;
struct mmu_fault_info;

void gm20b_fifo_trigger_mmu_fault(struct gk20a *g,
		unsigned long engine_ids_bitmask);
void gm20b_fifo_get_mmu_fault_gpc_desc(struct mmu_fault_info *mmufault);

#endif /* NVGPU_FIFO_MMU_FAULT_GM20B_H */
