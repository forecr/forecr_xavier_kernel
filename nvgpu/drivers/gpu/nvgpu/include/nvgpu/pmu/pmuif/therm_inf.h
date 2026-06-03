/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMU_PMUIF_THERM_INF_H
#define NVGPU_PMU_PMUIF_THERM_INF_H

#include <common/pmu/boardobj/ucode_boardobj_inf.h>

/* Some userspace builds have __packed defined already */
#if !defined(__packed)
#define __packed __attribute__((packed))
#endif /* __packed */

#define CTRL_THERMAL_THERM_DEVICE_CLASS_GPU                             0x01
#define CTRL_THERMAL_THERM_CHANNEL_CLASS_DEVICE                         0x01

#define NV_PMU_THERM_CMD_ID_BOARDOBJ_GRP_SET                         0x0000000B
#define NV_PMU_THERM_MSG_ID_BOARDOBJ_GRP_SET                         0x00000008
#define NV_PMU_THERM_CMD_ID_BOARDOBJ_GRP_GET_STATUS                  0x00000001U
#define NV_PMU_THERM_MSG_ID_BOARDOBJ_GRP_GET_STATUS                  0x00000001U

#define NV_TYPES_NVSFXP11_5_TO_NV_TEMP(x) ((s32)(x) >> 5)
#define NV_TEMP_TO_NV_TYPES_NVSFXP11_5(x) ((s32)(x) << 5)

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
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPU_GTZ_MAX                                     0x00U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPU_GTZ_AVG                                     0x02U
#define NV2080_CTRL_THERMAL_THERM_CHANNEL_TYPE_GPU_GTZ_MIN                                     0x09U
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

// TODO: copy from nvgpu/common/pmu/boardobj/ucode_boardobj_inf.h
/*!
 * Base structure describing a BOARDOBJ_INTERFACE object.
 */
typedef struct NV_PMU_BOARDOBJ_INTERFACE {
    u8 rsvd;
} NV_PMU_BOARDOBJ_INTERFACE;

/*!
 * @brief   Structure to represent the "model" in which a BOARDOBJ is
 *          implemented in the RM-PMU layer.
 */
typedef struct
{
    /*!
     * Base class. Must be first member.
     */
    NV_PMU_BOARDOBJ_INTERFACE super;
} NV_PMU_BOARDOBJ_IFACE_MODEL_THERM;

typedef struct
{
    /*!
     * Base class. Must be first member.
     */
    NV_PMU_BOARDOBJ_IFACE_MODEL_THERM super;
} NV_PMU_BOARDOBJ_IFACE_MODEL_10_THERM;


typedef struct NV2080_CTRL_BOARDOBJ_INTERFACE {
    /*!
     * @brief   Reserved for future use cases.
     */
    u8 rsvd;
} NV2080_CTRL_BOARDOBJ_INTERFACE;
typedef struct NV2080_CTRL_BOARDOBJ_INTERFACE *PNV2080_CTRL_BOARDOBJ_INTERFACE;

/*!
 * @brief   Structure representing INFO for a BOARDOBJ_IFACE_MODEL interface
 */
typedef struct NV2080_CTRL_BOARDOBJ_INFO_IFACE_MODEL {
    /*!
     * Super class INFO
     */
    NV2080_CTRL_BOARDOBJ_INTERFACE super;
} NV2080_CTRL_BOARDOBJ_INFO_IFACE_MODEL;

/*!
 * @brief   Structure representing INFO for a BOARDOBJ_IFACE_MODEL_10
 *          interface
 */
typedef struct NV2080_CTRL_BOARDOBJ_INFO_IFACE_MODEL_10 {
    /*!
     * Super class INFO
     */
    NV2080_CTRL_BOARDOBJ_INFO_IFACE_MODEL super;
} NV2080_CTRL_BOARDOBJ_INFO_IFACE_MODEL_10;


struct nv_pmu_therm_therm_device_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
};

struct nv_pmu_therm_therm_device_3x_boardobj_set {
	struct nv_pmu_boardobj super;
	u8 reserved1[2];
	u8 devType;
	u8 numProv;
	u32 provSupportMask;
	s32 errorTemp;
} __attribute__((packed));


struct nv_pmu_therm_therm_device_boardobj_set {
	struct nv_pmu_boardobj super;
};

struct nv_pmu_therm_therm_device_gpu_gpc_tsosc_boardobj_set {
	struct nv_pmu_therm_therm_device_3x_boardobj_set super;
	u8 gpc_tsosc_idx;
};

struct nv_pmu_therm_therm_device_gpu_sci_boardobj_set {
	struct nv_pmu_therm_therm_device_3x_boardobj_set super;
};

struct nv_pmu_therm_therm_device_i2c_boardobj_set {
	struct nv_pmu_therm_therm_device_3x_boardobj_set super;
	u8 i2c_dev_idx;
};

struct nv_pmu_therm_therm_device_hbm2_site_boardobj_set {
	struct nv_pmu_therm_therm_device_3x_boardobj_set super;
	u8 site_idx;
};

struct nv_pmu_therm_therm_device_hbm2_combined_boardobj_set {
	struct nv_pmu_therm_therm_device_3x_boardobj_set super;
};

struct nv_pmu_therm_therm_device_gpu_tz_v10_boardobj_set {
	struct nv_pmu_therm_therm_device_3x_boardobj_set super;
    u8 hwIdx;
};

struct nv_pmu_therm_therm_device_gpu_tsense_v10_boardobj_set {
	struct nv_pmu_therm_therm_device_3x_boardobj_set super;
    u8 location;
};

struct nv_pmu_therm_therm_device_gpu_mini_tsense_v10_boardobj_set {
	struct nv_pmu_therm_therm_device_3x_boardobj_set super;
    u8 location;
};

struct nv_pmu_therm_therm_device_mem_v10_boardobj_set {
	struct nv_pmu_therm_therm_device_3x_boardobj_set super;
    u8 sensorType;
    u16 refreshPeriodLimitMs;
};

union nv_pmu_therm_therm_device_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_therm_therm_device_boardobj_set               device;
	struct nv_pmu_therm_therm_device_gpu_gpc_tsosc_boardobj_set gpu_gpc_tsosc;
	struct nv_pmu_therm_therm_device_gpu_sci_boardobj_set       gpu_sci;
	struct nv_pmu_therm_therm_device_i2c_boardobj_set           i2c;
	struct nv_pmu_therm_therm_device_hbm2_site_boardobj_set     hbm2_site;
	struct nv_pmu_therm_therm_device_hbm2_combined_boardobj_set hbm2_combined;
    struct nv_pmu_therm_therm_device_3x_boardobj_set            v3x;
    struct nv_pmu_therm_therm_device_gpu_tz_v10_boardobj_set    gpuTz10;
    struct nv_pmu_therm_therm_device_gpu_tsense_v10_boardobj_set      gpuTsense10;
    struct nv_pmu_therm_therm_device_gpu_mini_tsense_v10_boardobj_set gpuMiniTsense10;
    struct nv_pmu_therm_therm_device_mem_v10_boardobj_set             mem10;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E32(therm, therm_device);

struct nv_pmu_therm_therm_channel_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e255 super;
	bool bSnapshot;
};

struct nv_pmu_therm_therm_channel_boardobj_set {
	struct nv_pmu_boardobj super;
	bool bScaling;
	s16 scaling;
	s16 offsetSw;
	bool bBounds;
	u8 reserveds1;
	s32 temp_min;
	s32 temp_max;
	bool tempsim_bSupported;
	bool tempsim_bEnabled;
	s32 tempsim_targetTemp;
	u8 chType;
	u8 reserveds3[7];
	bool bPublicVisibility;
	u8 reserveds4[3];
	u8 thermDevIdx;
	u8 reserveds5[1];
	u8 thermDevProvIdx;
};

struct nv_pmu_therm_therm_channel_3x_boardobj_set {
	struct nv_pmu_therm_therm_channel_boardobj_set super;
	bool bVisOverrideSupported;
};

struct nv_pmu_therm_therm_channel_model_10_boardobj_set {
	struct nv_pmu_therm_therm_channel_boardobj_set super;
	bool bCopyIn;
	bool bHdrExport;
};

struct nv_pmu_therm_therm_channel_provider_v10_boardobj_set {
	struct nv_pmu_therm_therm_channel_boardobj_set super;
	u8 providerIdx;
};

typedef struct nv_pmu_therm_therm_channel_3x_boardobj_set NV_PMU_THERM_THERM_CHANNEL_3X_BOARDOBJ_SET;
typedef struct nv_pmu_therm_therm_channel_model_10_boardobj_set NV_PMU_THERM_CHANNEL_MODEL_10_BOARDOBJ_SET;
typedef struct nv_pmu_therm_therm_channel_provider_v10_boardobj_set NV_PMU_THERM_THERM_CHANNEL_PROVIDER_V10_BOARDOBJ_SET;

struct nv_pmu_therm_therm_channel_device_boardobj_set {
	struct nv_pmu_therm_therm_channel_boardobj_set super;
};

union nv_pmu_therm_therm_channel_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_therm_therm_channel_boardobj_set channel;
	NV_PMU_THERM_CHANNEL_MODEL_10_BOARDOBJ_SET model10;
	NV_PMU_THERM_THERM_CHANNEL_PROVIDER_V10_BOARDOBJ_SET provider10;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E255(therm, therm_channel);

struct nv_pmu_therm_therm_channel_boardobjgrp_get_status_header {
	struct nv_pmu_boardobjgrp_e255 super;
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

/*!
 * Macros for Thermal Device Classes
 *
 * _INVALID             : Invalid Therm device class
 * _GPU                 : Therm device class type that uses on-chip tsense for
 *                        temperature sensing.
 *                        It is part of THERM_DEVICES_CLASS_TYPE_2X.
 * _GPU_GPC_TSOSC       : Therm device class type that uses on-chip tsosc for
 *                        temperature sensing.
 *                        It is part of THERM_DEVICES_CLASS_TYPE_2X.
 * _GPU_SCI             : Therm device class type that uses on-chip mini-tsense
 *                        for temperature sensing.
 *                        It is part of THERM_DEVICES_CLASS_TYPE_2X.
 * _GPU_GPC_COMBINED    : Therm device class type that uses on-chip GPC tsense
 *                        for temperature sensing.
 *                        It is part of THERM_DEVICES_CLASS_TYPE_2X.
 * _GDDR6_X_COMBINED    : Therm device class type that provides GDDR6X memory
 *                        temperature.
 *                        It is part of THERM_DEVICES_CLASS_TYPE_2X.
 * _I2CS_<xyz>          : Therm device class type that uses board level IC for
 *                        temperature sensing.
 *                        It is part of THERM_DEVICES_CLASS_TYPE_2X.
 * _RM                  : This device is no longer in used.
 * _HBM2_<xyz>          : Therm device class type that provides HBM2 memory
 *                        temperature.
 *                        It is part of THERM_DEVICES_CLASS_TYPE_2X.
 * _HBM3_<xyz>          : Therm device class type that provides HBM3 memory
 *                        temperature.
 *                        It is part of THERM_DEVICES_CLASS_TYPE_2X.
 * _GPU_GTZ_V10         : Therm device class type that uses on-chip General
 *                        Thermal Zone for temperature sensing.
 *                        It is part of THERM_DEVICES_CLASS_TYPE_3X.
 * _GPU_STZ_V10         : Therm device class type that uses on-chip Secure
 *                        Thermal Zone for temperature sensing.
 *                        It is part of THERM_DEVICES_CLASS_TYPE_3X.
 * _GPU_TSENSE_V10      : Therm device class type that uses on-chip tsense for
 *                        temperature sensing.
 *                        It is part of THERM_DEVICES_CLASS_TYPE_3X.
 * _GPU_MINI_TSENSE_V10 : Therm device class type that uses on-chip mini-tsense
 *                        for temperature sensing.
 *                        It is part of THERM_DEVICES_CLASS_TYPE_3X.
 * _MEM_GDDRX_SW_V10    : Therm device class type that provides GDDRX memory
 *                        temperature.
 *                        It is part of THERM_DEVICES_CLASS_TYPE_3X.
 * _MEM_V10             : Therm device class type that provides memory
 *                        temperature.
 *                        It is part of THERM_DEVICES_CLASS_TYPE_3X.
 * _BOARD_I2C_V10       : Therm device class type that uses board level IC for
 *                        temperature sensing.
 *                        It is part of THERM_DEVICES_CLASS_TYPE_3X.
 * _MEM_HBM_SW_V10      : Therm device class type that provides HBM memory
 *                        temperature.
 *                        It is part of THERM_DEVICES_CLASS_TYPE_3X.
 * _GPU_TZ_V10          : This is internal define used by FINN script to generate
 *                        GPU_TZ_V10 specific data. This is not pointing to any
 *                        valid device type used in production thus placed it close
 *                        to 0xFF.
 * _3X                  : This is internal define used by FINN script to generate
 *                        3X specific data. This is not pointing to any valid
 *                        device type used in production thus placed it close
 *                        to 0xFF.
 * _MODEL_10            : This is internal define used by FINN script to generate
 *                        model_10 specific data. This is not pointing to any
 *                        valid device type used in production thus placed it
 *                        close to 0xFF.
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_INVALID                                         0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU                                             0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_GPC_TSOSC                                   0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_SCI                                         0x03U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_GPC_COMBINED                                0x04U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GDDR6_X_COMBINED                                0x05U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_I2CS_GT21X                                      0x06U /* Not supported on Kepler and deprecated from Maxwell */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_I2CS_GF11X                                      0x07U /* Not supported on Kepler and deprecated from Maxwell */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_I2C_ADT7473_1                                   0x08U /* Not supported on Kepler and deprecated from Maxwell */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_I2C_ADM1032                                     0x09U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_I2C_VT1165                                      0x0AU /* Not supported on Kepler and deprecated from Maxwell */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_I2C_MAX6649                                     0x0BU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_I2C_TMP411                                      0x0CU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_I2C_ADT7461                                     0x0DU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_I2C_TMP451                                      0x0EU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_RM                                              0x0FU /* Not supported on Kepler and deprecated from Maxwell */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_HBM2_SITE                                       0x10U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_HBM2_COMBINED                                   0x11U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_HBM3_SITE                                       0x12U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_HBM3_COMBINED                                   0x13U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_GTZ_V10                                     0x14U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_STZ_V10                                     0x15U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_TSENSE_V10                                  0x16U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_MINI_TSENSE_V10                             0x17U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_UNUSED_0                                        0x18U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_MEM_GDDRX_SW_V10                                0x19U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_MEM_V10                                         0x1AU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_BOARD_I2C_V10                                   0x1BU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_MEM_HBM_SW_V10                                  NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_MEM_V10
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_GPU_TZ_V10                                      0xFCU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_3X                                              0xFDU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_CLASS_MODEL_10                                        0xFEU

#define NV2080_CTRL_THERMAL_THERM_DEVICE_PROV_MAPPING_INVALID                                  0xFFU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_PROV__NUM_PROV_INVALID                                0XFFU

/*!
 * Macros for GPU Thermal zone functional mode
 * _MAX : Max of all input temperatures
 * _AVG : Average of all input temperatures
 * _MIN : Min of all input temperatures
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_MODE_MAX             0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_MODE_AVG             0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_MODE_MIN             0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_MODE_INVALID         0x07U

/*!
 * Provider mappings supported by THERM_DEVICE_CLASS_GPU_TSENSE_V10
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_1_BJT                     0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_2_BJTS                    0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_3_BJTS                    0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_4_BJTS                    0x03U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_5_BJTS                    0x04U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_6_BJTS                    0x05U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_7_BJTS                    0x06U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_8_BJTS                    0x07U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_9_BJTS                    0x08U

/*!
 * Provider mapping Name: _GPU_TSENSE_V10_PROV_MAPPING_1_BJT
 *
 * List of providers supported by _PROV_MAPPING_1_BJT
 * _PROV_BJT0 : BJT0 temperature
 * _NUM_PROVS : Total number of provider supported by _PROV_MAPPING_1_BJT
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_1_BJT_PROV_BJT0           0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_1_BJT__NUM_PROVS          0x01U

/*!
 * Provider mapping Name: _GPU_TSENSE_V10_PROV_MAPPING_2_BJTS
 *
 * List of providers supported by _PROV_MAPPING_2_BJTS
 * _PROV_BJT0 : BJT0 temperature
 * _PROV_BJT1 : BJT1 temperature
 * _NUM_PROVS : Total number of provider supported by _PROV_MAPPING_2_BJTS
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_2_BJTS_PROV_BJT0          0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_2_BJTS_PROV_BJT1          0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_2_BJTS__NUM_PROVS         0x02U

/*!
 * Provider mapping Name: _GPU_TSENSE_V10_PROV_MAPPING_3_BJTS
 *
 * List of providers supported by _PROV_MAPPING_3_BJTS
 * _PROV_BJT0 : BJT0 temperature
 * _PROV_BJT1 : BJT1 temperature
 * _PROV_BJT2 : BJT2 temperature
 * _NUM_PROVS : Total number of provider supported by _PROV_MAPPING_3_BJTS
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_3_BJTS_PROV_BJT0          0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_3_BJTS_PROV_BJT1          0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_3_BJTS_PROV_BJT2          0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_3_BJTS__NUM_PROVS         0x03U

/*!
 * Provider mapping Name: _GPU_TSENSE_V10_PROV_MAPPING_4_BJTS
 *
 * List of providers supported by _PROV_MAPPING_4_BJTS
 * _PROV_BJT0 : BJT0 temperature
 * _PROV_BJT1 : BJT1 temperature
 * _PROV_BJT2 : BJT2 temperature
 * _PROV_BJT3 : BJT3 temperature
 * _NUM_PROVS : Total number of provider supported by _PROV_MAPPING_4_BJTS
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_4_BJTS_PROV_BJT0          0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_4_BJTS_PROV_BJT1          0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_4_BJTS_PROV_BJT2          0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_4_BJTS_PROV_BJT3          0x03U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_4_BJTS__NUM_PROVS         0x04U

/*!
 * Provider mapping Name: _GPU_TSENSE_V10_PROV_MAPPING_5_BJTS
 *
 * List of providers supported by _PROV_MAPPING_5_BJTS
 * _PROV_BJT0 : BJT0 temperature
 * _PROV_BJT1 : BJT1 temperature
 * _PROV_BJT2 : BJT2 temperature
 * _PROV_BJT3 : BJT3 temperature
 * _PROV_BJT4 : BJT4 temperature
 * _NUM_PROVS : Total number of provider supported by _PROV_MAPPING_5_BJTS
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_5_BJTS_PROV_BJT0          0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_5_BJTS_PROV_BJT1          0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_5_BJTS_PROV_BJT2          0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_5_BJTS_PROV_BJT3          0x03U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_5_BJTS_PROV_BJT4          0x04U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_5_BJTS__NUM_PROVS         0x05U

/*!
 * Provider mapping Name: _GPU_TSENSE_V10_PROV_MAPPING_6_BJTS
 *
 * List of providers supported by _PROV_MAPPING_6_BJTS
 * _PROV_BJT0 : BJT0 temperature
 * _PROV_BJT1 : BJT1 temperature
 * _PROV_BJT2 : BJT2 temperature
 * _PROV_BJT3 : BJT3 temperature
 * _PROV_BJT4 : BJT4 temperature
 * _PROV_BJT5 : BJT5 temperature
 * _NUM_PROVS : Total number of provider supported by _PROV_MAPPING_6_BJTS
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_6_BJTS_PROV_BJT0          0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_6_BJTS_PROV_BJT1          0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_6_BJTS_PROV_BJT2          0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_6_BJTS_PROV_BJT3          0x03U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_6_BJTS_PROV_BJT4          0x04U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_6_BJTS_PROV_BJT5          0x05U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_6_BJTS__NUM_PROVS         0x06U

/*!
 * Provider mapping Name: _GPU_TSENSE_V10_PROV_MAPPING_7_BJTS
 *
 * List of providers supported by _PROV_MAPPING_7_BJTS
 * _PROV_BJT0 : BJT0 temperature
 * _PROV_BJT1 : BJT1 temperature
 * _PROV_BJT2 : BJT2 temperature
 * _PROV_BJT3 : BJT3 temperature
 * _PROV_BJT4 : BJT4 temperature
 * _PROV_BJT5 : BJT5 temperature
 * _PROV_BJT6 : BJT6 temperature
 * _NUM_PROVS : Total number of provider supported by _PROV_MAPPING_7_BJTS
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_7_BJTS_PROV_BJT0          0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_7_BJTS_PROV_BJT1          0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_7_BJTS_PROV_BJT2          0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_7_BJTS_PROV_BJT3          0x03U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_7_BJTS_PROV_BJT4          0x04U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_7_BJTS_PROV_BJT5          0x05U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_7_BJTS_PROV_BJT6          0x06U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_7_BJTS__NUM_PROVS         0x07U

/*!
 * Provider mapping Name: _GPU_TSENSE_V10_PROV_MAPPING_8_BJTS
 *
 * List of providers supported by _PROV_MAPPING_8_BJTS
 * _PROV_BJT0 : BJT0 temperature
 * _PROV_BJT1 : BJT1 temperature
 * _PROV_BJT2 : BJT2 temperature
 * _PROV_BJT3 : BJT3 temperature
 * _PROV_BJT4 : BJT4 temperature
 * _PROV_BJT5 : BJT5 temperature
 * _PROV_BJT6 : BJT6 temperature
 * _PROV_BJT7 : BJT7 temperature
 * _NUM_PROVS : Total number of provider supported by _PROV_MAPPING_8_BJTS
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_8_BJTS_PROV_BJT0          0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_8_BJTS_PROV_BJT1          0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_8_BJTS_PROV_BJT2          0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_8_BJTS_PROV_BJT3          0x03U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_8_BJTS_PROV_BJT4          0x04U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_8_BJTS_PROV_BJT5          0x05U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_8_BJTS_PROV_BJT6          0x06U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_8_BJTS_PROV_BJT7          0x07U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_8_BJTS__NUM_PROVS         0x08U

/*!
 * Provider mapping Name: _GPU_TSENSE_V10_PROV_MAPPING_9_BJTS
 *
 * List of providers supported by _PROV_MAPPING_9_BJTS
 * _PROV_BJT0 : BJT0 temperature
 * _PROV_BJT1 : BJT1 temperature
 * _PROV_BJT2 : BJT2 temperature
 * _PROV_BJT3 : BJT3 temperature
 * _PROV_BJT4 : BJT4 temperature
 * _PROV_BJT5 : BJT5 temperature
 * _PROV_BJT6 : BJT6 temperature
 * _PROV_BJT7 : BJT7 temperature
 * _PROV_BJT8 : BJT8 temperature
 * _NUM_PROVS : Total number of provider supported by _PROV_MAPPING_9_BJTS
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_9_BJTS_PROV_BJT0          0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_9_BJTS_PROV_BJT1          0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_9_BJTS_PROV_BJT2          0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_9_BJTS_PROV_BJT3          0x03U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_9_BJTS_PROV_BJT4          0x04U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_9_BJTS_PROV_BJT5          0x05U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_9_BJTS_PROV_BJT6          0x06U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_9_BJTS_PROV_BJT7          0x07U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_9_BJTS_PROV_BJT8          0x08U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_9_BJTS__NUM_PROVS         0x09U

/*!
 * Max number of providers for THERM_DEVICE_GPU_TSENSE by considering all provider mapping.
 * Its max of _PROV_MAPPING_9_BJTS__NUM_PROVS, _PROV_MAPPING_2_BJTS__NUM_PROVS and _PROV_MAPPING_3_BJTS__NUM_PROVS
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV__NUM_PROVS                        NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV_MAPPING_9_BJTS__NUM_PROVS

#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_HBI_0     0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_HBI_1     0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_HBI_2     0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_HBI_3     0x03U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_HBI_4     0x04U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_HBI_5     0x05U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_NVL       0x06U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_0     0x07U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_1     0x08U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_2     0x09U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_3     0x0AU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_4     0x0BU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_5     0x0CU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_6     0x0DU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_7     0x0EU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_8     0x0FU
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_9     0x10U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_10    0x11U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_GPC_11    0x12U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_SW        0x13U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_SYS       0x14U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TZ_SENSOR_IDX_SOC_0     0x15U

/*!
 * Macros for GPU_TSENSE_V10 sensor location
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_HBI_0   0x00
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_HBI_1   0x01
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_NVL_0   0x02
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_NVL_1   0x03
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_GPC_0   0x04
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_GPC_1   0x05
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_GPC_2   0x06
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_GPC_3   0x07
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_GPC_4   0x08
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_GPC_5   0x09
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_GPC_6   0x0A
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_GPC_7   0x0B
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_GPC_8   0x0C
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_GPC_9   0x0D
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_GPC_10  0x0E
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_GPC_11  0x0F
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_SYS     0x10
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_SOC_0   0x11
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC__COUNT  0x12
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_INVALID 0xFF

/*!
 * Macros for GPU_MINI_TSENSE_V10 sensor location
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_MINI_TSENSE_LOC_SCI_0    0x00
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_MINI_TSENSE_LOC_SCI_1    0x01
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_MINI_TSENSE_LOC__COUNT   0x02
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_MINI_TSENSE_LOC_INVALID  0xFF

/*!
 * Provider mapping and list of providers supported by THERM_DEVICE_CLASS_GPU_MINI_TSENSE_V10
 *
 * Provider mapping Name: _PROV_MAPPING_DEFAULT
 *
 * List of providers supported by _PROV_MAPPING_DEFAULT
 * _PROV_DEFAULT : Mini tsense temperature
 * _NUM_PROVS    : Total number of provider supported by _PROV_MAPPING_DEFAULT
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_MINI_TSENSE_V10_PROV_MAPPING_DEFAULT              0x00U

#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_MINI_TSENSE_V10_PROV_MAPPING_DEFAULT_PROV_DEFAULT 0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_MINI_TSENSE_V10_PROV_MAPPING_DEFAULT__NUM_PROVS   0x01U

/*!
 * Provider mappings supported by THERM_DEVICE_CLASS_MEM_V10
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_1_SENSOR                         0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_2_SENSORS                        0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_3_SENSORS                        0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_4_SENSORS                        0x03U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_5_SENSORS                        0x04U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_6_SENSORS                        0x05U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_7_SENSORS                        0x06U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_8_SENSORS                        0x07U

/*!
 * Provider mapping Name: _PROV_MAPPING_1_SENSOR
 *
 * List of providers supported by _PROV_MAPPING_1_SENSOR
 * _PROV_SENSOR0 : SENSOR0 temperature
 * _NUM_PROVS  : Maximum number of providers supported by _PROV_MAPPING_1_SENSOR
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_1_SENSOR_PROV_SENSOR0            0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_1_SENSOR__NUM_PROVS              0x01U

/*!
 * Provider mapping Name: _PROV_MAPPING_2_SENSORS
 *
 * List of providers supported by _PROV_MAPPING_2_SENSORS
 * _PROV_SENSOR0 : SENSOR0 temperature
 * _PROV_SENSOR1 : SENSOR1 temperature
 * _NUM_PROVS    : Maximum number of providers supported by _PROV_MAPPING_2_SENSORS
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_2_SENSORS_PROV_SENSOR0           0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_2_SENSORS_PROV_SENSOR1           0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_2_SENSORS__NUM_PROVS             0x02U

/*!
 * Macros for MEM_V10 sensor type
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_SENSOR_TYPE_HBM3_SW_V10  0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_SENSOR_TYPE_GDDRX_SW_V10 0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_SENSOR_TYPE_HBM4_SW_V10  0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_SENSOR_TYPE__COUNT       0x03U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_SENSOR_TYPE_INVALID      0xFFU

#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_REFRESH_PERIOD_LIMIT_MAX_MS 1000U

/*!
 * Provider mapping Name: _PROV_MAPPING_3_SENSORS
 *
 * List of providers supported by _PROV_MAPPING_3_SENSORS
 * _PROV_SENSOR0 : SENSOR0 temperature
 * _PROV_SENSOR1 : SENSOR1 temperature
 * _PROV_SENSOR2 : SENSOR2 temperature
 * _NUM_PROVS    : Maximum number of providers supported by _PROV_MAPPING_3_SENSORS
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_3_SENSORS_PROV_SENSOR0           0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_3_SENSORS_PROV_SENSOR1           0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_3_SENSORS_PROV_SENSOR2           0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_3_SENSORS__NUM_PROVS             0x03U

/*!
 * Provider mapping Name: _PROV_MAPPING_4_SENSORS
 *
 * List of providers supported by _PROV_MAPPING_4_SENSORS
 * _PROV_SENSOR0 : SENSOR0 temperature
 * _PROV_SENSOR1 : SENSOR1 temperature
 * _PROV_SENSOR2 : SENSOR2 temperature
 * _PROV_SENSOR3 : SENSOR3 temperature
 * _NUM_PROVS  : Maximum number of providers supported by _PROV_MAPPING_4_SENSORS
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_4_SENSORS_PROV_SENSOR0           0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_4_SENSORS_PROV_SENSOR1           0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_4_SENSORS_PROV_SENSOR2           0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_4_SENSORS_PROV_SENSOR3           0x03U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_4_SENSORS__NUM_PROVS             0x04U

/*!
 * Provider mapping Name: _PROV_MAPPING_5_SENSORS
 *
 * List of providers supported by _PROV_MAPPING_5_SENSORS
 * _PROV_SENSOR0 : SENSOR0 temperature
 * _PROV_SENSOR1 : SENSOR1 temperature
 * _PROV_SENSOR2 : SENSOR2 temperature
 * _PROV_SENSOR3 : SENSOR3 temperature
 * _PROV_SENSOR4 : SENSOR4 temperature
 * _NUM_PROVS  : Maximum number of providers supported by _PROV_MAPPING_5_SENSORS
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_5_SENSORS_PROV_SENSOR0           0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_5_SENSORS_PROV_SENSOR1           0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_5_SENSORS_PROV_SENSOR2           0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_5_SENSORS_PROV_SENSOR3           0x03U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_5_SENSORS_PROV_SENSOR4           0x04U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_5_SENSORS__NUM_PROVS             0x05U

/*!
 * Provider mapping Name: _PROV_MAPPING_6_SENSORS
 *
 * List of providers supported by _PROV_MAPPING_6_SENSORS
 * _PROV_SENSOR0 : SENSOR0 temperature
 * _PROV_SENSOR1 : SENSOR1 temperature
 * _PROV_SENSOR2 : SENSOR2 temperature
 * _PROV_SENSOR3 : SENSOR3 temperature
 * _PROV_SENSOR4 : SENSOR4 temperature
 * _PROV_SENSOR5 : SENSOR5 temperature
 * _NUM_PROVS  : Maximum number of providers supported by _PROV_MAPPING_6_SENSORS
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_6_SENSORS_PROV_SENSOR0           0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_6_SENSORS_PROV_SENSOR1           0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_6_SENSORS_PROV_SENSOR2           0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_6_SENSORS_PROV_SENSOR3           0x03U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_6_SENSORS_PROV_SENSOR4           0x04U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_6_SENSORS_PROV_SENSOR5           0x05U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_6_SENSORS__NUM_PROVS             0x06U

/*!
 * Provider mapping Name: _PROV_MAPPING_7_SENSORS
 *
 * List of providers supported by _PROV_MAPPING_7_SENSORS
 * _PROV_SENSOR0 : SENSOR0 temperature
 * _PROV_SENSOR1 : SENSOR1 temperature
 * _PROV_SENSOR2 : SENSOR2 temperature
 * _PROV_SENSOR3 : SENSOR3 temperature
 * _PROV_SENSOR4 : SENSOR4 temperature
 * _PROV_SENSOR5 : SENSOR5 temperature
 * _PROV_SENSOR6 : SENSOR6 temperature
 * _NUM_PROVS  : Maximum number of providers supported by _PROV_MAPPING_7_SENSORS
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_7_SENSORS_PROV_SENSOR0           0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_7_SENSORS_PROV_SENSOR1           0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_7_SENSORS_PROV_SENSOR2           0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_7_SENSORS_PROV_SENSOR3           0x03U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_7_SENSORS_PROV_SENSOR4           0x04U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_7_SENSORS_PROV_SENSOR5           0x05U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_7_SENSORS_PROV_SENSOR6           0x06U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_7_SENSORS__NUM_PROVS             0x07U

/*!
 * Provider mapping Name: _PROV_MAPPING_8_SENSORS
 *
 * List of providers supported by _PROV_MAPPING_8_SENSORS
 * _PROV_SENSOR0 : SENSOR0 temperature
 * _PROV_SENSOR1 : SENSOR1 temperature
 * _PROV_SENSOR2 : SENSOR2 temperature
 * _PROV_SENSOR3 : SENSOR3 temperature
 * _PROV_SENSOR4 : SENSOR4 temperature
 * _PROV_SENSOR5 : SENSOR5 temperature
 * _PROV_SENSOR6 : SENSOR6 temperature
 * _PROV_SENSOR7 : SENSOR7 temperature
 * _NUM_PROVS  : Maximum number of providers supported by _PROV_MAPPING_8_SENSORS
 */
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_8_SENSORS_PROV_SENSOR0           0x00U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_8_SENSORS_PROV_SENSOR1           0x01U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_8_SENSORS_PROV_SENSOR2           0x02U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_8_SENSORS_PROV_SENSOR3           0x03U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_8_SENSORS_PROV_SENSOR4           0x04U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_8_SENSORS_PROV_SENSOR5           0x05U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_8_SENSORS_PROV_SENSOR6           0x06U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_8_SENSORS_PROV_SENSOR7           0x07U
#define NV2080_CTRL_THERMAL_THERM_DEVICE_MEM_V10_PROV_MAPPING_8_SENSORS__NUM_PROVS             0x08U

/*!
 * Structure representing info attributes specific to THERM_DEVICE_3X.
 */
typedef struct NV2080_CTRL_THERMAL_DEVICE_INFO_DATA_3X {
    /*!
     * Device type NV2080_CTRL_THERMAL_THERM_DEVICE_TYPE_<xyz>
     */
    u8                                          devType;
    /*!
     * Device temperature exposed over direct SMBus i.e by I2CS
     */
    bool                                        bI2csExposure;
    /*!
     * Device provider mapping HAL
     */
    u8                                          provMapping;
    /*!
     * Device provider support mask
     */
    u32                                         provSupportMask;
    /*!
     * Temperature reported by Device on error
     */
    s32                                         errorTemp;
} NV2080_CTRL_THERMAL_DEVICE_INFO_DATA_3X;

/*!
 * Structure of static information specific to GPU_TSENSE_V10 providers
 */
struct NV2080_CTRL_THERMAL_DEVICE_INFO_DATA_GPU_TSENSE_V10_PROV {
    /*!
     * Hotspot offset for given provider
     */
    u32 hsOffset;
};

/*!
 * Structure of static information specific to GPU_TSENSE_V10
 */
struct NV2080_CTRL_THERMAL_DEVICE_INFO_DATA_GPU_TSENSE_V10 {
    /*!
     * THERMAL_DEVICE_INFO_DATA_3X Super class.
     */
    struct NV2080_CTRL_THERMAL_DEVICE_INFO_DATA_3X super;

    /*!
     * Sensor location NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_LOC_<xyz>
     */
    u8 location;

    /*!
     * Per provider information for Tsense.
     */
    struct NV2080_CTRL_THERMAL_DEVICE_INFO_DATA_GPU_TSENSE_V10_PROV provider[NV2080_CTRL_THERMAL_THERM_DEVICE_GPU_TSENSE_V10_PROV__NUM_PROVS];
};
#endif /* NVGPU_PMU_PMUIF_THERM_H */
