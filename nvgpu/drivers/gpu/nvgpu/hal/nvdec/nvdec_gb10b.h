/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_NVDEC_GB10B_H
#define NVGPU_NVDEC_GB10B_H

#include <nvgpu/types.h>

#define GB10B_NVDEC_RISCV_UCODE_IMAGE             "g_nvdec_t264_riscv_dbg_image_debug_encrypted.bin"
#define GB10B_NVDEC_RISCV_UCODE_DESC              "g_nvdec_t264_riscv_dbg_desc.bin"
#define GB10B_NVDEC_RISCV_UCODE_PKC_SIG           "g_nvdec_t264_riscv_dbg_pkc_ls_sig.bin"
#define GB10B_NVDEC_RISCV_UCODE_LSB_PKC_SIG       "g_nvdec_t264_riscv_dbg_pkc_ls_lsb_header.bin"
#define GB10B_NVDEC_RISCV_UCODE_PROD_IMAGE        "g_nvdec_t264_riscv_prod_image_prod_encrypted.bin"
#define GB10B_NVDEC_RISCV_UCODE_PROD_DESC         "g_nvdec_t264_riscv_prod_desc.bin"
#define GB10B_NVDEC_RISCV_UCODE_PROD_PKC_SIG      "g_nvdec_t264_riscv_prod_pkc_ls_sig.bin"
#define GB10B_NVDEC_RISCV_UCODE_PROD_LSB_PKC_SIG  "g_nvdec_t264_riscv_prod_pkc_ls_lsb_header.bin"

struct gk20a;
struct nvgpu_firmware;

u32 gb10b_nvdec_base_addr(u32 inst_id);
u32 gb10b_nvdec_falcon2_base_addr(u32 inst_id);
void gb10b_nvdec_setup_boot_config(struct gk20a *g, u32 inst_id);
void gb10b_nvdec_halt_engine(struct gk20a *g, u32 inst_id);
int gb10b_nvdec_reset_engine(struct gk20a *g, u32 inst_id);
void gb10b_nvdec_get_intr_ctrl_msg(struct gk20a *g, u32 inst_id, bool enable,
		u32 *intr_ctrl, u32 *intr_notify_ctrl);
void gb10b_nvdec_enable_irq(struct gk20a *g, u32 inst_id, bool enable);
void gb10b_nvdec_isr(struct gk20a *g, u32 inst_id);
bool gb10b_nvdec_is_swgen0_set(struct gk20a *g, u32 inst_id);
void gb10b_nvdec_fw(struct gk20a *g, struct nvgpu_firmware **desc_fw,
			struct nvgpu_firmware **image_fw,
			struct nvgpu_firmware **pkc_sig);

#endif /* NVGPU_NVDEC_GB10B_H */
