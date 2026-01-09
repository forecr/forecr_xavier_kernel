/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LTC_VGPU_H
#define NVGPU_LTC_VGPU_H

struct gk20a;
struct gr_gk20a;

u64 vgpu_determine_L2_size_bytes(struct gk20a *g);
void vgpu_ltc_init_fs_state(struct gk20a *g);

#ifdef CONFIG_NVGPU_DEBUGGER
int vgpu_ltc_get_max_ways_evict_last(struct gk20a *g, struct nvgpu_tsg *tsg,
		u32 *num_ways);
int vgpu_ltc_set_max_ways_evict_last(struct gk20a *g, struct nvgpu_tsg *tsg,
		u32 num_ways);
#endif

int vgpu_ltc_set_sector_promotion(struct gk20a *g, struct nvgpu_tsg *tsg,
		u32 policy);

#endif /* NVGPU_LTC_VGPU_H */
