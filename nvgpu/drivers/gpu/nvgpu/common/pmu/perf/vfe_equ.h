/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PERF_VFE_EQU_H
#define NVGPU_PERF_VFE_EQU_H

#include "ucode_perf_vfe_inf.h"

struct vfe_params {
	u8 param_type;
	union {
		u32 literal_value;
		u8 var_idx;
		u16 equ_idx;
	} param;
};

struct vfe_equs {
	struct boardobjgrp_e2048 super;
	bool b_vfe_equ_16bit_idx_supported;
};

struct vfe_equ {
	struct pmu_board_obj super;
	u8 var_idx;
	u8 equ_idx_next;
	u8 output_type;
	u32 out_range_min;
	u32 out_range_max;
	struct boardobjgrpmask_e255 mask_depending_vars;
	int (*mask_depending_build)(struct gk20a *g,
			struct boardobjgrp *pboardobjgrp,
			struct vfe_equ *pvfe_equ);
	bool b_is_dynamic_valid;
	bool b_is_dynamic;
};

struct vfe_equ_compare {
	struct vfe_equ super;
	u8 func_id;
	bool b_is_criteria_equ;
	u8 equ_idx_true;
	u8 equ_idx_false;
	u32 criteria;
};

struct vfe_equ_minmax {
	struct vfe_equ super;
	bool b_max;
	u8 equ_idx0;
	u8 equ_idx1;
};

 // Type-Specific Parameter DWORD 0 - Type = _QUADRATIC_N
#define VBIOS_VFE_3X_EQU_ENTRY_PAR0_QUADRATIC_N_PARAM_START_IDX             15:0 // Index to the first entry in the param table for this equation

#define VBIOS_VFE_3X_EQU_ENTRY_PAR0_QUADRATIC_N_PARAM_START_IDX_MASK       0xFFFF
#define VBIOS_VFE_3X_EQU_ENTRY_PAR0_QUADRATIC_N_PARAM_START_IDX_SHIFT      0U
#define VBIOS_VFE_3X_EQU_ENTRY_PAR0_QUADRATIC_N_FLAVOR                     19:16 // Class or structure of this QuadraticN equation
#define VBIOS_VFE_3X_EQU_ENTRY_PAR0_QUADRATIC_N_FLAVOR_MASK                0xF0000U
#define VBIOS_VFE_3X_EQU_ENTRY_PAR0_QUADRATIC_N_FLAVOR_SHIFT               16U
#define VBIOS_VFE_3X_EQU_ENTRY_PAR0_QUADRATIC_N_FLAVOR_INVALID               0x0
#define VBIOS_VFE_3X_EQU_ENTRY_PAR0_QUADRATIC_N_FLAVOR_QUADRATIC_3           0x1
#define VBIOS_VFE_3X_EQU_ENTRY_PAR0_QUADRATIC_N_FLAVOR_QUADRATIC_6           0x2
#define VBIOS_VFE_3X_EQU_ENTRY_PAR0_QUADRATIC_N_FLAVOR_QUADRATIC_10          0x3
#define VBIOS_VFE_3X_EQU_ENTRY_PAR0_QUADRATIC_N_RSVD                       31:20 // Must be zero

#define CTRL_PERF_VFE_EQU_QUADRATIC_N_3_ARG_COUNT         0x01U
#define CTRL_PERF_VFE_EQU_QUADRATIC_N_6_ARG_COUNT         0x02U
#define CTRL_PERF_VFE_EQU_QUADRATIC_N_10_ARG_COUNT        0x03U

#define CTRL_PERF_VFE_EQU_QUADRATIC_N_3_COEFF_COUNT       0x03U
#define CTRL_PERF_VFE_EQU_QUADRATIC_N_6_COEFF_COUNT       0x06U
#define CTRL_PERF_VFE_EQU_QUADRATIC_N_10_COEFF_COUNT      0x0AU

#define CTRL_PERF_VFE_EQU_QUADRATIC_N_MAX_COEFF_COUNT     0x03U

struct vfe_equ_quadratic {
	struct vfe_equ super;
	u32 coeffs[CTRL_PERF_VFE_EQU_QUADRATIC_COEFF_COUNT];
};

struct vfe_equ_scalar {
	struct vfe_equ super;
	u8 equ_idx_to_scale;
};

struct vbios_vfe_param_3x {
	u8 type;
	u32 value;
}__attribute__((packed));

struct ctrl_perf_vfe_param {
	u8 param_type;
	union {
		u32 literal_value;
		u8 var_idx;
		u16 equ_idx;
	} param;
};

struct vfe_equ_quadratic_n {
	struct vfe_equ super;
	struct ctrl_perf_vfe_param args[CTRL_PERF_VFE_EQU_QUADRATIC_N_10_ARG_COUNT];
	u8 arg_count;
	struct ctrl_perf_vfe_param coeffs[CTRL_PERF_VFE_EQU_QUADRATIC_N_MAX_COEFF_COUNT];
	u8 coeff_count;
};

int perf_vfe_equ_sw_setup(struct gk20a *g);
int perf_vfe_equ_pmu_setup(struct gk20a *g);
#endif /* NVGPU_PERF_VFE_EQU_H */
