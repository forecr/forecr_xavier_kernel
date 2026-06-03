// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "fifo_intr_gb20c.h"

#include <nvgpu/hw/gb20c/hw_runlist_gb20c.h>

/**
 *     [runlist's tree 0 bit] <---------. .---------> [runlist's tree 1 bit]
 *                                       Y
 *                                       |
 *                                       |
 *     [runlist intr tree 0]             ^             [runlist intr tree 1]
 *                       ______________/   \______________
 *                      /                                  |
 *     NV_RUNLIST_INTR_VECTORID(0) msg       NV_RUNLIST_INTR_VECTORID(1) msg
 *                     |                                   |
 *               ______^______                       ______^______
 *              /             \                     /             \
 *             '_______________'                   '_______________'
 *              |||||||       |                     |       |||||||
 *            other tree0     |                     |     other tree1
 *          ANDed intr bits   ^                     ^   ANDed intr bits
 *                           AND                   AND
 *                           | |                   | |
 *                    _______. .______      _______. .________
 *                   /                 \   /                  \
 *RUNLIST_INTR_0_EN_SET_TREE(0)_intr_bit Y RUNLIST_INTR_0_EN_SET_TREE(1)_intr_bit
 *                                       |
 *                           NV_RUNLIST_INTR_0_intr_bit
 */

u32 gb20c_runlist_intr_ctrl(u32 intr_tree)
{
	return runlist_intr_ctrl_r(intr_tree);
}

u32 gb20c_runlist_intr_0(void)
{
	return runlist_intr_0_r();
}

u32 gb20c_runlist_intr_0_mask(void)
{
	/*
	 * Unlike gb10b, gb20c doesn't have eng2, so there are no registers to
	 * set/clear eng2 interrupts
	 */
	u32 mask = (runlist_intr_0_en_set_tree_ctxsw_timeout_eng0_enabled_f() |
		runlist_intr_0_en_set_tree_ctxsw_timeout_eng1_enabled_f() |
		runlist_intr_0_en_set_tree_pbdma0_intr_tree_0_enabled_f() |
		runlist_intr_0_en_set_tree_pbdma1_intr_tree_0_enabled_f() |
		runlist_intr_0_en_set_tree_bad_tsg_enabled_f());

	return mask;
}

u32 gb20c_runlist_intr_0_recover_mask(void)
{
	u32 mask = runlist_intr_0_en_clear_tree_ctxsw_timeout_eng0_enabled_f() |
		runlist_intr_0_en_clear_tree_ctxsw_timeout_eng1_enabled_f();

	return mask;
}

u32 gb20c_runlist_intr_0_recover_unmask(void)
{
	u32 mask = runlist_intr_0_en_set_tree_ctxsw_timeout_eng0_enabled_f() |
		runlist_intr_0_en_set_tree_ctxsw_timeout_eng1_enabled_f();

	return mask;
}

u32 gb20c_runlist_intr_0_ctxsw_timeout_mask(void)
{
	u32 mask = runlist_intr_0_en_clear_tree_ctxsw_timeout_eng0_enabled_f() |
		runlist_intr_0_en_clear_tree_ctxsw_timeout_eng1_enabled_f();

	return mask;
}

u32 gb20c_runlist_intr_0_en_clear_tree(u32 intr_tree)
{
	return runlist_intr_0_en_clear_tree_r(intr_tree);
}

u32 gb20c_runlist_intr_0_en_set_tree(u32 intr_tree)
{
	return runlist_intr_0_en_set_tree_r(intr_tree);
}

u32 gb20c_runlist_intr_bad_tsg(void)
{
	return runlist_intr_bad_tsg_r();
}

u32 gb20c_runlist_intr_retrigger_reg_off(u32 intr_tree)
{
	return runlist_intr_retrigger_r(intr_tree);
}