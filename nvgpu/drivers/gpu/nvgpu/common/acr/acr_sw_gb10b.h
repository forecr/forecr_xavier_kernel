/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef ACR_SW_GB10B_H
#define ACR_SW_GB10B_H

struct gk20a;
struct nvgpu_acr;
struct hs_acr;

u32 gb10b_acr_lsf_pmu_riscv_eb(struct gk20a *g, struct acr_lsf_config *lsf);
u32 nvgpu_gb10b_acr_lsf_riscv_eb_fecs(struct gk20a *g, struct acr_lsf_config *lsf);
u32 nvgpu_gb10b_acr_lsf_riscv_eb_gpccs(struct gk20a *g, struct acr_lsf_config *lsf);
u32 gb10b_acr_lsf_nvdec_riscv(struct gk20a *g, struct acr_lsf_config *lsf);
u32 gb10b_acr_lsf_nvenc_riscv(struct gk20a *g, struct acr_lsf_config *lsf);
u32 gb10b_acr_lsf_ofa_riscv(struct gk20a *g, struct acr_lsf_config *lsf);
int nvgpu_gb10b_acr_patch_wpr_info_to_ucode(struct gk20a *g,
	struct nvgpu_acr *acr, struct hs_acr *acr_desc, bool is_recovery);
int nvgpu_gb10b_bootstrap_hs_acr(struct gk20a *g, struct nvgpu_acr *acr);
void gb10b_acr_default_sw_init(struct gk20a *g, struct hs_acr *riscv_hs);
void nvgpu_gb10b_acr_sw_ipc_init(struct gk20a *g, struct nvgpu_acr *acr);
void nvgpu_gb10b_acr_sw_init(struct gk20a *g, struct nvgpu_acr *acr);

#endif /*ACR_SW_GA10B_H*/
