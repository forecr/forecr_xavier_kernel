/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_OFA_GB20C_H
#define NVGPU_OFA_GB20C_H

#include <nvgpu/types.h>

#define GB20C_OFA_RISCV_EB_UCODE_IMAGE            "g_ofa_t256_riscv_image_debug_encrypted.bin"
#define GB20C_OFA_RISCV_EB_UCODE_PROD_IMAGE       "g_ofa_t256_riscv_image_prod_encrypted.bin"
#define GB20C_OFA_RISCV_EB_UCODE_DESC             "g_ofa_t256_riscv_desc.bin"
#define GB20C_OFA_RISCV_EB_UCODE_PKC_SIG          "g_ofa_t256_riscv_pkc_ls_sig.bin"
#define GB20C_OFA_RISCV_EB_UCODE_LSB_PKC_SIG      "g_ofa_t256_riscv_pkc_ls_lsb_header.bin"

struct gk20a;
struct nvgpu_firmware;

void nvgpu_gb20c_ofa_fw(struct gk20a *g,
			struct nvgpu_firmware **desc_fw,
			struct nvgpu_firmware **image_fw,
			struct nvgpu_firmware **pkc_sig);

#endif /* NVGPU_OFA_GB20C_H */
