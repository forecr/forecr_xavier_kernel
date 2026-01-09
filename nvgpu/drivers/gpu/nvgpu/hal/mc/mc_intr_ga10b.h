/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_MC_INTR_GA10B_H
#define NVGPU_MC_INTR_GA10B_H

#include <nvgpu/types.h>

struct gk20a;

void ga10b_intr_host2soc_0_unit_config(struct gk20a *g, u32 unit, bool enable);
u32  ga10b_intr_host2soc_0(struct gk20a *g);
void ga10b_intr_host2soc_0_pause(struct gk20a *g);
void ga10b_intr_host2soc_0_resume(struct gk20a *g);
u32  ga10b_intr_isr_host2soc_0(struct gk20a *g);

#ifdef CONFIG_NVGPU_NON_FUSA
void ga10b_intr_log_pending_intrs(struct gk20a *g);
#endif /* CONFIG_NVGPU_NON_FUSA */

void ga10b_intr_mask_top(struct gk20a *g);
bool ga10b_intr_is_mmu_fault_pending(struct gk20a *g);

void ga10b_intr_stall_unit_config(struct gk20a *g, u32 unit, bool enable);
u32  ga10b_intr_stall(struct gk20a *g);
void ga10b_intr_stall_pause(struct gk20a *g);
void ga10b_intr_stall_resume(struct gk20a *g);
void ga10b_intr_isr_stall(struct gk20a *g);

u32 ga10b_intr_get_eng_nonstall_base_vector(struct gk20a *g);
u32 ga10b_intr_get_eng_stall_base_vector(struct gk20a *g);
bool ga10b_intr_is_stall_and_eng_intr_pending(struct gk20a *g, u32 engine_id,
			u32 *eng_intr_pending);
bool ga10b_mc_intr_get_unit_info(struct gk20a *g, u32 unit);

#endif /* NVGPU_MC_INTR_GA10B_H */
