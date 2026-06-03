/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef ACR_WPR_H
#define ACR_WPR_H

struct gk20a;
struct wpr_carveout_info;

struct wpr_carveout_info {
	u64 wpr_base;
	u64 nonwpr_base;
	u64 size;
};

void nvgpu_acr_wpr_info_sys(struct gk20a *g, struct wpr_carveout_info *inf);
#ifdef CONFIG_NVGPU_DGPU
void nvgpu_acr_wpr_info_vid(struct gk20a *g, struct wpr_carveout_info *inf);
#endif

#endif /* NVGPU_ACR_WPR_H */
