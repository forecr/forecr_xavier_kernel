/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FIFO_INTR_GK20A_H
#define NVGPU_FIFO_INTR_GK20A_H

#include <nvgpu/types.h>

struct gk20a;

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gk20a_fifo_intr_0_enable(struct gk20a *g, bool enable);
void gk20a_fifo_intr_0_isr(struct gk20a *g);
bool gk20a_fifo_handle_sched_error(struct gk20a *g);
bool gk20a_fifo_is_mmu_fault_pending(struct gk20a *g);
void gk20a_fifo_intr_set_recover_mask(struct gk20a *g);
void gk20a_fifo_intr_unset_recover_mask(struct gk20a *g);
#endif

void gk20a_fifo_intr_1_enable(struct gk20a *g, bool enable);
u32  gk20a_fifo_intr_1_isr(struct gk20a *g);

void gk20a_fifo_intr_handle_chsw_error(struct gk20a *g);
void gk20a_fifo_intr_handle_runlist_event(struct gk20a *g);
u32  gk20a_fifo_pbdma_isr(struct gk20a *g);

#endif /* NVGPU_FIFO_INTR_GK20A_H */
