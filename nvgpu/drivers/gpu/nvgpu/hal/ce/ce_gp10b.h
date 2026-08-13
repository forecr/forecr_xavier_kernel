/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_CE_GP10B_H
#define NVGPU_CE_GP10B_H

#include <nvgpu/types.h>

struct gk20a;

void gp10b_ce_stall_isr(struct gk20a *g, u32 inst_id, u32 pri_base,
				bool *needs_rc, bool *needs_quiesce);
#ifdef CONFIG_NVGPU_NONSTALL_INTR
u32 gp10b_ce_nonstall_isr(struct gk20a *g, u32 inst_id, u32 pri_base);
#endif /* NVGPU_HAL_NON_FUSA */

#endif /* NVGPU_CE_GP10B_H */
