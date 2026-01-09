/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RUNLIST_RAM_GK20A_H
#define NVGPU_RUNLIST_RAM_GK20A_H

#include <nvgpu/types.h>

struct nvgpu_channel;
struct nvgpu_tsg;
struct gk20a;

u32 gk20a_runlist_entry_size(struct gk20a *g);
u32 gk20a_runlist_max_timeslice(void);
void gk20a_runlist_get_tsg_entry(struct nvgpu_tsg *tsg,
		u32 *runlist, u32 timeslice);
void gk20a_runlist_get_ch_entry(struct nvgpu_channel *ch, u32 *runlist);
u32 gk20a_runlist_get_max_channels_per_tsg(void);

#endif /* NVGPU_RUNLIST_RAM_GK20A_H */
