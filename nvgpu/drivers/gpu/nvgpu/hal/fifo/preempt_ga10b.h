/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef FIFO_PREEMPT_GA10B_H
#define FIFO_PREEMPT_GA10B_H

#include <nvgpu/types.h>

struct gk20a;

u32 ga10b_runlist_preempt_reg(void);
void ga10b_fifo_preempt_trigger(struct gk20a *g,
	u32 runlist_id, u32 id, unsigned int id_type);

#endif /* FIFO_PREEMPT_GA10B_H */
