/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FIFO_GA10B_H
#define NVGPU_FIFO_GA10B_H

#include <nvgpu/types.h>

struct gk20a;

int ga10b_init_fifo_reset_enable_hw(struct gk20a *g);

/**
 * @brief Initialize FIFO hardware setup.
 *
 * This function performs the necessary initializations for the FIFO hardware
 * on GA10B architecture. It sets up the runlist interrupt vector IDs, configures
 * the maximum number of subcontexts, initializes usermode hardware, enables
 * FIFO interrupts, and configures the userd writeback timer.
 *
 * The steps performed by the function are as follows:
 * -# Initialize runlist interrupt vector IDs to ensure that vector IDs are set
 *    before enabling top-level interrupts using ga10b_fifo_runlist_intr_vectorid_init().
 * -# Set the maximum number of subcontexts supported by the GPU using the
 *    gops_gr_init.get_max_subctx_count() function from the graphics (gr) operations.
 * -# Initialize usermode hardware settings using the gops_usermode.setup_hw() function from
 *    the usermode operations.
 * -# Enable FIFO interrupts using ga10b_fifo_enable_intr().
 * -# Configure the userd writeback timer using ga10b_fifo_config_userd_writeback_timer().
 *
 * @param [in] g  Pointer to the GPU driver struct.
 *
 * @return Always returns 0 indicating success.
 */
int ga10b_init_fifo_setup_hw(struct gk20a *g);
u32 ga10b_fifo_mmu_fault_id_to_pbdma_id(struct gk20a *g, u32 mmu_fault_id);
void ga10b_fifo_enable_intr(struct gk20a *g);

#endif /* NVGPU_FIFO_GA10B_H */
