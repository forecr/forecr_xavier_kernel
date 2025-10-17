/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RUNLIST_RAM_GV11B_H
#define NVGPU_RUNLIST_RAM_GV11B_H

#include <nvgpu/types.h>
#include <nvgpu/gmmu.h>

struct nvgpu_channel;
struct nvgpu_tsg;

u32 gv11b_runlist_entry_size(struct gk20a *g);
u32 gv11b_runlist_max_timeslice(void);
void gv11b_runlist_get_tsg_entry(struct nvgpu_tsg *tsg,
		u32 *runlist, u32 timeslice);
void gv11b_runlist_get_ch_entry(struct nvgpu_channel *ch, u32 *runlist);
void gv11b_runlist_checker_get_ch_entry(struct nvgpu_channel *ch,
			enum nvgpu_aperture userd_aperture,
			enum nvgpu_aperture inst_block_aperture,
			u64 inst_block_addr,
			u32 *runlist);
u32 gv11b_runlist_get_max_channels_per_tsg(void);

#endif /* NVGPU_RUNLIST_RAM_GV11B_H */
