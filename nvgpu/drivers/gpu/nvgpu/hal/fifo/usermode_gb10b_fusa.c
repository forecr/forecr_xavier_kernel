// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/runlist.h>

#include "usermode_gb10b.h"

#include "hal/fifo/fifo_utils_ga10b.h"

#include <nvgpu/hw/gb10b/hw_runlist_gb10b.h>

#define GFID_INSTANCE_0		0U

/*
 * nvgpu_fifo.max_runlists:
 *      - Maximum runlists supported by hardware.
 * nvgpu_fifo.num_runlists:
 *      - Number of valid runlists detected during device info parsing and
 *        connected to a valid engine.
 * nvgpu_fifo.runlists[]:
 *      - This is an array of pointers to nvgpu_runlist_info structure.
 *      - This is indexed by hardware runlist_id from 0 to max_runlists.
 * nvgpu_fifo.active_runlists[]:
 *      - This is an array of nvgpu_runlist_info structure.
 *      - This is indexed by software [consecutive] runlist_ids from 0 to
 *        num_runlists.
 *
 * runlists[] pointers at valid runlist_id indices contain valid
 * nvgpu_runlist structures. runlist[] pointers at invalid runlist_id
 * indexes point to NULL. This is explained in the example below.
 *
 * for example: max_runlists = 10, num_runlists = 4
 *              say valid runlist_ids are = {0, 2, 3, 7}
 *
 *         runlist_info                           active_runlists
 *      0 ________________                  0 ___________________________
 *       |________________|----------------->|___________________________|
 *       |________________|   |------------->|___________________________|
 *       |________________|---|  |---------->|___________________________|
 *       |________________|------|  |------->|___________________________|
 *       |________________|         |    num_runlists
 *       |________________|         |
 *       |________________|         |
 *       |________________|---------|
 *       |________________|
 *       |________________|
 *  max_runlists
 *
 */

void gb10b_usermode_setup_hw(struct gk20a *g)
{
	u32 max_runlist = g->ops.runlist.count_max(g);
	struct nvgpu_runlist *runlist;
	u32 field_mask;
	u32 field_val;
	u32 reg_val, reg_off;
	u32 i;

	/*
	* At this moment, we are not supporting multiple GFIDs.
	* Only GFID 0 is supported and passed to
	* runlist_virtual_channel_cfg_r()
	*/
	reg_off = g->ops.runlist.get_virtual_channel_cfg_off(GFID_INSTANCE_0);

	for (i = 0U; i < max_runlist; i++) {
		runlist = g->fifo.runlists[i];
		if (!runlist)
			continue;

		reg_val = nvgpu_runlist_readl(g, runlist, reg_off);
		field_mask = runlist_virtual_channel_cfg_count_hw_m() |
			runlist_virtual_channel_cfg_pending_enable_m();
		field_val = runlist_virtual_channel_cfg_count_hw_init_f() |
			runlist_virtual_channel_cfg_pending_enable_true_f();
		reg_val = set_field(reg_val, field_mask, field_val);

		nvgpu_runlist_writel(g, runlist, reg_off, reg_val);
	}
}
