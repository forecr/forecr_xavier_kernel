/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FIFO_INTR_GA10B_H
#define NVGPU_FIFO_INTR_GA10B_H

#include <nvgpu/types.h>

struct gk20a;

u32 ga10b_runlist_intr_0(void);
u32 ga10b_runlist_intr_0_mask(void);
u32 ga10b_runlist_intr_0_recover_mask(void);
u32 ga10b_runlist_intr_0_recover_unmask(void);
u32 ga10b_runlist_intr_0_ctxsw_timeout_mask(void);
u32 ga10b_runlist_intr_bad_tsg(void);

/**
 * @brief Initializes the vector IDs for runlist interrupts.
 *
 * This function iterates through each runlist interrupt tree and initializes
 * the vector IDs for the runlist interrupts. It checks if the vector ID for
 * each interrupt unit is already set and if not, it reads the vector ID from
 * the hardware and initializes the interrupt controller with the new vector ID.
 *
 * The steps performed by the function are as follows:
 * -# Loop through each runlist interrupt tree.
 * -# Calculate the interrupt unit by adding the base interrupt unit to the
 *    current tree index.
 * -# Check if the interrupt unit information is already set by calling
 *    nvgpu_cic_mon_intr_is_unit_info_valid().
 * -# If the interrupt unit information is already set, skip to the next
 *    interrupt tree.
 * -# Initialize the number of vector IDs to 0.
 * -# Loop through each runlist.
 * -# Get the pointer to the current active runlist.
 * -# Read the interrupt vector ID register for the current runlist and interrupt
 *    tree by calling nvgpu_runlist_readl().
 * -# Extract the vector ID from the register value.
 * -# Store the vector ID in an array.
 * -# Increment the count of vector IDs.
 * -# After collecting all vector IDs for the interrupt tree, initialize the
 *    interrupt unit with these vector IDs by calling
 *    nvgpu_cic_mon_intr_unit_vectorid_init().
 *
 * @param [in]  g  Pointer to the GPU structure.
 */
void ga10b_fifo_runlist_intr_vectorid_init(struct gk20a *g);
void ga10b_fifo_intr_top_enable(struct gk20a *g, bool enable);
void ga10b_fifo_intr_0_enable(struct gk20a *g, bool enable);
void ga10b_fifo_intr_1_enable(struct gk20a *g, bool enable);
void ga10b_fifo_intr_0_isr(struct gk20a *g);

u32 ga10b_runlist_intr_0_en_clear_tree(u32 intr_tree);
u32 ga10b_runlist_intr_0_en_set_tree(u32 intr_tree);
void ga10b_fifo_intr_set_recover_mask(struct gk20a *g);
void ga10b_fifo_intr_unset_recover_mask(struct gk20a *g);

u32 ga10b_runlist_intr_retrigger_reg_off(u32 intr_tree);
void ga10b_fifo_runlist_intr_retrigger(struct gk20a *g, u32 intr_tree);

#endif /* NVGPU_FIFO_INTR_GA10B_H */
