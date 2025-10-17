/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PERF_VF_H
#define NVGPU_PERF_VF_H

struct gk20a;

void vf_perf_reset_pm_trigger_masks(struct gk20a *g, u32 pma_channel_id,
			u32 gr_instance_id, u32 reservation_id);
void vf_perf_enable_pm_trigger(struct gk20a *g, u32 gr_instance_id,
			u32 pma_channel_id, u32 reservation_id);
void vf_perf_deinit_inst_block(struct gk20a *g, u32 pma_channel_id);
int vf_perf_init_inst_block(struct gk20a *g, u32 pma_channel_id,
			u32 inst_blk_ptr, u32 aperture, u32 gfid);
#endif
