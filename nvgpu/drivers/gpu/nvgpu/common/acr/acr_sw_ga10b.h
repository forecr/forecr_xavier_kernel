/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef ACR_SW_GA10B_H
#define ACR_SW_GA10B_H

struct gk20a;
struct nvgpu_acr;
struct hs_acr;
struct acr_lsf_config;

int nvgpu_ga10b_safety_acr_patch_wpr_info_to_ucode(struct gk20a *g,
	    struct nvgpu_acr *acr, struct hs_acr *acr_desc, bool is_recovery);
u32 nvgpu_ga10b_acr_lsf_fecs(struct gk20a *g,
			struct acr_lsf_config *lsf);
u32 nvgpu_ga10b_acr_lsf_gpccs(struct gk20a *g,
			struct acr_lsf_config *lsf);
void nvgpu_ga10b_acr_sw_init(struct gk20a *g, struct nvgpu_acr *acr);

#endif /*ACR_SW_GA10B_H*/
