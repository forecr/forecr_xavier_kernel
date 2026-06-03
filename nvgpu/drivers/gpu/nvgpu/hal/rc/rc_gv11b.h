/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef RC_GV11B_H
#define RC_GV11B_H

#include <nvgpu/types.h>

struct mmu_fault_info;

void gv11b_fifo_recover(struct gk20a *g, u32 gfid, u32 runlist_id,
			u32 id, unsigned int id_type, unsigned int rc_type,
			bool should_defer_reset);
void gv11b_fifo_rc_cleanup_and_reenable_ctxsw(struct gk20a *g, u32 gfid,
	u32 runlist_id, u32 tsgid, bool deferred_reset_pending,
	unsigned int rc_type);

#endif /* RC_GV11B_H */
