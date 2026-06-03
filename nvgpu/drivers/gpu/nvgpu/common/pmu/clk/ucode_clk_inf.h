/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMUIF_CLK_H
#define NVGPU_PMUIF_CLK_H

#include <nvgpu/flcnif_cmn.h>
#include <nvgpu/pmu/volt.h>
#include <nvgpu/boardobjgrp_e255.h>
#include <nvgpu/bug.h>
#include <common/pmu/boardobj/ucode_boardobj_inf.h>

/*!
 * Various types of VIN calibration that the GPU can support
 */
#define CTRL_CLK_VIN_CAL_TYPE_V20				(0x00000001U)
#define CTRL_CLK_VIN_VFE_IDX_INVALID				(0xFFU)

/*!
 * Various Vin device table versions that are supported
 */
#define NV2080_CTRL_CLK_VIN_DEVICES_DISABLED			(0x00000000U)
#define NV2080_CTRL_CLK_VIN_DEVICES_V10				(0x00000001U)
#define NV2080_CTRL_CLK_VIN_DEVICES_V20				(0x00000002U)
#define NV2080_CTRL_CLK_ADC_DEVICES_V30				(0x00000004U)

/*!
 * Types of ADC device that are supported
 */
#define NV2080_CTRL_CLK_ADC_DEVICE_TYPE_V30			(0x00000003U)

/*!
 * ADC output MUX's various modes of operations.
 */
#define NV2080_CTRL_CLK_ADC_SW_OVERRIDE_ADC_USE_INVALID		(0x00000000U)
#define NV2080_CTRL_CLK_ADC_SW_OVERRIDE_ADC_USE_HW_REQ		(0x00000001U)    // HW mode
#define NV2080_CTRL_CLK_ADC_SW_OVERRIDE_ADC_USE_MIN			(0x00000002U)    // MIN(HW, SW)
#define NV2080_CTRL_CLK_ADC_SW_OVERRIDE_ADC_USE_SW_REQ		(0x00000003U)    // SW Mode

/*
 * Types of NAFLL devices that are supported
 */
#define NV2080_CTRL_CLK_NAFLL_DEVICE_TYPE_V35				(0x00000005U)
#define NV2080_CTRL_CLK_CLK_NAFLL_DEVICE_TYPE_V35_SECURE_V10	(0x00000006U)

/*
 * Types of CLK_ENUM classes that are supported
 */
#define NV2080_CTRL_CLK_CLK_ENUM_TYPE_1X			(0x01)

/*
 * CLK_VF_REL related macros
 */
/*!
 * Macros defining the positions of VF curves.
 */
#define NV2080_CTRL_CLK_CLK_VF_REL_VF_CURVE_IDX_INVALID		0xFFU
#define NV2080_CTRL_CLK_CLK_VF_REL_VF_CURVE_IDX_PRI			0x00U
#define NV2080_CTRL_CLK_CLK_VF_REL_VF_CURVE_IDX_SEC_0		0x01U
#define NV2080_CTRL_CLK_CLK_VF_REL_VF_CURVE_IDX_SEC_1		0x02U
#define NV2080_CTRL_CLK_CLK_VF_REL_VF_CURVE_IDX_SEC_2		0x03U
#define NV2080_CTRL_CLK_CLK_VF_REL_VF_CURVE_IDX_MAX		0x04U // MUST be last.

#define NV2080_CTRL_CLK_CLK_VF_REL_VF_ENTRY_SEC_MAX		0x03U
#define NV2080_CTRL_CLK_CLK_VF_REL_VF_ENTRY_MAX		NV2080_CTRL_CLK_CLK_VF_REL_VF_CURVE_IDX_MAX
/*!
 * Count of max supported secondary entries.
 */
#define NV2080_CTRL_CLK_CLK_VF_REL_RATIO_SECONDARY_ENTRIES_MAX		0x04U
#define NV2080_CTRL_CLK_CLK_VF_REL_TABLE_SECONDARY_ENTRIES_MAX		0x04U

#define NV2080_CTRL_CLK_CLK_VF_REL_RATIO_VOLT_VF_SMOOTH_DATA_ENTRIES_MAX	0x02U
/*
 * CLK_VF_TUPLE related macros
 */
#define NV2080_CTRL_CLK_CLK_VF_TUPLE_IDX_INVALID		0xFFFFU
/*
 * Macro representing maximum number of CLK_VF_TUPLE_VARIABLEs
 * per one CLK_VF_TUPLE.
 */
#define NV2080_CTRL_CLK_CLK_VF_TUPLE_VARIABLE_MAX		0x04U

#define NV2080_CTRL_PERF_VFE_EQU_INDEX_INVALID			0xFFFFU
#define NV2080_CTRL_CLK_CLK_VF_POINT_IDX_INVALID		0xFFFFU

/*
 * Macro defining max allowed voltage rail VF items per clk domain.
 */
#define NV_CTRL_CLK_CLK_DOMAIN_PROG_RAIL_VF_ITEM_MAX    0x6U
#define NV_CTRL_CLK_CLK_DOMAIN_50_PROG_PHYSICAL_FBVDD_VF_MAPPING_TABLE_MAX_MAPPINGS 16U

/*!
 * Enumeration of CLK_VF_REL class types.
 */
#define NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50				0x01U
#define NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50_RATIO		0x03U
#define NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50_TABLE		0x05U
#define NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50_TABLE_FREQ	0x07U
#define NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50_RATIO_VOLT	0x09U
#define NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50_RATIO_FREQ	0x0BU

#define NV2080_CTRL_CLK_CLK_PROP_TOP_HAL_GA10X_TESLA_3  0x05U
/*
 * Enumeration of clock propagation topology ids.
 */
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_INVALID         0xFFU
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS        0x00U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_COMPUTE         0x01U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_0          0x02U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_1          0x03U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_2          0x04U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_3          0x05U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_4          0x06U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_5          0x07U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_6          0x08U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_7          0x09U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_8          0x0AU
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_9          0x0BU
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_10         0x0CU
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_11         0x0DU
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_12         0x0EU
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_13         0x0FU
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_MAX             0x10U

#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS_MEMORY_0    NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_5
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS_MEMORY_1    NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_6
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS_MEMORY_2    NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_7
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS_MEMORY_3    NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_8
#define NV2080_CTRL_CLK_CLK_PROP_TOP_ID_GRAPHICS_MEMORY_4    NV2080_CTRL_CLK_CLK_PROP_TOP_ID_RSVD_9

/*
 * Macro for clock propagation topology index
 */
#define NV2080_CTRL_CLK_CLK_PROP_TOP_IDX_INVALID        0xFFFFU

#define NV2080_CTRL_CLK_CLK_PROP_TOP_FREQUENCY_SEARCH_SPACE_HAL_10  0x00

/*
 * Enumeration of CLK_PROP_TOP class types.
 */
#define NV2080_CTRL_CLK_CLK_PROP_TOP_TYPE_1X       0x00U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_TYPE_MODEL_10 0x01U

/*
 * Enumeration of CLK_PROP_TOP_REL class types.
 */
#define NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TYPE_BASE               0x00U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TYPE_MODEL_10           0x01U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TYPE_1X                 0x02U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TYPE_1X_RATIO           0x03U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TYPE_1X_TABLE           0x04U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TYPE_1X_VOLT            0x05U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TYPE_1X_VFE             0x06U
#define NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TYPE_1X_VOLT_MULTI_RAIL 0x07U

/*
 * Enumeration of CLK_PROP_TOP_RELS class types.
 */
#define NV2080_CTRL_CLK_CLK_PROP_TOP_RELS_TYPE_MODEL_10          0x00U

/*!
 * Enumeration of CLK_DOMAIN types.
 */
#define CTRL_CLK_CLK_DOMAIN_TYPE_3X					0x01U
#define CTRL_CLK_CLK_DOMAIN_TYPE_3X_FIXED				0x02U
#define CTRL_CLK_CLK_DOMAIN_TYPE_3X_PROG				0x03U
#define CTRL_CLK_CLK_DOMAIN_TYPE_3X_MASTER				0x04U
#define CTRL_CLK_CLK_DOMAIN_TYPE_3X_SLAVE				0x05U
#define CTRL_CLK_CLK_DOMAIN_TYPE_30_PROG				0x06U
#define CTRL_CLK_CLK_DOMAIN_TYPE_35_MASTER				0x07U
#define CTRL_CLK_CLK_DOMAIN_TYPE_35_SLAVE				0x08U
#define CTRL_CLK_CLK_DOMAIN_TYPE_35_PROG				0x09U
#define CTRL_CLK_CLK_DOMAIN_TYPE_50						0x0CU
#define CTRL_CLK_CLK_DOMAIN_TYPE_50_FIXED				0x0DU
#define CTRL_CLK_CLK_DOMAIN_TYPE_50_PROG				0x0EU
#define CTRL_CLK_CLK_DOMAIN_TYPE_50_PROG_LOGICAL		0x0FU
#define CTRL_CLK_CLK_DOMAIN_TYPE_50_PROG_PHYSICAL		0x10U
#define CTRL_CLK_CLK_DOMAIN_3X_PROG_ORDERING_INDEX_INVALID	 	0xFFU
#define CTRL_CLK_CLK_DOMAIN_INDEX_INVALID				0xFFU

#define CTRL_CLK_CLK_DOMAIN_VERSION_50					0x50U
#define CTRL_CLK_CLK_DOMAIN_50_PROG_PHYSICAL_RAIL_VF_TYPE_NONE      0x0U
#define CTRL_CLK_CLK_DOMAIN_50_PROG_PHYSICAL_RAIL_VF_TYPE_PRIMARY   0x1U
#define CTRL_CLK_CLK_DOMAIN_50_PROG_PHYSICAL_RAIL_VF_TYPE_SECONDARY 0x2U

#define VBIOS_CLOCKS_TABLE_1X_HAL_NUM_CLOCK_ENTRIES                 0xF
#define VBIOS_CLOCKS_TABLE_1X_NUM_HALS                              0xA
#define VBIOS_CLOCKS_TABLE_1X_NUM_FACTORY_OC_CLK_DOMAINS            0x2
#define VBIOS_CLOCKS_TABLE_1X_NUM_FACTORY_OC_VPSTATE_IDXS           0x2
#define VBIOS_CLOCKS_TABLE_1X_NUM_CPMU_CPLUT_MAPPINGS               0x10

#define NV_CTRL_CLK_CLIENT_CLK_DOMAIN_IDX_INVALID   0xFFU

/*!
 * Enumeration of clock domain types.
 */
#define VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_TYPE_LOGICAL                   0x0
#define VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_TYPE_PHYSICAL_WITH_LOGICAL     0x1
#define VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_TYPE_PHYSICAL                  0x2
#define VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_TYPE_INVALID                  0xFF
#define VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_IDX_INVALID                   0xFF

/*
 * Enumeration of FREQ_DOMAIN types.
 * As of Clocks 3.1, there is only one type.
 */
#define NV2080_CTRL_CLK_FREQ_DOMAIN_SCHEMA_CLK3     0x03U

/*!
 * Enumeration of CLK_PROG types.
 */
#define CTRL_CLK_CLK_PROG_TYPE_3X					0x00U
#define CTRL_CLK_CLK_PROG_TYPE_1X					0x01U
#define CTRL_CLK_CLK_PROG_TYPE_1X_MASTER				0x02U
#define CTRL_CLK_CLK_PROG_TYPE_1X_MASTER_RATIO				0x03U
#define CTRL_CLK_CLK_PROG_TYPE_1X_MASTER_TABLE				0x04U
#define CTRL_CLK_CLK_PROG_TYPE_35					0x05U
#define CTRL_CLK_CLK_PROG_TYPE_35_MASTER				0x06U
#define CTRL_CLK_CLK_PROG_TYPE_35_MASTER_RATIO				0x07U
#define CTRL_CLK_CLK_PROG_TYPE_35_MASTER_TABLE				0x08U
#define CTRL_CLK_CLK_PROG_TYPE_UNKNOWN					0xFFU
#define CTRL_CLK_CLK_PROG_1X_MASTER_VF_ENTRY_MAX_ENTRIES		0x4U
#define CTRL_CLK_CLK_PROG_35_MASTER_SEC_VF_ENTRY_VOLTRAIL_MAX		0x1U
#define CTRL_CLK_PROG_1X_MASTER_MAX_SLAVE_ENTRIES			0x6U
/*!
 * Enumeration of CLK_PROG source types.
 */
#define CTRL_CLK_PROG_1X_SOURCE_PLL					0x00U
#define CTRL_CLK_PROG_1X_SOURCE_ONE_SOURCE			 	0x01U
#define CTRL_CLK_PROG_1X_SOURCE_FLL					0x02U
#define CTRL_CLK_PROG_1X_SOURCE_INVALID					0xFFU

#define CTRL_CLK_CLK_VF_POINT_TYPE_FREQ					0x01U
#define CTRL_CLK_CLK_VF_POINT_TYPE_VOLT					0x02U
#define CTRL_CLK_CLK_VF_POINT_TYPE_35					0x04U
#define CTRL_CLK_CLK_VF_POINT_TYPE_35_FREQ				0x05U
#define CTRL_CLK_CLK_VF_POINT_TYPE_35_VOLT				0x06U
#define CTRL_CLK_CLK_VF_POINT_TYPE_35_VOLT_PRI				0x07U
#define CTRL_CLK_CLK_VF_POINT_TYPE_35_VOLT_SEC				0x08U

#define NV2080_CTRL_CLK_CLK_VF_POINT_TYPE_50_FREQ		15U
#define NV2080_CTRL_CLK_CLK_VF_POINT_TYPE_50_VOLT		16U
#define NV2080_CTRL_CLK_CLK_VF_POINT_TYPE_50_VOLT_PRI	17U
#define NV2080_CTRL_CLK_CLK_VF_POINT_TYPE_50_VOLT_SEC	18U
#define NV2080_CTRL_CLK_CLK_VF_POINT_TYPE_MAX			19U
#define NV2080_CTRL_CLK_CLK_VF_POINT_DVCO_OFFSET_CODE_INVALID 0xFFU

#define CTRL_CLK_CLK_VF_POINT_IDX_INVALID				0xFFU
#define CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE			0x5U

#define NV2080_CTRL_CLK_NAFLL_LUT_VF_CURVE_SEC_MAX              (0x00000003U)
#define NV2080_CTRL_CLK_NAFLL_NDIV_ACCUMULATOR_TYPE_MAX         (0x2U)

#define NV_PMU_RPC_ID_CLK_BOARD_OBJ_GRP_IFACE_MODEL_10_CMD		0x00
#define NV_PMU_RPC_ID_CLK_BOARD_OBJ_GRP_IFACE_MODEL_PMU_INIT_1X_CMD	0x01
#define NV_PMU_RPC_ID_CLK_CNTR_SAMPLE_DOMAIN				0x02
#define NV_PMU_RPC_ID_CLK_CLK_DOMAIN_PROG_VOLT_TO_FREQ			0x03
#define NV_PMU_RPC_ID_CLK_CLK_DOMAIN_PROG_FREQ_TO_VOLT			0x04
#define NV_PMU_RPC_ID_CLK_CLK_DOMAIN_PROG_FREQ_QUANTIZE			0x05
#define NV_PMU_RPC_ID_CLK_CLK_DOMAIN_PROG_CLIENT_FREQ_DELTA_ADJ		0x06
#define NV_PMU_RPC_ID_CLK_CLK_DOMAIN_PROG_FREQS_ENUM			0x07
#define NV_PMU_RPC_ID_CLK_FREQ_EFFECTIVE_AVG				0x08
#define NV_PMU_RPC_ID_CLK_LOAD						0x09
#define NV_PMU_RPC_ID_CLK_VF_CHANGE_INJECT				0x0A
#define NV_PMU_RPC_ID_CLK_MCLK_SWITCH					0x0B
#define NV_PMU_RPC_ID_CLK__COUNT					0x0C

/*!
 * Macros for the @ref feature parameter in the @ref RM_PMU_CLK_LOAD structure
 */
#define NV_RM_PMU_CLK_LOAD_FEATURE_INVALID                          (0x00000000U)
#define NV_RM_PMU_CLK_LOAD_FEATURE_NAFLL                            (0x00000001U)
#define NV_RM_PMU_CLK_LOAD_FEATURE_ADC                              (0x00000002U)
#define NV_RM_PMU_CLK_LOAD_FEATURE_FREQ_CONTROLLER                  (0x00000003U)
#define NV_RM_PMU_CLK_LOAD_FEATURE_FREQ_EFFECTIVE_AVG               (0x00000004U)
#define NV_RM_PMU_CLK_LOAD_FEATURE_CLK_DOMAIN                       (0x00000005U)
#define NV_RM_PMU_CLK_LOAD_FEATURE_CLK_CONTROLLER                   (0x00000006U)

/*!
 * Macros for the @ref actionMask parameter in the @ref RM_PMU_CLK_LOAD structure
 */
#define NV_RM_PMU_CLK_LOAD_ACTION_MASK_LUT_REPROGRAM                         (0:0)
#define NV_RM_PMU_CLK_LOAD_ACTION_MASK_LUT_REPROGRAM_NO             (0x00000000U)
#define NV_RM_PMU_CLK_LOAD_ACTION_MASK_LUT_REPROGRAM_YES            (0x00000001U)

#define NV_RM_PMU_CLK_LOAD_ACTION_MASK_FREQ_CONTROLLER_CALLBACK              (1:1)
#define NV_RM_PMU_CLK_LOAD_ACTION_MASK_FREQ_CONTROLLER_CALLBACK_NO  (0x00000000U)
#define NV_RM_PMU_CLK_LOAD_ACTION_MASK_FREQ_CONTROLLER_CALLBACK_YES (0x00000001U)

#define NV_RM_PMU_CLK_LOAD_ACTION_MASK_FREQ_EFFECTIVE_AVG_CALLBACK           (2:2)
#define NV_RM_PMU_CLK_LOAD_ACTION_MASK_FREQ_EFFECTIVE_AVG_CALLBACK_NO  (0x00000000U)
#define NV_RM_PMU_CLK_LOAD_ACTION_MASK_FREQ_EFFECTIVE_AVG_CALLBACK_YES (0x00000001U)

#define NV_RM_PMU_CLK_LOAD_ACTION_MASK_ADC_CALLBACK                          (3:3)
#define NV_RM_PMU_CLK_LOAD_ACTION_MASK_ADC_CALLBACK_NO              (0x00000000U)
#define NV_RM_PMU_CLK_LOAD_ACTION_MASK_ADC_CALLBACK_YES             (0x00000001U)

#define NV_RM_PMU_CLK_LOAD_ACTION_MASK_ADC_HW_CAL_PROGRAM                    (4:4)
#define NV_RM_PMU_CLK_LOAD_ACTION_MASK_ADC_HW_CAL_PROGRAM_NO        (0x00000000U)
#define NV_RM_PMU_CLK_LOAD_ACTION_MASK_ADC_HW_CAL_PROGRAM_YES       (0x00000001U)

/* Helper macro for ADC action mask with both ADC_CALLBACK and ADC_HW_CAL_PROGRAM enabled */
#define NV_RM_PMU_CLK_LOAD_ACTION_MASK_ADC_FULL \
	((NV_RM_PMU_CLK_LOAD_ACTION_MASK_ADC_CALLBACK_YES << 3) | \
	 (NV_RM_PMU_CLK_LOAD_ACTION_MASK_ADC_HW_CAL_PROGRAM_YES << 4))

/*!
 * Macros for the @ref feature parameter in the @ref NV_PMU_CLK_LOAD structure
 */
#define NV_NV_PMU_CLK_LOAD_FEATURE_INVALID			(0x00000000U)
#define NV_NV_PMU_CLK_LOAD_FEATURE_FLL				(0x00000001U)
#define NV_NV_PMU_CLK_LOAD_FEATURE_VIN				(0x00000002U)
#define NV_NV_PMU_CLK_LOAD_FEATURE_FREQ_CONTROLLER		(0x00000003U)
#define NV_NV_PMU_CLK_LOAD_FEATURE_FREQ_EFFECTIVE_AVG		(0x00000004U)
#define NV_NV_PMU_CLK_LOAD_FEATURE_CLK_DOMAIN			(0x00000005U)
#define NV_NV_PMU_CLK_LOAD_FEATURE_CLK_CONTROLLER		(0x00000006U)

/* CLK CMD ID definitions.*/
#define NV_PMU_CLK_CMD_ID_BOARDOBJ_GRP_SET			(0x00000001U)
#define NV_PMU_CLK_CMD_ID_BOARDOBJ_GRP_GET_STATUS		(0x00000002U)
/* CLK MSG ID definitions */
#define NV_PMU_CLK_MSG_ID_BOARDOBJ_GRP_SET			(0x00000001U)
#define NV_PMU_CLK_MSG_ID_BOARDOBJ_GRP_GET_STATUS		(0x00000002U)
#define NV_NV_PMU_CLK_LOAD_ACTION_MASK_VIN_HW_CAL_PROGRAM_YES	(0x00000001U)

#define CTRL_CLK_CLK_DELTA_MAX_VOLT_RAILS				4U
#define NV2080_CTRL_VOLT_VOLT_RAIL_CLIENT_MAX_RAILS_V2  0x08U

/* Enumeration of FREQ_DELTA offset types. */
#define CTRL_CLK_CLK_FREQ_DELTA_TYPE_STATIC				0x00U
#define CTRL_CLK_CLK_FREQ_DELTA_TYPE_PERCENT			0x01U

union ctrl_clk_freq_delta_data {
	s32 delta_khz;
	s16 delta_percent;	// SFXP4_12
};

struct ctrl_clk_freq_delta {
	u8 type;
	union ctrl_clk_freq_delta_data data;
};

struct ctrl_clk_clk_delta {
	struct ctrl_clk_freq_delta freq_delta;
	s32 volt_deltauv[NV2080_CTRL_VOLT_VOLT_RAIL_CLIENT_MAX_RAILS_V2];
};

struct ctrl_clk_domain_control_35_prog_clk_mon {
	u32 flags;
	u32 low_threshold_override;
	u32 high_threshold_override;
};

struct ctrl_clk_domain_info_35_prog_clk_mon {
	u8 low_threshold_vfe_idx;
	u8 high_threshold_vfe_idx;
};

struct ctrl_clk_clk_prog_1x_master_source_fll {
	u32 base_vfsmooth_volt_uv;
	u32 max_vf_ramprate;
	u32 max_freq_stepsize_mhz;
};

union ctrl_clk_clk_prog_1x_master_source_data {
	struct ctrl_clk_clk_prog_1x_master_source_fll fll;
};

struct ctrl_clk_clk_vf_point_info_freq {
	u16 freq_mhz;
};

struct ctrl_clk_clk_vf_point_info_volt {
	u32  sourceVoltageuV;
	u8  vfGainVfeEquIdx;
	u8  clkDomainIdx;
};

struct ctrl_clk_clk_prog_1x_master_vf_entry {
	u8 vfe_idx;
	u8 gain_vfe_idx;
	u8 vf_point_idx_first;
	u8 vf_point_idx_last;
};

struct ctrl_clk_clk_prog_35_master_sec_vf_entry {
	u8 vfe_idx;
	u8 dvco_offset_vfe_idx;
	u8 vf_point_idx_first;
	u8 vf_point_idx_last;
};

struct ctrl_clk_clk_prog_35_master_sec_vf_entry_voltrail {
	struct ctrl_clk_clk_prog_35_master_sec_vf_entry sec_vf_entries[
			CTRL_CLK_CLK_PROG_35_MASTER_SEC_VF_ENTRY_VOLTRAIL_MAX];
};

struct ctrl_clk_clk_prog_1x_master_ratio_slave_entry {
	u8 clk_dom_idx;
	u8 ratio;
};

struct ctrl_clk_clk_prog_1x_master_table_slave_entry {
	u8 clk_dom_idx;
	u16 freq_mhz;
};

struct ctrl_clk_clk_prog_1x_source_pll {
	u8 pll_idx;
	u8 freq_step_size_mhz;
};

struct ctrl_clk_vin_v10 {
	u32 slope;
	u32 intercept;
};

struct ctrl_clk_vin_v20 {
	s8 offset;
	s8 gain;
	u8 coarse_control;
	u8 offset_vfe_idx;
};

union ctrl_clk_vin_data_v20 {
	struct ctrl_clk_vin_v10 cal_v10;
	struct ctrl_clk_vin_v20 cal_v20;
};

struct ctrl_clk_vin_device_info_data_v10 {
	struct ctrl_clk_vin_v10 vin_cal;
};

struct ctrl_clk_vin_device_info_data_v20 {
	u8 cal_type;
	union ctrl_clk_vin_data_v20 vin_cal;
};

union ctrl_clk_clk_prog_1x_source_data {
	struct ctrl_clk_clk_prog_1x_source_pll source_pll;
};

struct ctrl_clk_vf_point_freq_tuple {
	u16 freqMHz;
};

struct ctrl_clk_vf_point_base_vf_tuple {
	struct ctrl_clk_vf_point_freq_tuple
		freqTuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
	u32 voltageuV;
};

struct ctrl_clk_vf_point_base_vf_tuple_sec {
	struct ctrl_clk_vf_point_base_vf_tuple  super;
	u8 dvco_offset_code;
};

struct ctrl_clk_vf_point_vf_tuple {
	u16 freqMHz;
	u32 voltageuV;
};

struct ctrl_clk_vf_input {
	u8 flags;
	u32 value;
};

struct ctrl_clk_vf_output {
	u32 input_best_match;
	u32 value;
};

struct nv_pmu_rpc_clk_domain_35_prog_freq_to_volt {
	/*
	 * [IN/OUT] Must be first field in RPC structure
	 */
	struct nv_pmu_rpc_header hdr;
	u8 clk_domain_idx;
	u8 volt_rail_idx;
	u8 voltage_type;
	struct ctrl_clk_vf_input input;
	struct ctrl_clk_vf_output output;
	u32 scratch[1];
};

/*
 * CLK_DOMAIN BOARDOBJGRP Header structure.  Describes global state about the
 * CLK_DOMAIN feature.
 */
struct nv_pmu_clk_clk_domain_10_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
	u32 vbios_domains;
	struct ctrl_boardobjgrp_mask_e32 prog_domains_mask;
	struct ctrl_boardobjgrp_mask_e32 master_domains_mask;
	struct ctrl_boardobjgrp_mask_e32 clkmon_domains_mask;
	u16 cntr_sampling_periodms;
	u16 clkmon_refwin_usec;
	u8 version;
	bool b_override_o_v_o_c;
	bool b_debug_mode;
	bool b_enforce_vf_monotonicity;
	bool b_enforce_vf_smoothening;
	u8 volt_rails_max;
	struct ctrl_clk_clk_delta deltas;
};

struct nv_pmu_clk_clk_domain_50_boardobjgrp_set_header {
	struct ctrl_boardobjgrp_mask_e32 progClientVisibleDomainsMask;
	struct ctrl_boardobjgrp_mask_e32 progPhysicalDomainsMask;
	struct ctrl_boardobjgrp_mask_e32 progPhysicalCpmuDomainsMask;
};

struct nv_pmu_clk_clk_domain_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
	struct ctrl_boardobjgrp_mask_e32 progDomainsMask;
	struct ctrl_boardobjgrp_mask_e32 clkMonDomainsMask;
	u16 cntrSamplingPeriodms;
	bool bClkMonEnabled;
	u16 clkMonRefWinUsec;
	u16 xbarBoostVfeIdx;
	u8 version;
	bool bOverrideOVOC;
	bool bDebugMode;
	bool bEnforceVfMonotonicity;
	bool bEnforceVfSmoothening;
	bool bGrdFreqOCEnabled;
	bool bInsAwareVFDisable;
	bool bSkipMclkSwitch;
	u8 voltRailsMax;
	struct ctrl_clk_clk_delta deltas;
	u8 clientToInternalIdxMap[NV2080_CTRL_CLK_CLK_DOMAIN_CLIENT_MAX_DOMAINS];
	union {
		struct nv_pmu_clk_clk_domain_50_boardobjgrp_set_header v50;
	}data;
};

struct nv_pmu_clk_clk_domain_boardobj_set {
	struct nv_pmu_boardobj super;
	u32 domain;		// CLKWHICH_<xzy>
	u32 api_domain;
	bool b_noise_aware_capable;
	u8 perf_domain_grp_idx;
};

struct nv_pmu_clk_clk_domain_3x_boardobj_set {
	struct nv_pmu_clk_clk_domain_boardobj_set super;
	bool b_noise_aware_capable;
};

struct nv_pmu_clk_clk_domain_3x_fixed_boardobj_set {
	struct nv_pmu_clk_clk_domain_3x_boardobj_set super;
	u16 freq_mhz;
};

struct nv_pmu_clk_clk_domain_3x_prog_boardobj_set {
	struct nv_pmu_clk_clk_domain_3x_boardobj_set super;
	u8 clk_prog_idx_first;
	u8 clk_prog_idx_last;
	bool b_force_noise_unaware_ordering;
	struct ctrl_clk_freq_delta factory_delta;
	short freq_delta_min_mhz;
	short freq_delta_max_mhz;
	struct ctrl_clk_clk_delta deltas;
};

struct nv_pmu_clk_clk_domain_30_prog_boardobj_set {
	struct nv_pmu_clk_clk_domain_3x_prog_boardobj_set super;
	u8 noise_unaware_ordering_index;
	u8 noise_aware_ordering_index;
};

struct nv_pmu_clk_clk_domain_3x_master_boardobj_set {
	u8 rsvd;	/* Stubbing for RM_PMU_BOARDOBJ_INTERFACE */
	u32 slave_idxs_mask;
};

struct nv_pmu_clk_clk_domain_30_master_boardobj_set {
	struct nv_pmu_clk_clk_domain_30_prog_boardobj_set super;
	struct nv_pmu_clk_clk_domain_3x_master_boardobj_set master;
};

struct nv_pmu_clk_clk_domain_3x_slave_boardobj_set {
	u8 rsvd;	/* Stubbing for RM_PMU_BOARDOBJ_INTERFACE */
	u8 master_idx;
};

struct nv_pmu_clk_clk_domain_30_slave_boardobj_set {
	struct nv_pmu_clk_clk_domain_30_prog_boardobj_set super;
	struct nv_pmu_clk_clk_domain_3x_slave_boardobj_set slave;
};

struct nv_pmu_clk_clk_domain_35_prog_boardobj_set {
	struct nv_pmu_clk_clk_domain_3x_prog_boardobj_set super;
	u8 pre_volt_ordering_index;
	u8 post_volt_ordering_index;
	u8 clk_pos;
	u8 clk_vf_curve_count;
	struct ctrl_clk_domain_info_35_prog_clk_mon clkmon_info;
	struct ctrl_clk_domain_control_35_prog_clk_mon clkmon_ctrl;
	u32 por_volt_delta_uv[CTRL_VOLT_VOLT_RAIL_CLIENT_MAX_RAILS];
};

struct nv_pmu_clk_clk_domain_35_master_boardobj_set {
	struct nv_pmu_clk_clk_domain_35_prog_boardobj_set super;
	struct nv_pmu_clk_clk_domain_3x_master_boardobj_set master;
	struct ctrl_boardobjgrp_mask_e32 master_slave_domains_grp_mask;
};

struct nv_pmu_clk_clk_domain_35_slave_boardobj_set {
	struct nv_pmu_clk_clk_domain_35_prog_boardobj_set super;
	struct nv_pmu_clk_clk_domain_3x_slave_boardobj_set slave;
};

struct ctrl_clk_domain_control_50_prog_physical_clk_mon {
	u32 flags;
	u32 low_threshold_override;		// UFXP20_12
	u32 high_threshold_override;	// UFXP20_12
};

struct ctrl_clk_domain_info_50_prog_physical_clk_mon {
	u16 low_threshold_vfe_idx;
	u16 high_threshold_vfe_idx;
};

/*
 * Data that is used to adjust power measurements made on the FBVDD rail.
 */
struct ctrl_clk_domain_50_prog_physical_fbvdd_pwr_adj {
	u32 slope;
	u32 interceptmW;
};

/*
 * Defines a mapping between max freq at required voltage.
 */
struct ctrl_clk_domain_50_prog_physical_fbvdd_vf_mapping {
	u32 max_freq_khz;
	u32 volt_uv;
};

/*
 * A lookup table from FBVDD frequencies to voltage.
 */
struct ctrl_clk_domain_50_prog_physical_fbvdd_vf_mapping_tbl {
	u8 number_mappings;
	struct ctrl_clk_domain_50_prog_physical_fbvdd_vf_mapping
		mappings[NV_CTRL_CLK_CLK_DOMAIN_50_PROG_PHYSICAL_FBVDD_VF_MAPPING_TABLE_MAX_MAPPINGS];
};

/*
 * FBVDD data that may be associated with a given clock domain.
 */
struct crtl_clk_domain_50_prog_physical_fbvdd_data {
	bool valid;
	struct ctrl_clk_domain_50_prog_physical_fbvdd_pwr_adj pwr_adjustment;
	struct ctrl_clk_domain_50_prog_physical_fbvdd_vf_mapping_tbl vf_mapping_tbl;
};

struct ctrl_clk_domain_info_50_physical_rail_vf_primary {
	u8 clk_vf_rel_idx_first;
	u8 clk_vf_rel_idx_last;
	struct ctrl_boardobjgrp_mask_e32 secondary_domain_mask;
	struct ctrl_boardobjgrp_mask_e32 primary_secondary_domain_mask;
};

struct ctrl_clk_domain_info_50_physical_rail_vf_secondary {
	u8 primary_idx;
};

struct ctrl_clk_domain_info_50_prog_physical_rail_vf_item {
	u8 type;
	u8 clk_position;
	union {
		struct ctrl_clk_domain_info_50_physical_rail_vf_primary primary;
		struct ctrl_clk_domain_info_50_physical_rail_vf_secondary secondary;
	} data;
};

/*!
 * RM_PMU structure representing the @ref NV2080_CTRL_CLK_CLK_DOMAIN_TYPE_MODEL_10
 * for the common base class for "MODEL_10" functionality.
 */
struct nv_pmu_clk_clk_domain_model_10_boardobj_set
{
    /*!
     * BOARDOBJ super class.  Must always be the first element in the structure.
     */
    struct nv_pmu_clk_clk_domain_boardobj_set  super;

    /*!
     * Interface for the 1.0 version of the BOARDOBJ model
     */
    u8 rsvd;
};

/*!
 * Clock Domain 30 Primary structure.  Contains information specific to a Primary
 * Clock Domain.
 */
struct nv_pmu_clk_clk_domain_prog_boardobj_set
{
    /*!
     * _PMU_BOARDOBJ_INTERFACE super class. Must always be first object in the
     * structure.
     */
    u8 rsvd;	/* Stubbing for _PMU_BOARDOBJ_INTERFACE */
    /*!
     * Mask of VOLT_RAILs on which the given CLK_DOMAIN_PROG has a
     * Vmin, and for which a VF curve has been specified.
     */
    struct ctrl_boardobjgrp_mask_e32  voltRailVminMask;
};

/*!
 * Clock domain info 50 structure.  Contains CLK_DOMAIN state that is specific
 * to the pstates 4.0 implemenation of CLK_DOMAINS.
 *
 */
struct nv_pmu_clk_clk_domain_50_boardobj_set
{
	/*!
	 * CLK_DOMAIN super class.  Must always be first object in the
	 * structure.
	 */
	struct nv_pmu_clk_clk_domain_model_10_boardobj_set super;
};

/*!
 * Clock Domain 50 Fixed structure.  Contains information specific to a Fixed
 * Clock Domain.
 */
struct nv_pmu_clk_clk_domain_50_fixed_boardobj_set
{
	/*!
	 * CLK_DOMAIN_50 super class.  Must always be first object in the
	 * structure.
	 */
	struct nv_pmu_clk_clk_domain_50_boardobj_set super;

	/*!
	 * Fixed frequency of the given CLK_DOMAIN in MHz.  This is the frequency
	 * that the VBIOS DEVINIT has programmed for the CLK_DOMAIN.
	 */
	u16  freqMHz;
};

/*!
 * Clock Domain 50 Prog structure. Contains information specific to a
 * Programmable Clock Domain.
 */
struct nv_pmu_clk_clk_domain_50_prog_boardobj_set
{
	/*!
	 * CLK_DOMAIN super class.  Must always be first object in the
	 * structure.
	 */
	struct nv_pmu_clk_clk_domain_50_boardobj_set    super;
	/*!
	 * CLK_DOMAIN_PROG super/interface class.
	 */
	struct nv_pmu_clk_clk_domain_prog_boardobj_set  prog;

	bool bFAVPointAvailable;
};

/*!
 * Clock Domain 50 Prog Physical structure. Contains information specific to a
 * Programmable Clock Domain.
 */
struct nv_pmu_clk_clk_domain_50_prog_physical_boardobj_set
{
	/*!
	 * CLK_DOMAIN super class.  Must always be first object in the
	 * structure.
	 */
	struct nv_pmu_clk_clk_domain_50_prog_boardobj_set super;
	/*!
	 * Pre-Volt ordering index for clock programming changes.
	 */
	u8    preVoltOrderingIndex;
	/*!
	 * Post-Volt ordering index for clock programming changes.
	 */
	u8    postVoltOrderingIndex;
	/*!
	 * Count of total number of (primary + secondary) curves supported on this clock domain.
	 */
	u8    clkVFCurveCount;
	/*!
	 * First index into the Clock Enumeration Table for this CLK_DOMAIN.
	 */
	u8    clkEnumIdxFirst;
	/*!
	 * Last index into the Clock Enumeration Table for this CLK_DOMAIN.
	 */
	u8    clkEnumIdxLast;
	/*!
	 * Index of the logical clock domain.
	 */
	u8    logicalDomainIdx;
	/*!
	 * Minimum frequency delta which can be applied to the CLK_DOMAIN.
	 */
	u16   freqDeltaMinMHz;
	/*!
	 * Maximum frequency delta which can be applied to the CLK_DOMAIN.
	 */
	s16   freqDeltaMaxMHz;
	/*!
	 * Factory OC frequency delta. We always apply this delta regardless of the
	 * frequency delta range reg@ freqDeltaMinMHz and ref@ freqDeltaMaxMHz as
	 * long as the clock programming entry has the OC feature enabled in it.
	 * ref@ CLK_PROG_1X_PRIMARY::bOCOVEnabled
	 */
	struct ctrl_clk_freq_delta  factoryDelta;
	/*!
	 * GRD OC frequency delta. This delta is programmed by POR team in clocks
	 * table. We will respect this delta IFF client explicitly opted for it by
	 * setting ref@ CLK_DOMAINS::bGrdFreqOCEnabled
	 */
	struct ctrl_clk_freq_delta      grdFreqDelta;
	/*!
	 * Local delta for each programmable CLK_DOMAIN
	 */
	struct ctrl_clk_clk_delta   deltas;
	/*!
	 * Clock Monitor specific information used to compute the threshold values.
	 */
	struct ctrl_clk_domain_info_50_prog_physical_clk_mon     clkMonInfo;
	/*!
	 * Clock Monitors specific information used to override the threshold values.
	 */
	struct ctrl_clk_domain_control_50_prog_physical_clk_mon  clkMonCtrl;

	/*!
	 * Data characterizing the FBVDD rail associated with this CLK_DOMAIN, if
	 * valid.
	 */
	struct crtl_clk_domain_50_prog_physical_fbvdd_data fbvddData;

	/*!
	 * Mask of volt rails on which this clock domain has its Vmin.
	 */
	u32 railMask;
	/*!
	 * Rail specific data for a given programmable clock domain.
	 * ref@ NV2080_CTRL_CLK_CLK_DOMAIN_INFO_50_PROG_RAIL_VF_ITEM
	 */
	struct ctrl_clk_domain_info_50_prog_physical_rail_vf_item
			railVfItem[NV_CTRL_CLK_CLK_DOMAIN_PROG_RAIL_VF_ITEM_MAX];
	/*!
	 * Clock identifier for this CLK_DOMAIN in the CPMU. This value is chip specific
	 * and dictated by the VBIOS Clocks Table HAL.
	 */
	NV2080_CTRL_CLK_CLK_DOMAIN_CPMU_CLOCK_ID cpmuClkId;
};

union nv_pmu_clk_clk_domain_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_clk_clk_domain_boardobj_set super;
	struct nv_pmu_clk_clk_domain_3x_boardobj_set v3x;
	struct nv_pmu_clk_clk_domain_3x_fixed_boardobj_set v3x_fixed;
	struct nv_pmu_clk_clk_domain_3x_prog_boardobj_set v3x_prog;
	struct nv_pmu_clk_clk_domain_30_prog_boardobj_set v30_prog;
	struct nv_pmu_clk_clk_domain_30_master_boardobj_set v30_master;
	struct nv_pmu_clk_clk_domain_30_slave_boardobj_set v30_slave;
	struct nv_pmu_clk_clk_domain_35_prog_boardobj_set v35_prog;
	struct nv_pmu_clk_clk_domain_35_master_boardobj_set v35_master;
	struct nv_pmu_clk_clk_domain_35_slave_boardobj_set v35_slave;
	struct nv_pmu_clk_clk_domain_50_boardobj_set v50;
	struct nv_pmu_clk_clk_domain_50_fixed_boardobj_set v50_fixed;
	struct nv_pmu_clk_clk_domain_50_prog_boardobj_set v50_prog;
	struct nv_pmu_clk_clk_domain_50_prog_physical_boardobj_set v50_prog_physical;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E32(clk, clk_domain);

struct nv_pmu_clk_clk_prog_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e255 super;
	u8 slave_entry_count;
	u8 vf_entry_count;
	u8 vf_sec_entry_count;
};

struct nv_pmu_clk_clk_prog_boardobj_set {
	struct nv_pmu_boardobj super;
};

struct nv_pmu_clk_clk_prog_1x_boardobj_set {
	struct nv_pmu_clk_clk_prog_boardobj_set super;
	u8 source;
	u16 freq_max_mhz;
	union ctrl_clk_clk_prog_1x_source_data source_data;
};

struct nv_pmu_clk_clk_prog_1x_master_boardobj_set {
	struct nv_pmu_clk_clk_prog_1x_boardobj_set super;
	u8 rsvd;	/* Stubbing for RM_PMU_BOARDOBJ_INTERFACE */
	bool b_o_c_o_v_enabled;
	struct ctrl_clk_clk_prog_1x_master_vf_entry vf_entries[
		CTRL_CLK_CLK_PROG_1X_MASTER_VF_ENTRY_MAX_ENTRIES];
	struct ctrl_clk_clk_delta deltas;
	union ctrl_clk_clk_prog_1x_master_source_data source_data;
};

struct nv_pmu_clk_clk_prog_1x_master_ratio_boardobj_set {
	struct nv_pmu_clk_clk_prog_1x_master_boardobj_set super;
	u8 rsvd;	/* Stubbing for RM_PMU_BOARDOBJ_INTERFACE */
	struct ctrl_clk_clk_prog_1x_master_ratio_slave_entry slave_entries[
		CTRL_CLK_PROG_1X_MASTER_MAX_SLAVE_ENTRIES];
};

struct nv_pmu_clk_clk_prog_1x_master_table_boardobj_set {
	struct nv_pmu_clk_clk_prog_1x_master_boardobj_set super;
	u8 rsvd;	/* Stubbing for RM_PMU_BOARDOBJ_INTERFACE */
	struct ctrl_clk_clk_prog_1x_master_table_slave_entry
	slave_entries[CTRL_CLK_PROG_1X_MASTER_MAX_SLAVE_ENTRIES];
};

struct nv_pmu_clk_clk_prog_3x_master_boardobj_set {
	u8 rsvd;	/* Stubbing for RM_PMU_BOARDOBJ_INTERFACE */
	bool b_o_c_o_v_enabled;
	struct ctrl_clk_clk_prog_1x_master_vf_entry vf_entries[
			CTRL_CLK_CLK_PROG_1X_MASTER_VF_ENTRY_MAX_ENTRIES];
	struct ctrl_clk_clk_delta deltas;
	union ctrl_clk_clk_prog_1x_master_source_data source_data;
};

struct nv_pmu_clk_clk_prog_3x_master_ratio_boardobj_set {
	u8 rsvd;	/* Stubbing for RM_PMU_BOARDOBJ_INTERFACE */
	struct ctrl_clk_clk_prog_1x_master_ratio_slave_entry slave_entries[
				CTRL_CLK_PROG_1X_MASTER_MAX_SLAVE_ENTRIES];
};

struct nv_pmu_clk_clk_prog_3x_master_table_boardobj_set {
	u8 rsvd;	/* Stubbing for RM_PMU_BOARDOBJ_INTERFACE */
	struct ctrl_clk_clk_prog_1x_master_table_slave_entry slave_entries[
				CTRL_CLK_PROG_1X_MASTER_MAX_SLAVE_ENTRIES];
};

struct nv_pmu_clk_clk_prog_35_master_boardobj_set {
	struct nv_pmu_clk_clk_prog_1x_boardobj_set super;
	struct nv_pmu_clk_clk_prog_3x_master_boardobj_set master;
	struct ctrl_clk_clk_prog_35_master_sec_vf_entry_voltrail
		voltrail_sec_vf_entries[
			CTRL_CLK_CLK_PROG_1X_MASTER_VF_ENTRY_MAX_ENTRIES];
};

struct nv_pmu_clk_clk_prog_35_master_ratio_boardobj_set {
	struct nv_pmu_clk_clk_prog_35_master_boardobj_set super;
	struct nv_pmu_clk_clk_prog_3x_master_ratio_boardobj_set ratio;
};

struct nv_pmu_clk_clk_prog_35_master_table_boardobj_set {
	struct nv_pmu_clk_clk_prog_35_master_boardobj_set super;
	struct nv_pmu_clk_clk_prog_3x_master_table_boardobj_set table;
};

union nv_pmu_clk_clk_prog_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_clk_clk_prog_boardobj_set super;
	struct nv_pmu_clk_clk_prog_1x_boardobj_set v1x;
	struct nv_pmu_clk_clk_prog_1x_master_boardobj_set v1x_master;
	struct nv_pmu_clk_clk_prog_1x_master_ratio_boardobj_set
							v1x_master_ratio;
	struct nv_pmu_clk_clk_prog_1x_master_table_boardobj_set
							v1x_master_table;
	struct nv_pmu_clk_clk_prog_35_master_boardobj_set v35_master;
	struct nv_pmu_clk_clk_prog_35_master_ratio_boardobj_set
							v35_master_ratio;
	struct nv_pmu_clk_clk_prog_35_master_table_boardobj_set
							v35_master_table;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E255(clk, clk_prog);

struct nv_pmu_clk_clk_fll_device_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
	struct ctrl_boardobjgrp_mask_e32 lut_prog_master_mask;
	u32 lut_step_size_uv;
	u32 lut_min_voltage_uv;
	u8 lut_num_entries;
	u16 max_min_freq_mhz;
};

struct nv_pmu_clk_lut_device_desc {
	u8 vselect_mode;
	u16 hysteresis_threshold;
	bool b_quick_slowdown_force_engage[NV2080_CTRL_CLK_NAFLL_LUT_VF_CURVE_SEC_MAX];
};

struct nv_pmu_clk_regime_desc {
	u8 regime_id;
	u8 target_regime_id_override;
	u16 fixed_freq_regime_limit_mhz;
};

struct nv_pmu_clk_clk_fll_device_boardobj_set {
	struct nv_pmu_boardobj super;
	u8 id;
	u8 mdiv;
	u8 vin_idx_logic;
	u8 vin_idx_sram;
	u8 rail_idx_for_lut;
	u16 input_freq_mhz;
	u32 clk_domain;
	struct nv_pmu_clk_lut_device_desc lut_device;
	struct nv_pmu_clk_regime_desc regime_desc;
	u8 min_freq_vfe_idx;
	u8 freq_ctrl_idx;
	bool b_skip_pldiv_below_dvco_min;
	bool b_dvco_1x;
	struct ctrl_boardobjgrp_mask_e32 lut_prog_broadcast_slave_mask;
};

union nv_pmu_clk_clk_fll_device_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_clk_clk_fll_device_boardobj_set super;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E32(clk, clk_fll_device);

struct nv_pmu_clk_clk_nafll_device_boardobj_set {
	struct nv_pmu_boardobj super;
	u8 id;
	u8 mdiv;
	u8 vin_idx_logic;
	u8 vin_idx_sram;
	u8 rail_idx_for_lut;
	u16 input_ref_freq_mhz;
	u16 input_ref_div_value;
	u32 clk_domain;
	struct nv_pmu_clk_lut_device_desc lut_device;
	struct nv_pmu_clk_regime_desc regime_desc;
	u16 dvco_min_freq_vfe_idx;
	u8 freq_ctrl_idx;
	bool b_skip_pldiv_below_dvco_min;
	bool b_mulltistep_pldiv_switch_enable;
	bool b_dvco_1x;
	struct ctrl_boardobjgrp_mask_e32 lut_prog_bcast_secndary_mask;
};

struct nv_pmu_clk_clk_fll_device_model10_boardobj_set {
	struct nv_pmu_clk_clk_nafll_device_boardobj_set super;
	u8 rsvd;
};

struct nv_pmu_clk_clk_fll_device_v35_boardobj_set {
	struct nv_pmu_clk_clk_fll_device_model10_boardobj_set super;
	bool b_hw_pldiv_enable;
};

union nv_pmu_clk_clk_nafll_device_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_clk_clk_nafll_device_boardobj_set super;
	struct nv_pmu_clk_clk_fll_device_v35_boardobj_set v35;
};

struct nv_pmu_clk_clk_nafll_device_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
	struct ctrl_boardobjgrp_mask_e32 lut_prog_master_mask;
	u32 lut_step_size_uv;
	u32 lut_min_voltage_uv;
	u8 lut_num_entries;
	u16 max_min_freq_mhz;
	struct ctrl_boardobjgrp_mask_e32 unicast_nafll_dev_mask;
	struct ctrl_boardobjgrp_mask_e32 broadcast_nafll_dev_mask;
	bool b_die1_all_gpc_floorswept;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E32(clk, clk_nafll_device);

struct nv_pmu_clk_clk_enum_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e255 super;
};

struct nv_pmu_clk_clk_enum_boardobj_set {
	struct nv_pmu_boardobj super;
	bool b_ocov_enabled;
	u16	freq_min_mhz;
	u16 freq_max_mhz;
};

struct nv_pmu_clk_clk_enum_model_10 {
	struct nv_pmu_clk_clk_enum_boardobj_set super;
	u8 rsvd;
};

union nv_pmu_clk_clk_enum_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_clk_clk_enum_boardobj_set super;
	struct nv_pmu_clk_clk_enum_model_10 model10;
};


NV_PMU_BOARDOBJ_GRP_SET_MAKE_E255(clk, clk_enum);

/*!
 * CLK_VF_REL BOARDOBJGRP Header structure. Describes global state about the
 * CLK_VF_REL feature.
 */
struct nv_pmu_clk_clk_vf_rel_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e255 super;
	u8 secondary_entry_count;
	u8 vf_entry_count_sec;
};

/*!
 * Structure describing the params required to generate primary VF curve of this
 * VF_REL class.
 */
struct nv_pmu_clk_clk_vf_rel_vf_entry_pri {
	u16 vfeIdx;
	u16 cpmMaxFreqOffsetVfeIdx;
	u16 vfPointIdxFirst;
	u16 vfPointIdxLast;
	u16 safeVfCurveTupleIdx;
	u16 vfTupleIdxFirst;
	u16 vfTupleIdxLast;
};

/*!
 * Structure describing secondary VF curves for primary clock domains.
 */
struct mv_pmu_clk_clk_vf_rel_vf_entry_sec {
	u16 vfeIdx;
	u16 dvcoOffsetVfeIdx;
	u16 vfPointIdxFirst;
	u16 vfPointIdxLast;
	u16 favDeratedSubCurveVFEEqnIdx;
};

/*!
 * Structure describing the static configuration/POR state of the CLK_VF_REL
 * super class.
 */
struct nv_pmu_clk_clk_vf_rel_boardobj_set {
	struct nv_pmu_boardobj super;
	u8 rsvd;

	/*!
	 * Index of the VOLTAGE_RAIL for this CLK_VF_REL object.
	 */
	u8 railIdx;

	/*!
	 * Boolean flag indicating whether this entry supports OC/OV when those
	 * settings are applied to the corresponding CLK_DOMAIN object.
	 */
	bool bOCOVEnabled;

	/*!
	 * Maximum frequency for this CLK_VF_REL entry. Entries for a given domain
	 * need to be specified in ascending maxFreqMhz.
	 */
	u16 freqMaxMHz;

	/*!
	 * This will give the deviation of given voltage from it's nominal value.
	 */
	s32 voltDeltauV;

	/*!
	 * Primary VF curve params.
	 */
	struct nv_pmu_clk_clk_vf_rel_vf_entry_pri vfEntryPri;

	/*!
	 * Array of secondary VF curve's param.
	 */
	struct mv_pmu_clk_clk_vf_rel_vf_entry_sec
		vfEntriesSec[NV2080_CTRL_CLK_CLK_VF_REL_VF_ENTRY_SEC_MAX];

	/*!
	 * This will give the deviation of given freq from it's nominal value.
	 */
	struct ctrl_clk_freq_delta freqDelta;
};

/*!
 * Structure describing the static configuration/POR state of the CLK_VF_REL_50
 * super class.
 */
struct nv_pmu_clk_clk_vf_rel_50_boardobj_set {
	struct nv_pmu_clk_clk_vf_rel_boardobj_set super;
	u16 safeVfCurveTupleIdx;
	u16 vfTupleIdxFirst;
	u16 vfTupleIdxLast;
	u16 pickedVfTupleIdx;
};

/*!
 * Structure describing a TABLE PRIMARY-SECONDARY relationship which specifies the
 * SECONDARY Clock Domain's VF curve is a table-lookup function of the PRIMARY Clock
 * Domain's VF curve.
 */
struct nv_pmu_clk_clk_vf_rel_table_secondary_entry {
	u8 clkDomIdx;
	u16 freqMHz;
};

struct nv_pmu_clk_clk_vf_rel_ratio_secondary_entry {
	u8 ratio;
	u8 clkDomIdx;
};

struct nv_pmu_clk_clk_vf_rel_4x_table_boardobj_set {
	struct nv_pmu_clk_clk_vf_rel_table_secondary_entry
		secondaryEntries[NV2080_CTRL_CLK_CLK_VF_REL_TABLE_SECONDARY_ENTRIES_MAX];
};

/*!
 * Structure describing the static configuration/POR state of the
 * CLK_VF_REL_50_TABLE super class.
 */
struct nv_pmu_clk_clk_vf_rel_50_table_boardobj_set {
	struct nv_pmu_clk_clk_vf_rel_50_boardobj_set super;
	struct nv_pmu_clk_clk_vf_rel_4x_table_boardobj_set table;
};

struct nv_pmu_clk_clk_vf_rel_4x_ratio_boardobj_set {
	struct nv_pmu_clk_clk_vf_rel_ratio_secondary_entry
		secondaryEntries[NV2080_CTRL_CLK_CLK_VF_REL_RATIO_SECONDARY_ENTRIES_MAX];
};

struct nv_pmu_clk_clk_vf_rel_ratio_volt_smooth_data {
	u32 baseVFSmoothVoltuV;
	/*
	 * Maximum ramp rate for a given voltage based VF curve. Driver will ensure
	 * that the generated VF curve will respect this ramp rate. If the VF curve
	 * has discontinuity in it, driver will smoothen the VF curve using this value.
	 * For Frequency based VF curve this is don't care. It will be initialized
	 * to NV_U32_MAX.
	 * The format of the data is upper 20 bits is unsigned whole integer,
	 * lower 12 bits is fraction.
	 */
	u32 maxVFRampRate;
	u16 maxFreqStepSizeMHz;
};

struct nv_pmu_clk_clk_vf_rel_ratio_volt_vf_smooth_data_grp {
	struct nv_pmu_boardobj obj;
	/*
	 * Contains the number of different VF-ramp rates that indicate the
	 * maximum step size.
	 */
	u8 vfSmoothDataEntriesCount;
	/*
	 * Array of VF-smoothing data and ramp rates allowing multiple
	 * ramp rates for the same VF relationship.
	 */
	struct nv_pmu_clk_clk_vf_rel_ratio_volt_smooth_data
		vfSmoothDataEntries[NV2080_CTRL_CLK_CLK_VF_REL_RATIO_VOLT_VF_SMOOTH_DATA_ENTRIES_MAX];
};

struct nv_pmu_clk_clk_vf_rel_50_ratio_boardobj_set {
	struct nv_pmu_clk_clk_vf_rel_50_boardobj_set super;
	struct nv_pmu_clk_clk_vf_rel_4x_ratio_boardobj_set ratio;
};

struct nv_pmu_clk_clk_vf_rel_4x_ratio_volt_boardobj_set {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_clk_clk_vf_rel_ratio_volt_vf_smooth_data_grp vfSmoothDataGrp;
};

struct nv_pmu_clk_clk_vf_rel_50_ratio_volt_boardobj_set {
	struct nv_pmu_clk_clk_vf_rel_50_ratio_boardobj_set super;
	struct nv_pmu_clk_clk_vf_rel_4x_ratio_volt_boardobj_set ratioVolt;
	u8 vfGenMethod;
	u16 devinitVfeIdx;
	u16 favDeratedSubCurveVFEEqnIdx;
};

/*!
 * Union of CLK_VF_REL classes. Used to describe a single CLK_VF_REL object.
 */
union nv_pmu_clk_clk_vf_rel_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_clk_clk_vf_rel_boardobj_set super;
	struct nv_pmu_clk_clk_vf_rel_50_boardobj_set v50;
	struct nv_pmu_clk_clk_vf_rel_50_ratio_boardobj_set v50Ratio;
	struct nv_pmu_clk_clk_vf_rel_50_ratio_volt_boardobj_set v50RatioVolt;
	struct nv_pmu_clk_clk_vf_rel_50_table_boardobj_set v50Table;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E255(clk, clk_vf_rel);

#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_MAX 20U

/*!
 * CLK_PROP_REGIME BOARDOBJGRP Header structure. Describes global state about the
 * CLK_PROP_REGIME feature.
 */
struct nv_pmu_clk_clk_prop_regime_boardobjgrp_set_header {
	/*!
	 * BOARDOBJGRP_E32 super class.  Must always be first element in structure.
	 */
	struct nv_pmu_boardobjgrp_e32 super;

	/*!
	 * Clock Propagation regime HAL.
	 * NV2080_CTRL_CLK_CLK_PROP_REGIME_HAL_<xyz>
	 */
	u8 regimeHal;

	/*!
	 * Regime id NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_<xyz> to board object
	 * index map.
	 */
	u16 regimeIdToIdxMap[NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_MAX];
};

/*!
 * Structure describing the static configuration/POR state of the CLK_PROP_REGIME
 * super class.
 */
struct nv_pmu_clk_clk_prop_regime_boardobj_set {
	/*!
	 * BOARDOBJ super class.  Must always be the first element in the structure.
	 */
	struct nv_pmu_boardobj super;

	/*!
	 * Clock Propagation Regime Id.
	 */
	u8 regimeId;
	/*!
	 * Mask of clock domains that must be programmed based on their
	 * clock propagation relationship.
	 */
	struct ctrl_boardobjgrp_mask_e32 clkDomainMask;
};

union nv_pmu_clk_clk_prop_regime_boardobj_set_union {
    struct nv_pmu_boardobj obj;
    struct nv_pmu_clk_clk_prop_regime_boardobj_set super;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E32(clk, clk_prop_regime);

#define NV2080_CTRL_CLK_CLK_DOMAIN_CLIENT_MAX_DOMAINS          16U

struct ctrl_clk_clk_prop_top_info_frequency_search_space {
	u16 minRatioClkPropTopIdx;
	u16 maxRatioClkPropTopIdx;
	u8 searchSpaceHal;
	bool bUseHigherIsoSecFmaxVminMhz;
};

/*
 * Array of source to destination clock propagation topology path for a given
 * programmable clock domain. Each path stores the clock propagation relationship
 * index which represents the next relationship index on the path from source
 * clock domain to desitnation clock domain.
 */
struct ctrl_clk_clk_prop_top_clk_domain_dst_path {
	u16 dstPath[NV2080_CTRL_CLK_CLK_DOMAIN_CLIENT_MAX_DOMAINS];
};

/*
 * Array of source to destination clock propagation topology path for all
 * programmable clock domains.
 */
struct ctrl_clk_clk_prop_top_clk_domains_dst_path {
	struct ctrl_clk_clk_prop_top_clk_domain_dst_path
		domainDstPath[NV2080_CTRL_CLK_CLK_DOMAIN_CLIENT_MAX_DOMAINS];
};

/*!
 * CLK_PROP_TOP BOARDOBJGRP Header structure. Describes global state about the
 * CLK_PROP_TOP feature.
 */
struct nv_pmu_clk_clk_prop_top_boardobjgrp_set_header {
	/*!
	 * BOARDOBJGRP_E32 super class. Must always be first element in structure.
	 */
	struct nv_pmu_boardobjgrp_e32  super;

	/*!
	 * Clock Propagation Topology HAL.
	 * @ref NV2080_CTRL_CLK_CLK_PROP_TOP_HAL_<xyz>
	 */
	u8 topHal;

	/*!
	 * Active Clock Propagation Topology Id.
	 * SW will dynamically select the active topology from set of available
	 * topologies based on the active workload hints coming from KMD/DX.
	 */
	u8 activeTopId;

	/*!
	 * Forced active Clock Propagation Topology Id.
	 * When client force an active topology id, the SW will respect the forced
	 * topology instead of selecting the topology based on workload.
	 *
	 * Logic:
	 * Use forced active topology if @ref activeTopIdForced == VALID
	 * otherwise use @ref activeTopId
	*/
	u8 activeTopIdForced;

	/*!
	 * Structure describing frequency search space related information required by
	 * external clients (power controller, DLPPE).
	 */
	struct ctrl_clk_clk_prop_top_info_frequency_search_space
						freqSearchSpace;
};

 /*!
 * Structure describing the static configuration/POR state of the CLK_PROP_TOP
 * super class.
 */
struct nv_pmu_clk_clk_prop_top_boardobj_set {
	/*!
	 * BOARDOBJ super class. Must always be the first element in the structure.
	 */
	struct nv_pmu_boardobj super;

	/*!
	 * Clock Propagation Topology Id.
	 */
	u8 topId;

	/*!
	 * Mask of clock propagation topology relationships that are valid for this clock
	 * propagation topology.
	 */
	struct ctrl_boardobjgrp_mask_e255 clkPropTopRelMask;

	struct ctrl_clk_clk_prop_top_clk_domains_dst_path domainsDstPath;
};

union nv_pmu_clk_clk_prop_top_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_clk_clk_prop_top_boardobj_set super;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E255(clk, clk_prop_top);

/*!
 * Structure describing the static configuration/POR state of the CLK_PROP_TOP_REL
 * super class.
 */
struct nv_pmu_clk_clk_prop_top_rel_boardobj_set {
    /*!
     * BOARDOBJ super class.  Must always be the first element in the structure.
     */
    struct nv_pmu_boardobj super;

    /*!
     * Source Clock Domain Index.
     */
    u8 clkDomainIdxSrc;

    /*!
     * Destination Clock Domain Index.
     */
    u8 clkDomainIdxDst;

    /*!
     * Boolean tracking whether bidirectional relationship enabled.
     */
    bool bBiDirectional;
};

struct clk_prop_top_rel_model_10_boardobj_set {
	struct nv_pmu_clk_clk_prop_top_rel_boardobj_set super;
	s16 offset;
};

struct clk_prop_top_rel_1x_boardobj_set {
	struct clk_prop_top_rel_model_10_boardobj_set super;
};

struct clk_prop_top_rel_1x_ratio_boardobj_set {
	struct clk_prop_top_rel_1x_boardobj_set super;
	u32 ratio;        // fixed point 16.16 format.
	u32 ratioInverse; // fixed point 16.16 format.
};

struct clk_prop_top_rel_1x_volt_boardobj_set {
	struct clk_prop_top_rel_1x_boardobj_set super;
	u8 voltRailIdx;
};

#define NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TABLE_REL_TUPLE_MAX 32
#define NV2080_CTRL_CLK_CLK_PROP_TOP_REL_CPMU_REL_TUPLE_MAX  32

struct ctrl_clk_prop_top_rel_table_rel_tuple {
	u16 freqMHzSrc;
	u16 freqMHzDst;
};

struct ctrl_clk_prop_top_rel_cpmu_rel_tuple {
	u16 clkDomainIdxSrc;
	u16 clkDomainIdxDst;
};

/*!
 * CLK_PROP_TOP_REL BOARDOBJGRP Header structure. Describes global state about the
 * CLK_PROP_TOP_REL feature.
 */
struct nv_pmu_clk_clk_prop_top_rel_boardobjgrp_set_header {
	/*!
	 * BOARDOBJGRP_E255 super class. Must always be first element in structure.
	 */
	struct nv_pmu_boardobjgrp_e255 super;

	/*!
	 * Count of valid table relationship tuple array entries.
	 */
	u8 tableRelTupleCount;

	/*!
	 * Array of frequency tuple for table based clock propagation topology relationships.
	 * Here valid indexes corresponds to [0, tableRelTupleCount]
	 */
	struct ctrl_clk_prop_top_rel_table_rel_tuple
	tableRelTuple[NV2080_CTRL_CLK_CLK_PROP_TOP_REL_TABLE_REL_TUPLE_MAX];

	/*!
	 * Count of valid CPMU relationship tuple array entries.
	 */
	u8 cpmuRelTupleCount;

	/*!
	 * All clk->clk frequency relationships that the CPMU needs represented in
	 * its CPLUT (Clock Propagation LUT).
	 */
	struct ctrl_clk_prop_top_rel_cpmu_rel_tuple
	cpmuRelTuple[NV2080_CTRL_CLK_CLK_PROP_TOP_REL_CPMU_REL_TUPLE_MAX];
};

union nv_pmu_clk_clk_prop_top_rel_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_clk_clk_prop_top_rel_boardobj_set super;
	struct clk_prop_top_rel_model_10_boardobj_set model10;
    struct clk_prop_top_rel_1x_boardobj_set       v1x;
    struct clk_prop_top_rel_1x_ratio_boardobj_set v1xRatio;
    struct clk_prop_top_rel_1x_volt_boardobj_set  v1xVolt;
};


NV_PMU_BOARDOBJ_GRP_SET_MAKE_E255(clk, clk_prop_top_rel);

struct nv_pmu_clk_freq_domain_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
	u32 initFlags;
	u16 mclkFreqMHzBootPstate;
};

struct nv_pmu_clk_freq_domain_boardobj_set {
	struct nv_pmu_boardobj obj;
	u8 rsvd;
	u32 clkDomain;
};

union nv_pmu_clk_freq_domain_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_clk_freq_domain_boardobj_set domain;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E32(clk, freq_domain);

struct nv_pmu_clk_clk_vin_device_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
	u8 version;
	bool b_vin_is_disable_allowed;
	u8 reserved[13];
};

struct nv_pmu_clk_clk_vin_device_boardobj_set {
	struct nv_pmu_boardobj super;
	u8 id;
	u8 volt_rail_idx;
	u8 por_override_mode;
	u8 override_mode;
	u32 flls_shared_mask;
};

struct nv_pmu_clk_clk_vin_device_v20_boardobj_set {
	struct nv_pmu_clk_clk_vin_device_boardobj_set super;
	struct ctrl_clk_vin_device_info_data_v20 data;
};

struct nv_pmu_clk_clk_adc_device_boardobj_set {
	struct nv_pmu_boardobj super;
	u8 id;
	u8 volt_rail_idx;
	bool dynamic_cal;
	u8 override_mode;
	u32 naflls_shared_mask;
	u8 logical_api_id;
	bool devinit_enabled;
};

struct ctrl_clk_adc_device_info_data_v30 {
	u8 rsvd;
	s8 offset;
	s8 gain;
	s8 course_offset;
	s8 course_gain;
	s8 low_temp_low_volt_err;
	s8 low_temp_hi_volt_err;
	s8 hi_temp_low_volt_err;
	s8 hi_temp_hi_volt_err;
	s8 adc_code_correction_offset;
};

struct nv_pmu_clkclk_adc_device_model_10_boardobj_set {
	struct nv_pmu_clk_clk_adc_device_boardobj_set super;
	u8 model10_rsvd;
};

struct nv_pmu_clk_clk_adc_device_v30_boardobj_set {
	// CLK_ADC_DEVICE super class. This should always be the first member!
	struct nv_pmu_clkclk_adc_device_model_10_boardobj_set super;
	// Structure containing static information specific to the V30 ADC device.
	struct ctrl_clk_adc_device_info_data_v30 data;
	// Padding to account for Isink V10 structure which is not used.
	u32 pad[8];
};

struct nv_pmu_clk_clk_adc_device_v20_boardobjgrp_set_header {
	s8 lowTemp;
	s8 highTemp;
	s8 refTemp;
	s8 adcCodeCorrectionOffsetMin;
	s8 adcCodeCorrectionOffsetMax;
	u8 tempVfeVarIdx;
	u32 lowVoltuV;
	u32 highVoltuV;
	u32 refVoltuV;
};

struct nv_pmu_clk_clk_adc_device_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
	//
	// Version of underlying ADC device table - usually
	// corresponding to the VBIOS table version.
	u8 version;
	//
	// Global disable control for all ADCs. If set to NV_FALSE, the
	// devices are not allowed to be disabled and powered off.
	bool bAdcIsDisableAllowed;
	// Mask of CLK_ADC_DEVICEs that support NAFLL functionality
	struct ctrl_boardobjgrp_mask_e32 nafllAdcDevicesMask;
	//
	// Union of ADC device table version specific data that
	// needs to be passed down to the PMU.
	union {
		struct nv_pmu_clk_clk_adc_device_v20_boardobjgrp_set_header adcsV20;
	} data;
};

union nv_pmu_clk_clk_adc_device_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_clk_clk_adc_device_v30_boardobj_set v30;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E32(clk, clk_adc_device);


struct nv_ctrl_clk_adc_devices_info_data_v20 {
	u8 model10_rsvd;
	u8 cal_invalid_fuse_rev_mask;
	u8 low_temp_err_invalid_fuse_rev_mask;
	u8 high_temp_err_invalid_fuse_rev_mask;
	u8 adc_cal_rev_fused;
	u8 adc_code_err_ht_rev_fused;
	u8 adc_code_err_lt_rev_fused;
	u8 temperature_vfe_var_idx;
	s8 low_temperature;
	s8 high_temperature;
	s8 ref_temperature;
	s8 adc_code_correction_offset_min;
	s8 adc_code_correction_offset_max;
	u32 low_volt_uv;
	u32 high_volt_uv;
	u32 ref_volt_uv;
};

union nv_pmu_clk_clk_vin_device_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_clk_clk_vin_device_boardobj_set super;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E32(clk, clk_vin_device);

struct nv_pmu_clk_clk_vf_point_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e2048 super;
	//
	// Mask setting for applying constant offset on primary and all secondary VF curves.
	// If any the bits are set, the feature will be active.
	u8 sparseMMAVFMarginEnabledMask;
};

struct nv_pmu_clk_clk_vf_point_sec_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e2048 super;
};

struct nv_pmu_clk_clk_vf_point_boardobj_set {
	struct nv_pmu_boardobj super;
	u8 vfe_equ_idx;
	u8 volt_rail_idx;
};

struct nv_pmu_clk_clk_vf_point_freq_boardobj_set {
	struct nv_pmu_clk_clk_vf_point_boardobj_set super;
	u16 freq_mhz;
	int volt_delta_uv;
};

struct nv_pmu_clk_clk_vf_point_volt_boardobj_set {
	struct nv_pmu_clk_clk_vf_point_boardobj_set super;
	u32 source_voltage_uv;
	struct ctrl_clk_freq_delta freq_delta;
};

struct nv_pmu_clk_clk_vf_point_volt_35_sec_boardobj_set {
	struct nv_pmu_clk_clk_vf_point_volt_boardobj_set super;
	u8 dvco_offset_code_override;
};

/*!
 * CLK_VF_POINT_50 - Defines a single point on a VF Curve.
 */
struct nv_pmu_clk_clk_vf_point_50_boardobj_set {
	/*!
	 * CLK_VF_POINT super class.  Must always be the first element in the structure.
	 */
	struct nv_pmu_clk_clk_vf_point_boardobj_set super;
};

/*!
 * Structure describing the static configuration/POR state of the _VOLT class.
 */
struct nv_pmu_clk_clk_vf_point_50_volt_boardobj_set {
	/*!
	 * CLK_VF_POINT_50 super class.  Must always be the first element in the structure.
	 */
	struct nv_pmu_clk_clk_vf_point_50_boardobj_set super;
	/*!
	 * Source voltage (uV) which was used to specify this CLK_VF_POINT_VOLT.
	 * These are the voltage values supported by the ADC/NAFLL.  This value will
	 * be rounded to the regulator size supported by the VOLTAGE_RAIL and stored
	 * in @ref super.voltageuV.  However, this source voltage value should be
	 * used when looking up data corresponding to the original ADC/NAFLL values.
	 */
	u32 sourceVoltageuV;
	/*!
	 * This will give the deviation of given freq from it's nominal value.
	 */
	struct ctrl_clk_freq_delta freqDelta;
};

/*!
 * Structure describing the static configuration/POR state of the _VOLT_SEC class.
 */
struct nv_pmu_clk_clk_vf_point_50_volt_sec_boardobj_set {
	/*!
	 * CLK_VF_POINT_50_VOLT super class.  Must always be the first element in the structure.
	 */
	struct nv_pmu_clk_clk_vf_point_50_volt_boardobj_set super;

	/*!
	 * DVCO offset override in terms of DVCO codes to trigger the fast
	 * slowdown while HW switches from reference NDIV point to secondary NDIV.
	 *
	 * This value will be override by SET CONTROL and used in PMU when not
	 * equal to @ref NV2080_CTRL_CLK_CLK_VF_POINT_DVCO_OFFSET_CODE_INVALID
	 */
	u8 dvcoOffsetCodeOverride;
};

/*!
 * Structure describing the static configuration/POR state of the _FREQ class.
 */
struct nv_pmu_clk_clk_vf_point_50_freq_boardobj_set {
	/*!
	 * CLK_VF_POINT_50 super class.  Must always be the first element in the structure.
	 */
	struct nv_pmu_clk_clk_vf_point_50_boardobj_set super;

	/*!
	 * Frequency (MHz) for this VF point.
	 */
	u16 freqMHz;
	/*!
	 * This will give the deviation of given voltage from it's nominal value.
	 */
	s32 voltDeltauV;
};

union nv_pmu_clk_clk_vf_point_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_clk_clk_vf_point_boardobj_set super;
	struct nv_pmu_clk_clk_vf_point_freq_boardobj_set freq;
	struct nv_pmu_clk_clk_vf_point_volt_boardobj_set volt;
	struct nv_pmu_clk_clk_vf_point_50_boardobj_set v50;
	struct nv_pmu_clk_clk_vf_point_50_freq_boardobj_set v50freq;
	struct nv_pmu_clk_clk_vf_point_50_volt_boardobj_set v50volt;
};

union nv_pmu_clk_clk_vf_point_sec_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_clk_clk_vf_point_boardobj_set super;
	struct nv_pmu_clk_clk_vf_point_freq_boardobj_set freq;
	struct nv_pmu_clk_clk_vf_point_volt_boardobj_set volt;
	struct nv_pmu_clk_clk_vf_point_volt_35_sec_boardobj_set v35_volt_sec;
	struct nv_pmu_clk_clk_vf_point_50_boardobj_set v50;
	struct nv_pmu_clk_clk_vf_point_50_freq_boardobj_set v50freq;
	struct nv_pmu_clk_clk_vf_point_50_volt_boardobj_set v50volt;
	struct nv_pmu_clk_clk_vf_point_50_volt_sec_boardobj_set v50voltSec;
};

/*
 * CUSTOM ALIGNED UNION MACRO FOR VF POINTS:
 * Creates 144-byte aligned unions for VF points instead of the default 16-byte alignment.
 * This ensures proper array stride matching PMU structure layout.
 */
#define NV_PMU_MAKE_ALIGNED_UNION_VF_POINT(name, type) \
	union name##_aligned { \
		union name data; \
		u8 pad[144]; \
	}

/*
 * CUSTOM VF POINT GET_STATUS MACRO:
 * Modified version of NV_PMU_BOARDOBJ_GRP_GET_STATUS_MAKE that uses 144-byte alignment
 */
#define NV_PMU_BOARDOBJ_GRP_GET_STATUS_MAKE_VF_POINT(_eng, _class, _slots) \
	NV_PMU_MAKE_ALIGNED_STRUCT( \
	nv_pmu_##_eng##_##_class##_boardobjgrp_get_status_header, struct); \
	NV_PMU_MAKE_ALIGNED_UNION_VF_POINT( \
	nv_pmu_##_eng##_##_class##_boardobj_get_status_union, union); \
	struct nv_pmu_##_eng##_##_class##_boardobj_grp_get_status { \
	union nv_pmu_##_eng##_##_class##_boardobjgrp_get_status_header_aligned \
	hdr; \
	union nv_pmu_##_eng##_##_class##_boardobj_get_status_union_aligned \
	objects[(_slots)]; \
	}

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E2048(clk, clk_vf_point);
NV_PMU_BOARDOBJ_GRP_SET_MAKE_E2048(clk, clk_vf_point_sec);

/*
 * Pack of two CLK VF POINTS board object groups. These two must go
 * together down to PMU and PMU will construct them in one CMD.
 */
struct nv_pmu_clk_clk_vf_point_boardobj_grp_set_pack {
	struct nv_pmu_clk_clk_vf_point_boardobj_grp_set pri;
	struct nv_pmu_clk_clk_vf_point_sec_boardobj_grp_set sec;
};

struct nv_pmu_clk_clk_vf_point_boardobjgrp_get_status_header {
	struct nv_pmu_boardobjgrp_e2048 super;
	u32 vf_points_cahce_counter;
};

struct nv_pmu_clk_clk_vf_point_35_freq_boardobj_get_status {
	struct nv_pmu_boardobj_query super;
	struct ctrl_clk_vf_point_base_vf_tuple base_vf_tuple;
	struct ctrl_clk_vf_point_vf_tuple
		offseted_vf_tuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
};

struct nv_pmu_clk_clk_vf_point_35_volt_pri_boardobj_get_status {
	struct nv_pmu_boardobj_query super;
	struct ctrl_clk_vf_point_base_vf_tuple base_vf_tuple;
	struct ctrl_clk_vf_point_vf_tuple
		offseted_vf_tuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
};

struct nv_pmu_clk_clk_vf_point_35_volt_sec_boardobj_get_status {
	struct nv_pmu_boardobj_query super;
	struct ctrl_clk_vf_point_base_vf_tuple_sec base_vf_tuple;
	struct ctrl_clk_vf_point_vf_tuple
		offseted_vf_tuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
};

struct ctrl_clk_vf_pair {
	u16 freq_mhz;
	u32 voltage_uv;
};

struct nv_pmu_clk_clk_vf_point_boardobj_get_status {
	struct nv_pmu_boardobj_query super;
	struct ctrl_clk_vf_pair pair;
	u8 dummy[38];
};

struct nv_pmu_clk_clk_vf_point_volt_boardobj_get_status {
	struct nv_pmu_clk_clk_vf_point_boardobj_get_status super;
	u16 vf_gain_value;
};

/* Define structures used in v50 get_status */
struct nv2080_ctrl_clk_clk_vf_point_base_vf_tuple {
	u32 voltageuV;
	struct ctrl_clk_vf_point_freq_tuple freqTuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
	u16 cpmMaxFreqOffsetMHz;
};

struct nv2080_ctrl_clk_clk_vf_point_base_vf_tuple_sec {
	u32 voltageuV;
	struct ctrl_clk_vf_point_freq_tuple freqTuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
	u16 cpmMaxFreqOffsetMHz;
};

struct nv2080_ctrl_clk_clk_vf_point_vf_tuple {
	u32 voltageuV;
	u16 freqMHz;
};

struct nv2080_ctrl_clk_offset_vf_tuple {
	s32 voltageOffsetuV;
	s16 freqOffsetMHz;
};


/* v50 get_status structures */
struct nv_pmu_clk_clk_vf_point_50_freq_boardobj_get_status {
	struct nv_pmu_boardobj_query super;  /* Fixed: Use nv_pmu_boardobj_query */
	struct nv2080_ctrl_clk_clk_vf_point_base_vf_tuple activeBaseVFTuple;
	struct nv2080_ctrl_clk_clk_vf_point_base_vf_tuple pickedBaseVFTuple;
	struct nv2080_ctrl_clk_clk_vf_point_vf_tuple
		activeOffsetedVFTuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
	struct nv2080_ctrl_clk_offset_vf_tuple
		activeOffsetVFTuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
};

struct nv_pmu_clk_clk_vf_point_50_volt_pri_boardobj_get_status {
	struct nv_pmu_boardobj_query super;  /* Fixed: Use nv_pmu_boardobj_query */
	struct nv2080_ctrl_clk_clk_vf_point_base_vf_tuple activeBaseVFTuple;
	struct nv2080_ctrl_clk_clk_vf_point_base_vf_tuple pickedBaseVFTuple;
	struct nv2080_ctrl_clk_clk_vf_point_vf_tuple
		activeOffsetedVFTuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
	struct nv2080_ctrl_clk_offset_vf_tuple
		activeOffsetVFTuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
};

struct nv_pmu_clk_clk_vf_point_50_volt_sec_boardobj_get_status {
	struct nv_pmu_boardobj_query super;  /* Fixed: Use nv_pmu_boardobj_query */
	struct nv2080_ctrl_clk_clk_vf_point_base_vf_tuple_sec activeBaseVFTuple;
	struct nv2080_ctrl_clk_clk_vf_point_base_vf_tuple_sec pickedBaseVFTuple;
	struct nv2080_ctrl_clk_clk_vf_point_vf_tuple
		activeOffsetedVFTuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
	struct nv2080_ctrl_clk_offset_vf_tuple
		activeOffsetVFTuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
};


/* v40 get_status structures - missing from union */
struct nv_pmu_clk_clk_vf_point_40_freq_boardobj_get_status {
	struct nv_pmu_boardobj_query super;  /* Fixed: Use nv_pmu_boardobj_query */
	struct nv2080_ctrl_clk_clk_vf_point_base_vf_tuple baseVFTuple;
	struct nv2080_ctrl_clk_clk_vf_point_vf_tuple
		offsetedVFTuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
	struct nv2080_ctrl_clk_offset_vf_tuple
		offsetVFTuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
};

struct nv_pmu_clk_clk_vf_point_40_volt_pri_boardobj_get_status {
	struct nv_pmu_boardobj_query super;  /* Fixed: Use nv_pmu_boardobj_query */
	struct nv2080_ctrl_clk_clk_vf_point_base_vf_tuple baseVFTuple;
	struct nv2080_ctrl_clk_clk_vf_point_vf_tuple
		offsetedVFTuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
	struct nv2080_ctrl_clk_offset_vf_tuple
		offsetVFTuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
};

struct nv_pmu_clk_clk_vf_point_40_volt_sec_boardobj_get_status {
	struct nv_pmu_boardobj_query super;  /* Fixed: Use nv_pmu_boardobj_query */
	struct nv2080_ctrl_clk_clk_vf_point_base_vf_tuple_sec baseVFTuple;
	struct nv2080_ctrl_clk_clk_vf_point_vf_tuple
		offsetedVFTuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
	struct nv2080_ctrl_clk_offset_vf_tuple
		offsetVFTuple[CTRL_CLK_CLK_VF_POINT_FREQ_TUPLE_MAX_SIZE];
};

/* v30 get_status structures */
struct nv_pmu_clk_clk_vf_point_30_boardobj_get_status {
	struct nv_pmu_boardobj_query super;  /* Use nv_pmu_boardobj_query */
	struct ctrl_clk_vf_pair pair;
	u8 padding[38];  /* Match struct size from PMU side */
};

union nv_pmu_clk_clk_vf_point_boardobj_get_status_union {
	struct nv_pmu_boardobj_query obj;  /* Use nv_pmu_boardobj_query instead of nv_pmu_boardobj */
	struct nv_pmu_clk_clk_vf_point_boardobj_get_status super;
	struct nv_pmu_clk_clk_vf_point_volt_boardobj_get_status volt;
	struct nv_pmu_clk_clk_vf_point_30_boardobj_get_status v30;
	struct nv_pmu_clk_clk_vf_point_35_freq_boardobj_get_status v35_freq;
	struct nv_pmu_clk_clk_vf_point_35_volt_pri_boardobj_get_status
								v35_volt_pri;
	struct nv_pmu_clk_clk_vf_point_35_volt_sec_boardobj_get_status
								v35_volt_sec;
	struct nv_pmu_clk_clk_vf_point_40_freq_boardobj_get_status v40_freq;
	struct nv_pmu_clk_clk_vf_point_40_volt_pri_boardobj_get_status v40_volt_pri;
	struct nv_pmu_clk_clk_vf_point_40_volt_sec_boardobj_get_status v40_volt_sec;
	struct nv_pmu_clk_clk_vf_point_50_freq_boardobj_get_status v50_freq;
	struct nv_pmu_clk_clk_vf_point_50_volt_pri_boardobj_get_status v50_volt_pri;
	struct nv_pmu_clk_clk_vf_point_50_volt_sec_boardobj_get_status v50_volt_sec;
	/* Force union to be exactly 144 bytes to match PMU side */
	u8 pmu_size_match[144];
};

/*
 * STRUCTURE SIZE VERIFICATION:
 * These static assertions ensure that NVGPU structures match PMU structure sizes.
 * The PMU uses 144-byte structures for VF points, so NVGPU must match exactly.
 *
 * IMPORTANT: Individual structures don't need to be 144 bytes - only the union
 * needs to be 144 bytes due to the pmu_size_match[144] array. The individual
 * structures are naturally smaller (around 60-80 bytes) but the union size
 * determines the array element size for proper PMU communication.
 */
nvgpu_static_assert(sizeof(union nv_pmu_clk_clk_vf_point_boardobj_get_status_union) == 144);

NV_PMU_BOARDOBJ_GRP_GET_STATUS_MAKE_VF_POINT(clk, clk_vf_point, CTRL_BOARDOBJGRP_E2048_MAX_OBJECTS);

/* Verify that our custom GET_STATUS aligned union is exactly 144 bytes */
nvgpu_static_assert(sizeof(union nv_pmu_clk_clk_vf_point_boardobj_get_status_union_aligned) == 144);

struct nv_pmu_clk_load {
	u8 feature;
	u32 action_mask;
};

struct nv_pmu_clk_freq_effective_avg {
	u32 clkDomainMask;
	u32 freqkHz[CTRL_BOARDOBJ_MAX_BOARD_OBJECTS];
};

struct nv_pmu_rpc_struct_clk_load {
	struct nv_pmu_rpc_header hdr;
	struct nv_pmu_clk_load clk_load;
	u32 scratch[1];
};

struct nv_pmu_clk_cmd_rpc {
	u8 cmd_type;
	u8 pad[3];
	struct nv_pmu_allocation request;
};

struct nv_pmu_clk_cmd_generic {
	u8 cmd_type;
	bool b_perf_daemon_cmd;
	u8 pad[2];
};

struct nv_pmu_clk_cmd {
	union {
		u8 cmd_type;
		struct nv_pmu_boardobj_cmd_grp grp_set;
		struct nv_pmu_clk_cmd_generic generic;
		struct nv_pmu_clk_cmd_rpc rpc;
		struct nv_pmu_boardobj_cmd_grp grp_get_status;
	};
};

struct nv_pmu_clk_msg_rpc {
	u8 msg_type;
	u8 rsvd[3];
	struct nv_pmu_allocation response;
};

struct nv_pmu_clk_msg {
	union {
		u8 msg_type;
		struct nv_pmu_boardobj_msg_grp grp_set;
		struct nv_pmu_clk_msg_rpc rpc;
		struct nv_pmu_boardobj_msg_grp grp_get_status;
	};
};

struct nv_pmu_clk_clk_vin_device_boardobjgrp_get_status_header {
	struct nv_pmu_boardobjgrp_e32 super;
};

struct nv_pmu_clk_clk_vin_device_boardobj_get_status {
	struct nv_pmu_boardobj_query super;
	u32 actual_voltage_uv;
	u32 corrected_voltage_uv;
	u8 sampled_code;
	u8 override_code;
};

union nv_pmu_clk_clk_vin_device_boardobj_get_status_union {
	struct nv_pmu_boardobj_query obj;
	struct nv_pmu_clk_clk_vin_device_boardobj_get_status super;
};

NV_PMU_BOARDOBJ_GRP_GET_STATUS_MAKE_E32(clk, clk_vin_device);

struct nv_pmu_clk_lut_vf_entry {
	u32 entry;
};

struct nv_pmu_clk_clk_fll_device_boardobjgrp_get_status_header {
	struct nv_pmu_boardobjgrp_e32 super;
};

struct nv_pmu_clk_clk_fll_device_boardobj_get_status {
	struct nv_pmu_boardobj_query super;
	u8 current_regime_id;
	bool b_dvco_min_reached;
	u16 min_freq_mhz;
	struct nv_pmu_clk_lut_vf_entry
		lut_vf_curve[NV_UNSIGNED_ROUNDED_DIV(
					CTRL_CLK_LUT_NUM_ENTRIES_MAX, 2)];
};

union nv_pmu_clk_clk_fll_device_boardobj_get_status_union {
	struct nv_pmu_boardobj_query obj;
	struct nv_pmu_clk_clk_fll_device_boardobj_get_status super;
};

NV_PMU_BOARDOBJ_GRP_GET_STATUS_MAKE_E32(clk, clk_fll_device);

struct nv_pmu_clk_clk_nafll_device_v2_boardobjgrp_get_status_header {
	struct nv_pmu_boardobjgrp_e32 super;
};

struct nv_ctrl_clk_lut_20_vf_entry {
	u16 ndiv;
	u8 ndivOffset;
	u8 dvcoOffset;
};

struct nv_ctrl_clk_lut_30_vf_entry {
	u16 ndiv;
	u8 ndivOffset[NV2080_CTRL_CLK_NAFLL_LUT_VF_CURVE_SEC_MAX];
	u8 dvcoOffset[NV2080_CTRL_CLK_NAFLL_LUT_VF_CURVE_SEC_MAX];
	u16 cpmMaxNdivOffset;
};

union nv_pmu_clk_lut_vf_entry_data_v2 {
	u32 entry;
	struct nv_pmu_clk_lut_vf_entry lut10;
	struct nv_ctrl_clk_lut_20_vf_entry lut20;
	struct nv_ctrl_clk_lut_30_vf_entry lut30;
};

struct nv_pmu_clk_lut_vf_entry_v2 {
	u8 type;
	union nv_pmu_clk_lut_vf_entry_data_v2 data;
};

struct nv_pmu_clk_lut_vf_curve_v2 {
	struct nv_pmu_clk_lut_vf_entry_v2 lutVfEntries[CTRL_CLK_LUT_NUM_ENTRIES_MAX];
};

struct nv_pmu_clk_clk_nafll_device_v2_boardobj_get_status {
	struct nv_pmu_boardobj_query super;
	u8 current_regime_id;
	bool b_dvco_min_reached;
	u16 dvco_min_freq_mhz;
	u8 sw_override_mode;
	union nv_pmu_clk_lut_vf_entry_data_v2 sw_override;
	struct nv_pmu_clk_lut_vf_curve_v2 lutVfCurve;
	u16 nafll_acc_freq_mhz[NV2080_CTRL_CLK_NAFLL_NDIV_ACCUMULATOR_TYPE_MAX];
};

union nv_pmu_clk_clk_nafll_device_v2_boardobj_get_status_union {
	struct nv_pmu_boardobj_query obj;
	struct nv_pmu_clk_clk_nafll_device_v2_boardobj_get_status super;
};

NV_PMU_BOARDOBJ_GRP_GET_STATUS_MAKE_E32(clk, clk_nafll_device_v2);

#endif /* NVGPU_PMUIF_CLK_H */
