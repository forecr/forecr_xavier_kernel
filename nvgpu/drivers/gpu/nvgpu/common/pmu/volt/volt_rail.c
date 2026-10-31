// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

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

#define NV_PMU_PERF_RPC_VFE_EQU_MONITOR_COUNT_MAX                            16U

static int volt_rail_state_init(struct gk20a *g,
		struct voltage_rail *pvolt_rail)
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

static int volt_rail_init_pmudata_super(struct gk20a *g,
	struct pmu_board_obj *obj, struct nv_pmu_boardobj *pmu_obj)
{
	int status = 0;
	struct voltage_rail *prail;
	struct nv_pmu_volt_volt_rail_boardobj_set *rail_pmu_data;
	u32 i;

	nvgpu_log_info(g, " ");

	status = pmu_board_obj_pmu_data_init_super(g, obj, pmu_obj);
	if (status != 0) {
		return status;
	}

	prail = (struct voltage_rail *)(void *)obj;
	rail_pmu_data = (struct nv_pmu_volt_volt_rail_boardobj_set *)(void *)
		pmu_obj;

	rail_pmu_data->rel_limit_vfe_equ_idx = prail->rel_limit_vfe_equ_idx;
	rail_pmu_data->alt_rel_limit_vfe_equ_idx =
			prail->alt_rel_limit_vfe_equ_idx;
	rail_pmu_data->ov_limit_vfe_equ_idx = prail->ov_limit_vfe_equ_idx;
	rail_pmu_data->vmin_limit_vfe_equ_idx = prail->vmin_limit_vfe_equ_idx;
	rail_pmu_data->volt_margin_limit_vfe_equ_idx =
			prail->volt_margin_limit_vfe_equ_idx;
	rail_pmu_data->pwr_equ_idx = prail->pwr_equ_idx;
	rail_pmu_data->volt_dev_idx_default = prail->volt_dev_idx_default;
	rail_pmu_data->volt_scale_exp_pwr_equ_idx =
			prail->volt_scale_exp_pwr_equ_idx;
	rail_pmu_data->volt_dev_idx_ipc_vmin = prail->volt_dev_idx_ipc_vmin;

	for (i = 0; i < CTRL_VOLT_RAIL_VOLT_DELTA_MAX_ENTRIES; i++) {
		rail_pmu_data->volt_delta_uv[i] = prail->volt_delta_uv[i] +
			(int)g->pmu->volt->volt_metadata->volt_rail_metadata.ext_rel_delta_uv[i];
	}

	status = nvgpu_boardobjgrpmask_export(&prail->volt_dev_mask.super,
				prail->volt_dev_mask.super.bitcount,
				&rail_pmu_data->volt_dev_mask.super);
	if (status != 0) {
		nvgpu_err(g,
			"Failed to export BOARDOBJGRPMASK of VOLTAGE_DEVICEs");
	}

	nvgpu_log_info(g, "Done");

	return status;
}

static struct voltage_rail *volt_construct_volt_rail(struct gk20a *g, void *pargs)
{
	struct pmu_board_obj *obj = NULL;
	struct voltage_rail *ptemp_rail = (struct voltage_rail *)pargs;
	struct voltage_rail  *board_obj_volt_rail_ptr = NULL;
	int status;

	nvgpu_log_info(g, " ");

	board_obj_volt_rail_ptr = nvgpu_kzalloc(g, sizeof(struct voltage_rail));
	if (board_obj_volt_rail_ptr == NULL) {
		return NULL;
	}

	status = pmu_board_obj_construct_super(g,
			(struct pmu_board_obj *)(void *)board_obj_volt_rail_ptr,
			pargs);
	if (status != 0) {
		return NULL;
	}

	obj = (struct pmu_board_obj *)(void *)board_obj_volt_rail_ptr;
	/* override super class interface */
	obj->pmudatainit = volt_rail_init_pmudata_super;

	board_obj_volt_rail_ptr->boot_voltage_uv =
			ptemp_rail->boot_voltage_uv;
	board_obj_volt_rail_ptr->rel_limit_vfe_equ_idx =
			ptemp_rail->rel_limit_vfe_equ_idx;
	board_obj_volt_rail_ptr->alt_rel_limit_vfe_equ_idx =
			ptemp_rail->alt_rel_limit_vfe_equ_idx;
	board_obj_volt_rail_ptr->ov_limit_vfe_equ_idx =
			ptemp_rail->ov_limit_vfe_equ_idx;
	board_obj_volt_rail_ptr->pwr_equ_idx =
			ptemp_rail->pwr_equ_idx;
	board_obj_volt_rail_ptr->boot_volt_vfe_equ_idx =
			ptemp_rail->boot_volt_vfe_equ_idx;
	board_obj_volt_rail_ptr->vmin_limit_vfe_equ_idx =
			ptemp_rail->vmin_limit_vfe_equ_idx;
	board_obj_volt_rail_ptr->volt_margin_limit_vfe_equ_idx =
			ptemp_rail->volt_margin_limit_vfe_equ_idx;
	board_obj_volt_rail_ptr->volt_scale_exp_pwr_equ_idx =
			ptemp_rail->volt_scale_exp_pwr_equ_idx;

	nvgpu_log_info(g, "Done");

	return (struct voltage_rail *)(void *)obj;
}

static int volt_get_volt_rail_table_1x(struct gk20a *g,
		struct voltage_rail_metadata *pvolt_rail_metadata)
{
	int status = 0;
	u8 *volt_rail_table_ptr = NULL;
	struct voltage_rail *prail = NULL;
	struct vbios_voltage_rail_table_1x_header header = { 0 };
	struct vbios_voltage_rail_table_1x_entry entry = { 0 };
	u8 i;
	u8 volt_domain;
	u8 *entry_ptr;
	union rail_type {
		struct pmu_board_obj obj;
		struct voltage_rail volt_rail;
	} rail_type_data;

	volt_rail_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			nvgpu_bios_get_bit_token(g, NVGPU_BIOS_PERF_TOKEN),
			VOLTAGE_RAIL_TABLE);
	if (volt_rail_table_ptr == NULL) {
		status = -EINVAL;
		goto done;
	}

	nvgpu_memcpy((u8 *)&header, volt_rail_table_ptr,
			sizeof(struct vbios_voltage_rail_table_1x_header));

	pvolt_rail_metadata->version = header.version;
	pvolt_rail_metadata->volt_domain_hal = (u8)header.volt_domain_hal;

	for (i = 0; i < header.num_table_entries; i++) {
		entry_ptr = (volt_rail_table_ptr + header.header_size +
			(i * header.table_entry_size));

		(void) memset(&rail_type_data, 0x0, sizeof(rail_type_data));

		nvgpu_memcpy((u8 *)&entry, entry_ptr,
			sizeof(struct vbios_voltage_rail_table_1x_entry));

		volt_domain = volt_rail_vbios_volt_domain_convert_to_internal(g,
			i);
		if (volt_domain == CTRL_VOLT_DOMAIN_INVALID) {
			continue;
		}

		rail_type_data.obj.type = volt_domain;
		rail_type_data.volt_rail.boot_voltage_uv =
			(u32)entry.boot_voltage_uv;
		rail_type_data.volt_rail.rel_limit_vfe_equ_idx =
			(u8)entry.rel_limit_vfe_equ_idx;
		rail_type_data.volt_rail.alt_rel_limit_vfe_equ_idx =
			(u8)entry.alt_rel_limit_vfe_equidx;
		rail_type_data.volt_rail.ov_limit_vfe_equ_idx =
			(u8)entry.ov_limit_vfe_equ_idx;

		if (header.table_entry_size >=
			NV_VBIOS_VOLTAGE_RAIL_1X_ENTRY_SIZE_0C) {
			rail_type_data.volt_rail.volt_scale_exp_pwr_equ_idx =
				(u8)entry.volt_scale_exp_pwr_equ_idx;
		} else {
			rail_type_data.volt_rail.volt_scale_exp_pwr_equ_idx =
				CTRL_BOARDOBJ_IDX_INVALID;
		}

		if (header.table_entry_size >=
			NV_VBIOS_VOLTAGE_RAIL_1X_ENTRY_SIZE_0B) {
			rail_type_data.volt_rail.volt_margin_limit_vfe_equ_idx =
				(u8)entry.volt_margin_limit_vfe_equ_idx;
		} else {
			rail_type_data.volt_rail.volt_margin_limit_vfe_equ_idx =
				CTRL_BOARDOBJ_IDX_INVALID;
		}

		if (header.table_entry_size >=
			NV_VBIOS_VOLTAGE_RAIL_1X_ENTRY_SIZE_0A) {
			rail_type_data.volt_rail.vmin_limit_vfe_equ_idx =
				(u8)entry.vmin_limit_vfe_equ_idx;
		} else {
			rail_type_data.volt_rail.vmin_limit_vfe_equ_idx =
				CTRL_BOARDOBJ_IDX_INVALID;
		}

		if (header.table_entry_size >=
			NV_VBIOS_VOLTAGE_RAIL_1X_ENTRY_SIZE_09) {
			rail_type_data.volt_rail.boot_volt_vfe_equ_idx =
				(u8)entry.boot_volt_vfe_equ_idx;
		} else {
			rail_type_data.volt_rail.boot_volt_vfe_equ_idx =
				CTRL_BOARDOBJ_IDX_INVALID;
		}

		if (header.table_entry_size >=
			NV_VBIOS_VOLTAGE_RAIL_1X_ENTRY_SIZE_08) {
			rail_type_data.volt_rail.pwr_equ_idx =
				(u8)entry.pwr_equ_idx;
		} else {
			rail_type_data.volt_rail.pwr_equ_idx =
				CTRL_PMGR_PWR_EQUATION_INDEX_INVALID;
		}

		prail = volt_construct_volt_rail(g, &rail_type_data);

		status = boardobjgrp_objinsert(
				&pvolt_rail_metadata->volt_rails.super,
				(void *)(struct pmu_board_obj *)prail, i);
	}

done:
	return status;
}

static int volt_get_volt_rail_table(struct gk20a *g,
		struct voltage_rail_metadata *pvolt_rail_metadata)
{
	/*
	 * Can't use volt_rail_metadata.version here because it's not set yet.
	 * So, use version 3x for gb20c and gb20f and use version 1 for all other chips
	 */
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);
	if (ver == NVGPU_GPUID_GB20C || ver == NVGPU_GPUID_GB20C_NEXT) {
		return volt_get_volt_rail_table_3x(g, pvolt_rail_metadata);
	} else {
		return volt_get_volt_rail_table_1x(g, pvolt_rail_metadata);
	}
}

static int volt_rail_devgrp_pmudata_instget(struct gk20a *g,
	struct nv_pmu_boardobjgrp *pmuboardobjgrp, struct nv_pmu_boardobj
	**pmu_obj, u8 idx)
{
	struct nv_pmu_volt_volt_rail_boardobj_grp_set *pgrp_set =
		(struct nv_pmu_volt_volt_rail_boardobj_grp_set *)
		pmuboardobjgrp;

	nvgpu_log_info(g, " ");

	/*check whether pmuboardobjgrp has a valid boardobj in index*/
	if (((u32)BIT(idx) &
		pgrp_set->hdr.data.super.obj_mask.super.data[0]) == 0U) {
		return -EINVAL;
	}

	*pmu_obj = (struct nv_pmu_boardobj *)
		&pgrp_set->objects[idx].data.obj;
	nvgpu_log_info(g, " Done");
	return 0;
}

static int volt_rail_devgrp_pmustatus_instget(struct gk20a *g,
	void *pboardobjgrppmu, struct nv_pmu_boardobj_query
	**obj_pmu_status, u8 idx)
{
	struct nv_pmu_volt_volt_rail_boardobj_grp_get_status *pgrp_get_status =
		(struct nv_pmu_volt_volt_rail_boardobj_grp_get_status *)
		pboardobjgrppmu;

	(void)g;
	/*check whether pmuboardobjgrp has a valid boardobj in index*/
	if (((u32)BIT(idx) &
		pgrp_get_status->hdr.data.super.obj_mask.super.data[0]) == 0U) {
		return -EINVAL;
	}

	*obj_pmu_status = (struct nv_pmu_boardobj_query *)
			&pgrp_get_status->objects[idx].data.obj;
	return 0;
}

static int volt_rail_obj_update_1x(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj)
{
	struct voltage_rail *volt_rail_obj;
	struct nv_pmu_volt_volt_rail_boardobj_get_status *pstatus;

	nvgpu_log_info(g, " ");

	volt_rail_obj = (struct voltage_rail *)(void *)obj;
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

static int volt_rail_obj_update(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj)
{
	if (g->pmu->volt->volt_metadata->volt_rail_metadata.version == CTRL_VOLT_RAIL_VERSION_10) {
		return volt_rail_obj_update_1x(g, obj, pmu_obj);
	} else if (g->pmu->volt->volt_metadata->volt_rail_metadata.version == CTRL_VOLT_RAIL_VERSION_30) {
		return volt_rail_obj_update_3x(g, obj, pmu_obj);
	} else {
		nvgpu_err(g, "Unsupported volt rail version");
		return -EINVAL;
	}
}

static int volt_rail_boardobj_grp_get_status(struct gk20a *g)
{
	struct boardobjgrp *pboardobjgrp;
	struct boardobjgrpmask *pboardobjgrpmask;
	struct nv_pmu_boardobjgrp_super *pboardobjgrppmu;
	struct pmu_board_obj *obj = NULL;
	struct nv_pmu_boardobj_query *pboardobjpmustatus = NULL;
	int status;
	u8 index;

	nvgpu_log_info(g, " ");

	pboardobjgrp = &g->pmu->volt->volt_metadata->volt_rail_metadata.volt_rails.super;
	pboardobjgrpmask = &g->pmu->volt->volt_metadata->volt_rail_metadata.volt_rails.mask.super;
	status = pboardobjgrp->pmugetstatus(g, pboardobjgrp, pboardobjgrpmask);
	if (status != 0) {
		nvgpu_err(g, "err getting boardobjs from pmu");
		return status;
	}
	pboardobjgrppmu = pboardobjgrp->pmu.getstatus.buf;

	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj*, obj, index) {
		status = pboardobjgrp->pmustatusinstget(g,
				(struct nv_pmu_boardobjgrp *)(void *)pboardobjgrppmu,
				&pboardobjpmustatus, index);
		if (status != 0) {
			nvgpu_err(g, "could not get status object instance");
			return status;
		}
		status = volt_rail_obj_update(g, obj,
				(struct nv_pmu_boardobj *)(void *)pboardobjpmustatus);
		if (status != 0) {
			nvgpu_err(g, "could not update volt rail status");
			return status;
		}
	}
	return 0;
}

int volt_rail_sw_setup(struct gk20a *g)
{
	int status = 0;
	struct boardobjgrp *pboardobjgrp = NULL;
	struct voltage_rail *pvolt_rail;
	struct voltage_rail_3x *pvolt_rail_3x;
	u8 i;

	nvgpu_log_info(g, " ");

	status = nvgpu_boardobjgrp_construct_e32(g,
			&g->pmu->volt->volt_metadata->volt_rail_metadata.volt_rails);
	if (status != 0) {
		nvgpu_err(g,
			"error creating boardobjgrp for volt rail, "
			"status - 0x%x", status);
		goto done;
	}

	pboardobjgrp = &g->pmu->volt->volt_metadata->volt_rail_metadata.volt_rails.super;

	g->pmu->volt->volt_metadata->volt_rail_metadata.pct_delta =
			NV_PMU_VOLT_VALUE_0V_IN_UV;

	/* Obtain Voltage Rail Table from VBIOS */
	status = volt_get_volt_rail_table(g, &g->pmu->volt->volt_metadata->
			volt_rail_metadata);
	if (status != 0) {
		goto done;
	}

	/* Populate data for the VOLT_RAIL PMU interface */
	BOARDOBJGRP_PMU_CONSTRUCT(pboardobjgrp, VOLT, VOLT_RAIL);

	/* update calibration to fuse */
	if (g->pmu->volt->volt_metadata->volt_rail_metadata.version == CTRL_VOLT_RAIL_VERSION_10) {
		pboardobjgrp->pmudatainstget = volt_rail_devgrp_pmudata_instget;
		pboardobjgrp->pmustatusinstget = volt_rail_devgrp_pmustatus_instget;

		status = BOARDOBJGRP_PMU_CMD_GRP_SET_CONSTRUCT(g, pboardobjgrp,
				volt, VOLT, volt_rail, VOLT_RAIL);
		if (status != 0) {
			nvgpu_err(g,
				"error constructing PMU_BOARDOBJ_CMD_GRP_SET interface - 0x%x",
				status);
			goto done;
		}

		status = BOARDOBJGRP_PMU_CMD_GRP_GET_STATUS_CONSTRUCT(g,
			&g->pmu->volt->volt_metadata->volt_rail_metadata.volt_rails.super,
				volt, VOLT, volt_rail, VOLT_RAIL);
		if (status != 0) {
			nvgpu_err(g,
				"error constructing PMU_BOARDOBJ_CMD_GRP_SET interface - 0x%x",
				status);
			goto done;
		}

		BOARDOBJGRP_FOR_EACH(&(g->pmu->volt->volt_metadata->volt_rail_metadata.
				       volt_rails.super),
				     struct voltage_rail *, pvolt_rail, i) {
			status = volt_rail_state_init(g, pvolt_rail);
			if (status != 0) {
				nvgpu_err(g,
					"Failure while executing RAIL's state init railIdx = %d",
					i);
				goto done;
			}
		}
	} else if (g->pmu->volt->volt_metadata->volt_rail_metadata.version == CTRL_VOLT_RAIL_VERSION_30) {
		pboardobjgrp->pmudatainstget = volt_rail_devgrp_pmudata_instget_3x;
		pboardobjgrp->pmustatusinstget = volt_rail_devgrp_pmustatus_instget;
		pboardobjgrp->pmuhdrdatainit = volt_rail_devgrp_pmu_hdr_data_init_3x;

		status = BOARDOBJGRP_PMU_CMD_GRP_SET_CONSTRUCT(g, pboardobjgrp,
				volt, VOLT, volt_rail_3x, VOLT_RAIL);
		if (status != 0) {
			nvgpu_err(g,
				"error constructing PMU_BOARDOBJ_CMD_GRP_SET interface - 0x%x",
				status);
			goto done;
		}

		status = BOARDOBJGRP_PMU_CMD_GRP_GET_STATUS_CONSTRUCT(g,
			&g->pmu->volt->volt_metadata->volt_rail_metadata.volt_rails.super,
				volt, VOLT, volt_rail_3x, VOLT_RAIL);
		if (status != 0) {
			nvgpu_err(g,
				"error constructing PMU_BOARDOBJ_CMD_GRP_SET interface - 0x%x",
				status);
			goto done;
		}

		BOARDOBJGRP_FOR_EACH(&(g->pmu->volt->volt_metadata->volt_rail_metadata.
				       volt_rails.super),
				     struct voltage_rail_3x *, pvolt_rail_3x, i) {
			status = volt_rail_state_init_3x(g, pvolt_rail_3x);
			if (status != 0) {
				nvgpu_err(g,
					"Failure while executing RAIL's state init railIdx = %d",
					i);
				goto done;
			}
		}
	} else {
		nvgpu_err(g, "Unsupported volt rail version");
		status = -EINVAL;
		goto done;
	}

done:
	nvgpu_log_info(g, " done status %x", status);
	return status;
}

int volt_rail_pmu_setup(struct gk20a *g)
{
	int status;
	struct boardobjgrp *pboardobjgrp = NULL;

	nvgpu_log_info(g, " ");

	pboardobjgrp = &g->pmu->volt->volt_metadata->volt_rail_metadata.volt_rails.super;

	if (!pboardobjgrp->bconstructed) {
		return -EINVAL;
	}

	status = pboardobjgrp->pmuinithandle(g, pboardobjgrp);

	nvgpu_log_info(g, "Done");
	return status;
}

u8 volt_rail_vbios_volt_domain_convert_to_internal(struct gk20a *g,
	u8 vbios_volt_domain)
{
	switch (g->pmu->volt->volt_metadata->volt_rail_metadata.volt_domain_hal) {
	case CTRL_VOLT_DOMAIN_HAL_GP10X_SINGLE_RAIL:
		switch (vbios_volt_domain) {
		case 0:
			return CTRL_VOLT_DOMAIN_LOGIC;
		default:
			break;
		}
		break;
	case CTRL_VOLT_DOMAIN_HAL_GP10X_SPLIT_RAIL:
		switch (vbios_volt_domain) {
		case 0:
			return CTRL_VOLT_DOMAIN_LOGIC;
		case 1:
			return CTRL_VOLT_DOMAIN_SRAM;
		default:
			break;
		}
		break;
	case CTRL_VOLT_DOMAIN_HAL_GA10X_MULTI_RAIL:
		switch (vbios_volt_domain) {
		case 0:
			return CTRL_VOLT_DOMAIN_LOGIC;
		case 1:
			return CTRL_VOLT_DOMAIN_MSVDD;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return CTRL_VOLT_DOMAIN_INVALID;
}

static int volt_rail_volt_dev_register_1x(struct gk20a *g, struct voltage_rail
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

int volt_rail_volt_dev_register(struct gk20a *g, void *pvolt_rail,
	u8 volt_dev_idx, u8 operation_type)
{
	int status = 0;
	u8 version = g->pmu->volt->volt_metadata->volt_rail_metadata.version;

	if (version == CTRL_VOLT_RAIL_VERSION_10) {
		status = volt_rail_volt_dev_register_1x(g,
			(struct voltage_rail *)pvolt_rail,
			volt_dev_idx, operation_type);
	} else if (version == CTRL_VOLT_RAIL_VERSION_30) {
		status = volt_rail_volt_dev_register_3x(g,
			(struct voltage_rail_3x *)pvolt_rail,
			volt_dev_idx, operation_type);
	} else {
		status = -EINVAL;
	}

	if (status != 0) {
		nvgpu_err(g, "Failed to register VOLTAGE_DEVICE");
	}

	return status;
}

u8 nvgpu_pmu_volt_rail_volt_domain_convert_to_idx(struct gk20a *g, u8 volt_domain)
{
	switch (g->pmu->volt->volt_metadata->volt_rail_metadata.volt_domain_hal) {
	case CTRL_VOLT_DOMAIN_HAL_GP10X_SINGLE_RAIL:
		switch (volt_domain) {
		case CTRL_VOLT_DOMAIN_LOGIC:
			return 0;
		}
		break;

	case CTRL_VOLT_DOMAIN_HAL_GP10X_SPLIT_RAIL:
		switch (volt_domain) {
		case CTRL_VOLT_DOMAIN_LOGIC:
			return 0;
		case CTRL_VOLT_DOMAIN_SRAM:
			return 1;
		}
		break;

	case CTRL_VOLT_DOMAIN_HAL_GA10X_MULTI_RAIL:
		switch (volt_domain) {
		case CTRL_VOLT_DOMAIN_LOGIC:
			return 0;
		case CTRL_VOLT_DOMAIN_MSVDD:
			return 1;
		}
		break;
	}

	return CTRL_BOARDOBJ_IDX_INVALID;
}

int nvgpu_pmu_volt_get_vmin_vmax_ps35(struct gk20a *g, u32 *vmin_uv, u32 *vmax_uv)
{
	struct boardobjgrp *pboardobjgrp;
	struct pmu_board_obj *obj = NULL;
	struct voltage_rail *volt_rail = NULL;
	struct voltage_rail_3x *volt_rail_3x = NULL;
	int status;
	u8 index;

	status = volt_rail_boardobj_grp_get_status(g);
	if (status != 0) {
		nvgpu_err(g, "Vfe_var get status failed");
		return status;
	}

	pboardobjgrp = &g->pmu->volt->volt_metadata->volt_rail_metadata.volt_rails.super;

	/*
	 * Currently NVGPU driver only supports changing GPC clock, but we need to add
	 * support for NVD and SYS clocks. Since NVD and SYS clocks are on a different
	 * volt rail than GPC clocks, will likely need to have a rail ID parameter as
	 * an input to this function and return the vmin/vmax values from the appropriate
	 * volt_rail object.
	 * To Do: Add this support when the calling change_seq function is refactored.
	 */
	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj*, obj, index) {
		if (g->pmu->volt->volt_metadata->volt_rail_metadata.version == CTRL_VOLT_RAIL_VERSION_10) {
			volt_rail = (struct voltage_rail *)(void *)obj;
			if ((volt_rail->vmin_limitu_v != 0U) &&
				(volt_rail->max_limitu_v != 0U)) {
				*vmin_uv = volt_rail->vmin_limitu_v;
				*vmax_uv = volt_rail->max_limitu_v;
				return status;
			}
		} else if (g->pmu->volt->volt_metadata->volt_rail_metadata.version == CTRL_VOLT_RAIL_VERSION_30) {
			volt_rail_3x = (struct voltage_rail_3x *)(void *)obj;
			if ((volt_rail_3x->vmin_limitu_v != 0U) &&
				(volt_rail_3x->max_limitu_v != 0U)) {
				*vmin_uv = volt_rail_3x->vmin_limitu_v;
				*vmax_uv = volt_rail_3x->max_limitu_v;
				return status;
			}
		} else {
			nvgpu_err(g, "Unsupported volt rail version");
			return status;
		}
	}
	return status;
}

int nvgpu_pmu_volt_get_curr_volt_ps35(struct gk20a *g, u32 *vcurr_uv)
{
	struct boardobjgrp *pboardobjgrp;
	struct pmu_board_obj *obj = NULL;
	struct voltage_rail *volt_rail = NULL;
	struct voltage_rail_3x *volt_rail_3x = NULL;
	int status;
	u8 index;

	status = volt_rail_boardobj_grp_get_status(g);
	if (status != 0) {
		nvgpu_err(g, "volt rail get status failed");
		return status;
	}

	pboardobjgrp = &g->pmu->volt->volt_metadata->volt_rail_metadata.volt_rails.super;

	/*
	 * To Do: Return vcurr value from the appropriate volt rail object after
	 * adding support for NVD and SYS clocks.
	 */
	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj*, obj, index) {
		if (g->pmu->volt->volt_metadata->volt_rail_metadata.version == CTRL_VOLT_RAIL_VERSION_10) {
			volt_rail = (struct voltage_rail *)(void *)obj;
			if (volt_rail->current_volt_uv != 0U) {
				*vcurr_uv = volt_rail->current_volt_uv;
				return status;
			}
		} else if (g->pmu->volt->volt_metadata->volt_rail_metadata.version == CTRL_VOLT_RAIL_VERSION_30) {
			volt_rail_3x = (struct voltage_rail_3x *)(void *)obj;
			if (volt_rail_3x->current_volt_uv != 0U) {
				*vcurr_uv = volt_rail_3x->current_volt_uv;
				return status;
			}
		} else {
			nvgpu_err(g, "Unsupported volt rail version");
			return status;
		}
	}
	return status;
}

u8 nvgpu_pmu_volt_get_vmargin_ps35(struct gk20a *g)
{
	struct boardobjgrp *pboardobjgrp;
	struct pmu_board_obj *obj = NULL;
	struct voltage_rail *volt_rail = NULL;
	struct voltage_rail_3x *volt_rail_3x = NULL;
	u8 index, vmargin_uv;

	pboardobjgrp = &g->pmu->volt->volt_metadata->volt_rail_metadata.volt_rails.super;

	BOARDOBJGRP_FOR_EACH(pboardobjgrp, struct pmu_board_obj *, obj, index) {
		if (g->pmu->volt->volt_metadata->volt_rail_metadata.version == CTRL_VOLT_RAIL_VERSION_10) {
			volt_rail = (struct voltage_rail *)(void *)obj;
			if (volt_rail->volt_margin_limit_vfe_equ_idx != 255U) {
				vmargin_uv = volt_rail->volt_margin_limit_vfe_equ_idx;
				return vmargin_uv;
			}
		} else if (g->pmu->volt->volt_metadata->volt_rail_metadata.version == CTRL_VOLT_RAIL_VERSION_30) {
			volt_rail_3x = (struct voltage_rail_3x *)(void *)obj;
			if (volt_rail_3x->volt_margin_limit_vfe_equ_idx != 255U) {
				vmargin_uv = volt_rail_3x->volt_margin_limit_vfe_equ_idx;
				return vmargin_uv;
			}
		} else {
			nvgpu_err(g, "Unsupported volt rail version");
			return 0U;
		}
	}
	return 0U;
}

