/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_CE2_GK20A_H
#define NVGPU_CE2_GK20A_H

#include <nvgpu/types.h>

struct gk20a;

void gk20a_ce2_stall_isr(struct gk20a *g, u32 inst_id, u32 pri_base,
				bool *needs_rc, bool *needs_quiesce);
u32 gk20a_ce2_nonstall_isr(struct gk20a *g, u32 inst_id, u32 pri_base);

#endif /*NVGPU_CE2_GK20A_H*/
