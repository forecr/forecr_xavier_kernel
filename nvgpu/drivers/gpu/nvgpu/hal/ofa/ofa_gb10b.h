/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_OFA_GB10B_H
#define NVGPU_OFA_GB10B_H

#include <nvgpu/types.h>

#define GB10B_OFA_RISCV_EB_UCODE_IMAGE            "g_ofa_t264_riscv_eb_image_debug_encrypted.bin"
#define GB10B_OFA_RISCV_EB_UCODE_PROD_IMAGE       "g_ofa_t264_riscv_eb_image_prod_encrypted.bin"
#define GB10B_OFA_RISCV_EB_UCODE_DESC             "g_ofa_t264_riscv_eb_desc.bin"
#define GB10B_OFA_RISCV_EB_UCODE_PKC_SIG          "g_ofa_t264_riscv_eb_pkc_ls_sig.bin"
#define GB10B_OFA_RISCV_EB_UCODE_LSB_PKC_SIG      "g_ofa_t264_riscv_eb_pkc_ls_lsb_header.bin"

struct gk20a;
struct nvgpu_firmware;

u32 gb10b_ofa_base_addr(u32 inst_id);
u32 gb10b_ofa_falcon2_base_addr(u32 inst_id);
void gb10b_ofa_setup_boot_config(struct gk20a *g, u32 inst_id);
void gb10b_ofa_halt_engine(struct gk20a *g, u32 inst_id);
int gb10b_ofa_reset_engine(struct gk20a *g, u32 inst_id);
void gb10b_ofa_get_intr_ctrl_msg(struct gk20a *g, u32 inst_id, bool enable,
		u32 *intr_ctrl, u32 *intr_notify_ctrl);
void gb10b_ofa_set_ecc_intr(struct gk20a *g, u32 inst_id, bool enable);
void gb10b_ofa_enable_irq(struct gk20a *g, u32 inst_id, bool enable);
void gb10b_ofa_isr(struct gk20a *g, u32 inst_id);
void gb10b_ofa_fw(struct gk20a *g,
		struct nvgpu_firmware **desc_fw,
		struct nvgpu_firmware **image_fw,
		struct nvgpu_firmware **pkc_sig);

#endif /* NVGPU_OFA_GB10B_H */
