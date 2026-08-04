/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GB10B_FIRMWARE_H
#define NVGPU_GB10B_FIRMWARE_H

struct gk20a;
struct nvgpu_firmware;

s32 nvgpu_gb10b_acr_lsf_pmu_riscv_eb_ucode_details(struct gk20a *g, void *lsf_ucode_img);
s32 nvgpu_gb10b_acr_lsf_fecs_riscv_eb_ucode_details(struct gk20a *g, void *lsf_ucode_img);
s32 nvgpu_gb10b_acr_lsf_gpccs_riscv_eb_ucode_details(struct gk20a *g, void *lsf_ucode_img);
s32 nvgpu_gb10b_acr_lsf_nvdec_riscv_ucode_details(struct gk20a *g, void *lsf_ucode_img);
s32 nvgpu_gb10b_acr_lsf_nvenc_riscv_eb_ucode_details(struct gk20a *g, void *lsf_ucode_img);
s32 nvgpu_gb10b_acr_lsf_ofa_riscv_eb_ucode_details(struct gk20a *g, void *lsf_ucode_img);
s32 nvgpu_gb10b_acr_lsf_nvjpg_riscv_eb_ucode_details(struct gk20a *g, void *lsf_ucode_img);

#endif
