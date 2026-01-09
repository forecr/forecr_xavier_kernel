/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_NETLIST_GB20C_H
#define NVGPU_NETLIST_GB20C_H

#include <nvgpu/types.h>
#include <nvgpu/netlist_defs.h>

struct gk20a;

#define GB20C_NETLIST_IMAGE_FW_NAME        "NETC_img.bin"
#define GB20C_SIM_FECS_PKC_FW_NAME         "fecs_riscv_pkc_sig.bin"
#define GB20C_SIM_GPCCS_PKC_FW_NAME        "gpccs_riscv_pkc_sig.bin"

/* Encrypted fecs and gpccs net pkc bins*/
#define NVGPU_NETLIST_FECS_RISCV_A  "g_fecs_riscv_gb20c_NETA_pkc_ls_sig.bin"
#define NVGPU_NETLIST_FECS_RISCV_B  "g_fecs_riscv_gb20c_NETB_pkc_ls_sig.bin"
#define NVGPU_NETLIST_FECS_RISCV_C  "g_fecs_riscv_gb20c_NETC_pkc_ls_sig.bin"
#define NVGPU_NETLIST_FECS_RISCV_D  "g_fecs_riscv_gb20c_NETD_pkc_ls_sig.bin"

#define NVGPU_NETLIST_GPCCS_RISCV_A  "g_gpccs_riscv_gb20c_NETA_pkc_ls_sig.bin"
#define NVGPU_NETLIST_GPCCS_RISCV_B  "g_gpccs_riscv_gb20c_NETB_pkc_ls_sig.bin"
#define NVGPU_NETLIST_GPCCS_RISCV_C  "g_gpccs_riscv_gb20c_NETC_pkc_ls_sig.bin"
#define NVGPU_NETLIST_GPCCS_RISCV_D  "g_gpccs_riscv_gb20c_NETD_pkc_ls_sig.bin"

/* Encrypted fecs and gpccs net LSB pkc bins*/
#define NVGPU_NETLIST_FECS_RISCV_LSB_PKC_A   "g_fecs_riscv_gb20c_NETA_pkc_ls_lsb_header.bin"
#define NVGPU_NETLIST_FECS_RISCV_LSB_PKC_B   "g_fecs_riscv_gb20c_NETB_pkc_ls_lsb_header.bin"
#define NVGPU_NETLIST_FECS_RISCV_LSB_PKC_C   "g_fecs_riscv_gb20c_NETC_pkc_ls_lsb_header.bin"
#define NVGPU_NETLIST_FECS_RISCV_LSB_PKC_D   "g_fecs_riscv_gb20c_NETD_pkc_ls_lsb_header.bin"

#define NVGPU_NETLIST_GPCCS_RISCV_LSB_PKC_A  "g_gpccs_riscv_gb20c_NETA_pkc_ls_lsb_header.bin"
#define NVGPU_NETLIST_GPCCS_RISCV_LSB_PKC_B  "g_gpccs_riscv_gb20c_NETB_pkc_ls_lsb_header.bin"
#define NVGPU_NETLIST_GPCCS_RISCV_LSB_PKC_C  "g_gpccs_riscv_gb20c_NETC_pkc_ls_lsb_header.bin"
#define NVGPU_NETLIST_GPCCS_RISCV_LSB_PKC_D  "g_gpccs_riscv_gb20c_NETD_pkc_ls_lsb_header.bin"

/* ENCRYPTED Netlists used for gb20c */
#define GB20C_NETLIST_DBG_IMAGE_FW_NAME              NVGPU_NETLIST_DBG_IMAGE_C
#define GB20C_NETLIST_PROD_IMAGE_FW_NAME             NVGPU_NETLIST_PROD_IMAGE_C
#define GB20C_FECS_RISCV_UCODE_ENCRYPT_PKC_SIG       NVGPU_NETLIST_FECS_RISCV_C
#define GB20C_GPCCS_RISCV_UCODE_ENCRYPT_PKC_SIG      NVGPU_NETLIST_GPCCS_RISCV_C
#define GB20C_FECS_RISCV_UCODE_ENCRYPT_LSB_PKC_SIG   NVGPU_NETLIST_FECS_RISCV_LSB_PKC_C
#define GB20C_GPCCS_RISCV_UCODE_ENCRYPT_LSB_PKC_SIG  NVGPU_NETLIST_GPCCS_RISCV_LSB_PKC_C
#define GB20C_FECS_RISCV_UCODE_DESC                  NVGPU_FECS_DESC_C
#define GB20C_GPCCS_RISCV_UCODE_DESC                 NVGPU_GPCCS_DESC_C

int gb20c_netlist_get_name(struct gk20a *g, int index, char *name);
void gb20c_fecs_pkc_get_name(struct gk20a *g, char *name);
void gb20c_gpccs_pkc_get_name(struct gk20a *g, char *name);
void gb20c_fecs_desc_get_name(struct gk20a *g, char *name);
void gb20c_gpccs_desc_get_name(struct gk20a *g, char *name);
bool gb20c_netlist_is_firmware_defined(void);

#endif /* NVGPU_NETLIST_GB20C_H */
