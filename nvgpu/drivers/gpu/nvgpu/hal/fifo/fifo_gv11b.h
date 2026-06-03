/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FIFO_GV11B_H
#define NVGPU_FIFO_GV11B_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * FIFO HAL GV11B.
 */

struct gk20a;

int gv11b_init_fifo_reset_enable_hw(struct gk20a *g);
/**
 * @brief Update userd configuration and read FIFO chip settings.
 *
 * @param g [in]	The GPU driver struct.
 *                      - The function does not perform g parameter validation.
 *
 * Get fifo pointer from GPU pointer as g->fifo.
 * Set maximum number of VEIDs supported by chip in #nvgpu_fifo.max_subctx_count
 * using \ref gops_gr_init.get_max_subctx_count
 * "gops_gr_init.get_max_subctx_count()".
 * Set fifo_userd_writeback_timer_f() of register fifo_userd_writeback_r() to
 * fifo_userd_writeback_timer_100us_v().
 *
 * For gv11b, this function is mapped to \ref gops_fifo.init_fifo_setup_hw
 * "gops_fifo.init_fifo_setup_hw(g)".
 *
 * @retval 0 in case of success.
 */
int gv11b_init_fifo_setup_hw(struct gk20a *g);
u32 gv11b_fifo_mmu_fault_id_to_pbdma_id(struct gk20a *g, u32 mmu_fault_id);

#endif /* NVGPU_FIFO_GV11B_H */
