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

#include "volt.h"
#include "ucode_volt_inf.h"
#include "volt_policy.h"

static int volt_policy_pmu_data_init_super(struct gk20a *g,
	struct pmu_board_obj *obj, struct nv_pmu_boardobj *pmu_obj)
{
	return pmu_board_obj_pmu_data_init_super(g, obj, pmu_obj);
}

static int volt_construct_volt_policy(struct gk20a *g,
	struct pmu_board_obj **obj, size_t size, void *pArgs)
{
	struct voltage_policy *pvolt_policy = NULL;
	int status = 0;

	pvolt_policy = nvgpu_kzalloc(g, size);
	if (pvolt_policy == NULL) {
		return -ENOMEM;
	}

	status = pmu_board_obj_construct_super(g,
			(struct pmu_board_obj *)(void *)pvolt_policy, pArgs);
	if (status != 0) {
		return -EINVAL;
	}

	*obj = (struct pmu_board_obj *)(void *)pvolt_policy;

	pvolt_policy->super.pmudatainit = volt_policy_pmu_data_init_super;

	return status;
}

static int volt_construct_volt_policy_single_rail(struct gk20a *g,
	struct pmu_board_obj **obj, size_t size, void *pArgs)
{
	struct voltage_policy_single_rail *ptmp_policy  =
			(struct voltage_policy_single_rail *)pArgs;
	struct voltage_policy_single_rail *pvolt_policy = NULL;
	int status = 0;

	status = volt_construct_volt_policy(g, obj, size, pArgs);
	if (status != 0) {
		return status;
	}

	pvolt_policy = (struct voltage_policy_single_rail *)(void *)*obj;

	pvolt_policy->rail_idx = ptmp_policy->rail_idx;

	return status;
}

static int volt_policy_pmu_data_init_single_rail(struct gk20a *g,
	struct pmu_board_obj *obj, struct nv_pmu_boardobj *pmu_obj)
{
	int status = 0;
	struct voltage_policy_single_rail *ppolicy;
	struct nv_pmu_volt_volt_policy_sr_boardobj_set *pset;

	status = volt_policy_pmu_data_init_super(g, obj, pmu_obj);
	if (status != 0) {
		goto done;
	}

	ppolicy = (struct voltage_policy_single_rail *)(void *)obj;
	pset = (struct nv_pmu_volt_volt_policy_sr_boardobj_set *)(void *)
				pmu_obj;
	pset->rail_idx = ppolicy->rail_idx;

done:
	return status;
}

static int volt_construct_volt_policy_single(struct gk20a *g,
	struct pmu_board_obj **obj, size_t size, void *pArgs)
{
	struct pmu_board_obj *obj_tmp = NULL;
	int status = 0;

	status = volt_construct_volt_policy_single_rail(g, obj, size, pArgs);
	if (status != 0x0) {
		return status;
	}

	obj_tmp = *obj;
	obj_tmp->pmudatainit = volt_policy_pmu_data_init_single_rail;

	return status;
}

static int volt_policy_pmu_data_init_sr_multi_step(struct gk20a *g,
	struct pmu_board_obj *obj, struct nv_pmu_boardobj *pmu_obj)
{
	int status = 0;
	struct voltage_policy_single_rail_multi_step *ppolicy;
	struct nv_pmu_volt_volt_policy_sr_multi_step_boardobj_set *pset;

	status = volt_policy_pmu_data_init_single_rail(g, obj, pmu_obj);
	if (status != 0) {
		goto done;
	}

	ppolicy = (struct voltage_policy_single_rail_multi_step *)(void *)obj;
	pset = (struct nv_pmu_volt_volt_policy_sr_multi_step_boardobj_set *)
				(void *)pmu_obj;

	pset->ramp_up_step_size_uv   = ppolicy->ramp_up_step_size_uv;
	pset->ramp_down_step_size_uv = ppolicy->ramp_down_step_size_uv;
	pset->inter_switch_delay_us = ppolicy->inter_switch_delay_us;

done:
	return status;
}

static int volt_construct_volt_policy_single_rail_multi_step(struct gk20a *g,
	struct pmu_board_obj **obj, size_t size, void *pargs)
{
	struct pmu_board_obj *obj_tmp = NULL;
	struct voltage_policy_single_rail_multi_step *p_volt_policy = NULL;
	struct voltage_policy_single_rail_multi_step *tmp_policy =
		(struct voltage_policy_single_rail_multi_step *)pargs;
	int status = 0;

	status = volt_construct_volt_policy_single_rail(g, obj, size, pargs);
	if (status != 0) {
		return status;
	}

	obj_tmp = (*obj);
	p_volt_policy = (struct voltage_policy_single_rail_multi_step *)
						*obj;

	obj_tmp->pmudatainit = volt_policy_pmu_data_init_sr_multi_step;

	p_volt_policy->ramp_up_step_size_uv =
		tmp_policy->ramp_up_step_size_uv;
	p_volt_policy->ramp_down_step_size_uv =
		tmp_policy->ramp_down_step_size_uv;
	p_volt_policy->inter_switch_delay_us =
		tmp_policy->inter_switch_delay_us;

	return status;
}

static int volt_policy_pmu_data_init_multi_rail(struct gk20a *g,
	struct pmu_board_obj *obj, struct nv_pmu_boardobj *pmu_obj)
{
	int status = 0;
	struct voltage_policy_multi_rail *ppolicy;
	struct nv_pmu_volt_volt_policy_multi_rail_boardobj_set *pset;

	status = volt_policy_pmu_data_init_super(g, obj, pmu_obj);
	if (status != 0) {
		goto done;
	}

	ppolicy = (struct voltage_policy_multi_rail *)(void *)obj;
	pset = (struct nv_pmu_volt_volt_policy_multi_rail_boardobj_set *)(void *)
				pmu_obj;

	status = nvgpu_boardobjgrpmask_export(&ppolicy->volt_rail_mask.super,
				ppolicy->volt_rail_mask.super.bitcount,
				&pset->volt_rail_mask.super);
	if (status != 0) {
		nvgpu_err(g, "Failed to export BOARDOBJGRPMASK of VOLTAGE_RAILs");
		goto done;
	}

done:
	return status;
}

static int volt_construct_volt_policy_multi_rail(struct gk20a *g,
	struct pmu_board_obj **obj, size_t size, void *pargs)
{
	struct pmu_board_obj *obj_tmp = NULL;
	struct voltage_policy_multi_rail *p_volt_policy = NULL;
	struct voltage_policy_multi_rail *tmp_policy =
		(struct voltage_policy_multi_rail *)pargs;
	int status = 0;

	status = volt_construct_volt_policy(g, obj, size, pargs);
	if (status != 0) {
		return status;
	}

	obj_tmp = *obj;
	obj_tmp->pmudatainit = volt_policy_pmu_data_init_multi_rail;

	// set the mask
	p_volt_policy = (struct voltage_policy_multi_rail *)*obj;
	status = boardobjgrpmask_e32_init(&p_volt_policy->volt_rail_mask, NULL);
	if (status != 0) {
		nvgpu_err(g, "Failed to initialize BOARDOBJGRPMASK for voltage rail mask");
		goto done;
	}

	status = nvgpu_boardobjgrpmask_copy(&p_volt_policy->volt_rail_mask.super, &tmp_policy->volt_rail_mask.super);
	if (status != 0) {
		nvgpu_err(g, "Failed to copy BOARDOBJGRPMASK for voltage rail mask");
		goto done;
	}

done:
	return status;
}

static struct voltage_policy *volt_volt_policy_construct(struct gk20a *g, void *pargs)
{
	struct pmu_board_obj *obj = NULL;
	int status = 0;

	switch (pmu_board_obj_get_type(pargs)) {
	case CTRL_VOLT_POLICY_TYPE_SINGLE_RAIL_MULTI_STEP:
		status = volt_construct_volt_policy_single_rail_multi_step(g,
			&obj, sizeof(struct voltage_policy_single_rail_multi_step),
			pargs);
		if (status != 0) {
			nvgpu_err(g,
				"Could not allocate memory for voltage_policy");
			obj = NULL;
		}
		break;
	case CTRL_VOLT_POLICY_TYPE_SINGLE_RAIL:
		status = volt_construct_volt_policy_single(g,
			&obj, sizeof(struct voltage_policy_single_rail), pargs);
		if (status != 0) {
			nvgpu_err(g,
				"Could not allocate memory for voltage_policy");
			obj = NULL;
		}
		break;
	case CTRL_VOLT_POLICY_TYPE_MULTI_RAIL:
		status = volt_construct_volt_policy_multi_rail(g,
			&obj, sizeof(struct voltage_policy_multi_rail), pargs);
		if (status != 0) {
			nvgpu_err(g, "Could not allocate memory for voltage_policy");
			obj = NULL;
		}
		break;
	default:
		nvgpu_err(g, "Invalid voltage policy type %d", pmu_board_obj_get_type(pargs));
		status = -EINVAL;
		break;
	}

	return (struct voltage_policy *)(void *)obj;
}

static u8 volt_policy_type_convert_1x(u8 vbios_type)
{
	switch (vbios_type) {
	case NV_VBIOS_VOLTAGE_POLICY_1X_ENTRY_TYPE_SINGLE_RAIL:
		return CTRL_VOLT_POLICY_TYPE_SINGLE_RAIL;
	case NV_VBIOS_VOLTAGE_POLICY_1X_ENTRY_TYPE_SINGLE_RAIL_MULTI_STEP:
		return CTRL_VOLT_POLICY_TYPE_SINGLE_RAIL_MULTI_STEP;
	}

	return CTRL_VOLT_POLICY_TYPE_INVALID;
}

static u8 volt_policy_type_convert_2x(u8 vbios_type)
{
	// Only multi rail is expected in gb20c VBIOS
	if (vbios_type == NV_VBIOS_VOLTAGE_POLICY_2X_ENTRY_TYPE_MULTI_RAIL) {
		return CTRL_VOLT_POLICY_TYPE_MULTI_RAIL;
	}

	return CTRL_VOLT_POLICY_TYPE_INVALID;
}

static int volt_get_volt_policy_table(struct gk20a *g,
		struct voltage_policy_metadata *pvolt_policy_metadata)
{
	int status = 0;
	u8 *voltage_policy_table_ptr = NULL;
	struct voltage_policy *ppolicy = NULL;
	struct vbios_voltage_policy_table_1x_header header = { 0 };
	struct vbios_voltage_policy_table_1x_entry entry  = { 0 };
	u8 i, j;
	u8 policy_type = 0;
	u8 *entry_offset;
	u32 volt_rail_mask = 0;
	union policy_type {
		struct pmu_board_obj obj;
		struct voltage_policy	volt_policy;
		struct voltage_policy_single_rail_multi_step single_rail_ms;
		struct voltage_policy_single_rail single_rail;
		struct voltage_policy_multi_rail multi_rail;
	} policy_type_data;

	voltage_policy_table_ptr =
		(u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			nvgpu_bios_get_bit_token(g, NVGPU_BIOS_PERF_TOKEN),
			VOLTAGE_POLICY_TABLE);
	if (voltage_policy_table_ptr == NULL) {
		status = -EINVAL;
		goto done;
	}

	nvgpu_memcpy((u8 *)&header, voltage_policy_table_ptr,
			sizeof(struct vbios_voltage_policy_table_1x_header));

	/* Set Voltage Policy Table Index for Perf Core VF Sequence client. */
	if (header.version == NV_VBIOS_VOLTAGE_POLICY_1X_VERSION) {
		pvolt_policy_metadata->perf_core_vf_seq_policy_idx =
			(u8)header.perf_core_vf_seq_policy_idx;
	} else if (header.version == NV_VBIOS_VOLTAGE_POLICY_2X_VERSION) {
		pvolt_policy_metadata->perf_core_vf_seq_policy_idx =
			(u8)NV_VBIOS_VOLTAGE_POLICY_2X_PERF_CHANGE_SEQ_POLICY_ID;
	} else {
		status = -EINVAL;
		nvgpu_err(g, "Invalid voltage policy table version %d", header.version);
		goto done;
	}

	/* Read in the entries. */
	for (i = 0; i < header.num_table_entries; i++) {
		entry_offset = (voltage_policy_table_ptr + header.header_size +
						i * header.table_entry_size);

		nvgpu_memcpy((u8 *)&entry, entry_offset,
			sizeof(struct vbios_voltage_policy_table_1x_entry));

		(void) memset(&policy_type_data, 0x0,
			sizeof(policy_type_data));

		if (header.version == NV_VBIOS_VOLTAGE_POLICY_1X_VERSION) {
			policy_type = volt_policy_type_convert_1x((u8)entry.type);
		} else {
			policy_type = volt_policy_type_convert_2x((u8)entry.type);
		}

		switch (policy_type) {
		case CTRL_VOLT_POLICY_TYPE_SINGLE_RAIL_MULTI_STEP:
			policy_type_data.single_rail_ms.inter_switch_delay_us =
				BIOS_GET_FIELD(u16, entry.param1,
				NV_VBIOS_VPT_ENTRY_PARAM1_SR_SETTLE_TIME_INTERMEDIATE);
			policy_type_data.single_rail_ms.ramp_up_step_size_uv =
				BIOS_GET_FIELD(u32, entry.param2,
				NV_VBIOS_VPT_ENTRY_PARAM2_SR_RAMP_UP_STEP_SIZE_UV);
			policy_type_data.single_rail_ms.ramp_down_step_size_uv =
				BIOS_GET_FIELD(u32, entry.param3,
				NV_VBIOS_VPT_ENTRY_PARAM3_SR_RAMP_DOWN_STEP_SIZE_UV);
			break;
		case CTRL_VOLT_POLICY_TYPE_SINGLE_RAIL:
			policy_type_data.single_rail.rail_idx =
				BIOS_GET_FIELD(u8, entry.param0,
				NV_VBIOS_VPT_ENTRY_PARAM0_SINGLE_RAIL_VOLT_DOMAIN);
			break;
		case CTRL_VOLT_POLICY_TYPE_MULTI_RAIL:
			status = boardobjgrpmask_e32_init(&policy_type_data.multi_rail.volt_rail_mask, NULL);
			if (status != 0) {
				nvgpu_err(g, "Failed to initialize BOARDOBJGRPMASK for voltage rail mask");
				goto done;
			}
			volt_rail_mask = (u32)entry.param0;
			for (j = 0; volt_rail_mask != 0U; j++, volt_rail_mask >>= 1U) {
				if ((BIT32(0) & volt_rail_mask) == 0U) {
					continue;
				}
				status = nvgpu_boardobjgrpmask_bit_set(&policy_type_data.multi_rail.volt_rail_mask.super, j);
				if (status != 0) {
					nvgpu_err(g, "Failed to set voltage rail mask bit %d", j);
					goto done;
				}
			}
			break;
		default:
			nvgpu_err(g, "Invalid voltage policy type %d", policy_type);
			status = -EINVAL;
			goto done;
		}

		policy_type_data.obj.type = policy_type;

		ppolicy = volt_volt_policy_construct(g,
				(void *)&policy_type_data);
		if (ppolicy == NULL) {
			nvgpu_err(g,
				"Failure to construct VOLT_POLICY object.");
			status = -EINVAL;
			goto done;
		}

		status = boardobjgrp_objinsert(
				&pvolt_policy_metadata->volt_policies.super,
				(struct pmu_board_obj *)ppolicy, i);
		if (status != 0) {
			nvgpu_err(g,
				"could not add volt_policy for entry %d into boardobjgrp ",
				i);
			goto done;
		}
	}

done:
	return status;
}
static int volt_policy_devgrp_pmudata_instget(struct gk20a *g,
	struct nv_pmu_boardobjgrp *pmuboardobjgrp,
	struct nv_pmu_boardobj **pmu_obj, u8 idx)
{
	struct nv_pmu_volt_volt_policy_boardobj_grp_set *pgrp_set =
		(struct nv_pmu_volt_volt_policy_boardobj_grp_set *)
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

static int volt_policy_grp_pmudatainit_super(struct gk20a *g,
	struct boardobjgrp *pboardobjgrp,
	struct nv_pmu_boardobjgrp_super *pboardobjgrppmu)
{
	struct nv_pmu_volt_volt_policy_boardobjgrp_set_header *pset =
		(struct nv_pmu_volt_volt_policy_boardobjgrp_set_header *)
		pboardobjgrppmu;
	struct voltage_policy_metadata *volt  =
			(struct voltage_policy_metadata *)pboardobjgrp;
	int status = 0;

	status = boardobjgrp_pmudatainit_e32(g, pboardobjgrp, pboardobjgrppmu);
	if (status != 0) {
		nvgpu_err(g,
			"error updating pmu boardobjgrp for volt policy 0x%x",
			 status);
		goto done;
	}
	pset->perf_core_vf_seq_policy_idx =
		volt->perf_core_vf_seq_policy_idx;

done:
	return status;
}

int volt_policy_pmu_setup(struct gk20a *g)
{
	int status;
	struct boardobjgrp *pboardobjgrp = NULL;

	nvgpu_log_info(g, " ");

	pboardobjgrp =
		&g->pmu->volt->volt_metadata->volt_policy_metadata.volt_policies.super;

	if (!pboardobjgrp->bconstructed) {
		return -EINVAL;
	}

	status = pboardobjgrp->pmuinithandle(g, pboardobjgrp);

	nvgpu_log_info(g, "Done");
	return status;
}

int volt_policy_sw_setup(struct gk20a *g)
{
	int status = 0;
	struct boardobjgrp *pboardobjgrp = NULL;

	nvgpu_log_info(g, " ");

	status = nvgpu_boardobjgrp_construct_e32(g,
			&g->pmu->volt->volt_metadata->volt_policy_metadata.volt_policies);
	if (status != 0) {
		nvgpu_err(g,
			"error creating boardobjgrp for volt policy, "
			"status - 0x%x", status);
		goto done;
	}

	pboardobjgrp =
		&g->pmu->volt->volt_metadata->volt_policy_metadata.volt_policies.super;

	pboardobjgrp->pmudatainstget  = volt_policy_devgrp_pmudata_instget;
	pboardobjgrp->pmudatainit = volt_policy_grp_pmudatainit_super;

	/* Obtain Voltage Policy Table from VBIOS */
	status = volt_get_volt_policy_table(g, &g->pmu->volt->volt_metadata->
			volt_policy_metadata);
	if (status != 0) {
		goto done;
	}

	/* Populate data for the VOLT_RAIL PMU interface */
	BOARDOBJGRP_PMU_CONSTRUCT(pboardobjgrp, VOLT, VOLT_POLICY);

	status = BOARDOBJGRP_PMU_CMD_GRP_SET_CONSTRUCT(g, pboardobjgrp,
			volt, VOLT, volt_policy, VOLT_POLICY);
	if (status != 0) {
		nvgpu_err(g,
			"error constructing PMU_BOARDOBJ_CMD_GRP_SET interface - 0x%x",
			status);
		goto done;
	}

done:
	nvgpu_log_info(g, " done status %x", status);
	return status;
}
