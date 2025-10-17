/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_CE_GA10B_H
#define NVGPU_CE_GA10B_H

struct gk20a;
#ifdef CONFIG_NVGPU_NONSTALL_INTR
void ga10b_ce_init_hw(struct  gk20a *g);
#endif
void ga10b_ce_intr_enable(struct gk20a *g, bool enable);
void ga10b_ce_stall_isr(struct gk20a *g, u32 inst_id, u32 pri_base,
				bool *needs_rc, bool *needs_quiesce);
void ga10b_ce_intr_retrigger(struct gk20a *g, u32 inst_id);
void ga10b_ce_request_idle(struct gk20a *g);

#endif /* NVGPU_CE_GA10B_H */
