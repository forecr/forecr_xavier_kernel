/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FIFO_INTR_GV11B_H
#define NVGPU_FIFO_INTR_GV11B_H

#include <nvgpu/types.h>

/*
* ERROR_CODE_BAD_TSG indicates that Host encountered a badly formed TSG header
* or a badly formed channel type runlist entry in the runlist. This is typically
* caused by encountering a new TSG entry in the middle of a TSG definition.
* A channel type entry having wrong runqueue selector can also cause this.
* Additionally this error code can indicate when a channel is encountered on
* the runlist which is outside of a TSG.
*/
#define SCHED_ERROR_CODE_BAD_TSG           0x00000020U

struct gk20a;

void gv11b_fifo_intr_0_enable(struct gk20a *g, bool enable);
void gv11b_fifo_intr_0_isr(struct gk20a *g);

bool gv11b_fifo_handle_sched_error(struct gk20a *g);

void gv11b_fifo_intr_set_recover_mask(struct gk20a *g);
void gv11b_fifo_intr_unset_recover_mask(struct gk20a *g);

#endif /* NVGPU_FIFO_INTR_GV11B_H */
