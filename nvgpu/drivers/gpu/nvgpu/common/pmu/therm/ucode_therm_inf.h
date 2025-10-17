/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_THERM_INF_H
#define NVGPU_PMU_THERM_INF_H

#define CTRL_THERMAL_THERM_DEVICE_CLASS_GPU                             0x01
#define CTRL_THERMAL_THERM_CHANNEL_CLASS_DEVICE                         0x01

#define NV_PMU_THERM_CMD_ID_BOARDOBJ_GRP_SET                         0x0000000B
#define NV_PMU_THERM_MSG_ID_BOARDOBJ_GRP_SET                         0x00000008
#define NV_PMU_THERM_CMD_ID_BOARDOBJ_GRP_GET_STATUS                  0x00000001U
#define NV_PMU_THERM_MSG_ID_BOARDOBJ_GRP_GET_STATUS                  0x00000001U

#define NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_INVALID                                        0x00U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_DEVICE                                         0x01U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_PROVIDER_V10                                   0x02U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_DEBUG_V10                                      0x03U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_AGGREGATION_V10                                0x04U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_ESTIMATION_V10                                 0x05U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_3X                                             0xFDU
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_CLASS_MODEL_10                                       0xFEU

/*!
 * Macros for THERM_CHANNELS class-type
 *
 * _BASE     : Base version of THERMAL_CHANNELS
 * _2X       : 2X version of THERMAL_CHANNELS
 * _3X       : 3X version of THERMAL_CHANNELS
 * _MODEL_10 : This is internal define used by FINN script to generate
 *             model_10 specific data. This is not pointing to any valid
 *             channel clas type used in production thus placed it close
 *             to 0xFF.
 * _INVALID  : Invalid class type
 */
#define NV2080_CTRL_THERMAL_THERM_CHANNELS_CLASS_TYPE_BASE                                     0x00U
#define NV2080_CTRL_THERMAL_THERM_CHANNELS_CLASS_TYPE_2X                                       0x01U
#define NV2080_CTRL_THERMAL_THERM_CHANNELS_CLASS_TYPE_3X                                       0x02U
#define NV2080_CTRL_THERMAL_THERM_CHANNELS_CLASS_TYPE_MODEL_10                                 0xFEU
#define NV2080_CTRL_THERMAL_THERM_CHANNELS_CLASS_TYPE_INVALID                                  NV2080_CTRL_BOARDOBJGRP_CLASS_TYPE_INVALID

/*!
 * Macros for Thermal Channel Types. BOARDOBJ Index for these channel types can
 * be found using @ref NV2080_CTRL_THERMAL_CHANNEL_INFO_PARAMS::priChIdx
 *
 * These channel types are used from MAXWELL and later GPUs. i.e Therm 2.0 and later.
 */
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPU_AVG                                         0x00U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPU_OFFSET_MAX                                  0x01U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_BOARD                                           0x02U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_MEM_MAX                                         0x03U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_PWR_SUPPLY                                      0x04U

/*!
 * Max number of channels that are expose as part of NV2080_CTRL_THERMAL_CHANNEL_INFO_PARAMS::priChIdx.
 * These are all legacy channels. We would like to keep this list as minimum as possible.
 * Here is the list -
 * - GPU_AVG
 * - GPU_OFFSET_MAX
 * - BOARD
 * - MEM_MAX
 * - PWR_SUPPLY
 */
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_MAX_COUNT                                       0x05U

/*!
 * Macros for Thermal Channel Types. BOARDOBJ Index for these channel types CAN NOT
 * be found using @ref NV2080_CTRL_THERMAL_CHANNEL_INFO_PARAMS::priChIdx
 *
 * These channel types are used from BLACKWELL and later GPUs. i.e Therm 3.0 and later.
 */
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPC_OFFSET_MAX                                  0x10U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_HBI_OFFSET_MAX                                  0x11U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_NVL_OFFSET_MAX                                  0x12U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPC_AVG                                         0x13U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPC_MIN                                         0x14U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DIE0_OFFSET_MAX                                 0x15U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DIE0_AVG                                        0x16U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DIE0_MIN                                        0x17U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DIE0_GPC_OFFSET_MAX                             0x18U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DIE0_GPC_AVG                                    0x19U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DIE0_GPC_MIN                                    0x1AU
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DIE1_OFFSET_MAX                                 0x1BU
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DIE1_AVG                                        0x1CU
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DIE1_MIN                                        0x1DU
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DIE1_GPC_OFFSET_MAX                             0x1EU
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DIE1_GPC_AVG                                    0x1FU
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DIE1_GPC_MIN                                    0x20U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DBG_SCI_0                                       0x21U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DBG_SCI_1                                       0x22U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DBG_HBI_0                                       0x23U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DBG_HBI_1                                       0x24U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DBG_NVL                                         0x25U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DBG_GPC_0                                       0x26U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DBG_GPC_1                                       0x27U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DBG_GPC_2                                       0x28U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DBG_GPC_3                                       0x29U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_UNUSED_0                                        0x32U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DBG_MEM                                         0x33U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DBG_SYS                                         0x34U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_SYS_OFFSET_MAX                                  0x35U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_SYS_AVG                                         0x36U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_SYS_MIN                                         0x37U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPU_OFFSET_MAX_SECURE                           0x38U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_DBG_SOC_0                                       0x39U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPU_MIN                                         0x3AU
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE__COUNT                                          0x3BU

// Channel type is not assigned to given channel
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_NOT_ASSIGNED                                    0xFFU

#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPU_MAX                                         NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPU_OFFSET_MAX
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_MEMORY                                          NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_MEM_MAX

/*!
 * Special value corresponding to an invalid Channel Index.
 */
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_INDEX_INVALID                                        NV2080_CTRL_BOARDOBJ_IDX_INVALID_8BIT

/*!
 * Macros for Channel's Relative Location
 */
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_REL_LOC_INT                                          0x00U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_REL_LOC_EXT                                          0x01U

/*!
 * Macros for Channel's target GPU
 */
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TGT_GPU_0                                            0x00U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TGT_GPU_1                                            0x01U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TGT_GPU_2                                            0x02U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TGT_GPU_3                                            0x03U

/*!
 * AGGREGATION_V10 supports following modes of operation:
 * _INVALID : Invalid mode
 * _MAX     : Finds max temperature among all inputs
 * _AVG     : Finds average of all inputs
 */
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_AGGREGATION_MODE_INVALID 0x0U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_AGGREGATION_MODE_MAX     0x1U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_AGGREGATION_MODE_AVG     0x2U

/*!
 * ESTIMATION_V10 supports following modes of operation:
 * _INVALID : Invalid mode
 * _SUM     : Finds sum of Channel1 and Channel2
 * _DIFF    : Finds difference of Channel1 and Channel2
 */
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_ESTIMATION_MODE_INVALID 0x0U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_ESTIMATION_MODE_SUM     0x1U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_ESTIMATION_MODE_DIFF    0x2U

struct nv_pmu_therm_therm_device_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
};

struct nv_pmu_therm_therm_device_boardobj_set {
	struct nv_pmu_boardobj super;
};

struct nv_pmu_therm_therm_device_gpu_gpc_tsosc_boardobj_set {
	struct nv_pmu_therm_therm_device_boardobj_set super;
	u8 gpc_tsosc_idx;
};

struct nv_pmu_therm_therm_device_gpu_sci_boardobj_set {
	struct nv_pmu_therm_therm_device_boardobj_set super;
};

struct nv_pmu_therm_therm_device_i2c_boardobj_set {
	struct nv_pmu_therm_therm_device_boardobj_set super;
	u8 i2c_dev_idx;
};

struct nv_pmu_therm_therm_device_hbm2_site_boardobj_set {
	struct nv_pmu_therm_therm_device_boardobj_set super;
	u8 site_idx;
};

struct nv_pmu_therm_therm_device_hbm2_combined_boardobj_set {
	struct nv_pmu_therm_therm_device_boardobj_set super;
};

union nv_pmu_therm_therm_device_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_therm_therm_device_boardobj_set device;
	struct nv_pmu_therm_therm_device_gpu_gpc_tsosc_boardobj_set
							gpu_gpc_tsosc;
	struct nv_pmu_therm_therm_device_gpu_sci_boardobj_set gpu_sci;
	struct nv_pmu_therm_therm_device_i2c_boardobj_set i2c;
	struct nv_pmu_therm_therm_device_hbm2_site_boardobj_set hbm2_site;
	struct nv_pmu_therm_therm_device_hbm2_combined_boardobj_set
							hbm2_combined;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E32(therm, therm_device);

struct nv_pmu_therm_therm_channel_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
};

struct nv_pmu_therm_therm_channel_boardobj_set {
	struct nv_pmu_boardobj super;
	s16 scaling;
	s16 offsetSW;
	s16 offsetHW;
	s32 temp_min;
	s32 temp_max;
};

struct nv_pmu_therm_therm_channel_device_boardobj_set {
	struct nv_pmu_therm_therm_channel_boardobj_set super;
	u8 therm_dev_idx;
	u8 therm_dev_prov_idx;
};

union nv_pmu_therm_therm_channel_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_therm_therm_channel_boardobj_set channel;
	struct nv_pmu_therm_therm_channel_device_boardobj_set device;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E32(therm, therm_channel);

struct nv_pmu_therm_therm_channel_boardobjgrp_get_status_header {
	struct nv_pmu_boardobjgrp_e32 super;
};

struct nv_pmu_therm_therm_channel_boardobj_get_status
{
	struct nv_pmu_boardobj_query super;
	u32 current_temp;
};

union nv_pmu_therm_therm_channel_boardobj_get_status_union
{
	struct nv_pmu_boardobj_query obj;
	struct nv_pmu_therm_therm_channel_boardobj_get_status therm_channel;
};

NV_PMU_BOARDOBJ_GRP_GET_STATUS_MAKE_E32(therm, therm_channel);

#endif /* NVGPU_PMU_THERM_INF_H */
