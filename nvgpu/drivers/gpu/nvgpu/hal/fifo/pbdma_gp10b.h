/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PBDMA_GP10B_H
#define NVGPU_PBDMA_GP10B_H

#include <nvgpu/types.h>

struct gk20a;

u32 gp10b_pbdma_get_signature(struct gk20a *g);

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
u32 gp10b_pbdma_channel_fatal_0_intr_descs(void);
u32 gp10b_pbdma_allowed_syncpoints_0_index_f(u32 syncpt);
u32 gp10b_pbdma_allowed_syncpoints_0_valid_f(void);
u32 gp10b_pbdma_allowed_syncpoints_0_index_v(u32 offset);
#endif

u32 gp10b_pbdma_get_fc_runlist_timeslice(void);
u32 gp10b_pbdma_get_config_auth_level_privileged(void);

#endif /* NVGPU_PBDMA_GP10B_H */
