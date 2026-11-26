/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef LSFM_H
#define LSFM_H

struct gk20a;
struct nvgpu_pmu_lsfm;

struct nvgpu_pmu_lsfm {
	bool is_wpr_init_done;
	u32 loaded_falcon_id;
	int (*init_wpr_region)(struct gk20a *g, struct nvgpu_pmu *pmu);
	int (*bootstrap_ls_falcon)(struct gk20a *g,
		struct nvgpu_pmu *pmu, struct nvgpu_pmu_lsfm *lsfm,
		u32 falcon_id_mask);
	int (*ls_pmu_cmdline_args_copy)(struct gk20a *g, struct nvgpu_pmu *pmu);
};

int nvgpu_pmu_lsfm_bootstrap_ls_falcon(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_pmu_lsfm *lsfm, u32 falcon_id_mask);
int nvgpu_pmu_lsfm_ls_pmu_cmdline_args_copy(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_pmu_lsfm *lsfm);
void nvgpu_pmu_lsfm_rpc_handler(struct gk20a *g,
	struct rpc_handler_payload *rpc_payload);
int nvgpu_pmu_lsfm_init(struct gk20a *g, struct nvgpu_pmu_lsfm **lsfm);
void nvgpu_pmu_lsfm_clean(struct gk20a *g, struct nvgpu_pmu *pmu,
	struct nvgpu_pmu_lsfm *lsfm);
void nvgpu_pmu_lsfm_deinit(struct gk20a *g, struct nvgpu_pmu *pmu,
	struct nvgpu_pmu_lsfm *lsfm);

#endif /*LSFM_H*/
