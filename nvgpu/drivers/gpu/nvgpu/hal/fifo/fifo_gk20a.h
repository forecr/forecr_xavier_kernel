/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_FIFO_GK20A_H
#define NVGPU_FIFO_GK20A_H

#include <nvgpu/types.h>

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
int gk20a_init_fifo_reset_enable_hw(struct gk20a *g);
int gk20a_init_fifo_setup_hw(struct gk20a *g);
void gk20a_fifo_bar1_snooping_disable(struct gk20a *g);
#endif
u32 gk20a_fifo_get_runlist_timeslice(struct gk20a *g);
u32 gk20a_fifo_get_pb_timeslice(struct gk20a *g);
bool gk20a_fifo_find_pbdma_for_runlist(struct gk20a *g,
				       u32 runlist_id, u32 *pbdma_mask);

#endif /* NVGPU_FIFO_GK20A_H */
