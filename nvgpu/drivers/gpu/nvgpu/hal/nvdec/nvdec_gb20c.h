/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_NVDEC_GB20C_H
#define NVGPU_NVDEC_GB20C_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_firmware;
#define GB20C_NVDEC_RISCV_UCODE_IMAGE             "g_nvdec_t256_riscv_dbg_image_debug_encrypted.bin"
#define GB20C_NVDEC_RISCV_UCODE_DESC              "g_nvdec_t256_riscv_dbg_desc.bin"
#define GB20C_NVDEC_RISCV_UCODE_PKC_SIG           "g_nvdec_t256_riscv_dbg_pkc_ls_sig.bin"
#define GB20C_NVDEC_RISCV_UCODE_LSB_PKC_SIG       "g_nvdec_t256_riscv_dbg_pkc_ls_lsb_header.bin"
#define GB20C_NVDEC_RISCV_UCODE_PROD_IMAGE        "g_nvdec_t256_riscv_prod_image_prod_encrypted.bin"
#define GB20C_NVDEC_RISCV_UCODE_PROD_DESC         "g_nvdec_t256_riscv_prod_desc.bin"
#define GB20C_NVDEC_RISCV_UCODE_PROD_PKC_SIG      "g_nvdec_t256_riscv_prod_pkc_ls_sig.bin"
#define GB20C_NVDEC_RISCV_UCODE_PROD_LSB_PKC_SIG  "g_nvdec_t256_riscv_prod_pkc_ls_lsb_header.bin"

bool nvgpu_gb20c_nvdec_fw_present(struct gk20a *g);
void nvgpu_gb20c_nvdec_fw(struct gk20a *g, struct nvgpu_firmware **desc_fw,
			struct nvgpu_firmware **image_fw,
			struct nvgpu_firmware **pkc_sig);

#endif /* NVGPU_NVDEC_GB10B_H */
