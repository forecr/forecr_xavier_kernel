/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

/*!
 * @ref VBIOS_PERF_CF_TOPOLOGY_TABLE_HEADER.topologyHAL determines the set
 * of topologies available for selection. This allows the PERF_CF Topology Table
 * to be used with different sets of engines for different GPU families without
 * changing the structure and the revision of the PERF_CF Topology Table.
 */
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_HEADER_HAL_TYPE_TU10X                      0x00U
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_HEADER_HAL_TYPE_GA10X                      0x01U
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_HEADER_HAL_TYPE_GH100                      0x02U

#define VBIOS_PERF_CF_SENSOR_TABLE_HEADER_VERSION_1X                            0x10U

/*!
 * Macros for PERF_CF Topology labels
 */
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_GR                              0x00U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_FB                              0x01U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_VID                             0x02U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_PCIE_TX                         0x03U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_PCIE_RX                         0x04U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC0                            0x05U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_ENC0                            0x06U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC3                            0x07U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC4                            0x08U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_JPG0                            0x0CU
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC1                            0x0DU
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC2                            0x0EU
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_OFA                             0x0FU
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_GSP                             0x12U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC5                            0x13U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC6                            0x14U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC7                            0x15U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_NONE                            0xFFU

/*!
 * Macros for PERF_CF Topology GPUMON tags
 */
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_FB                         0x00U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_GR                         0x01U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_NVENC                      0x02U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_NVDEC                      0x03U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_VID                        0x04U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_NVJPG                      0x05U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_NVOFA                      0x06U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_NONE                       0xFFU

/*!
 * Macros for PERF_CF Topology units
 */
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_UNIT_PERCENTAGE                       0x00U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_UNIT_BYTES_PER_NSEC                   0x01U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_UNIT_GHZ                              0x02U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_UNIT_NS                               0x03U

#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_MASK                           0x0FU
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_TYPE_DISABLED                  0x00U
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_TYPE_SENSED_BASE               0x01U
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_TYPE_MIN_MAX                   0x02U
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_TYPE_SENSED                    0x04U
// Values 0x03..0x0F are reserved
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_UNIT_SHIFT                     0x04U
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_UNIT_PERCENTAGE                0x00U
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_UNIT_BYTES_PER_NSEC            0x01U
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_UNIT_GHZ                       0x02U
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_UNIT_NS                        0x03U
// Values 0x03..0x0F are reserved

#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_SENSED_BASE_SENSOR_IDX_MASK   0xFFU
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_SENSED_BASE_BASE_S_SHIFT      0x08U
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_SENSED_BASE_BASE_S_IDX_MASK   0xFFU

#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_MIN_MAX_TOPOLOGY_1_MASK       0xFFU
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_MIN_MAX_TOPOLOGY_2_SHIFT      0x08U
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_MIN_MAX_TOPOLOGY_2_MASK       0xFFU
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_MIN_MAX_CRITERIA_SHIFT        0x10U
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_MIN_MAX_CRITERIA_MASK         0b1
#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_MIN_MAX_CRITERIA_MAX          0b1

#define VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_SENSED_SENSOR_IDX_MASK        0xFFU

/**
 * @defgroup NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPES
 *
 * Macros for PERF_CF Topology types
 *
 * @{
 */
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_BASE                             0x00U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_SENSED_BASE                      0x01U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_MIN_MAX                          0x02U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_SENSED                           0x03U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_PMU_INIT_1X                      0x04U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_SENSED_20                        0x05U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_SENSED_BASE_20                   0x07U
#define NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_MIN_MAX_20                       0x06U
/** @} */

#define NV_PMU_PERF_CF_CMD_ID_BOARDOBJ_GRP_SET                                  0x00U
#define NV_PMU_PERF_CF_MSG_ID_BOARDOBJ_GRP_SET                                  0x00U
#define NV_PMU_PERF_CF_CMD_ID_BOARDOBJ_GRP_GET_STATUS                           0x01U
#define NV_PMU_PERF_CF_MSG_ID_BOARDOBJ_GRP_GET_STATUS                           0x01U

/**
 * @brief Virtual BOARDOBJ child providing attributes common to all PERF_CF Topologies.
 */
struct perf_cf_topology {
	/**
	 * @brief BOARDOBJ super-class.
	 * Must be first element of the structure!
	 */
	struct pmu_board_obj super;

	/* VBIOS data */
	/**
	 * @brief Unit of topology reading.
	 * @ref NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_UNIT_<xyz>.
	 */
	u8 unit;
	/**
	 * @brief Label.
	 * @ref NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_<xyz>.
	 */
	u8 label;

	/* Non-VBIOS data */
	/**
	 * @brief Tagged for GPUMON logging.
	 * @ref NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_<xyz>.
	 */
	u8 gpumon_tag;
	/**
	 * @brief This topology is not actually available (e.g. engine is floorswept).
	 */
	bool not_available;
};

/**
 * @brief PERF_CF_TOPOLOGY child class providing attributes of
 * Sensed Base PERF_CF Topology.
 */
struct perf_cf_topology_sensed_base {
	/**
	 * @brief PERF_CF_TOPOLOGY parent class.
	 * Must be first element of the structure!
	 */
	struct perf_cf_topology super;
	/**
	 * @brief Index into the Performance Sensor Table for sensor.
	 */
	u8 sensor_idx;
	/**
	 * @brief Index into the Performance Sensor Table for base sensor.
	 */
	u8 base_sensor_idx;
};

/**
 * @brief PERF_CF_TOPOLOGY child class providing attributes of
 * Min-max PERF_CF Topology.
 */
struct perf_cf_topology_min_max {
	/**
	 * @brief PERF_CF_TOPOLOGY parent class.
	 * Must be first element of the structure!
	 */
	struct perf_cf_topology super;
	/**
	 * @brief Index into the Performance Topology Table for 1st topology.
	 */
	u8 topology_idx1;
	/**
	 * @brief Index into the Performance Topology Table for 2nd topology.
	 */
	u8 topology_idx2;
	/**
	 * @brief true = max, false = min.
	 */
	bool is_max;
};

/**
 * @brief PERF_CF_TOPOLOGY child class providing attributes of
 * Sensed PERF_CF Topology.
 */
struct perf_cf_topology_sensed {
	/**
	 * @brief PERF_CF_TOPOLOGY parent class.
	 * Must be first element of the structure!
	 */
	struct perf_cf_topology super;
	/**
	 * @brief Index into the Performance Sensor Table for sensor.
	 */
	u8 sensor_idx; // NvBoardObjIdx
};

struct perf_cf_topology_table_header {
	u8 version;		/* PERF_CF Topology Table Version */
	u8 header_size;		/* Size of PERF_CF Topology Table Header in bytes */
	u8 entry_size;		/* Size of PERF_CF Topology Table Entry in bytes */
	u8 entry_count;		/* Number of PERF_CF Topology Entries */
	u8 topology_hal;	/* List of enum. to use when interpreting Topology entries */
	u16 adv_poll_period_ms;	/* RM Advertized polling period [ms] */
} __attribute__((packed));

struct perf_cf_topology_table_entry {
	u8 flags;		/* Flags including PERF_CF Topology type */
	u32 param0;		/* Type-specific parameter 0 */
} __attribute__((packed));

int perf_cf_topology_sw_setup(struct gk20a *g);
int perf_cf_topology_pmu_setup(struct gk20a *g);
