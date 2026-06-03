// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/pmu/pmuif/nvgpu_cmdif.h>
#include <nvgpu/bios.h>
#include <nvgpu/kmem.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/pmu/boardobjgrp_classes.h>
#include <nvgpu/string.h>
#include <nvgpu/pmu/perf_cf.h>
#include <nvgpu/pmu/pmu_perfmon.h>

#include "perf_cf_topology.h"
#include "ucode_perf_cf_inf.h"

/*
 * Perf-CF Topology:
 * nvgpu parses VBIOS Perf-CF Topology table. Creates driver board objects, PMU board objects
 * and then sends an SET_RPC command to PMU.
 *
 * To get utilization data from PMU, nvgpu driver issues a GET_RPC command to PMU. Each entry
 * returned by PMU is represented by "union nv_pmu_perf_cf_topology_boardobj_get_status_union".
 * Let's take "struct nv_pmu_perf_cf_topology_sensed_base_get_status sensed_base" as an example.
 * In this struct, 'last_polled_reading' represents the utilization over the last 200 ms.
 * 'reading' represents the utilization since the last GET_RPC call to PMU.
 *
 * To get 'reading' value, nvgpu driver should send 'last_sensor_reading' and
 * 'last_base_sensor_reading' as an input to GET_RPC command. 'last_sensor_reading' and
 * 'last_base_sensor_reading' are in/out params i.e. at time t0 when nvgpu driver issues GET_RPC
 * command, the returned values of these params should be saved by the driver and should be sent
 * as 'in' params while issuing GET_RPC at time t1. PMU uses these values as shown below
 * reading = ((t1_last_sensor_reading - t0_last_sensor_reading) /
 * (t1_last_base_sensor_reading - t0_last_base_sensor_reading))
 * 't1_last_sensor_reading', 't1_last_base_sensor_reading' and 'reading' are returned to nvgpu driver.
 * Currently, nvgpu supports reading only 'last_polled_reading' and is returned to userspace.
 *
 * Time period 200 ms is decided by VBIOS. If samples are needed for any other time period then
 * rely on 'reading' value from PMU. There is an internal thread in PMU that probes the HW counters
 * and prepares the values for a block average of 200 ms, hence the fixed time period.
 */

static int perf_cf_topology_vbios_class_to_internal(struct gk20a *g,
	u8 vbios_class, u8 *type)
{
	if (type == NULL) {
		return -EINVAL;
	}

	switch (vbios_class) {
	case VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_TYPE_SENSED_BASE:
		*type = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_SENSED_BASE;
		break;
	case VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_TYPE_MIN_MAX:
		*type = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_MIN_MAX;
		break;
	case VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_TYPE_SENSED:
		*type = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_SENSED;
		break;
	default:
		nvgpu_err(g, "unknown vbios class 0x%02x", vbios_class);
		return -EINVAL;
	}

	return 0;
}

static int perf_cf_topology_get_label_from_hal(struct gk20a *g, u8 hal_val,
	u8 entry_idx, u8 *label)
{
	if (label == NULL) {
		return -EINVAL;
	}

	if (hal_val != VBIOS_PERF_CF_TOPOLOGY_TABLE_HEADER_HAL_TYPE_GA10X) {
		nvgpu_err(g, "Invalid HAL value: %d", hal_val);
		return -EINVAL;
	}

	*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_NONE;

	switch (entry_idx) {
	case 0:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_GR;
		break;
	case 1:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_FB;
		break;
	case 2:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_VID;
		break;
	case 3:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_PCIE_TX;
		break;
	case 4:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_PCIE_RX;
		break;
	case 5:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC0;
		break;
	case 6:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_ENC0;
		break;
	case 7:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC3;
		break;
	case 8:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC4;
		break;
	case 12:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_JPG0;
		break;
	case 13:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC1;
		break;
	case 14:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC2;
		break;
	case 15:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_OFA;
		break;
	case 18:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_GSP;
		break;
	case 19:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC5;
		break;
	case 20:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC6;
		break;
	case 21:
		*label = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC7;
		break;
	default:
		// Just return NV2080_CTRL_PERF_PERF_CF_POLICY_LABEL_NONE
		break;
	}

	return 0;
}

static u8 perf_cf_topology_get_gpumon_tag_from_label(u8 label)
{
	switch (label) {
	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_GR:
		return NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_GR;

	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_FB:
		return NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_FB;

	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC0:
	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC1:
	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC2:
	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC3:
	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC4:
	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC5:
	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC6:
	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC7:
		return NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_NVDEC;

	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_ENC0:
		return NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_NVENC;

	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_VID:
		return NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_VID;

	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_JPG0:
		return NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_NVJPG;

	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_OFA:
		return NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_NVOFA;

	default:
		return NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_GPUMON_TAG_NONE;
	}
}

static int perf_cf_topology_unit_map_vbios_to_internal(struct gk20a *g,
	u8 vbios_unit, u8 *unit)
{
	int status = 0;

	if (unit == NULL) {
		return -EINVAL;
	}

	switch (vbios_unit) {
	case VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_UNIT_PERCENTAGE:
		*unit = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_UNIT_PERCENTAGE;
		break;
	case VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_UNIT_BYTES_PER_NSEC:
		*unit = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_UNIT_BYTES_PER_NSEC;
		break;
	case VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_UNIT_GHZ:
		*unit = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_UNIT_GHZ;
		break;
	case VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_UNIT_NS:
		*unit = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_UNIT_NS;
		break;
	default:
		nvgpu_err(g, "Unknown vbios_unit = 0x%02x", vbios_unit);
		status = -EINVAL;
		break;
	}

	return status;
}

static int perf_cf_topology_construct_super(struct gk20a *g,
	struct pmu_board_obj **obj, size_t size, void *data)
{
	struct perf_cf_topology *ptemp_cf_top = (struct perf_cf_topology *)data;
	struct perf_cf_topology *pperf_cf_top = NULL;
	int status = 0;

	pperf_cf_top = nvgpu_kzalloc(g, size);
	if (pperf_cf_top == NULL) {
		return -ENOMEM;
	}

	status = pmu_board_obj_construct_super(g,
			(struct pmu_board_obj *)(void *)pperf_cf_top, data);
	if (status != 0) {
		return -EINVAL;
	}

	*obj = (struct pmu_board_obj *)(void *)pperf_cf_top;

	// Set member variables
	pperf_cf_top->unit           = ptemp_cf_top->unit;
	pperf_cf_top->label          = ptemp_cf_top->label;
	pperf_cf_top->gpumon_tag     = ptemp_cf_top->gpumon_tag;
	pperf_cf_top->not_available  = ptemp_cf_top->not_available;

	return status;
}

static int perf_cf_topology_pmu_data_init_super(struct gk20a *g,
	struct pmu_board_obj *obj, struct nv_pmu_boardobj *pmu_obj)
{
	struct nv_pmu_perf_cf_topology_boardobj_set *pset;
	struct perf_cf_topology *ptop;
	int status = 0;

	// Call super-class implementation
	status = pmu_board_obj_pmu_data_init_super(g, obj, pmu_obj);
	if (status != 0) {
		return status;
	}

	ptop = (struct perf_cf_topology *)(void *)obj;
	pset = (struct nv_pmu_perf_cf_topology_boardobj_set *)(void *)
				pmu_obj;

	// Set member variables
	pset->gpumon_tag = ptop->gpumon_tag;
	pset->not_available = ptop->not_available;

	return 0;
}

static int perf_cf_topology_pmu_data_init_sensed_base(struct gk20a *g,
	struct pmu_board_obj *obj, struct nv_pmu_boardobj *pmu_obj)
{
	struct nv_pmu_perf_cf_topology_sensed_base_boardobj_set *pset;
	struct perf_cf_topology_sensed_base *ptop_sensed_base;
	int status = 0;

	// Call super-class implementation
	status = perf_cf_topology_pmu_data_init_super(g, obj, pmu_obj);
	if (status != 0) {
		return status;
	}

	ptop_sensed_base = (struct perf_cf_topology_sensed_base *)(void *)obj;
	pset = (struct nv_pmu_perf_cf_topology_sensed_base_boardobj_set *)(void *)
				pmu_obj;

	// Copy member variables
	pset->sensor_idx = ptop_sensed_base->sensor_idx;
	pset->base_sensor_idx = ptop_sensed_base->base_sensor_idx;

	return 0;
}

static int perf_cf_topology_construct_sensed_base(struct gk20a *g,
	struct pmu_board_obj **obj, size_t size, void *data)
{
	struct perf_cf_topology_sensed_base *ptemp_cf_top_sb =
		(struct perf_cf_topology_sensed_base *)data;
	struct perf_cf_topology_sensed_base *pperf_cf_top_sb = NULL;
	struct pmu_board_obj *obj_tmp = NULL;
	int status = 0;

	// Call super-class constructor
	ptemp_cf_top_sb->super.super.type_mask |=
		BIT(NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_SENSED_BASE);
	status = perf_cf_topology_construct_super(g, obj, size, data);
	if (status != 0) {
		nvgpu_err(g, "Failed to construct perf cf topology super object");
		return status;
	}

	obj_tmp = *obj;
	obj_tmp->pmudatainit = perf_cf_topology_pmu_data_init_sensed_base;

	pperf_cf_top_sb = (struct perf_cf_topology_sensed_base *)(void *)*obj;

	// Set member variables
	pperf_cf_top_sb->sensor_idx = ptemp_cf_top_sb->sensor_idx;
	pperf_cf_top_sb->base_sensor_idx = ptemp_cf_top_sb->base_sensor_idx;

	return 0;
}

static int perf_cf_topology_pmu_data_init_min_max(struct gk20a *g,
	struct pmu_board_obj *obj, struct nv_pmu_boardobj *pmu_obj)
{
	struct nv_pmu_perf_cf_topology_min_max_boardobj_set *pset;
	struct perf_cf_topology_min_max *ptop_min_max;
	int status = 0;

	// Call super-class implementation
	status = perf_cf_topology_pmu_data_init_super(g, obj, pmu_obj);
	if (status != 0) {
		return status;
	}

	ptop_min_max = (struct perf_cf_topology_min_max *)(void *)obj;
	pset = (struct nv_pmu_perf_cf_topology_min_max_boardobj_set *)(void *)
				pmu_obj;

	// Copy member variables
	pset->topology_idx1 = ptop_min_max->topology_idx1;
	pset->topology_idx2 = ptop_min_max->topology_idx2;
	pset->is_max = ptop_min_max->is_max;

	return 0;
}

static int perf_cf_topology_construct_min_max(struct gk20a *g,
	struct pmu_board_obj **obj, size_t size, void *data)
{
	struct perf_cf_topology_min_max *ptemp_cf_top_mm =
		(struct perf_cf_topology_min_max *)data;
	struct perf_cf_topology_min_max *pperf_cf_top_mm = NULL;
	struct pmu_board_obj *obj_tmp = NULL;
	int status = 0;

	// Call super-class constructor
	ptemp_cf_top_mm->super.super.type_mask |=
		BIT(NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_MIN_MAX);
	status = perf_cf_topology_construct_super(g, obj, size, data);
	if (status != 0) {
		nvgpu_err(g, "Failed to construct perf cf topology super object");
		return status;
	}

	obj_tmp = *obj;
	obj_tmp->pmudatainit = perf_cf_topology_pmu_data_init_min_max;

	pperf_cf_top_mm = (struct perf_cf_topology_min_max *)(void *)*obj;

	// Set member variables
	pperf_cf_top_mm->topology_idx1 = ptemp_cf_top_mm->topology_idx1;
	pperf_cf_top_mm->topology_idx2 = ptemp_cf_top_mm->topology_idx2;
	pperf_cf_top_mm->is_max = ptemp_cf_top_mm->is_max;

	return 0;
}

static int perf_cf_topology_pmu_data_init_sensed(struct gk20a *g,
	struct pmu_board_obj *obj, struct nv_pmu_boardobj *pmu_obj)
{
	struct nv_pmu_perf_cf_topology_sensed_boardobj_set *pset;
	struct perf_cf_topology_sensed *ptop_sensed;
	int status = 0;

	// Call super-class implementation
	status = perf_cf_topology_pmu_data_init_super(g, obj, pmu_obj);
	if (status != 0) {
		return status;
	}

	ptop_sensed = (struct perf_cf_topology_sensed *)(void *)obj;
	pset = (struct nv_pmu_perf_cf_topology_sensed_boardobj_set *)(void *)
				pmu_obj;

	// Copy member variables
	pset->sensor_idx = ptop_sensed->sensor_idx;

	return 0;
}

static int perf_cf_topology_construct_sensed(struct gk20a *g,
	struct pmu_board_obj **obj, size_t size, void *data)
{
	struct perf_cf_topology_sensed *ptemp_cf_top_s =
		(struct perf_cf_topology_sensed *)data;
	struct perf_cf_topology_sensed *pperf_cf_top_s = NULL;
	struct pmu_board_obj *obj_tmp = NULL;
	int status = 0;

	// Call super-class constructor
	ptemp_cf_top_s->super.super.type_mask |=
		BIT(NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_SENSED);
	status = perf_cf_topology_construct_super(g, obj, size, data);
	if (status != 0) {
		nvgpu_err(g, "Failed to construct perf cf topology super object");
		return status;
	}

	obj_tmp = *obj;
	obj_tmp->pmudatainit = perf_cf_topology_pmu_data_init_sensed;

	pperf_cf_top_s = (struct perf_cf_topology_sensed *)(void *)*obj;

	// Set member variables
	pperf_cf_top_s->sensor_idx = ptemp_cf_top_s->sensor_idx;

	return 0;
}

static struct perf_cf_topology *construct_perf_cf_topology(struct gk20a *g, void *data)
{
	u8 type = pmu_board_obj_get_type(data);
	struct pmu_board_obj *obj = NULL;
	int status = 0;

	switch (type) {
	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_SENSED_BASE:
		status = perf_cf_topology_construct_sensed_base(g, &obj,
			sizeof(struct perf_cf_topology_sensed_base), data);
		break;
	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_MIN_MAX:
		status = perf_cf_topology_construct_min_max(g, &obj,
			sizeof(struct perf_cf_topology_min_max), data);
		break;
	case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_SENSED:
		status = perf_cf_topology_construct_sensed(g, &obj,
			sizeof(struct perf_cf_topology_sensed), data);
		break;
	default:
		nvgpu_err(g, "Unknown type: %d", type);
		return NULL;
	}

	// Check if allocation succeeded
	if (status != 0) {
		nvgpu_err(g, "Failed to allocate PERF_CF_TOPOLOGY object type (0x%02x).\n", type);
		return NULL;
	}

	return (struct perf_cf_topology *)obj;
}

static int perf_get_perf_cf_topologys(struct gk20a *g,
	struct perf_cf_topology_metadata *pperf_cf_top_metadata)
{
	struct perf_cf_topology_table_header header = { 0 };
	struct perf_cf_topology *pperf_cf_top = NULL;
	u8 *topologys_table_ptr = NULL, i;
	u8 *entry_offset;
	int status = 0;
	union {
		struct pmu_board_obj                  boardobj;
		struct perf_cf_topology               super;
		struct perf_cf_topology_sensed_base   sensed_base;
		struct perf_cf_topology_min_max       min_max;
		struct perf_cf_topology_sensed        sensed;
	} data;

	topologys_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			nvgpu_bios_get_bit_token(g, NVGPU_BIOS_PERF_TOKEN),
			PERF_CF_TOPOLOGY_TABLE);
	if (topologys_table_ptr == NULL) {
		nvgpu_err(g, "Failed to get PERF_CF Topology table pointer");
		status = -EINVAL;
		goto done;
	}

	nvgpu_memcpy((u8 *)&header, topologys_table_ptr,
			sizeof(struct perf_cf_topology_table_header));

	if (header.version != VBIOS_PERF_CF_SENSOR_TABLE_HEADER_VERSION_1X) {
		nvgpu_err(g, "Unsupported PERF_CF Topology table version: %d", header.version);
		status = -EINVAL;
		goto done;
	}

	// Parse header data
	pperf_cf_top_metadata->polling_period_ms = header.adv_poll_period_ms;
	pperf_cf_top_metadata->hal_val = header.topology_hal;

	nvgpu_pmu_dbg(g, "Header data. version: %d, header_size: %d, entry_count: %d, "
		"entry_size: %d, topology_hal: %d, adv_poll_period_ms: %d",
		header.version, header.header_size, header.entry_count,
		header.entry_size, header.topology_hal, header.adv_poll_period_ms);

	// Read the entries
	for (i = 0; i < header.entry_count; i++) {
		struct perf_cf_topology_table_entry entry = { 0 };
		u8 class_id;

		entry_offset = topologys_table_ptr + header.header_size +
				(i * header.entry_size);
		nvgpu_memcpy((u8 *)&entry, entry_offset, header.entry_size);

		if ((entry.flags & VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_MASK) ==
		     VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_TYPE_DISABLED) {
			continue;
		}

		// Parse common data
		status = perf_cf_topology_vbios_class_to_internal(g,
			entry.flags & VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_MASK,
			&class_id);
		if (status != 0) {
			nvgpu_err(g, "Failed to convert vbios class to internal type");
			goto done;
		}

		data.boardobj.type = class_id;

		status = perf_cf_topology_get_label_from_hal(g, pperf_cf_top_metadata->hal_val,
				i, &data.super.label);
		if (status != 0) {
			nvgpu_err(g, "Failed to get label from hal");
			goto done;
		}

		data.super.gpumon_tag = perf_cf_topology_get_gpumon_tag_from_label(
					data.super.label);

		status = perf_cf_topology_unit_map_vbios_to_internal(g,
				entry.flags >> VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_FLAGS_UNIT_SHIFT,
				&data.super.unit);
		if (status != 0) {
			nvgpu_err(g, "Failed to map vbios unit to internal unit");
			goto done;
		}

		/*
		 * VBIOS perf-cf topology table has entries for various engines like DEC6/7, JPG.
		 * RM enumerates the engines supported in a given chip and marks the floorswept
		 * engines as unavailable. But, when I compared RM dump of boardobjects,
		 * not_available is set to 0x00 for all entries. So, setting it to 0x00 for now.
		 * TODO: Populate this value correctly by enumerating engines supported in a
		 * given chip.
		 */
		data.super.not_available = 0x00;

		// Parse class specific data
		switch (class_id) {
		case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_SENSED_BASE:
			data.sensed_base.sensor_idx = (u8)(entry.param0 &
				VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_SENSED_BASE_SENSOR_IDX_MASK);
			data.sensed_base.base_sensor_idx = (u8)((entry.param0 >>
				VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_SENSED_BASE_BASE_S_SHIFT) &
				VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_SENSED_BASE_BASE_S_IDX_MASK);
			nvgpu_pmu_dbg(g, "sensed_base.sensor_idx: %d, sensed_base.base_sensor_idx: %d",
				data.sensed_base.sensor_idx, data.sensed_base.base_sensor_idx);
			break;
		case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_MIN_MAX:
			data.min_max.topology_idx1 = (u8)(entry.param0 &
				VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_MIN_MAX_TOPOLOGY_1_MASK);
			data.min_max.topology_idx2 = (u8)((entry.param0 >>
				VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_MIN_MAX_TOPOLOGY_2_SHIFT) &
				VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_MIN_MAX_TOPOLOGY_2_MASK);
			data.min_max.is_max = ((entry.param0 >>
				VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_MIN_MAX_CRITERIA_SHIFT) &
				VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_MIN_MAX_CRITERIA_MASK) ==
				VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_MIN_MAX_CRITERIA_MAX;
			nvgpu_pmu_dbg(g, "min_max.topology_idx1: %d, min_max.topology_idx2: %d, "
				"min_max.is_max: %d", data.min_max.topology_idx1,
				data.min_max.topology_idx2, data.min_max.is_max);
			break;
		case NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_TYPE_SENSED:
			data.sensed.sensor_idx = (u8)(entry.param0 &
				VBIOS_PERF_CF_TOPOLOGY_TABLE_ENTRY_PARAM0_SENSED_SENSOR_IDX_MASK);
			nvgpu_pmu_dbg(g, "sensed.sensor_idx: %d", data.sensed.sensor_idx);
			break;
		default:
			nvgpu_err(g, "Unknown class id: %d", class_id);
			status = -EINVAL;
			goto done;
		}

		// Create a PERF_CF Topology driver board object
		pperf_cf_top = construct_perf_cf_topology(g, (void *)&data);
		if (pperf_cf_top == NULL) {
			nvgpu_err(g, "Failed to construct perf CF topology object");
			goto done;
		}

		// Insert object into the group
		status = boardobjgrp_objinsert(&pperf_cf_top_metadata->topologies.super,
			&pperf_cf_top->super, i);
		if (status != 0) {
			nvgpu_err(g, "Failed to add entry %d into boardobjgrp, status: 0x%x",
				i, status);
			goto done;
		}

		nvgpu_pmu_dbg(g, "type: %d, idx: %d, type_mask: 0x%x, label: %d, gpumon_tag: %d, "
			"unit: %d, not_available: %d", pperf_cf_top->super.type,
			pperf_cf_top->super.idx, pperf_cf_top->super.type_mask, pperf_cf_top->label,
			pperf_cf_top->gpumon_tag, pperf_cf_top->unit, pperf_cf_top->not_available);

	}

done:
	return status;
}

static int perf_cf_topology_devgrp_pmudata_instget(struct gk20a *g,
	struct nv_pmu_boardobjgrp *pmuboardobjgrp,
	struct nv_pmu_boardobj **pmu_obj, u16 idx)
{
	struct nv_pmu_perf_cf_topology_boardobj_grp_set *pgrp_set =
		(struct nv_pmu_perf_cf_topology_boardobj_grp_set *)pmuboardobjgrp;

	nvgpu_log_info(g, " ");

	// Check whether pmuboardobjgrp has a valid boardobj in index
	if (((u32)BIT(idx) &
		pgrp_set->hdr.data.super.obj_mask.super.data[0]) == 0U) {
		return -EINVAL;
	}

	*pmu_obj = (struct nv_pmu_boardobj *)&pgrp_set->objects[idx].data.obj;

	nvgpu_log_info(g, "Done");
	return 0;
}

static int perf_cf_topology_grp_pmudatainit_super(struct gk20a *g,
	struct boardobjgrp *pboardobjgrp,
	struct nv_pmu_boardobjgrp_super *pboardobjgrppmu)
{
	struct nv_pmu_perf_cf_topology_boardobjgrp_set_header *pset =
		(struct nv_pmu_perf_cf_topology_boardobjgrp_set_header *)pboardobjgrppmu;
	struct perf_cf_topology_metadata *pperf_cf_top =
		(struct perf_cf_topology_metadata *)pboardobjgrp;
	int status = 0;

	nvgpu_log_info(g, " ");

	status = boardobjgrp_pmudatainit_e32(g, pboardobjgrp, pboardobjgrppmu);
	if (status != 0) {
		nvgpu_err(g, "Failed to update pmu boardobjgrp, status: 0x%x", status);
		return status;
	}

	pset->polling_period_ms = pperf_cf_top->polling_period_ms;

	nvgpu_log_info(g, "Done");
	return 0;
}

int perf_cf_topology_pmu_setup(struct gk20a *g)
{
	struct boardobjgrp *pboardobjgrp = NULL;
	int status;

	nvgpu_log_info(g, " ");

	pboardobjgrp =
		&g->pmu->perf_cf->perf_cf_metadata->perf_cf_top_metadata.topologies.super;

	if (!pboardobjgrp->bconstructed) {
		return -EINVAL;
	}

	status = pboardobjgrp->pmuinithandle(g, pboardobjgrp);

	nvgpu_log_info(g, "Done");
	return status;
}

static int convert_perfmon_class_to_perf_cf_entry(enum nvgpu_pmu_perfmon_class perfmon_class)
{
	int entry;

	switch (perfmon_class) {
	case NVGPU_PMU_PERFMON_CLASS_GR:
		entry = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_GR;
		break;
	case NVGPU_PMU_PERFMON_CLASS_NVENC0:
		entry = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_ENC0;
		break;
	case NVGPU_PMU_PERFMON_CLASS_NVDEC0:
		entry = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_DEC0;
		break;
	case NVGPU_PMU_PERFMON_CLASS_OFA:
		entry = NV2080_CTRL_PERF_PERF_CF_TOPOLOGY_LABEL_OFA;
		break;
	default:
		entry = -EINVAL;
	}

	return entry;
}

/**
 * fxp_to_scaled_u64 - Convert a fixed point value to a scaled integer value
 *
 * @fxp_val: fixed point value
 * @frac_bits: number of fractional bits in FXP format
 * @scale: scale factor
 *
 * A scale factor of 100 will return 2 decimal digits (as integer)
 *
 * Return: scaled integer value
 */
static u64 fxp_to_scaled_u64(u64 fxp_val, u32 frac_bits, u32 scale)
{
	return div64_u64(fxp_val * scale, (1ULL << frac_bits));
}

static int perf_cf_topology_obj_get_load(struct gk20a *g, struct pmu_board_obj *obj,
	struct nv_pmu_boardobj_query *pmu_obj, u32 *utilization)
{
	struct perf_cf_topology_sensed_base *pperf_cf_top_sensed_base;
	struct nv_pmu_perf_cf_topology_sensed_base_get_status *pstatus;
	u64 last_polled_reading_real;

	pperf_cf_top_sensed_base = (struct perf_cf_topology_sensed_base *)(void *)obj;
	pstatus = (struct nv_pmu_perf_cf_topology_sensed_base_get_status *)(void *)pmu_obj;

	if (pstatus->super.super.type != pperf_cf_top_sensed_base->super.super.type) {
		nvgpu_err(g, "pmu boardobj type: 0x%x, drv boardobj type: 0x%x mismatch",
			pstatus->super.super.type, pperf_cf_top_sensed_base->super.super.type);
		return -EINVAL;
	}

	last_polled_reading_real = fxp_to_scaled_u64(pstatus->super.last_polled_reading, 24, 100);

	nvgpu_pmu_dbg(g, "last_polled_reading from PMU: 0x%llx, last_polled_reading_real: %llu",
		pstatus->super.last_polled_reading, last_polled_reading_real);

	*utilization = nvgpu_safe_cast_u64_to_u32(last_polled_reading_real);
	return 0;
}

static int perf_cf_topology_devgrp_pmustatus_instget(struct gk20a *g,
	void *pboardobjgrppmu, struct nv_pmu_boardobj_query
	**obj_pmu_status, u16 idx)
{
	struct nv_pmu_perf_cf_topology_boardobj_grp_get_status *pgrp_get_status =
		(struct nv_pmu_perf_cf_topology_boardobj_grp_get_status *)pboardobjgrppmu;

	// Check whether pmuboardobjgrp has a valid boardobj in index
	if (((u32)BIT(idx) & pgrp_get_status->hdr.data.super.obj_mask.super.data[0]) == 0U) {
		nvgpu_err(g, "Invalid boardobj at index: %d", idx);
		return -EINVAL;
	}

	*obj_pmu_status = (struct nv_pmu_boardobj_query *)&pgrp_get_status->objects[idx].data.obj;

	return 0;
}

static int perf_cf_topology_boardobj_grp_get_status(struct gk20a *g, u32 *utilization, u8 entry)
{
	struct nv_pmu_boardobj_query *pboardobjpmustatus = NULL;
	struct nv_pmu_boardobjgrp_super *pboardobjgrppmu;
	struct boardobjgrpmask *pboardobjgrpmask;
	struct boardobjgrp_e32 *pboardobjgrpe32;
	struct boardobjgrp *pboardobjgrp;
	struct pmu_board_obj *obj = NULL;
	int status;
	u16 index;

	nvgpu_log_info(g, " ");

	pboardobjgrpe32 = &g->pmu->perf_cf->perf_cf_metadata->perf_cf_top_metadata.topologies;
	pboardobjgrp = &pboardobjgrpe32->super;
	pboardobjgrpmask = &pboardobjgrpe32->mask.super;

	status = pboardobjgrp->pmugetstatus(g, pboardobjgrp, pboardobjgrpmask);
	if (status != 0) {
		nvgpu_err(g, "Failed to get boardobjs from pmu, error: %d", status);
		return status;
	}
	pboardobjgrppmu = pboardobjgrp->pmu.getstatus.buf;

	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj *, obj, index) {
		if (index != entry) {
			continue;
		}

		status = pboardobjgrp->pmustatusinstget(g,
			(struct nv_pmu_boardobjgrp *)(void *)pboardobjgrppmu,
			&pboardobjpmustatus, index);
		if (status != 0) {
			return status;
		}

		status = perf_cf_topology_obj_get_load(g, obj, pboardobjpmustatus, utilization);
		if (status != 0) {
			return status;
		}
	}

	nvgpu_log_info(g, "Done");

	return 0;
}

int perf_cf_topology_get_load(struct gk20a *g, u32 *utilization,
	enum nvgpu_pmu_perfmon_class perfmon_class)
{
	int entry;

	if (utilization == NULL) {
		return -EINVAL;
	}

	entry = convert_perfmon_class_to_perf_cf_entry(perfmon_class);
	if (entry < 0) {
		nvgpu_err(g, "Invalid perfmon class: %d", perfmon_class);
		return entry;
	}

	return perf_cf_topology_boardobj_grp_get_status(g, utilization, entry);
}

int perf_cf_topology_sw_setup(struct gk20a *g)
{
	struct boardobjgrp *pboardobjgrp = NULL;
	int status = 0;

	nvgpu_log_info(g, " ");

	/* If already constructed, do not re-construct (suspend/resume pattern) */
	pboardobjgrp = &g->pmu->perf_cf->perf_cf_metadata->perf_cf_top_metadata.topologies.super;
	if (pboardobjgrp->bconstructed) {
		nvgpu_pmu_dbg(g, "perf_cf_topology boardobjgrp already constructed, skipping reinit");
		return 0;
	}

	nvgpu_pmu_dbg(g, "Constructing perf_cf_topology boardobjgrp for first time");

	status = nvgpu_boardobjgrp_construct_e32(g,
		&g->pmu->perf_cf->perf_cf_metadata->perf_cf_top_metadata.topologies);
	if (status != 0) {
		nvgpu_err(g, "Failed to create boardobjgrp, status: 0x%x", status);
		return status;
	}

	pboardobjgrp = &g->pmu->perf_cf->perf_cf_metadata->perf_cf_top_metadata.topologies.super;

	pboardobjgrp->pmudatainstget = perf_cf_topology_devgrp_pmudata_instget;
	pboardobjgrp->pmudatainit = perf_cf_topology_grp_pmudatainit_super;
	pboardobjgrp->pmustatusinstget = perf_cf_topology_devgrp_pmustatus_instget;

	// Parse PERF_CF Topology table from VBIOS and create driver board objects
	status = perf_get_perf_cf_topologys(g,
		&g->pmu->perf_cf->perf_cf_metadata->perf_cf_top_metadata);
	if (status != 0) {
		return status;
	}

	// Populate data for the Perf-CF PMU interface
	BOARDOBJGRP_PMU_CONSTRUCT(pboardobjgrp, PERF_CF, PERF_CF_TOPOLOGY);

	status = BOARDOBJGRP_PMU_CMD_GRP_SET_CONSTRUCT(g, pboardobjgrp,
		perf_cf, PERF_CF, topology, PERF_CF_TOPOLOGY);
	if (status != 0) {
		nvgpu_err(g, "Failed to construct PMU_CMD_GRP_SET, status: 0x%x", status);
		return status;
	}

	status = BOARDOBJGRP_PMU_CMD_GRP_GET_STATUS_CONSTRUCT(g, pboardobjgrp,
		perf_cf, PERF_CF, topology, PERF_CF_TOPOLOGY);
	if (status != 0) {
		nvgpu_err(g, "Failed to construct PMU_CMD_GRP_GET_STATUS, status: 0x%x", status);
		return status;
	}

	nvgpu_log_info(g, "Done");

	return status;
}
