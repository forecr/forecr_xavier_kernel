/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_NETLIST_GB10B_H
#define NVGPU_NETLIST_GB10B_H

#include <nvgpu/types.h>
#include <nvgpu/netlist_defs.h>

struct gk20a;

#define GB10B_NETLIST_IMAGE_FW_NAME        "NETD_img.bin"

/* Encrypted fecs and gpccs net pkc bins*/
#define NVGPU_NETLIST_FECS_RISCV_A  "g_fecs_riscv_gb10b_NETA_pkc_ls_sig.bin"
#define NVGPU_NETLIST_FECS_RISCV_B  "g_fecs_riscv_gb10b_NETB_pkc_ls_sig.bin"
#define NVGPU_NETLIST_FECS_RISCV_C  "g_fecs_riscv_gb10b_NETC_pkc_ls_sig.bin"
#define NVGPU_NETLIST_FECS_RISCV_D  "g_fecs_riscv_gb10b_NETD_pkc_ls_sig.bin"

#define NVGPU_NETLIST_GPCCS_RISCV_A  "g_gpccs_riscv_gb10b_NETA_pkc_ls_sig.bin"
#define NVGPU_NETLIST_GPCCS_RISCV_B  "g_gpccs_riscv_gb10b_NETB_pkc_ls_sig.bin"
#define NVGPU_NETLIST_GPCCS_RISCV_C  "g_gpccs_riscv_gb10b_NETC_pkc_ls_sig.bin"
#define NVGPU_NETLIST_GPCCS_RISCV_D  "g_gpccs_riscv_gb10b_NETD_pkc_ls_sig.bin"

/* Encrypted fecs and gpccs prod/test nsr net pkc bins*/
#define NVGPU_PROD_TEST_NSR_NETLIST_FECS_RISCV_A  "prod_test_nsr/g_fecs_riscv_gb10b_NETA_pkc_ls_sig.bin"

#define NVGPU_PROD_TEST_NSR_NETLIST_GPCCS_RISCV_A  "prod_test_nsr/g_gpccs_riscv_gb10b_NETA_pkc_ls_sig.bin"

/* Encrypted safety fecs and gpccs net pkc bins*/
#define NVGPU_SAFETY_NETLIST_FECS_RISCV_A  "safety/g_fecs_riscv_gb10b_NETA_pkc_ls_sig.bin"
#define NVGPU_SAFETY_NETLIST_FECS_RISCV_B  "safety/g_fecs_riscv_gb10b_NETB_pkc_ls_sig.bin"
#define NVGPU_SAFETY_NETLIST_FECS_RISCV_C  "safety/_fecs_riscv_gb10b_NETC_pkc_ls_sig.bin"
#define NVGPU_SAFETY_NETLIST_FECS_RISCV_D  "safety/g_fecs_riscv_gb10b_NETD_pkc_ls_sig.bin"

#define NVGPU_SAFETY_NETLIST_GPCCS_RISCV_A  "safety/g_gpccs_riscv_gb10b_NETA_pkc_ls_sig.bin"
#define NVGPU_SAFETY_NETLIST_GPCCS_RISCV_B  "safety/g_gpccs_riscv_gb10b_NETB_pkc_ls_sig.bin"
#define NVGPU_SAFETY_NETLIST_GPCCS_RISCV_C  "safety/g_gpccs_riscv_gb10b_NETC_pkc_ls_sig.bin"
#define NVGPU_SAFETY_NETLIST_GPCCS_RISCV_D  "safety/g_gpccs_riscv_gb10b_NETD_pkc_ls_sig.bin"

/* ENCRYPTED Netlists used for gb10b */
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
#define GB10B_NETLIST_DBG_IMAGE_FW_NAME         NVGPU_NETLIST_DBG_IMAGE_A
#define GB10B_NETLIST_PROD_IMAGE_FW_NAME        NVGPU_NETLIST_PROD_IMAGE_A
#define GB10B_FECS_RISCV_UCODE_ENCRYPT_PKC_SIG  NVGPU_NETLIST_FECS_RISCV_A
#define GB10B_GPCCS_RISCV_UCODE_ENCRYPT_PKC_SIG NVGPU_NETLIST_GPCCS_RISCV_A
#define GB10B_FECS_RISCV_UCODE_DESC             NVGPU_FECS_DESC_A
#define GB10B_GPCCS_RISCV_UCODE_DESC            NVGPU_GPCCS_DESC_A
#define GB10B_PROD_TEST_NSR_NETLIST_DBG_IMAGE_FW_NAME         NVGPU_PROD_TEST_NSR_NETLIST_DBG_IMAGE_A
#define GB10B_PROD_TEST_NSR_NETLIST_PROD_IMAGE_FW_NAME        NVGPU_PROD_TEST_NSR_NETLIST_PROD_IMAGE_A
#define GB10B_PROD_TEST_NSR_FECS_RISCV_UCODE_ENCRYPT_PKC_SIG  NVGPU_PROD_TEST_NSR_NETLIST_FECS_RISCV_A
#define GB10B_PROD_TEST_NSR_GPCCS_RISCV_UCODE_ENCRYPT_PKC_SIG NVGPU_PROD_TEST_NSR_NETLIST_GPCCS_RISCV_A
#define GB10B_PROD_TEST_NSR_FECS_RISCV_UCODE_DESC             NVGPU_PROD_TEST_NSR_FECS_DESC_A
#define GB10B_PROD_TEST_NSR_GPCCS_RISCV_UCODE_DESC            NVGPU_PROD_TEST_NSR_GPCCS_DESC_A
#else
#define GB10B_NETLIST_DBG_IMAGE_FW_NAME         NVGPU_SAFETY_NETLIST_DBG_IMAGE_A
#define GB10B_NETLIST_PROD_IMAGE_FW_NAME        NVGPU_SAFETY_NETLIST_PROD_IMAGE_A
#define GB10B_FECS_RISCV_UCODE_ENCRYPT_PKC_SIG  NVGPU_SAFETY_NETLIST_FECS_RISCV_A
#define GB10B_GPCCS_RISCV_UCODE_ENCRYPT_PKC_SIG NVGPU_SAFETY_NETLIST_GPCCS_RISCV_A
#define GB10B_FECS_RISCV_UCODE_DESC             NVGPU_SAFETY_FECS_DESC_A
#define GB10B_GPCCS_RISCV_UCODE_DESC            NVGPU_SAFETY_GPCCS_DESC_A
#endif

int gb10b_netlist_get_name(struct gk20a *g, int index, char *name);
void gb10b_fecs_pkc_get_name(struct gk20a *g, char *name);
void gb10b_gpccs_pkc_get_name(struct gk20a *g, char *name);
void gb10b_fecs_desc_get_name(struct gk20a *g, char *name);
void gb10b_gpccs_desc_get_name(struct gk20a *g, char *name);
bool gb10b_netlist_is_firmware_defined(void);

#endif /* NVGPU_NETLIST_GB10B_H */
