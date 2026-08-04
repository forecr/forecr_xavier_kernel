// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bios.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/pmu/boardobjgrp_classes.h>
#include <nvgpu/string.h>
#include <nvgpu/pmu/clk/clk.h>
#include <nvgpu/pmu/perf.h>
#include <nvgpu/pmu/volt.h>

#include "volt.h"
#include "ucode_volt_inf.h"
#include "volt_rail.h"

#define NV_PMU_PERF_RPC_VFE_EQU_MONITOR_COUNT_MAX				16U

/* ------------------------ Macros ----------------------------------------- */
#define VBIOS_VOLT_TABLE_3X_NUM_HALS						0x4U
#define VBIOS_VOLT_TABLE_3X_HALS_NUM_VOLT_RAIL_ENTRIES				0xFFU

/*!
 * Enumeration of voltage rail types.
 */
#define VOLTAGE_RAIL_TABLE_3X_HAL_VOLT_RAIL_TYPE_LOGICAL			0x0U
#define VOLTAGE_RAIL_TABLE_3X_HAL_VOLT_RAIL_TYPE_PHYSICAL			0x1U
#define VOLTAGE_RAIL_TABLE_3X_HAL_VOLT_RAIL_TYPE_INVALID			0xFFU

/*!
 * Invalid index of voltage rails.
 */
#define VOLTAGE_RAIL_TABLE_3X_HAL_VOLT_RAIL_IDX_INVALID				U8_MAX

// VBIOS for GB20C is not expected to have any value other than GA10X_MULTI_RAIL
#define VBIOS_VOLTAGE_RAIL_3X_VOLT_DOMAIN_HAL_GP10X_SINGLE_RAIL 		0x00
#define VBIOS_VOLTAGE_RAIL_3X_VOLT_DOMAIN_HAL_GA10X_MULTI_RAIL 			0x01

/* ------------------------ Datatypes -------------------------------------- */
/*!
 * @brief   Type reserved for voltage rail type enumerations.
 */
typedef u8 voltage_rail_table_3x_hal_voltage_rail_type;

/*!
 * Structure with logical voltage rail type specific data.
 */
struct voltage_rail_table_3x_hal_voltage_rail_logical {
	/*!
	 * Index of the first physical voltage rail abstracted by the given logical rail
	 */
	u8 volt_rail_physical_idx_first;

	/*!
	 * Index of the last physical voltage rail abstracted by the given logical rail
	 */
	u8 volt_rail_physical_idx_last;
};

/*!
 * Structure with physical voltage rail type specific data.
 */
struct voltage_rail_table_3x_hal_voltage_rail_physical {
	/*!
	 * Index of the logical voltage rail which includes given physical rail
	 */
	u8 volt_rail_logical_idx;
};

/*!
 * Type-specific data union.
 */
union voltage_rail_table_3x_hal_voltage_rail_data {
	/*!
	 * Type specific data of logical rails
	 */
	struct voltage_rail_table_3x_hal_voltage_rail_logical logical;

	/*!
	 * Type specific data of physical rails
	 */
	struct voltage_rail_table_3x_hal_voltage_rail_physical physical;
};

struct voltage_rail_table_3x_hal_voltage_rail {
	/*!
	 *  Domain of the Voltage Rail
	 */
	u8 domain;

	/*!
	 *  Type of the Voltage Rail
	 */
	voltage_rail_table_3x_hal_voltage_rail_type volt_rail_type;

	/*!
	*  Type specific data
	*/
	union voltage_rail_table_3x_hal_voltage_rail_data volt_rail_data;
};

struct volt_table_3x_hal {
	/*!
	 * RM Volt HAL Enumeration value
	 */
	u8 rm_volt_hal;

	/*!
	 * Array of Voltage Rail Specific values.
	 */
	struct voltage_rail_table_3x_hal_voltage_rail rails[VBIOS_VOLT_TABLE_3X_HALS_NUM_VOLT_RAIL_ENTRIES];
};

/* ------------------------ Global Variables  ------------------------------ */
/*!
 * Table Voltage Rail Table 3.0 Voltage Rail HALs.
 */
static struct volt_table_3x_hal vbios_volt_3x_volt_rails_hals[VBIOS_VOLT_TABLE_3X_NUM_HALS] = {
	{
		.rm_volt_hal = CTRL_VOLT_DOMAIN_HAL_GP10X_SINGLE_RAIL,
		.rails = {
			{
				.domain = CTRL_VOLT_DOMAIN_LOGIC,
				.volt_rail_type = CTRL_VOLT_RAIL_TYPE_2X_PHYSICAL,
				.volt_rail_data.physical = {0}
			},
		},
	},
	{
		.rm_volt_hal = CTRL_VOLT_DOMAIN_HAL_GA10X_MULTI_RAIL,
		.rails = {
			{
				.domain = CTRL_VOLT_DOMAIN_LOGIC,
				.volt_rail_type = CTRL_VOLT_RAIL_TYPE_2X_PHYSICAL,
				.volt_rail_data.physical = {0}
			},
			{
				.domain = CTRL_VOLT_DOMAIN_MSVDD,
				.volt_rail_type = CTRL_VOLT_RAIL_TYPE_2X_PHYSICAL,
				.volt_rail_data.physical = {0}
			},
		},
	}
};

int volt_rail_state_init_3x(struct gk20a *g,
		struct voltage_rail_3x *pvolt_rail)
{
	int status = 0;
	u32 i;

	pvolt_rail->volt_dev_idx_default = CTRL_BOARDOBJ_IDX_INVALID;
	pvolt_rail->volt_dev_idx_ipc_vmin = CTRL_BOARDOBJ_IDX_INVALID;

	for (i = 0; i < CTRL_VOLT_RAIL_VOLT_DELTA_MAX_ENTRIES; i++) {
		pvolt_rail->volt_delta_uv[i] = (int)NV_PMU_VOLT_VALUE_0V_IN_UV;
		g->pmu->volt->volt_metadata->volt_rail_metadata.ext_rel_delta_uv[i] =
			NV_PMU_VOLT_VALUE_0V_IN_UV;
	}

	pvolt_rail->volt_margin_limit_vfe_equ_mon_handle =
		NV_PMU_PERF_RPC_VFE_EQU_MONITOR_COUNT_MAX;
	pvolt_rail->rel_limit_vfe_equ_mon_handle =
		NV_PMU_PERF_RPC_VFE_EQU_MONITOR_COUNT_MAX;
	pvolt_rail->alt_rel_limit_vfe_equ_mon_handle =
		NV_PMU_PERF_RPC_VFE_EQU_MONITOR_COUNT_MAX;
	pvolt_rail->ov_limit_vfe_equ_mon_handle =
		NV_PMU_PERF_RPC_VFE_EQU_MONITOR_COUNT_MAX;

	status = boardobjgrpmask_e32_init(&pvolt_rail->volt_dev_mask, NULL);
	if (status != 0) {
		nvgpu_err(g,
			"Failed to initialize BOARDOBJGRPMASK of VOLTAGE_DEVICEs");
	}

	return status;
}

static int volt_rail_init_pmudata_super_3x(struct gk20a *g,
	struct pmu_board_obj *obj, struct nv_pmu_boardobj *pmu_obj)
{
	int status = 0;
	struct voltage_rail_3x *prail;
	struct nv_pmu_volt_volt_rail_boardobj_set_2x *rail_pmu_data;
	struct nv_pmu_volt_volt_rail_2x_physical_boardobj_set *rail_pmu_data_physical;
	u32 i;

	nvgpu_log_info(g, " ");

	status = pmu_board_obj_pmu_data_init_super(g, obj, pmu_obj);
	if (status != 0) {
		return status;
	}

	prail = (struct voltage_rail_3x *)(void *)obj;
	rail_pmu_data_physical = (struct nv_pmu_volt_volt_rail_2x_physical_boardobj_set *)(void *)
		pmu_obj;
	rail_pmu_data = &rail_pmu_data_physical->super.super.super;

	rail_pmu_data->rel_limit_vfe_equ_idx = prail->rel_limit_vfe_equ_idx;
	rail_pmu_data->alt_rel_limit_vfe_equ_idx =
			prail->alt_rel_limit_vfe_equ_idx;
	rail_pmu_data->ov_limit_vfe_equ_idx = prail->ov_limit_vfe_equ_idx;
	rail_pmu_data->vmin_limit_vfe_equ_idx = prail->vmin_limit_vfe_equ_idx;
	rail_pmu_data->volt_margin_limit_vfe_equ_idx =
			prail->volt_margin_limit_vfe_equ_idx;
	rail_pmu_data->leakage_pwr_equ_idx = prail->leakage_pwr_equ_idx;
	rail_pmu_data->volt_dev_idx_default = prail->volt_dev_idx_default;
	rail_pmu_data->dynamic_pwr_equ_idx = prail->dynamic_pwr_equ_idx;
	rail_pmu_data->volt_dev_idx_ipc_vmin = prail->volt_dev_idx_ipc_vmin;
	rail_pmu_data->vbios_boot_voltage_uv = prail->vbios_boot_voltage_uv;
	rail_pmu_data->ba_scaling_pwr_eqn_idx = prail->ba_scaling_pwr_eqn_idx;
	rail_pmu_data->domain = prail->domain;
	rail_pmu_data->b_hw_clvc_en = prail->b_hw_clvc_en;

	for (i = 0; i < CTRL_VOLT_RAIL_VOLT_DELTA_MAX_ENTRIES; i++) {
		rail_pmu_data->volt_delta_uv[i] = prail->volt_delta_uv[i] +
			(int)g->pmu->volt->volt_metadata->volt_rail_metadata.ext_rel_delta_uv[i];
	}

	status = nvgpu_boardobjgrpmask_export(&prail->adc_dev_mask.super,
				prail->adc_dev_mask.super.bitcount,
				&rail_pmu_data->adc_dev_mask.super);
	if (status != 0) {
		nvgpu_err(g, "Failed to export BOARDOBJGRPMASK of ADC_DEVICEs");
		return status;
	}

	status = nvgpu_boardobjgrpmask_export(&prail->volt_dev_mask.super,
				prail->volt_dev_mask.super.bitcount,
				&rail_pmu_data->volt_dev_mask.super);
	if (status != 0) {
		nvgpu_err(g, "Failed to export BOARDOBJGRPMASK of VOLTAGE_DEVICEs");
		return status;
	}

	status = nvgpu_boardobjgrpmask_export(&prail->clk_domains_prog_mask.super,
				prail->clk_domains_prog_mask.super.bitcount,
				&rail_pmu_data->clk_domains_prog_mask.super);
	if (status != 0) {
		nvgpu_err(g, "Failed to export BOARDOBJGRPMASK of CLK_DOMAIN_PROGs");
		return status;
	}

	status = nvgpu_boardobjgrpmask_export(&prail->volt_device_mask.super,
				prail->volt_device_mask.super.bitcount,
				&rail_pmu_data_physical->volt_device_mask.super);
	if (status != 0) {
		nvgpu_err(g, "Failed to export BOARDOBJGRPMASK of VOLTAGE_DEVICEs");
		return status;
	}

	status = nvgpu_boardobjgrpmask_export(&prail->volt_threshold_controller_mask.super,
				prail->volt_threshold_controller_mask.super.bitcount,
				&rail_pmu_data_physical->volt_threshold_controller_mask.super);
	if (status != 0) {
		nvgpu_err(g, "Failed to export BOARDOBJGRPMASK of VOLT_THRESHOLD_CONTROLLERs");
		return status;
	}

	nvgpu_log_info(g, "Done");

	return status;
}

static struct voltage_rail_3x *volt_construct_volt_rail_3x(struct gk20a *g, void *pargs)
{
	struct pmu_board_obj *obj = NULL;
	struct voltage_rail_3x *ptemp_rail = (struct voltage_rail_3x *)pargs;
	struct voltage_rail_3x *board_obj_volt_rail_ptr = NULL;
	int status;

	nvgpu_log_info(g, " ");

	board_obj_volt_rail_ptr = nvgpu_kzalloc(g, sizeof(struct voltage_rail_3x));
	if (board_obj_volt_rail_ptr == NULL) {
		return NULL;
	}

	status = pmu_board_obj_construct_super(g,
			(struct pmu_board_obj *)(void *)board_obj_volt_rail_ptr,
			pargs);
	if (status != 0) {
		nvgpu_err(g, "Failed to construct PMU_BOARD_OBJ object");
		return NULL;
	}

	obj = (struct pmu_board_obj *)(void *)board_obj_volt_rail_ptr;
	/* override super class interface */
	obj->pmudatainit = volt_rail_init_pmudata_super_3x;

	board_obj_volt_rail_ptr->boot_voltage_uv =
			ptemp_rail->boot_voltage_uv;
	board_obj_volt_rail_ptr->rel_limit_vfe_equ_idx =
			ptemp_rail->rel_limit_vfe_equ_idx;
	board_obj_volt_rail_ptr->alt_rel_limit_vfe_equ_idx =
			ptemp_rail->alt_rel_limit_vfe_equ_idx;
	board_obj_volt_rail_ptr->ov_limit_vfe_equ_idx =
			ptemp_rail->ov_limit_vfe_equ_idx;
	board_obj_volt_rail_ptr->leakage_pwr_equ_idx =
			ptemp_rail->leakage_pwr_equ_idx;
	board_obj_volt_rail_ptr->boot_volt_vfe_equ_idx =
			ptemp_rail->boot_volt_vfe_equ_idx;
	board_obj_volt_rail_ptr->boot_volt_vfield_id =
			ptemp_rail->boot_volt_vfield_id;
	board_obj_volt_rail_ptr->vmin_limit_vfe_equ_idx =
			ptemp_rail->vmin_limit_vfe_equ_idx;
	board_obj_volt_rail_ptr->volt_margin_limit_vfe_equ_idx =
			ptemp_rail->volt_margin_limit_vfe_equ_idx;
	board_obj_volt_rail_ptr->dynamic_pwr_equ_idx =
			ptemp_rail->dynamic_pwr_equ_idx;
	board_obj_volt_rail_ptr->ba_scaling_pwr_eqn_idx =
			ptemp_rail->ba_scaling_pwr_eqn_idx;
	board_obj_volt_rail_ptr->domain =
			ptemp_rail->domain;

	status = boardobjgrpmask_e32_init(&board_obj_volt_rail_ptr->adc_dev_mask, NULL);
	if (status != 0) {
		nvgpu_err(g, "Failed to initialize BOARDOBJGRPMASK of ADC_DEVICEs");
		return NULL;
	}
	status = nvgpu_boardobjgrpmask_copy(&board_obj_volt_rail_ptr->adc_dev_mask.super, &ptemp_rail->adc_dev_mask.super);
	if (status != 0) {
		nvgpu_err(g, "Failed to copy BOARDOBJGRPMASK of ADC_DEVICEs");
		return NULL;
	}

	status = boardobjgrpmask_e32_init(&board_obj_volt_rail_ptr->clk_domains_prog_mask, NULL);
	if (status != 0) {
		nvgpu_err(g, "Failed to initialize BOARDOBJGRPMASK of CLK_DOMAIN_PROGs");
		return NULL;
	}

	board_obj_volt_rail_ptr->logical_rail_idx = ptemp_rail->logical_rail_idx;
	board_obj_volt_rail_ptr->boot_volt_fuse_id = ptemp_rail->boot_volt_fuse_id;

	status = boardobjgrpmask_e32_init(&board_obj_volt_rail_ptr->volt_device_mask, NULL);
	if (status != 0) {
		nvgpu_err(g, "Failed to initialize BOARDOBJGRPMASK of VOLTAGE_DEVICEs");
		return NULL;
	}
	status = nvgpu_boardobjgrpmask_copy(&board_obj_volt_rail_ptr->volt_device_mask.super, &ptemp_rail->volt_device_mask.super);
	if (status != 0) {
		nvgpu_err(g, "Failed to copy BOARDOBJGRPMASK of VOLTAGE_DEVICEs");
		return NULL;
	}

	status = boardobjgrpmask_e32_init(&board_obj_volt_rail_ptr->volt_threshold_controller_mask, NULL);
	if (status != 0) {
		nvgpu_err(g, "Failed to initialize BOARDOBJGRPMASK of VOLT_THRESHOLD_CONTROLLERs");
		return NULL;
	}
	status = nvgpu_boardobjgrpmask_copy(&board_obj_volt_rail_ptr->volt_threshold_controller_mask.super, &ptemp_rail->volt_threshold_controller_mask.super);
	if (status != 0) {
		nvgpu_err(g, "Failed to copy BOARDOBJGRPMASK of VOLT_THRESHOLD_CONTROLLERs");
		return NULL;
	}

	nvgpu_log_info(g, "Done");

	return (struct voltage_rail_3x *)(void *)obj;
}

/*
 * Converts VBIOS voltage rail type to internal voltage rail type
 */
static u8 volt_rail_vbios_volt_rail_type_convert_to_internal_3x(struct gk20a *g,
	u8 vbios_volt_rail_type)
{
	switch (vbios_volt_rail_type) {
	case VBIOS_VOLT_TABLE_3X_HAL_VOLT_RAIL_TYPE_LOGICAL:
		return CTRL_VOLT_RAIL_TYPE_2X_LOGICAL;
	case VBIOS_VOLT_TABLE_3X_HAL_VOLT_RAIL_TYPE_PHYSICAL:
		return CTRL_VOLT_RAIL_TYPE_2X_PHYSICAL;
	}

	return CTRL_VOLT_RAIL_TYPE_INVALID;
}

static u8 volt_rail_volt_domain_hal_convert_to_internal_3x(struct gk20a *g,
	u8 vbios_volt_domain_hal)
{
	switch (vbios_volt_domain_hal) {
	case VBIOS_VOLTAGE_RAIL_3X_VOLT_DOMAIN_HAL_GP10X_SINGLE_RAIL:
		return CTRL_VOLT_DOMAIN_HAL_GP10X_SINGLE_RAIL;

	case VBIOS_VOLTAGE_RAIL_3X_VOLT_DOMAIN_HAL_GA10X_MULTI_RAIL:
		return CTRL_VOLT_DOMAIN_HAL_GA10X_MULTI_RAIL;
	}

	return CTRL_VOLT_DOMAIN_HAL_INVALID;
}

static int volt_rail_table_hal_translate_3x(struct gk20a *g,
	u8 volt_rail_hal, struct volt_table_3x_hal **volt_rail_hal_ptr)
{
	if ((volt_rail_hal >= VBIOS_VOLT_TABLE_3X_NUM_HALS) ||
	    (vbios_volt_3x_volt_rails_hals[volt_rail_hal].rm_volt_hal ==
	     CTRL_VOLT_DOMAIN_HAL_INVALID)) {
		nvgpu_err(g, "Unrecognized Voltage Rail HAL - voltRailHAL=0x%02x", volt_rail_hal);
		return -EINVAL;
	}

	*volt_rail_hal_ptr = &vbios_volt_3x_volt_rails_hals[volt_rail_hal];
	return 0;
}

int volt_get_volt_rail_table_3x(struct gk20a *g,
		struct voltage_rail_metadata *pvolt_rail_metadata)
{
	int status = 0;
	u8 *volt_rail_table_ptr = NULL;
	struct voltage_rail_3x *prail = NULL;
	struct vbios_voltage_rail_table_1x_header header = { 0 };
	struct vbios_voltage_rail_table_3x_entry entry = { 0 };
	u8 i;
	u8 volt_domain;
	u8 vbios_volt_domain_hal;
	u8 vbios_volt_rail_type;
	u8 volt_rail_type;
	u8 *entry_ptr;
	struct volt_table_3x_hal *volt_rail_hal_ptr = NULL;
	union rail_type {
		struct pmu_board_obj obj;
		struct voltage_rail_3x volt_rail;
	} rail_type_data;

	volt_rail_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			nvgpu_bios_get_bit_token(g, NVGPU_BIOS_PERF_TOKEN),
			VOLTAGE_RAIL_TABLE);
	if (volt_rail_table_ptr == NULL) {
		status = -EINVAL;
		goto done;
	}

	// Header format is same for both 1.0 and 3.0
	nvgpu_memcpy((u8 *)&header, volt_rail_table_ptr,
			sizeof(struct vbios_voltage_rail_table_1x_header));

	if (header.version != CTRL_VOLT_RAIL_VERSION_30) {
		nvgpu_err(g, "Unsupported volt rail header version: %d", header.version);
		status = -EINVAL;
		goto done;
	}

	pvolt_rail_metadata->version = header.version;
	vbios_volt_domain_hal = (u8)header.volt_domain_hal;

	// Initialize global state from the Voltage Rail Table HAL
	status = volt_rail_table_hal_translate_3x(g, vbios_volt_domain_hal, &volt_rail_hal_ptr);
	if (status != 0) {
		goto done;
	}

	pvolt_rail_metadata->volt_domain_hal = volt_rail_volt_domain_hal_convert_to_internal_3x(g,
						vbios_volt_domain_hal);
	if (pvolt_rail_metadata->volt_domain_hal == CTRL_VOLT_DOMAIN_HAL_INVALID) {
		nvgpu_err(g, "Invalid VBIOS Voltage Domain HAL - volt_domain_hal=0x%02x", vbios_volt_domain_hal);
		status = -EINVAL;
		goto done;
	}

	status = boardobjgrpmask_e32_init(&pvolt_rail_metadata->volt_rails_physical_rail_mask, NULL);
	if (status != 0) {
		nvgpu_err(g, "Failed to initialize BOARDOBJGRPMASK of Physical Rail Mask");
		goto done;
	}

	status = boardobjgrpmask_e32_init(&pvolt_rail_metadata->volt_rails_logical_rail_mask, NULL);
	if (status != 0) {
		nvgpu_err(g, "Failed to initialize BOARDOBJGRPMASK of Logical Rail Mask");
		goto done;
	}

	for (i = 0; i < header.num_table_entries; i++) {
		entry_ptr = (volt_rail_table_ptr + header.header_size +
			(i * header.table_entry_size));

		(void) memset(&rail_type_data, 0x0, sizeof(rail_type_data));

		nvgpu_memcpy((u8 *)&entry, entry_ptr,
			sizeof(struct vbios_voltage_rail_table_3x_entry));

		// Obtain voltage domain for the VOLTAGE_RAIL object
		volt_domain = volt_rail_vbios_volt_domain_convert_to_internal(g, i);
		if (volt_domain == CTRL_VOLT_DOMAIN_INVALID) {
			nvgpu_err(g, "Invalid VBIOS Voltage Domain");
			status = -EINVAL;
			goto done;
		}

		vbios_volt_rail_type = BIOS_GET_FIELD(u8, entry.flags,
					NV_VBIOS_VOLT_RAIL_TABLE_3X_ENTRY_FLAGS_TYPE);
		volt_rail_type = volt_rail_vbios_volt_rail_type_convert_to_internal_3x(g,
			vbios_volt_rail_type);
		if (volt_rail_type == CTRL_VOLT_RAIL_TYPE_INVALID) {
			nvgpu_err(g, "Invalid VBIOS Voltage Rail Type");
			status = -EINVAL;
			goto done;
		}

		if (volt_rail_type == CTRL_VOLT_RAIL_TYPE_2X_PHYSICAL) {
			nvgpu_boardobjgrpmask_bit_set(
				&pvolt_rail_metadata->volt_rails_physical_rail_mask.super, i);
		} else if (volt_rail_type == CTRL_VOLT_RAIL_TYPE_2X_LOGICAL) {
			nvgpu_boardobjgrpmask_bit_set(
				&pvolt_rail_metadata->volt_rails_logical_rail_mask.super, i);
		}

		/*
		 * RM says "Remove this once VBIOS changes for boot voltage
		 * fuseId propoagate to TOT (currently GB102 breaks w/ this)"
		 * To Do: Check if this is required for GB20C or not
		 */
		rail_type_data.volt_rail.boot_volt_vfield_id = 0xFFU;

		/*
		 * In VBIOS volt rail table version 3.0, volt_rail_type is
		 * physical, so added code to support only that functionality
		 */
		if (volt_rail_type != CTRL_VOLT_RAIL_TYPE_2X_PHYSICAL) {
			nvgpu_err(g, "Invalid VBIOS Voltage Rail Type - vbios_volt_rail_type=0x%02x", vbios_volt_rail_type);
			status = -EINVAL;
			goto done;
		}

		if (volt_rail_hal_ptr->rails[i].volt_rail_type != CTRL_VOLT_RAIL_TYPE_2X_PHYSICAL) {
			nvgpu_err(g, "volt_rail_hal_ptr.volt_rail_type doesn't match VBIOS Voltage Rail Type");
			status = -EINVAL;
			goto done;
		}
		rail_type_data.obj.type = volt_rail_type;

		if (volt_rail_hal_ptr->rails[i].domain != volt_domain) {
			nvgpu_err(g, "volt_rail_hal_ptr.domain doesn't match VBIOS Voltage Domain");
			status = -EINVAL;
			goto done;
		}
		rail_type_data.volt_rail.domain = volt_domain;

		status = boardobjgrpmask_e32_init(&rail_type_data.volt_rail.volt_device_mask, NULL);
		if (status != 0) {
			nvgpu_err(g, "Failed to initialize BOARDOBJGRPMASK for voltage device mask");
			goto done;
		}
		rail_type_data.volt_rail.volt_device_mask.super.data[0] = (u32)entry.param0;

		status = boardobjgrpmask_e32_init(&rail_type_data.volt_rail.volt_threshold_controller_mask, NULL);
		if (status != 0) {
			nvgpu_err(g, "Failed to initialize BOARDOBJGRPMASK for volt threshold controller mask");
			goto done;
		}
		rail_type_data.volt_rail.volt_threshold_controller_mask.super.data[0] = (u32)entry.param1;

		rail_type_data.volt_rail.logical_rail_idx = volt_rail_hal_ptr->rails[i].volt_rail_data.physical.volt_rail_logical_idx;

		status = boardobjgrpmask_e32_init(&rail_type_data.volt_rail.adc_dev_mask, NULL);
		if (status != 0) {
			nvgpu_err(g, "Failed to initialize BOARDOBJGRPMASK for adc device mask");
			goto done;
		}
		rail_type_data.volt_rail.adc_dev_mask.super.data[0] = (u32)entry.param2;

		rail_type_data.volt_rail.rel_limit_vfe_equ_idx =
			BIOS_GET_FIELD(u16, entry.param3,
			NV_VBIOS_VOLT_RAIL_TABLE_3X_ENTRY_PARAM3_REL_LIMIT_VFE_EQU_IDX);

		rail_type_data.volt_rail.alt_rel_limit_vfe_equ_idx =
			BIOS_GET_FIELD(u16, entry.param3,
			NV_VBIOS_VOLT_RAIL_TABLE_3X_ENTRY_PARAM3_ALT_REL_LIMIT_VFE_EQU_IDX);

		rail_type_data.volt_rail.ov_limit_vfe_equ_idx =
			BIOS_GET_FIELD(u16, entry.param4,
			NV_VBIOS_VOLT_RAIL_TABLE_3X_ENTRY_PARAM4_OV_LIMIT_VFE_EQU_IDX);

		rail_type_data.volt_rail.vmin_limit_vfe_equ_idx =
			BIOS_GET_FIELD(u16, entry.param4,
			NV_VBIOS_VOLT_RAIL_TABLE_3X_ENTRY_PARAM4_VMIN_LIMIT_VFE_EQU_IDX);

		rail_type_data.volt_rail.ba_scaling_pwr_eqn_idx =
			BIOS_GET_FIELD(u16, entry.param5,
			NV_VBIOS_VOLT_RAIL_TABLE_3X_ENTRY_PARAM5_BA_SCALING_PWR_EQN_IDX);

		rail_type_data.volt_rail.leakage_pwr_equ_idx =
			BIOS_GET_FIELD(u16, entry.param5,
			NV_VBIOS_VOLT_RAIL_TABLE_3X_ENTRY_PARAM5_LEAKAGE_PWR_EQU_IDX);

		rail_type_data.volt_rail.boot_voltage_uv =
			BIOS_GET_FIELD(u32, entry.param6,
			NV_VBIOS_VOLT_RAIL_TABLE_3X_ENTRY_PARAM6_BOOT_VOLTAGE_UV);

		rail_type_data.volt_rail.boot_volt_vfe_equ_idx =
			BIOS_GET_FIELD(u16, entry.param8,
			NV_VBIOS_VOLT_RAIL_TABLE_3X_ENTRY_PARAM8_BOOT_VOLT_VFE_EQU_IDX);

		rail_type_data.volt_rail.volt_margin_limit_vfe_equ_idx =
			BIOS_GET_FIELD(u16, entry.param8,
			NV_VBIOS_VOLT_RAIL_TABLE_3X_ENTRY_PARAM8_VOLT_MARGIN_LIMIT_VFE_EQU_IDX);

		rail_type_data.volt_rail.boot_volt_fuse_id =
			BIOS_GET_FIELD(u8, entry.param9,
			NV_VBIOS_VOLT_RAIL_TABLE_3X_ENTRY_PARAM9_BOOT_VOLT_FUSE_ID);

		rail_type_data.volt_rail.dynamic_pwr_equ_idx =
			BIOS_GET_FIELD(u16, entry.param9,
			NV_VBIOS_VOLT_RAIL_TABLE_3X_ENTRY_PARAM9_DYNAMIC_PWR_EQU_IDX);

		prail = volt_construct_volt_rail_3x(g, &rail_type_data);
		if (prail == NULL) {
			nvgpu_err(g, "Failed to construct VOLT_RAIL object");
			status = -ENOMEM;
			goto done;
		}

		status = boardobjgrp_objinsert(
				&pvolt_rail_metadata->volt_rails.super,
				(void *)(struct pmu_board_obj *)prail, i);
		if (status != 0) {
			nvgpu_err(g, "Failed to insert VOLT_RAIL object");
			goto done;
		}
	}

done:
	return status;
}

int volt_rail_obj_update_3x(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj)
{
	struct voltage_rail_3x *volt_rail_obj;
	struct nv_pmu_volt_volt_rail_boardobj_get_status *pstatus;

	nvgpu_log_info(g, " ");

	volt_rail_obj = (struct voltage_rail_3x *)(void *)obj;
	pstatus = (struct nv_pmu_volt_volt_rail_boardobj_get_status *)
		(void *)pmu_obj;

	if (pstatus->super.type != volt_rail_obj->super.type) {
		nvgpu_err(g, "pmu data and boardobj type not matching");
		return -EINVAL;
	}

	/* Updating only vmin as per requirement, later other fields can be added */
	volt_rail_obj->vmin_limitu_v = pstatus->vmin_limitu_v;
	volt_rail_obj->max_limitu_v = pstatus->max_limitu_v;
	volt_rail_obj->current_volt_uv = pstatus->curr_volt_defaultu_v;

	return 0;
}

int volt_rail_volt_dev_register_3x(struct gk20a *g, struct voltage_rail_3x
	*pvolt_rail, u8 volt_dev_idx, u8 operation_type)
{
	int status = 0;

	if (operation_type == CTRL_VOLT_DEVICE_OPERATION_TYPE_DEFAULT) {
		if (pvolt_rail->volt_dev_idx_default ==
				CTRL_BOARDOBJ_IDX_INVALID) {
			pvolt_rail->volt_dev_idx_default = volt_dev_idx;
		} else {
			status = -EINVAL;
			goto exit;
		}
	} else if (operation_type ==
		CTRL_VOLT_VOLT_DEVICE_OPERATION_TYPE_IPC_VMIN) {
		if (pvolt_rail->volt_dev_idx_ipc_vmin ==
			CTRL_BOARDOBJ_IDX_INVALID) {
			pvolt_rail->volt_dev_idx_ipc_vmin = volt_dev_idx;
			/*
			 * Exit on purpose as we do not want to register
			 * IPC_VMIN device against the rail to avoid
			 * setting current voltage instead of
			 * IPC Vmin voltage.
			 */
			goto exit;
		} else {
			status = -EINVAL;
			goto exit;
		}
	} else {
		goto exit;
	}

	status = nvgpu_boardobjgrpmask_bit_set(&pvolt_rail->volt_dev_mask.super,
			volt_dev_idx);

exit:
	return status;
}

int volt_rail_devgrp_pmudata_instget_3x(struct gk20a *g,
	struct nv_pmu_boardobjgrp *pmuboardobjgrp, struct nv_pmu_boardobj
	**pmu_obj, u8 idx)
{
	struct nv_pmu_volt_volt_rail_3x_boardobj_grp_set *pgrp_set =
		(struct nv_pmu_volt_volt_rail_3x_boardobj_grp_set *)
		pmuboardobjgrp;

	nvgpu_log_info(g, " ");

	/* check whether pmuboardobjgrp has a valid boardobj in index */
	if (((u32)BIT(idx) &
		pgrp_set->hdr.data.super.obj_mask.super.data[0]) == 0U) {
		return -EINVAL;
	}

	*pmu_obj = (struct nv_pmu_boardobj *) &pgrp_set->objects[idx].data.obj;
	nvgpu_log_info(g, " Done");

	return 0;
}

int volt_rail_devgrp_pmu_hdr_data_init_3x(struct gk20a *g,
	struct boardobjgrp *pboardobjgrp,
	struct nv_pmu_boardobjgrp_super *pboardobjgrppmu,
	struct boardobjgrpmask *mask)
{
	int status;
	struct nv_pmu_volt_volt_rail_3x_boardobj_grp_set *pgrp_set =
		(struct nv_pmu_volt_volt_rail_3x_boardobj_grp_set *)
		pboardobjgrppmu;
	struct voltage_rail_metadata *pvolt_rail_metadata =
		&g->pmu->volt->volt_metadata->volt_rail_metadata;

	status = boardobjgrp_pmu_hdr_data_init_e32(g, pboardobjgrp, pboardobjgrppmu, mask);
	if (status != 0) {
		nvgpu_err(g, "Failed to initialize PMU header data for VOLT_RAIL");
		return status;
	}

	pgrp_set->hdr.data.volt_domain_hal = pvolt_rail_metadata->volt_domain_hal;

	status = nvgpu_boardobjgrpmask_export(
			&pvolt_rail_metadata->volt_rails_physical_rail_mask.super,
			pvolt_rail_metadata->volt_rails_physical_rail_mask.super.bitcount,
			&pgrp_set->hdr.data.volt_rails_physical_rail_mask.super);
	if (status != 0) {
		nvgpu_err(g, "Failed to export BOARDOBJGRPMASK of Physical Rail Mask");
		return status;
	}

	status = nvgpu_boardobjgrpmask_export(
			&pvolt_rail_metadata->volt_rails_logical_rail_mask.super,
			pvolt_rail_metadata->volt_rails_logical_rail_mask.super.bitcount,
			&pgrp_set->hdr.data.volt_rails_logical_rail_mask.super);
	if (status != 0) {
		nvgpu_err(g, "Failed to export BOARDOBJGRPMASK of Logical Rail Mask");
		return status;
	}

	return 0;
}
