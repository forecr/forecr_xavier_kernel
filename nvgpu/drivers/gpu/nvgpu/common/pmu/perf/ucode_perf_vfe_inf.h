/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMUIF_PERFVFE_H
#define NVGPU_PMUIF_PERFVFE_H

#define NV_PMU_PERF_RPC_VFE_EQU_EVAL_VAR_COUNT_MAX                   2U
#define NV_PMU_VFE_VAR_SINGLE_SENSED_FUSE_SEGMENTS_MAX               1U

#define CTRL_PERF_VFE_VAR_TYPE_INVALID                               0x00U
#define CTRL_PERF_VFE_VAR_TYPE_DERIVED                               0x01U
#define CTRL_PERF_VFE_VAR_TYPE_DERIVED_PRODUCT                       0x02U
#define CTRL_PERF_VFE_VAR_TYPE_DERIVED_SUM                           0x03U
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE                                0x04U
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE_FREQUENCY                      0x05U
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE_SENSED                         0x06U
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE_SENSED_FUSE                    0x07U
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE_SENSED_TEMP                    0x08U
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE_VOLTAGE                        0x09U
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE_CALLER_SPECIFIED               0x0AU
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE_GLOBALLY_SPECIFIED             0x0BU
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE_SENSED_FUSE_BASE               0x0CU
#define CTRL_PERF_VFE_VAR_TYPE_SINGLE_SENSED_FUSE_20                 0x0DU
#define CTRL_PERF_VFE_VAR_TYPE_MODEL_10                              0x0EU

#define CTRL_PERF_VFE_PARAMS_TYPE_INVALID          0x00U
#define CTRL_PERF_VFE_PARAMS_TYPE_LITERAL_CONSTANT 0x01U
#define CTRL_PERF_VFE_PARAMS_TYPE_VAR_IDX          0x02U
#define CTRL_PERF_VFE_PARAMS_TYPE_EQU_IDX          0x03U

#define CTRL_FUSE_ID_STRAP_SPEEDO                                     0x00  // SPEEDO value
#define CTRL_FUSE_ID_STRAP_SPEEDO_VERSION                             0x01  // SPEEDO_VERSION
#define CTRL_FUSE_ID_STRAP_IDDQ                                       0x02  // IDDQ value for voltage rail 0
#define CTRL_FUSE_ID_STRAP_IDDQ_VERSION                               0x03  // IDDQ_VERSION
#define CTRL_FUSE_ID_STRAP_IDDQ_1                                     0x04  // IDDQ value for voltage rail 1
#define CTRL_FUSE_ID_STRAP_BOARD_BINNING                              0x05  // Speedo value for BOARD_BINNING
#define CTRL_FUSE_ID_STRAP_BOARD_BINNING_VERSION                      0x06  // BOARD_BINNING_VERSION
#define CTRL_FUSE_ID_STRAP_SRAM_VMIN                                  0x07  // SRAM_VMIN
#define CTRL_FUSE_ID_STRAP_SRAM_VMIN_VERSION                          0x08  // SRAM_VMIN_VERSION
#define CTRL_FUSE_ID_STRAP_BOOT_VMIN_NVVDD                            0x09  // NVVDD Boot Vmin
#define CTRL_FUSE_ID_ISENSE_VCM_OFFSET                                0x0A  // ISENSE_VCM_OFFSET
#define CTRL_FUSE_ID_ISENSE_DIFF_GAIN                                 0x0B  // ISENSE_DIFFERENTIAL_GAIN
#define CTRL_FUSE_ID_ISENSE_DIFF_OFFSET                               0x0C  // ISENSE_DIFFERENTIAL_OFFSET
#define CTRL_FUSE_ID_ISENSE_CALIBRATION_VERSION                       0x0D  // ISENSE_CALIBRATION_VERSION This is a common vers
#define CTRL_FUSE_ID_KAPPA                                            0x0E  // KAPPA - Will link to fuse opt_kappa_info
#define CTRL_FUSE_ID_KAPPA_VERSION                                    0x0F  // KAPPA_VERSION
#define CTRL_FUSE_ID_STRAP_SPEEDO_1                                   0x10  // SPEEDO_1
#define CTRL_FUSE_ID_CPM_VERSION                                      0x11  // Fuse OPT_CPM_REV
#define CTRL_FUSE_ID_CPM_0                                            0x12  // Fuse OPT_CPM0
#define CTRL_FUSE_ID_CPM_1                                            0x13  // Fuse OPT_CPM1
#define CTRL_FUSE_ID_CPM_2                                            0x14  // Fuse OPT_CPM2
#define CTRL_FUSE_ID_ISENSE_VCM_COARSE_OFFSET                         0x15  // ISENSE_VCM_COARSE_OFFSET
#define CTRL_FUSE_ID_STRAP_BOOT_VMIN_MSVDD                            0x16  // MSVDD Boot Vmin
#define CTRL_FUSE_ID_KAPPA_VALID                                      0x17  // KAPPA fuse
#define CTRL_FUSE_ID_IDDQ_NVVDD                                       0x18  // NVVDD IDDQ
#define CTRL_FUSE_ID_IDDQ_MSVDD                                       0x19  // MSVDD IDDQ
#define CTRL_FUSE_ID_STRAP_SPEEDO_2                                   0x1A  // SPEEDO_2
#define CTRL_FUSE_ID_OC_BIN                                           0x1B  // OC_BIN
#define CTRL_FUSE_ID_LV_FMAX_KNOB                                     0x1C  // LV_FMAX_KNOB
#define CTRL_FUSE_ID_MV_FMAX_KNOB                                     0x1D  // MV_FMAX_KNOB
#define CTRL_FUSE_ID_HV_FMAX_KNOB                                     0x1E  // HV_FMAX_KNOB
#define CTRL_FUSE_ID_PSTATE_VMIN_KNOB                                 0x1F  // PSTATE_VMIN_KNOB
#define CTRL_FUSE_ID_ISENSE_DIFFERENTIAL_COARSE_GAIN                  0x20  // ISENSE_DIFFERENTIAL_COARSE_GRAIN
#define CTRL_FUSE_ID_VB_FMAX_KNOB_REV                                 0x21  // VB_FMAX_KNOB_REV
#define CTRL_FUSE_ID_VB_ATEKAPPA0                                     0x22  // VB_ATEKAPPA0
#define CTRL_FUSE_ID_VB_ATEKAPPA1                                     0x23  // VB_ATEKAPPA1
#define CTRL_FUSE_ID_VB_ATEKAPPA2                                     0x24  // VB_ATEKAPPA2
#define CTRL_FUSE_ID_VB_ATEKAPPA_VALID                                0x25  // VB_ATEKAPPA_VALID
#define CTRL_FUSE_ID_VB_ATEKAPPA_REV                                  0x26  // VB_ATEKAPPA_REV
#define CTRL_FUSE_ID_BI_DONE                                          0x27  // BI_DONE
#define CTRL_FUSE_ID_STRAP_SRAM_VMIN_HVA_CP1_NVVDD                    0x28  // STRAP_SRAM_VMIN_HVA_CP1_NVVDD
#define CTRL_FUSE_ID_STRAP_SRAM_VMIN_HVA_CP2_NVVDD                    0x29  // STRAP_SRAM_VMIN_HVA_CP2_NVVDD
#define CTRL_FUSE_ID_STRAP_SRAM_VMIN_NATURAL_CP1_NVVDD                0x2A  // STRAP_SRAM_VMIN_NATURAL_CP1_NVVDD
#define CTRL_FUSE_ID_STRAP_SRAM_VMIN_NATURAL_CP2_NVVDD                0x2B  // STRAP_SRAM_VMIN_NATURAL_CP2_NVVDD
#define CTRL_FUSE_ID_STRAP_SRAM_VMIN_LVA_CP1_NVVDD                    0x2C  // STRAP_SRAM_VMIN_LVA_CP1_NVVDD
#define CTRL_FUSE_ID_STRAP_SRAM_VMIN_LVA_CP2_NVVDD                    0x2D  // STRAP_SRAM_VMIN_LVA_CP2_NVVDD
#define CTRL_FUSE_ID_STRAP_STRAP_SRAM_VMAX_HVA_NVVDD                  0x2E  // STRAP_STRAP_SRAM_VMAX_HVA_NVVDD
#define CTRL_FUSE_ID_STRAP_SRAM_VMAX_NATURAL_NVVDD                    0x2F  // STRAP_SRAM_VMAX_NATURAL_NVVDD
#define CTRL_FUSE_ID_STRAP_SRAM_VMAX_LVA_NVVDD                        0x30  // STRAP_SRAM_VMAX_LVA_NVVDD
#define CTRL_FUSE_ID_STRAP_SRAM_VMIN_HVA_CP1_MSVDD                    0x31  // STRAP_SRAM_VMIN_HVA_CP1_MSVDD
#define CTRL_FUSE_ID_STRAP_SRAM_VMIN_HVA_CP2_MSVDD                    0x32  // STRAP_SRAM_VMIN_HVA_CP2_MSVDD
#define CTRL_FUSE_ID_STRAP_SRAM_VMIN_NATURAL_CP1_MSVDD                0x33  // STRAP_SRAM_VMIN_NATURAL_CP1_MSVDD
#define CTRL_FUSE_ID_STRAP_SRAM_VMIN_NATURAL_CP2_MSVDD                0x34  // STRAP_SRAM_VMIN_NATURAL_CP2_MSVDD
#define CTRL_FUSE_ID_STRAP_SRAM_VMIN_LVA_CP1_MSVDD                    0x35  // STRAP_SRAM_VMIN_LVA_CP1_MSVDD
#define CTRL_FUSE_ID_STRAP_SRAM_VMIN_LVA_CP2_MSVDD                    0x36  // STRAP_SRAM_VMIN_LVA_CP2_MSVDD
#define CTRL_FUSE_ID_STRAP_SRAM_VMAX_HVA_MSVDD                        0x37  // STRAP_SRAM_VMAX_HVA_MSVDD
#define CTRL_FUSE_ID_STRAP_SRAM_VMAX_NATURAL_MSVDD                    0x38  // STRAP_SRAM_VMAX_NATURAL_MSVDD
#define CTRL_FUSE_ID_STRAP_SRAM_VMAX_LVA_MSVDD                        0x39  // STRAP_SRAM_VMAX_LVA_MSVDD
#define CTRL_FUSE_ID_UVD_CAL_HI_SLOPE                                 0x3A  // UVD_CAL_HI_SLOPE
#define CTRL_FUSE_ID_UVD_CAL_HI_INTERCEPT                             0x3B  // UVD_CAL_HI_INTERCEPT
#define CTRL_FUSE_ID_UVD_CAL_MID_SLOPE                                0x3C  // UVD_CAL_MID_SLOPE
#define CTRL_FUSE_ID_UVD_CAL_MID_INTERCEPT                            0x3D  // UVD_CAL_MID_INTERCEPT
#define CTRL_FUSE_ID_UVD_CAL_LO_SLOPE                                 0x3E  // UVD_CAL_LO_SLOPE
#define CTRL_FUSE_ID_UVD_CAL_LO_INTERCEPT                             0x3F  // UVD_CAL_LO_INTERCEPT
#define CTRL_FUSE_ID_UVD_CAL_FUSE_REV                                 0x40  // UVD_CAL_FUSE_REV
#define CTRL_FUSE_ID_VMIN_RC_OFFSET                                   0x41  // VMIN_RC_OFFSET
#define CTRL_FUSE_ID_VMAX_RC_OFFSET                                   0x42  // VMAX_RC_OFFSET
#define CTRL_FUSE_ID_POWER_RATIO                                      0x43  // POWER_RATIO
#define CTRL_FUSE_ID_POWER_RATIO_VERSION                              0x44  // POWER_RATIO_VERSION
#define CTRL_FUSE_ID_VMIN_BLT_OFFSET                                  0x45  // VMIN_BLT_OFFSET
#define CTRL_FUSE_ID_VMAX_BLT_OFFSET                                  0x46  // VMAX_BLT_OFFSET
#define CTRL_FUSE_ID_KAPPA_BLT_OFFSET                                 0x47  // KAPPA_BLT_OFFSET
#define CTRL_FUSE_ID_GPC_BLT_OFFSET                                   0x48  // GPC_BLT_OFFSET
#define CTRL_FUSE_ID_XBAR_BLT_OFFSET                                  0x49  // XBAR_BLT_OFFSET
#define CTRL_FUSE_ID_NVD_BLT_OFFSET                                   0x4A  // NVD_BLT_OFFSET
#define CTRL_FUSE_ID_SYS_BLT_OFFSET                                   0x4B  // SYS_BLT_OFFSET
#define CTRL_FUSE_ID_VMIN_SLT_MAX_OFFSET                              0x4C  // VMIN_SLT_MAX_OFFSET
#define CTRL_FUSE_ID_VMIN_SLT_MIN_OFFSET                              0x4D  // VMIN_SLT_MIN_OFFSET
#define CTRL_FUSE_ID_VMAX_SLT_MAX_OFFSET                              0x4E  // VMAX_SLT_MAX_OFFSET
#define CTRL_FUSE_ID_VMAX_SLT_MIN_OFFSET                              0x4F  // VMAX_SLT_MIN_OFFSET
#define CTRL_FUSE_ID_KAPPA_SLT_MAX_OFFSET                             0x50  // KAPPA_SLT_MAX_OFFSET
#define CTRL_FUSE_ID_KAPPA_SLT_MIN_OFFSET                             0x51  // KAPPA_SLT_MIN_OFFSET
#define CTRL_FUSE_ID_PER_PART_POWER_LIMITS_VERSION                    0x52  // PER_PART_POWER_LIMITS_VERSION
#define CTRL_FUSE_ID_PER_PART_POWER_LIMITS_TGP_OFFSET_MULTIPLIER_STEP 0x53  // PER_PART_POWER_LIMITS_TGP_OFFSET_MULTIPLIER_STEP
#define CTRL_FUSE_ID_SRAM_VMAX_BIN_REV                                0x54  // SRAM_VMAX_BIN_REV
#define CTRL_FUSE_ID_GPC_SRAM_HVA_VMIN_COLD                           0x55  // GPC_SRAM_HVA_VMIN_COLD
#define CTRL_FUSE_ID_GPC_SRAM_HVA_VMIN_HOT                            0x56  // GPC_SRAM_HVA_VMIN_HOT
#define CTRL_FUSE_ID_GPC_SRAM_HVA_VMIN_MID                            0x57  // GPC_SRAM_HVA_VMIN_MID
#define CTRL_FUSE_ID_GPC_SRAM_LVA_VMIN_COLD                           0x58  // GPC_SRAM_LVA_VMIN_COLD
#define CTRL_FUSE_ID_GPC_SRAM_LVA_VMIN_HOT                            0x59  // GPC_SRAM_LVA_VMIN_HOT
#define CTRL_FUSE_ID_GPC_SRAM_LVA_VMIN_MID                            0x5A  // GPC_SRAM_LVA_VMIN_MID
#define CTRL_FUSE_ID_GPC_SRAM_NA_VMIN_COLD                            0x5B  // GPC_SRAM_NA_VMIN_COLD
#define CTRL_FUSE_ID_GPC_SRAM_NA_VMIN_HOT                             0x5C  // GPC_SRAM_NA_VMIN_HOT
#define CTRL_FUSE_ID_GPC_SRAM_NA_VMIN_MID                             0x5D  // GPC_SRAM_NA_VMIN_MID
#define CTRL_FUSE_ID_GPM_SRAM_HVA_VMIN_COLD                           0x5E  // GPM_SRAM_HVA_VMIN_COLD
#define CTRL_FUSE_ID_GPM_SRAM_HVA_VMIN_HOT                            0x5F  // GPM_SRAM_HVA_VMIN_HOT
#define CTRL_FUSE_ID_GPM_SRAM_HVA_VMIN_MID                            0x60  // GPM_SRAM_HVA_VMIN_MID
#define CTRL_FUSE_ID_GPM_SRAM_LVA_VMIN_COLD                           0x61  // GPM_SRAM_LVA_VMIN_COLD
#define CTRL_FUSE_ID_GPM_SRAM_LVA_VMIN_HOT                            0x62  // GPM_SRAM_LVA_VMIN_HOT
#define CTRL_FUSE_ID_GPM_SRAM_LVA_VMIN_MID                            0x63  // GPM_SRAM_LVA_VMIN_MID
#define CTRL_FUSE_ID_GPM_SRAM_NA_VMIN_COLD                            0x64  // GPM_SRAM_NA_VMIN_COLD
#define CTRL_FUSE_ID_GPM_SRAM_NA_VMIN_HOT                             0x65  // GPM_SRAM_NA_VMIN_HOT
#define CTRL_FUSE_ID_GPM_SRAM_NA_VMIN_MID                             0x66  // GPM_SRAM_NA_VMIN_MID
#define CTRL_FUSE_ID_GPC_SRAM_WA_VMAX_COLD                            0x67  // GPC_SRAM_WA_VMAX_COLD
#define CTRL_FUSE_ID_GPC_SRAM_WA_VMAX_HOT                             0x68  // GPC_SRAM_WA_VMAX_HOT
#define CTRL_FUSE_ID_GPM_SRAM_WA_VMAX_COLD                            0x69  // GPM_SRAM_WA_VMAX_COLD
#define CTRL_FUSE_ID_GPM_SRAM_WA_VMAX_HOT                             0x6A  // GPM_SRAM_WA_VMAX_HOT
#define CTRL_FUSE_ID_GPM_SRAM_HVA_VMAX_COLD                           0x6B  // GPM_SRAM_HVA_VMAX_COLD
#define CTRL_FUSE_ID_GPC_SRAM_NA_VMAX_COLD                            0x6C  // GPC_SRAM_NA_VMAX_COLD
#define CTRL_FUSE_ID_GPM_SRAM_NA_VMAX_HOT                             0x6D  // GPM_SRAM_NA_VMAX_HOT
#define CTRL_FUSE_ID_GPC_SRAM_HVA_VMAX_COLD                           0x6E  // GPC_SRAM_HVA_VMAX_COLD
#define CTRL_FUSE_ID_GPC_SRAM_NA_VMAX_HOT                             0x6F  // GPC_SRAM_NA_VMAX_HOT
#define CTRL_FUSE_ID_GPM_SRAM_HVA_VMAX_HOT                            0x70  // GPM_SRAM_HVA_VMAX_HOT
#define CTRL_FUSE_ID_GPM_SRAM_NA_VMAX_COLD                            0x71  // GPM_SRAM_NA_VMAX_COLD
#define CTRL_FUSE_ID_GPC_SRAM_HVA_VMAX_HOT                            0x72  // GPC_SRAM_HVA_VMAX_HOT


#define CTRL_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_NONE                  0x00U
#define CTRL_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_VALUE                 0x01U
#define CTRL_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_OFFSET                0x02U
#define CTRL_PERF_VFE_VAR_SINGLE_OVERRIDE_TYPE_SCALE                 0x03U

#define CTRL_PERF_VFE_EQU_TYPE_INVALID                               0x00U
#define CTRL_PERF_VFE_EQU_TYPE_COMPARE                               0x01U
#define CTRL_PERF_VFE_EQU_TYPE_MINMAX                                0x02U
#define CTRL_PERF_VFE_EQU_TYPE_QUADRATIC                             0x03U
#define CTRL_PERF_VFE_EQU_TYPE_SCALAR                                0x04U
#define CTRL_PERF_VFE_EQU_TYPE_MODEL_10                              0x05U
#define CTRL_PERF_VFE_EQU_TYPE_QUADRATIC_N                           0x06U

#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_UNITLESS                       0x00U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_FREQ_MHZ                       0x01U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_VOLT_UV                        0x02U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_VF_GAIN                        0x03U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_VOLT_DELTA_UV                  0x04U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_WORK_TYPE                      0x06U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_UTIL_RATIO                     0x07U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_WORK_FB_NORM                   0x08U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_POWER_MW                       0x09U
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_PWR_OVER_UTIL_SLOPE            0x0AU
#define CTRL_PERF_VFE_EQU_OUTPUT_TYPE_VIN_CODE                       0x0BU

#define CTRL_PERF_VFE_EQU_QUADRATIC_COEFF_COUNT                      0x03U

#define CTRL_PERF_VFE_EQU_COMPARE_FUNCTION_EQUAL                     0x00U
#define CTRL_PERF_VFE_EQU_COMPARE_FUNCTION_GREATER_EQ                0x01U
#define CTRL_PERF_VFE_EQU_COMPARE_FUNCTION_GREATER                   0x02U

union nv_pmu_perf_vfe_var_type_data {
	u8 uid;
	u8 clk_domain_idx;
};

struct nv_pmu_perf_vfe_var_value {
	u8 var_type;
	union nv_pmu_perf_vfe_var_type_data var_type_data;
	u8 reserved[2];
	u32 var_value;
};

union nv_pmu_perf_vfe_equ_result {
	u32 freq_m_hz;
	u32 voltu_v;
	u32 vf_gain;
	int volt_deltau_v;
	u32 work_type;
	u32 util_ratio;
	u32 work_fb_norm;
	u32 power_mw;
	u32 pwr_over_util_slope;
	int vin_code;
};

struct nv_pmu_perf_rpc_vfe_equ_eval {
	u8 equ_idx;
	u8 var_count;
	u8 output_type;
	struct nv_pmu_perf_vfe_var_value var_values[
		NV_PMU_PERF_RPC_VFE_EQU_EVAL_VAR_COUNT_MAX];
	union nv_pmu_perf_vfe_equ_result result;
};

struct nv_pmu_rpc_struct_perf_vfe_eval {
	/*[IN/OUT] Must be first field in RPC structure */
	struct nv_pmu_rpc_header hdr;
	struct nv_pmu_perf_rpc_vfe_equ_eval data;
	u32  scratch[1];
};

struct nv_pmu_perf_rpc_vfe_load {
	bool b_load;
};

struct nv_pmu_perf_vfe_var_boardobjgrp_get_status_header {
	struct nv_pmu_boardobjgrp_e32 super;
};

struct nv_pmu_perf_vfe_var_get_status_super {
	struct nv_pmu_boardobj_query obj;
};

union ctrl_perf_vfe_var_single_sensed_fuse_value_data {
	int signed_value;
	u32 unsigned_value;
};

struct ctrl_perf_vfe_var_single_sensed_fuse_value {
	bool b_signed;
	union ctrl_perf_vfe_var_single_sensed_fuse_value_data data;
};

struct nv_pmu_perf_vfe_var_single_sensed_fuse_get_status {
	struct nv_pmu_perf_vfe_var_get_status_super super;
	struct ctrl_perf_vfe_var_single_sensed_fuse_value fuse_value_integer;
	struct ctrl_perf_vfe_var_single_sensed_fuse_value fuse_value_hw_integer;
	u8 fuse_version;
	bool b_version_check_failed;
};

union nv_pmu_perf_vfe_var_boardobj_get_status_union {
	struct nv_pmu_boardobj_query obj;
	struct nv_pmu_perf_vfe_var_get_status_super super;
	struct nv_pmu_perf_vfe_var_single_sensed_fuse_get_status fuse_status;
};

NV_PMU_BOARDOBJ_GRP_GET_STATUS_MAKE_E32(perf, vfe_var);

struct nv_pmu_perf_vfe_var_boardobj_grp_get_status_pack {
	struct nv_pmu_perf_vfe_var_boardobj_grp_get_status pri;
	struct nv_pmu_perf_vfe_var_boardobj_grp_get_status rppm;
};

struct nv_pmu_vfe_var {
	struct nv_pmu_boardobj super;
	u32 out_range_min;
	u32 out_range_max;
	struct ctrl_boardobjgrp_mask_e32 mask_dependent_vars;
	struct ctrl_boardobjgrp_mask_e255 mask_dependent_equs;
};

struct nv_pmu_vfe_var_derived {
	struct nv_pmu_vfe_var super;
};

struct nv_pmu_vfe_var_derived_product {
	struct nv_pmu_vfe_var_derived super;
	u8 var_idx0;
	u8 var_idx1;
};

struct nv_pmu_vfe_var_derived_sum {
	struct nv_pmu_vfe_var_derived super;
	u8 var_idx0;
	u8 var_idx1;
};

struct nv_pmu_vfe_var_single {
	struct nv_pmu_vfe_var super;
	u8 override_type;
	bool b_override_die_aware;
	u32 override_value[2];
};

struct nv_pmu_vfe_var_single_frequency {
	struct nv_pmu_vfe_var_single super;
	u8 clk_domain_idx;
};

struct nv_pmu_vfe_var_single_caller_specified {
	struct nv_pmu_vfe_var_single super;
	u8 uid;
};

struct nv_pmu_vfe_var_single_sensed {
	struct nv_pmu_vfe_var_single super;
};

struct ctrl_bios_vfield_register_segment_super {
	u8 low_bit;
	u8 high_bit;
};

struct ctrl_bios_vfield_register_segment_reg {
	struct ctrl_bios_vfield_register_segment_super super;
	u32 addr;
};

struct ctrl_bios_vfield_register_segment_index_reg {
	struct ctrl_bios_vfield_register_segment_super super;
	u32 addr;
	u32 reg_index;
	u32 index;
};

union ctrl_bios_vfield_register_segment_data {
	struct ctrl_bios_vfield_register_segment_reg reg;
	struct ctrl_bios_vfield_register_segment_index_reg index_reg;
};

struct ctrl_bios_vfield_register_segment {
	u8 type;
	union ctrl_bios_vfield_register_segment_data data;
};

struct ctrl_perf_vfe_var_single_sensed_fuse_info {
	u8 segment_count;
	struct ctrl_bios_vfield_register_segment
		segments[NV_PMU_VFE_VAR_SINGLE_SENSED_FUSE_SEGMENTS_MAX];
};

struct ctrl_perf_vfe_var_single_sensed_fuse_override_info {
	u32 fuse_val_override;
	u8 b_fuse_regkey_override;
};

struct ctrl_perf_vfe_var_single_sensed_fuse_vfield_info {
	struct ctrl_perf_vfe_var_single_sensed_fuse_info fuse;
	u32 fuse_val_default;
	u32 hw_correction_scale;
	int hw_correction_offset;
	u8 v_field_id;
};

struct ctrl_perf_vfe_var_single_sensed_fuse_ver_info {
	u8 ver_expected;
	bool b_ver_expected_is_mask;
	bool b_ver_check;
	bool b_ver_check_ignore;
	bool b_use_default_on_ver_checkfail;
};


struct ctrl_perf_vfe_var_single_sensed_fuse_ver_vfield_info {
	struct ctrl_perf_vfe_var_single_sensed_fuse_info fuse;
	u8 ver_expected;
	bool b_ver_expected_is_mask;
	bool b_ver_check;
	bool b_ver_check_ignore;
	bool b_use_default_on_ver_check_fail;
	u8 v_field_id_ver;
};

struct nv_pmu_vfe_var_single_sensed_fuse {
	struct nv_pmu_vfe_var_single_sensed super;
	struct ctrl_perf_vfe_var_single_sensed_fuse_override_info override_info;
	struct ctrl_perf_vfe_var_single_sensed_fuse_vfield_info vfield_info;
	struct ctrl_perf_vfe_var_single_sensed_fuse_ver_vfield_info
								vfield_ver_info;
	struct ctrl_perf_vfe_var_single_sensed_fuse_value fuse_val_default;
	bool b_fuse_value_signed;
};

struct nv_pmu_vfe_var_single_sensed_fuse_base {
	struct nv_pmu_vfe_var_single_sensed super;
	struct ctrl_perf_vfe_var_single_sensed_fuse_override_info       override_info;
	bool   b_fuse_value_signed;
	struct ctrl_perf_vfe_var_single_sensed_fuse_value fuse_val_integer;
	struct ctrl_perf_vfe_var_single_sensed_fuse_value fuse_val_hw_integer;
	struct ctrl_perf_vfe_var_single_sensed_fuse_value fuse_val_default;
	u8 fuse_version;
	bool b_version_check_done;
	struct ctrl_perf_vfe_var_single_sensed_fuse_ver_info fuse_ver_info;
	u32 hw_correction_scale;
	s32 hw_correction_offset;
};

struct nv_pmu_vfe_var_single_sensed_fuse_20 {
	struct nv_pmu_vfe_var_single_sensed_fuse_base super;
	u16 fuse_id;
	u16 fuse_id_ver;
};

struct nv_pmu_vfe_var_single_sensed_temp {
	struct nv_pmu_vfe_var_single_sensed super;
	u8 therm_channel_index;
	int temp_hysteresis_positive;
	int temp_hysteresis_negative;
	int temp_default;
};

struct nv_pmu_vfe_var_single_voltage {
	struct nv_pmu_vfe_var_single super;
};

struct nv_pmu_perf_vfe_var_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
	u8 polling_periodms;
};

union nv_pmu_perf_vfe_var_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_vfe_var var;
	struct nv_pmu_vfe_var_derived var_derived;
	struct nv_pmu_vfe_var_derived_product var_derived_product;
	struct nv_pmu_vfe_var_derived_sum var_derived_sum;
	struct nv_pmu_vfe_var_single var_single;
	struct nv_pmu_vfe_var_single_frequency var_single_frequiency;
	struct nv_pmu_vfe_var_single_sensed var_single_sensed;
	struct nv_pmu_vfe_var_single_sensed_fuse var_single_sensed_fuse;
	struct nv_pmu_vfe_var_single_sensed_temp var_single_sensed_temp;
	struct nv_pmu_vfe_var_single_voltage var_single_voltage;
	struct nv_pmu_vfe_var_single_caller_specified
					var_single_caller_specified;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E255(perf, vfe_var);

struct nv_pmu_perf_vfe_var_boardobj_grp_set_pack {
	struct nv_pmu_perf_vfe_var_boardobj_grp_set pri;
	struct nv_pmu_perf_vfe_var_boardobj_grp_set rppm;
};

struct nv_pmu_vfe_equ {
	struct nv_pmu_boardobj super;
	u8 var_idx;
	u8 equ_idx_next;
	u8 output_type;
	u32 out_range_min;
	u32 out_range_max;
};

struct nv_pmu_vfe_equ_compare {
	struct nv_pmu_vfe_equ super;
	u8 func_id;
	u8 equ_idx_true;
	u8 equ_idx_false;
	u32 criteria;
};

struct nv_pmu_vfe_equ_minmax {
	struct nv_pmu_vfe_equ super;
	bool b_max;
	u8 equ_idx0;
	u8 equ_idx1;
};

struct nv_pmu_vfe_equ_quadratic {
	struct nv_pmu_vfe_equ super;
	u32 coeffs[CTRL_PERF_VFE_EQU_QUADRATIC_COEFF_COUNT];
};

struct nv_pmu_vfe_equ_scalar {
	struct nv_pmu_vfe_equ super;
	u8 equ_idx_to_scale;
};

struct nv_pmu_perf_vfe_equ_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e255 super;
};

union nv_pmu_perf_vfe_equ_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_vfe_equ equ;
	struct nv_pmu_vfe_equ_compare equ_comapre;
	struct nv_pmu_vfe_equ_minmax equ_minmax;
	struct nv_pmu_vfe_equ_quadratic equ_quadratic;
	struct nv_pmu_vfe_equ_scalar equ_scalar;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E2048(perf, vfe_equ);

struct nv_pmu_perf_vfe_equ_boardobj_grp_set_pack {
	struct nv_pmu_perf_vfe_equ_boardobj_grp_set pri;
	struct nv_pmu_perf_vfe_var_boardobj_grp_set rppm;
};

#endif /* NVGPU_PMUIF_PERFVFE_H */
