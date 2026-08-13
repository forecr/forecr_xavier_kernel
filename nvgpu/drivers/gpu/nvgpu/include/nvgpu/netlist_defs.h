/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_NETLIST_DEFS_H
#define NVGPU_NETLIST_DEFS_H

#include <nvgpu/types.h>

/* emulation netlists, match majorV with HW */
#define NVGPU_NETLIST_IMAGE_A	"NETA_img.bin"
#define NVGPU_NETLIST_IMAGE_B	"NETB_img.bin"
#define NVGPU_NETLIST_IMAGE_C	"NETC_img.bin"
#define NVGPU_NETLIST_IMAGE_D	"NETD_img.bin"

/* Encrypted dbg and prod netlists */
#define NVGPU_NETLIST_DBG_IMAGE_A	"NETA_img_debug_encrypted.bin"
#define NVGPU_NETLIST_DBG_IMAGE_B	"NETB_img_debug_encrypted.bin"
#define NVGPU_NETLIST_DBG_IMAGE_C	"NETC_img_debug_encrypted.bin"
#define NVGPU_NETLIST_DBG_IMAGE_D	"NETD_img_debug_encrypted.bin"

#define NVGPU_NETLIST_PROD_IMAGE_A	"NETA_img_prod_encrypted.bin"
#define NVGPU_NETLIST_PROD_IMAGE_B	"NETB_img_prod_encrypted.bin"
#define NVGPU_NETLIST_PROD_IMAGE_C	"NETC_img_prod_encrypted.bin"
#define NVGPU_NETLIST_PROD_IMAGE_D	"NETD_img_prod_encrypted.bin"

#define NVGPU_PROD_TEST_NSR_NETLIST_DBG_IMAGE_A		"prod_test_nsr/NETA_img_debug_encrypted.bin"

#define NVGPU_PROD_TEST_NSR_NETLIST_PROD_IMAGE_A	"prod_test_nsr/NETA_img_prod_encrypted.bin"

#define NVGPU_SAFETY_NETLIST_DBG_IMAGE_A	"safety/NETA_img_debug_encrypted.bin"
#define NVGPU_SAFETY_NETLIST_DBG_IMAGE_B	"safety/NETB_img_debug_encrypted.bin"
#define NVGPU_SAFETY_NETLIST_DBG_IMAGE_C	"safety/NETC_img_debug_encrypted.bin"
#define NVGPU_SAFETY_NETLIST_DBG_IMAGE_D	"safety/NETD_img_debug_encrypted.bin"

#define NVGPU_SAFETY_NETLIST_PROD_IMAGE_A	"safety/NETA_img_prod_encrypted.bin"
#define NVGPU_SAFETY_NETLIST_PROD_IMAGE_B	"safety/NETB_img_prod_encrypted.bin"
#define NVGPU_SAFETY_NETLIST_PROD_IMAGE_C	"safety/NETC_img_prod_encrypted.bin"
#define NVGPU_SAFETY_NETLIST_PROD_IMAGE_D	"safety/NETD_img_prod_encrypted.bin"

/* Encrypted fecs and gpccs netlists*/
#define NVGPU_FECS_DESC_A  "NETA_fecs_riscv_desc.bin"
#define NVGPU_FECS_DESC_B  "NETB_fecs_riscv_desc.bin"
#define NVGPU_FECS_DESC_C  "NETC_fecs_riscv_desc.bin"
#define NVGPU_FECS_DESC_D  "NETD_fecs_riscv_desc.bin"

#define NVGPU_GPCCS_DESC_A  "NETA_gpccs_riscv_desc.bin"
#define NVGPU_GPCCS_DESC_B  "NETB_gpccs_riscv_desc.bin"
#define NVGPU_GPCCS_DESC_C  "NETC_gpccs_riscv_desc.bin"
#define NVGPU_GPCCS_DESC_D  "NETD_gpccs_riscv_desc.bin"

#define NVGPU_PROD_TEST_NSR_FECS_DESC_A  "prod_test_nsr/NETA_fecs_riscv_desc.bin"

#define NVGPU_PROD_TEST_NSR_GPCCS_DESC_A  "prod_test_nsr/NETA_gpccs_riscv_desc.bin"

#define NVGPU_SAFETY_FECS_DESC_A  "safety/NETA_fecs_riscv_desc.bin"
#define NVGPU_SAFETY_FECS_DESC_B  "safety/NETB_fecs_riscv_desc.bin"
#define NVGPU_SAFETY_FECS_DESC_C  "safety/NETC_fecs_riscv_desc.bin"
#define NVGPU_SAFETY_FECS_DESC_D  "safety/NETD_fecs_riscv_desc.bin"

#define NVGPU_SAFETY_GPCCS_DESC_A  "safety/NETA_gpccs_riscv_desc.bin"
#define NVGPU_SAFETY_GPCCS_DESC_B  "safety/NETB_gpccs_riscv_desc.bin"
#define NVGPU_SAFETY_GPCCS_DESC_C  "safety/NETC_gpccs_riscv_desc.bin"
#define NVGPU_SAFETY_GPCCS_DESC_D  "safety/NETD_gpccs_riscv_desc.bin"

/* index for emulation netlists */
#define NETLIST_FINAL		-1
#define NETLIST_SLOT_A		0
#define NETLIST_SLOT_B		1
#define NETLIST_SLOT_C		2
#define NETLIST_SLOT_D		3
#define MAX_NETLIST		4

#endif /* NVGPU_NETLIST_DEFS_H */
