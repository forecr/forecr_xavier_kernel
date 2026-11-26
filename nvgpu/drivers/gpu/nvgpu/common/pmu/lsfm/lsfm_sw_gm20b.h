/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LSFM_SW_GM20B_H
#define NVGPU_LSFM_SW_GM20B_H

int gm20b_pmu_lsfm_init_acr_wpr_region(struct gk20a *g, struct nvgpu_pmu *pmu);
void gm20b_pmu_lsfm_handle_bootstrap_falcon_msg(struct gk20a *g,
	struct pmu_msg *msg, void *param, u32 status);
int gm20b_pmu_lsfm_pmu_cmd_line_args_copy(struct gk20a *g,
	struct nvgpu_pmu *pmu);

void nvgpu_gm20b_lsfm_sw_init(struct gk20a *g, struct nvgpu_pmu_lsfm *lsfm);

#endif /*NVGPU_LSFM_SW_GM20B_H*/
