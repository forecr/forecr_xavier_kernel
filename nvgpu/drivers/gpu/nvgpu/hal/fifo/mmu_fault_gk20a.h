/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FIFO_MMU_FAULT_GK20A_H
#define NVGPU_FIFO_MMU_FAULT_GK20A_H

#include <nvgpu/types.h>

struct gk20a;
struct mmu_fault_info;

void gk20a_fifo_get_mmu_fault_desc(struct mmu_fault_info *mmufault);
void gk20a_fifo_get_mmu_fault_client_desc(struct mmu_fault_info *mmufault);
void gk20a_fifo_get_mmu_fault_gpc_desc(struct mmu_fault_info *mmufault);
void gk20a_fifo_get_mmu_fault_info(struct gk20a *g, u32 mmu_fault_id,
	struct mmu_fault_info *mmufault);

void gk20a_fifo_mmu_fault_info_dump(struct gk20a *g, u32 engine_id,
	u32 mmu_fault_id, bool fake_fault, struct mmu_fault_info *mmufault);

void gk20a_fifo_handle_dropped_mmu_fault(struct gk20a *g);

void gk20a_fifo_handle_mmu_fault(struct gk20a *g, u32 mmu_fault_engines,
	u32 hw_id, bool id_is_tsg);

void gk20a_fifo_handle_mmu_fault_locked(struct gk20a *g, u32 mmu_fault_engines,
	u32 hw_id, bool id_is_tsg);

#endif /* NVGPU_FIFO_MMU_FAULT_GK20A_H */
