// SPDX-License-Identifier: GPL-2.0-only OR MIT
/* SPDX-FileCopyrightText: Copyright (c) 2016-2025, NVIDIA CORPORATION & AFFILIATES.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <nvgpu/bios.h>
#include <nvgpu/pmu/pmuif/nvgpu_cmdif.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/bug.h>
#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/pmu/boardobjgrp_classes.h>
#include <nvgpu/string.h>
#include <nvgpu/pmu/clk/clk.h>
#include <nvgpu/pmu/cmd.h>
#include <nvgpu/pmu/volt.h>

#include "ucode_clk_inf.h"
#include "clk_vin.h"
#include "clk.h"

static s32 devinit_get_vin_device_table(struct gk20a *g,
		struct nvgpu_avfsvinobjs *pvinobjs);

static s32 vin_device_construct_v20(struct gk20a *g,
		struct pmu_board_obj **obj, size_t size, void *pargs);
static s32 vin_device_construct_super(struct gk20a *g,
		struct pmu_board_obj **obj, size_t size, void *pargs);
static struct clk_vin_device *construct_vin_device(
		struct gk20a *g, void *pargs);

static s32 vin_device_init_pmudata_v20(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj);
static s32 vin_device_init_pmudata_super(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj);

static s32 devinit_get_adc_device_table_v2x(struct gk20a *g,
		struct nvgpu_avfsvinobjs *pAdcObjs);

static s32 adc_device_init_pmudata_super(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj);

struct clk_vin_device *clk_get_vin_from_index(
		struct nvgpu_avfsvinobjs *pvinobjs, u8 idx)
{
	return ((struct clk_vin_device *)BOARDOBJGRP_OBJ_GET_BY_IDX(
		((struct boardobjgrp *)&(pvinobjs->super.super)), idx));
}

static s32 nvgpu_clk_avfs_get_vin_cal_fuse_v20(struct gk20a *g,
		struct nvgpu_avfsvinobjs *pvinobjs,
		struct vin_device_v20 *pvindev)
{
	s32 status = 0;
	s8 gain, offset;
	u8 i;

	if (pvinobjs->calibration_rev_vbios ==
			g->ops.fuse.read_vin_cal_fuse_rev(g)) {
		BOARDOBJGRP_FOR_EACH(&(pvinobjs->super.super),
				struct vin_device_v20 *, pvindev, i) {
			gain = 0;
			offset = 0;
			pvindev = (struct vin_device_v20 *)(void *)
					clk_get_vin_from_index(pvinobjs, i);
			status = g->ops.fuse.read_vin_cal_gain_offset_fuse(g,
					pvindev->super.id, &gain, &offset);
			if (status != 0) {
				nvgpu_err(g,
				"err reading vin cal for id %x", pvindev->super.id);
				return status;
			}
			pvindev->data.vin_cal.cal_v20.gain = gain;
			pvindev->data.vin_cal.cal_v20.offset = offset;
		}
	}
	return status;

}

static s32 clk_vin_devgrp_pmudatainit_super(struct gk20a *g,
		struct boardobjgrp *pboardobjgrp,
		struct nv_pmu_boardobjgrp_super *pboardobjgrppmu)
{
	struct nv_pmu_clk_clk_vin_device_boardobjgrp_set_header *pset =
		(struct nv_pmu_clk_clk_vin_device_boardobjgrp_set_header *)
		pboardobjgrppmu;
	struct nvgpu_avfsvinobjs *pvin_obbj = (struct nvgpu_avfsvinobjs *)
			(void *)pboardobjgrp;
	s32 status = 0;

	nvgpu_log_info(g, " ");

	status = boardobjgrp_pmudatainit_e32(g, pboardobjgrp, pboardobjgrppmu);

	pset->b_vin_is_disable_allowed = pvin_obbj->vin_is_disable_allowed;
	pset->version = pvin_obbj->version;

	nvgpu_log_info(g, " Done. status %d", status);
	return status;
}

static s32 clk_vin_devgrp_pmudata_instget(struct gk20a *g,
		struct nv_pmu_boardobjgrp *pmuboardobjgrp,
		struct nv_pmu_boardobj **pmu_obj, u8 idx)
{
	struct nv_pmu_clk_clk_vin_device_boardobj_grp_set *pgrp_set =
		(struct nv_pmu_clk_clk_vin_device_boardobj_grp_set *)
		pmuboardobjgrp;

	nvgpu_log_info(g, " ");

	/*check whether pmuboardobjgrp has a valid boardobj in index*/
	if (((u32)BIT(idx) &
		pgrp_set->hdr.data.super.obj_mask.super.data[0]) == 0U)
		return -EINVAL;

	*pmu_obj = (struct nv_pmu_boardobj *)
		&pgrp_set->objects[idx].data.obj;
	nvgpu_log_info(g, " Done");
	return 0;
}

static s32 clk_vin_devgrp_pmustatus_instget(struct gk20a *g,
		void *pboardobjgrppmu,
		struct nv_pmu_boardobj_query **obj_pmu_status, u8 idx)
{
	struct nv_pmu_clk_clk_vin_device_boardobj_grp_get_status
	*pgrp_get_status =
		(struct nv_pmu_clk_clk_vin_device_boardobj_grp_get_status *)
		pboardobjgrppmu;

	(void)g;

	/*check whether pmuboardobjgrp has a valid boardobj in index*/
	if (((u32)BIT(idx) &
		pgrp_get_status->hdr.data.super.obj_mask.super.data[0]) == 0U)
		return -EINVAL;

	*obj_pmu_status = (struct nv_pmu_boardobj_query *)
			&pgrp_get_status->objects[idx].data.obj;
	return 0;
}

s32 clk_vin_sw_setup(struct gk20a *g)
{
	s32 status;
	struct boardobjgrp *pboardobjgrp = NULL;
	struct vin_device_v20 *pvindev = NULL;
	struct nvgpu_avfsvinobjs *pvinobjs;

	nvgpu_log_info(g, " ");

	status = nvgpu_boardobjgrp_construct_e32(g,
			&g->pmu->clk_pmu->avfs_vinobjs->super);
	if (status != 0) {
		nvgpu_err(g,
			"error creating boardobjgrp for clk vin, statu - 0x%x",
			status);
		goto done;
	}

	pboardobjgrp = &g->pmu->clk_pmu->avfs_vinobjs->super.super;
	pvinobjs = g->pmu->clk_pmu->avfs_vinobjs;

	BOARDOBJGRP_PMU_CONSTRUCT(pboardobjgrp, CLK, VIN_DEVICE);

	status = BOARDOBJGRP_PMU_CMD_GRP_SET_CONSTRUCT(g, pboardobjgrp,
			clk, CLK, clk_vin_device, CLK_VIN_DEVICE);
	if (status != 0) {
		nvgpu_err(g,
			"error constructing PMU_BOARDOBJ_CMD_GRP_SET interface - 0x%x",
			status);
		goto done;
	}

// _mg_ Todo: pmudata...() need updates for ADC_DEVICE_V30
	pboardobjgrp->pmudatainit  = clk_vin_devgrp_pmudatainit_super;
	pboardobjgrp->pmudatainstget  = clk_vin_devgrp_pmudata_instget;
	pboardobjgrp->pmustatusinstget  = clk_vin_devgrp_pmustatus_instget;

	status = devinit_get_vin_device_table(g, g->pmu->clk_pmu->avfs_vinobjs);
	if (status != 0)
		goto done;

	/* update vin calibration to fuse only if non-v30 device */
	if (pvinobjs->version != NV2080_CTRL_CLK_ADC_DEVICES_V30) {
		status = nvgpu_clk_avfs_get_vin_cal_fuse_v20(g, pvinobjs, pvindev);
		if (status != 0) {
			nvgpu_err(g, "clk_avfs_get_vin_cal_fuse_v20 failed err=%d",
				status);
			goto done;
		}
	}

	status = BOARDOBJGRP_PMU_CMD_GRP_GET_STATUS_CONSTRUCT(g,
				&g->pmu->clk_pmu->avfs_vinobjs->super.super,
				clk, CLK, clk_vin_device, CLK_VIN_DEVICE);
	if (status != 0) {
		nvgpu_err(g,
			"error constructing PMU_BOARDOBJ_CMD_GRP_SET interface - 0x%x",
			status);
		goto done;
	}

done:
	nvgpu_log_info(g, " done status %d", status);
	return status;
}

s32 clk_vin_pmu_setup(struct gk20a *g)
{
	s32 status;
	struct boardobjgrp *pboardobjgrp = NULL;

	nvgpu_log_info(g, " ");

	pboardobjgrp = &g->pmu->clk_pmu->avfs_vinobjs->super.super;

	if (!pboardobjgrp->bconstructed)
		return -EINVAL;

	status = pboardobjgrp->pmuinithandle(g, pboardobjgrp);

	nvgpu_log_info(g, "Done");
	return status;
}

static s32 devinit_get_vin_device_table(struct gk20a *g,
		struct nvgpu_avfsvinobjs *pvinobjs)
{
	s32 status = 0;
	u8 *vin_table_ptr = NULL;
	struct vin_descriptor_header_10 vin_desc_table_header = { 0 };
	struct vin_descriptor_entry_10 vin_desc_table_entry = { 0 };
	u8 *vin_tbl_entry_ptr = NULL;
	u32 index = 0;
	s8 offset = 0, gain = 0;
	struct clk_vin_device *pvin_dev;
	u32 cal_type;

	union {
		struct pmu_board_obj obj;
		struct clk_vin_device vin_device;
		struct vin_device_v20 vin_device_v20;
	} vin_device_data;

	nvgpu_log_info(g, " ");

	vin_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			nvgpu_bios_get_bit_token(g, NVGPU_BIOS_CLOCK_TOKEN),
						VIN_TABLE);
	if (vin_table_ptr == NULL) {
		status = -1;
		goto done;
	}

	nvgpu_memcpy((u8 *)&vin_desc_table_header, vin_table_ptr,
			sizeof(struct vin_descriptor_header_10));
	/* We support 0x10 and 0x20 versions only */
	if (vin_desc_table_header.version == NV_ADC_DESCRIPTOR_20_HEADER_VERSION) {
		status = devinit_get_adc_device_table_v2x(g, pvinobjs);
		goto done;
	}
	pvinobjs->version = (vin_desc_table_header.version == NV_VIN_DESCRIPTOR_10_HEADER_VERSION) ?
			NV2080_CTRL_CLK_VIN_DEVICES_V10 :
			NV2080_CTRL_CLK_VIN_DEVICES_DISABLED;
	pvinobjs->calibration_rev_vbios =
			BIOS_GET_FIELD(u8, vin_desc_table_header.flags0,
				NV_VIN_DESC_FLAGS0_VIN_CAL_REVISION);
	pvinobjs->vin_is_disable_allowed =
			BIOS_GET_FIELD(bool, vin_desc_table_header.flags0,
				NV_VIN_DESC_FLAGS0_DISABLE_CONTROL);
	cal_type = BIOS_GET_FIELD(u32, vin_desc_table_header.flags0,
				NV_VIN_DESC_FLAGS0_VIN_CAL_TYPE);
	if (cal_type != CTRL_CLK_VIN_CAL_TYPE_V20) {
		nvgpu_err(g, " Unsupported Vin calibration type: %d",
					cal_type);
		status = -1;
		goto done;
	}

	offset = BIOS_GET_FIELD(s8, vin_desc_table_header.vin_cal,
			NV_VIN_DESC_VIN_CAL_OFFSET);
	gain = BIOS_GET_FIELD(s8, vin_desc_table_header.vin_cal,
			NV_VIN_DESC_VIN_CAL_GAIN);

	/* Read table entries*/
	vin_tbl_entry_ptr = vin_table_ptr + vin_desc_table_header.header_sizee;
	for (index = 0; index < vin_desc_table_header.entry_count; index++) {
		nvgpu_memcpy((u8 *)&vin_desc_table_entry, vin_tbl_entry_ptr,
				sizeof(struct vin_descriptor_entry_10));

		if (vin_desc_table_entry.vin_device_type ==
				CTRL_CLK_VIN_TYPE_DISABLED) {
			vin_tbl_entry_ptr += vin_desc_table_header.entry_size;
			continue;
		}

		vin_device_data.obj.type =
			(u8)vin_desc_table_entry.vin_device_type;
		vin_device_data.vin_device.id =
				(u8)vin_desc_table_entry.vin_device_id;
		vin_device_data.vin_device.volt_domain_vbios =
			(u8)vin_desc_table_entry.volt_domain_vbios;
		vin_device_data.vin_device.flls_shared_mask = 0;
		vin_device_data.vin_device.por_override_mode =
				CTRL_CLK_VIN_SW_OVERRIDE_VIN_USE_HW_REQ;
		vin_device_data.vin_device.override_mode =
				CTRL_CLK_VIN_SW_OVERRIDE_VIN_USE_HW_REQ;
		vin_device_data.vin_device_v20.data.cal_type = (u8) cal_type;
		vin_device_data.vin_device_v20.data.vin_cal.cal_v20.offset =
				offset;
		vin_device_data.vin_device_v20.data.vin_cal.cal_v20.gain =
				gain;
		vin_device_data.vin_device_v20.data.vin_cal.cal_v20.offset_vfe_idx =
					CTRL_CLK_VIN_VFE_IDX_INVALID;

		pvin_dev = construct_vin_device(g, (void *)&vin_device_data);

		status = boardobjgrp_objinsert(&pvinobjs->super.super,
				(struct pmu_board_obj *)pvin_dev, (u8)index);

		vin_tbl_entry_ptr += vin_desc_table_header.entry_size;
	}

done:
	nvgpu_log_info(g, " done status %d", status);
	return status;
}

static s32 vin_device_construct_v20(struct gk20a *g,
		struct pmu_board_obj **obj, size_t size, void *pargs)
{
	struct pmu_board_obj *obj_tmp = (struct pmu_board_obj *)pargs;
	struct vin_device_v20 *pvin_device_v20;
	struct vin_device_v20 *ptmpvin_device_v20 = (struct vin_device_v20 *)pargs;
	s32 status = 0;

	if (pmu_board_obj_get_type(pargs) != CTRL_CLK_VIN_TYPE_V20)
		return -EINVAL;

	obj_tmp->type_mask |= BIT32(CTRL_CLK_VIN_TYPE_V20);
	status = vin_device_construct_super(g, obj, size, pargs);
	if (status != 0)
		return -EINVAL;

	pvin_device_v20 = (struct vin_device_v20 *)(void *)*obj;

	pvin_device_v20->super.super.pmudatainit =
			vin_device_init_pmudata_v20;

	pvin_device_v20->data.cal_type = ptmpvin_device_v20->data.cal_type;
	pvin_device_v20->data.vin_cal.cal_v20.offset =
			ptmpvin_device_v20->data.vin_cal.cal_v20.offset;
	pvin_device_v20->data.vin_cal.cal_v20.gain =
			ptmpvin_device_v20->data.vin_cal.cal_v20.gain;
	pvin_device_v20->data.vin_cal.cal_v20.offset_vfe_idx =
			ptmpvin_device_v20->data.vin_cal.cal_v20.offset_vfe_idx;

	return status;
}

static s32 vin_device_construct_super(struct gk20a *g,
		struct pmu_board_obj **obj, size_t size, void *pargs)
{
	struct clk_vin_device *pvin_device;
	struct clk_vin_device *ptmpvin_device =
		(struct clk_vin_device *)pargs;
	s32 status = 0;

	pvin_device = nvgpu_kzalloc(g, size);
	if (pvin_device == NULL)
		return -ENOMEM;

	status = pmu_board_obj_construct_super(g,
			(struct pmu_board_obj *)(void *)pvin_device, pargs);
	if (status != 0)
		return -EINVAL;

	*obj = (struct pmu_board_obj *)(void *)pvin_device;

	pvin_device->super.pmudatainit =
			vin_device_init_pmudata_super;

	pvin_device->id = ptmpvin_device->id;
	pvin_device->volt_domain_vbios = ptmpvin_device->volt_domain_vbios;
	pvin_device->flls_shared_mask = ptmpvin_device->flls_shared_mask;
	pvin_device->volt_domain = CTRL_VOLT_DOMAIN_LOGIC;
	pvin_device->por_override_mode = ptmpvin_device->por_override_mode;
	pvin_device->override_mode = ptmpvin_device->override_mode;

	return status;
}

static struct clk_vin_device *construct_vin_device(
		struct gk20a *g, void *pargs)
{
	struct pmu_board_obj *obj = NULL;
	s32 status;

	nvgpu_log_info(g, " Type: %d", pmu_board_obj_get_type(pargs));

	status = vin_device_construct_v20(g, &obj,
				sizeof(struct vin_device_v20), pargs);

	if (status != 0) {
		nvgpu_err(g, "Constructing ADC Device object failed. status: %d",
					status);
		return NULL;
	}

	nvgpu_log_info(g, " Done. status: %d", status);
	return (struct clk_vin_device *)(void *)obj;
}

static s32 vin_device_init_pmudata_v20(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;
	struct vin_device_v20 *pvin_dev_v20;
	struct nv_pmu_clk_clk_vin_device_v20_boardobj_set *perf_pmu_data;

	nvgpu_log_info(g, " ");

	status = vin_device_init_pmudata_super(g, obj, pmu_obj);
	if (status != 0)
		return status;

	pvin_dev_v20 = (struct vin_device_v20 *)(void *)obj;
	perf_pmu_data = (struct nv_pmu_clk_clk_vin_device_v20_boardobj_set *)
		pmu_obj;

	perf_pmu_data->data.cal_type = pvin_dev_v20->data.cal_type;
	perf_pmu_data->data.vin_cal.cal_v20.offset =
			pvin_dev_v20->data.vin_cal.cal_v20.offset;
	perf_pmu_data->data.vin_cal.cal_v20.gain =
			pvin_dev_v20->data.vin_cal.cal_v20.gain;
	perf_pmu_data->data.vin_cal.cal_v20.offset_vfe_idx =
			pvin_dev_v20->data.vin_cal.cal_v20.offset_vfe_idx;

	nvgpu_log_info(g, " Done");

	return status;
}

static s32 vin_device_init_pmudata_super(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;
	struct clk_vin_device *pvin_dev;
	struct nv_pmu_clk_clk_vin_device_boardobj_set *perf_pmu_data;

	nvgpu_log_info(g, " ");

	status = pmu_board_obj_pmu_data_init_super(g, obj, pmu_obj);
	if (status != 0)
		return status;

	pvin_dev = (struct clk_vin_device *)(void *)obj;
	perf_pmu_data = (struct nv_pmu_clk_clk_vin_device_boardobj_set *)
		pmu_obj;

	perf_pmu_data->id = pvin_dev->id;
	perf_pmu_data->volt_rail_idx =
			nvgpu_pmu_volt_rail_volt_domain_convert_to_idx(
					g, pvin_dev->volt_domain);
	perf_pmu_data->flls_shared_mask = pvin_dev->flls_shared_mask;
	perf_pmu_data->por_override_mode = pvin_dev->por_override_mode;
	perf_pmu_data->override_mode = pvin_dev->override_mode;

	nvgpu_log_info(g, " Done");

	return status;
}

static s32 adc_device_init_pmudata_super(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;
	struct clk_adc_device *padc_dev;
	struct nv_pmu_clk_clk_adc_device_boardobj_set *perf_pmu_data;

	nvgpu_log_info(g, " ");

	status = pmu_board_obj_pmu_data_init_super(g, obj, pmu_obj);
	if (status != 0)
		goto done;

	padc_dev = (struct clk_adc_device *)obj;
	perf_pmu_data = (struct nv_pmu_clk_clk_adc_device_boardobj_set *)
		pmu_obj;

	perf_pmu_data->id = padc_dev->id;
	perf_pmu_data->naflls_shared_mask = padc_dev->naflls_shared_mask;
	perf_pmu_data->override_mode = padc_dev->override_mode;
	// _mg_ Todo: volt rail idx needs to be copied to nafll dev obj and in state_load callback.
	perf_pmu_data->volt_rail_idx =
			nvgpu_pmu_volt_rail_volt_domain_convert_to_idx(
					g, padc_dev->volt_domain);
	perf_pmu_data->dynamic_cal = padc_dev->dynamic_cal;
	perf_pmu_data->logical_api_id = padc_dev->logical_api_id;
	perf_pmu_data->devinit_enabled = padc_dev->devinit_enabled;

done:
	nvgpu_log_info(g, " Done. status %d", status);

	return status;
}

static s32 adc_device_construct_v30(struct gk20a *g,
		struct pmu_board_obj **obj, size_t size,
		struct ctrl_clk_adc_device_v30 *pargs)
{
	struct pmu_board_obj *pobj_tmp = (struct pmu_board_obj *)pargs;
	struct pmu_board_obj *pobj_new = NULL;
	struct clk_adc_device *pDev = NULL;
	struct clk_adc_device *ptmpDev =
		(struct clk_adc_device *)pargs;
	struct ctrl_clk_adc_device_v30 *pDev30 = NULL;
	struct ctrl_clk_adc_device_v30 *ptmpDev30 = pargs;
	s32 status = 0;

	nvgpu_log_info(g, " ");

	/* Construct ADC super first */
	pDev30 = nvgpu_kzalloc(g, size);
	if (pDev30 == NULL) {
		status = -ENOMEM;
		goto done;
	}
	pobj_new = &pDev30->super.super.super;
	/* construct super class and insert in the object list. */
	status = pmu_board_obj_construct_super(g,
				pobj_new, pargs);
	if (status != 0) {
		status = -ENOMEM;
		goto done;
	}
	pobj_new->type = pobj_tmp->type;

	pDev = &pDev30->super.super;
	pDev->id = ptmpDev->id;
	pDev->volt_domain = ptmpDev->volt_domain;
	pDev->por_override_mode = ptmpDev->por_override_mode;
	pDev->override_mode = ptmpDev->override_mode;
	pDev->naflls_shared_mask = ptmpDev->naflls_shared_mask;
	pDev->dynamic_cal = ptmpDev->dynamic_cal;
	pDev->logical_api_id = ptmpDev->logical_api_id;
	pDev->devinit_enabled = ptmpDev->devinit_enabled;
	pDev->volt_domain_vbios = ptmpDev->volt_domain_vbios;

	/*
	 * state_load configures
	 * the adc device's volt_domain
	 * and the naffl device's volt_rail_idx.
	 */
	pDev->state_load = NULL; /* _mg_ Todo: Implement v30 state_load.*/
	/* fuse rev check not needed for V30 ADC */
	pDev->cal_fuse_rev_check = NULL;

	*obj = pobj_new;

	pDev30->data = ptmpDev30->data;
	pobj_new->pmudatainit = adc_device_init_pmudata_super;

done:
	nvgpu_log_info(g, " Done. status: %d", status);
	return status;
}

s32 clk_pmu_vin_load(struct gk20a *g)
{
	s32 status;
	struct nvgpu_pmu *pmu = g->pmu;
	struct nv_pmu_rpc_struct_clk_load clk_load_rpc;

	(void) memset(&clk_load_rpc, 0,
			sizeof(struct nv_pmu_rpc_struct_clk_load));

	clk_load_rpc.clk_load.feature = NV_NV_PMU_CLK_LOAD_FEATURE_VIN;
	clk_load_rpc.clk_load.action_mask =
		NV_NV_PMU_CLK_LOAD_ACTION_MASK_VIN_HW_CAL_PROGRAM_YES << 4;

	/* Continue with PMU setup, assume FB map is done  */
	PMU_RPC_EXECUTE_CPB(status, pmu, CLK, LOAD, &clk_load_rpc, 0);
	if (status != 0) {
		nvgpu_err(g,
			"Failed to execute Clock Load RPC status=0x%x",
			status);
	}

	return status;
}

s32 clk_vin_init_pmupstate(struct gk20a *g)
{
	/* If already allocated, do not re-allocate */
	if (g->pmu->clk_pmu->avfs_vinobjs != NULL)
		return 0;
	/* allocate memory for the larger of the union
	 * of boardobjs avfs_vinobjs (v10) and
	 * nvgpu_avfs_adcobjs_v20
	 */
	g->pmu->clk_pmu->avfs_vinobjs = nvgpu_kzalloc(g,
			sizeof(struct nvgpu_avfs_adcobjs_v20));
	if (g->pmu->clk_pmu->avfs_vinobjs == NULL)
		return -ENOMEM;

	return 0;
}

void clk_vin_free_pmupstate(struct gk20a *g)
{
	nvgpu_kfree(g, g->pmu->clk_pmu->avfs_vinobjs);
	g->pmu->clk_pmu->avfs_vinobjs = NULL;
}

/* This table is used to translate the
 * ADC Device ID (VBIOS table entry index) to API ID.
 */
static u8 vbios_adc_table_table_entry_to_clock_api_id[] = {
			CTRL_CLK_VIN_ID_SYS,
			CTRL_CLK_VIN_ID_GPC0
};

static s32 adc_translate_adc_id_to_api(u32 entryIdx, u8 *api_id)
{
	s32 status = 0;

	if (entryIdx >=
		(sizeof(vbios_adc_table_table_entry_to_clock_api_id)/sizeof(u8)))
		status = -EINVAL;
	else
		*api_id = vbios_adc_table_table_entry_to_clock_api_id[entryIdx];

	return status;
}

static struct clk_adc_device *construct_adc_device(
		struct gk20a *g, struct ctrl_clk_adc_device_v30 *pargs)
{
	struct pmu_board_obj *obj = NULL;
	s32 status = -EINVAL;

	nvgpu_log_info(g, " Type: %d", pmu_board_obj_get_type(pargs));

	if (pmu_board_obj_get_type(pargs) == NV2080_CTRL_CLK_ADC_DEVICE_TYPE_V30) {
		status = adc_device_construct_v30(g, &obj,
				sizeof(struct ctrl_clk_adc_device_v30), pargs);
	}
	if (status != 0) {
		nvgpu_err(g, "Constructing ADC Device object failed. status: %d",
					status);
		return NULL;
	}

	nvgpu_log_info(g, " Done.");

	return (struct clk_adc_device *)(void *)obj;
}

static s32 devinit_get_adc_device_table_v2x(struct gk20a *g,
		struct nvgpu_avfsvinobjs *pAdcObjs)
{
	s32 status = 0;
	u8 *adc_table_ptr = NULL;
	struct nvgpu_avfs_adcobjs_v20 *pAdcObjs20 =
			(struct nvgpu_avfs_adcobjs_v20 *) pAdcObjs;
	struct adc_descriptor_header_20 adc_desc_table_header = { 0 };
	struct adc_descriptor_entry_20 adc_desc_table_entry = { 0 };
	u8 *adc_tbl_entry_ptr = NULL;
	u32 index = 0;
	u8 vbiosPorOverrideMode;
	struct clk_adc_device            *padc_dev = NULL;
	struct pmu_board_obj             *pboardObj;
	struct clk_adc_device            *padcDevice;
	struct ctrl_clk_adc_device_v30   *padcDeviceV30;

	nvgpu_log_info(g, " ");

	padcDeviceV30 = (struct ctrl_clk_adc_device_v30 *)
			nvgpu_kzalloc(g, sizeof(struct ctrl_clk_adc_device_v30));
	if (padcDeviceV30 == NULL) {
		nvgpu_err(g, "Unable to allocate memory.");
		status = -ENOMEM;
		goto done_no_free;
	}
	/* Setup pointers for ease of readability */
	padcDevice = &padcDeviceV30->super.super;
	pboardObj  = &padcDevice->super;


	adc_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			nvgpu_bios_get_bit_token(g, NVGPU_BIOS_CLOCK_TOKEN),
						VIN_TABLE);
	if (adc_table_ptr == NULL) {
		status = -1;
		goto done;
	}

	nvgpu_memcpy((u8 *)&adc_desc_table_header, adc_table_ptr,
			sizeof(struct adc_descriptor_header_20));

	if (adc_desc_table_header.version != NV_ADC_DESCRIPTOR_20_HEADER_VERSION) {
		nvgpu_err(g, "Unsupported ADC table version: %x  want: %x",
				adc_desc_table_header.version,
				NV_ADC_DESCRIPTOR_20_HEADER_VERSION);
		status = -EINVAL;
		goto done;
	}
	pAdcObjs->version = NV2080_CTRL_CLK_ADC_DEVICES_V30;

	/* populate boardobjs with VBIOS table data */

	// Disable/power control of the ADC from the adc_param0 field
	pAdcObjs20->super.super.super.vin_is_disable_allowed =
		BIOS_GET_FIELD(bool, adc_desc_table_header.adc_param0,
				NV_ADC_PARAM0_PWR_DOWN_ENABLE);

	// Minimum cap on ADC code correction offset
	pAdcObjs20->data.adc_code_correction_offset_min =
		EXT_VAL_SIGNED(s8, adc_desc_table_header.adc_param0,
				NV_ADC_PARAM0_CORRECTION_OFFSET_MIN);
	// Maximum cap on ADC code correction offset
	pAdcObjs20->data.adc_code_correction_offset_max =
		EXT_VAL_SIGNED(s8, adc_desc_table_header.adc_param0,
				NV_ADC_PARAM0_CORRECTION_OFFSET_MAX);

	// Mask of invalid fuse revisions for opt_adc_cal
	pAdcObjs20->data.cal_invalid_fuse_rev_mask =
			adc_desc_table_header.cal_invalid_fuse_rev_mask;

	// Mask of invalid fuse revisions for opt_adc_code_err_lt
	pAdcObjs20->data.low_temp_err_invalid_fuse_rev_mask =
			adc_desc_table_header.low_temp_err_invalid_fuse_rev_mask;

	// Mask of invalid fuse revisions for opt_adc_code_err_ht
	pAdcObjs20->data.high_temp_err_invalid_fuse_rev_mask =
			adc_desc_table_header.high_temp_err_invalid_fuse_rev_mask;

	// Temperature at which low temperature error deltas are calculated
	pAdcObjs20->data.low_temperature = adc_desc_table_header.low_temperature;

	// Temperature at which high temperature error deltas are calculated
	pAdcObjs20->data.high_temperature = adc_desc_table_header.high_temperature;

	// Reference temperature at which we do our base ADC calibration at ATE
	pAdcObjs20->data.ref_temperature = adc_desc_table_header.ref_temperature;

	// Low voltage at which the low/high temperature error deltas are calculated
	pAdcObjs20->data.low_volt_uv = adc_desc_table_header.low_volt;

	// High voltage at which the low/high temperature error deltas are calculated
	pAdcObjs20->data.high_volt_uv = adc_desc_table_header.high_volt;

	// Populate the excess header data now
	if (adc_desc_table_header.header_size >= NV_ADC_DESCRIPTOR_20_HEADER_SIZE) {
		// Reference voltage at which we do our base ADC calibration at ATE
		pAdcObjs20->data.ref_volt_uv = adc_desc_table_header.ref_volt;

		// Index of VFE variable that helps us get the current
		// temperature needed for runtime ADC calibration
		pAdcObjs20->data.temperature_vfe_var_idx =
				adc_desc_table_header.temperature_vfe_var_idx;
	}

	// Initialize mask of ADC_DEVICEs that support NAFLL functionality
	status = boardobjgrpmask_e32_init(
			&pAdcObjs20->super.super.super.naffl_adc_devices_mask,
			NULL);
	if (status != 0)
		goto done;

	/* Read table entries.  We only support ADC device type v30 */

	/* This is set during NAFLL_DEVICE VBIOS parsing. Init to 0 here. */
	padcDevice->naflls_shared_mask = 0;

	/* This is set during ADC state load. Init to 0 here. */
	padcDevice->volt_domain = 0;

	/* Populate an ADC object for each valid table entry */
	adc_tbl_entry_ptr = adc_table_ptr + adc_desc_table_header.header_size;
	for (index = 0; index < adc_desc_table_header.entry_count; index++,
			adc_tbl_entry_ptr += adc_desc_table_header.entry_size) {

		u8 apiAdcId;

		/* To avoid MISRA violation,
		 * this is a single point of exit
		 * from this for loop in the case
		 * when an error is encountered.
		 */
		if (status != 0)
			break;

		nvgpu_memcpy((u8 *)&adc_desc_table_entry, adc_tbl_entry_ptr,
				sizeof(struct adc_descriptor_entry_20));

		if (adc_desc_table_entry.adc_device_type ==
				NV_ADC_DEVICE_2X_TYPE_DISABLED)
			continue;

		if (adc_desc_table_entry.adc_device_type !=
				NV_ADC_DEVICE_2X_TYPE_V30) {
			nvgpu_err(g, "Unsupported ADC type: 0x%x  want: 0x%x",
				adc_desc_table_entry.adc_device_type,
				NV_ADC_DEVICE_2X_TYPE_V30);
			status = -EINVAL;
			continue; /* to avoid MISRA violation, exit at top of loop */
		}

		status = adc_translate_adc_id_to_api(index, &apiAdcId);
		if (status != 0) {
			nvgpu_err(g,
				"VBIOS ADC ID index invalid or couldn't be translated to a valid API ID - %d\n",
				index);
			status = -EINVAL;
			continue; /* to avoid MISRA violation, exit at top of loop */
		}

		/* we only support ADC device type V30 */
		pboardObj->type = NV2080_CTRL_CLK_ADC_DEVICE_TYPE_V30;

		padcDeviceV30->data.offset =
				BIOS_GET_FIELD(s8, adc_desc_table_header.adc_param0,
						NV_ADC_PARAM0_OFFSET);

		padcDeviceV30->data.gain =
				BIOS_GET_FIELD(s8, adc_desc_table_header.adc_param0,
						NV_ADC_PARAM0_GAIN);

		padcDeviceV30->data.course_offset =
				BIOS_GET_FIELD(s8, adc_desc_table_header.adc_param0,
						NV_ADC_PARAM0_COARSE_OFFSET);

		padcDeviceV30->data.course_gain =
				BIOS_GET_FIELD(s8, adc_desc_table_header.adc_param0,
						NV_ADC_PARAM0_COARSE_GAIN);

		/* Now populate all the generic (ADC type agnostic) data. */
		padcDevice->id = apiAdcId;
		padcDevice->volt_domain_vbios = adc_desc_table_entry.volt_domain_vbios;
		padcDevice->logical_api_id = apiAdcId;

		/* Update POR override mode. */
		if (adc_desc_table_header.entry_size >= NV_ADC_DESCRIPTOR_ENTRY_20_SIZE_03) {
			vbiosPorOverrideMode =
					BIOS_GET_FIELD(u8, adc_desc_table_entry.flags,
					NV_ADC_FLAG_OP_MODE);

			switch (vbiosPorOverrideMode) {
			case NV_ADC_FLAG_OP_MODE_HW:
			{
				padcDevice->por_override_mode =
					NV2080_CTRL_CLK_ADC_SW_OVERRIDE_ADC_USE_HW_REQ;
				break;
			}
			case NV_ADC_FLAG_OP_MODE_SW:
			{
				padcDevice->por_override_mode =
					NV2080_CTRL_CLK_ADC_SW_OVERRIDE_ADC_USE_SW_REQ;
				break;
			}
			case NV_ADC_FLAG_OP_MODE_MIN:
			{
				padcDevice->por_override_mode =
					NV2080_CTRL_CLK_ADC_SW_OVERRIDE_ADC_USE_MIN;
				break;
			}
			default:
			{
				status = -EINVAL;
				nvgpu_err(g,
					"Unsupported ADC device POR override mode: 0x%02x.",
					vbiosPorOverrideMode);
				break;
			}
			}

			if (status == -EINVAL)
				continue; /* to avoid MISRA violation, exit at top of loop */

			/* Init override mode to POR override mode. */
			padcDevice->override_mode = padcDevice->por_override_mode;

			/* Populate other parameters from flag */
			padcDevice->dynamic_cal =
					BIOS_GET_FIELD(bool,
					adc_desc_table_entry.flags,
					NV_ADC_FLAG_DYNAMIC_CAL);

			padcDevice->devinit_enabled =
					BIOS_GET_FIELD(bool,
					adc_desc_table_entry.flags,
					NV_ADC_FLAG_DEVINIT_ENABLE);

			status = nvgpu_boardobjgrpmask_bit_set(
					&pAdcObjs->naffl_adc_devices_mask.super, (u8)index);
			if (status != 0) {
				nvgpu_err(g, "Setting NAFFLL ADC device mask failed - %d\n", index);
				continue; /* to avoid MISRA violation, exit at top of loop */
			}

		} else {
			status = -EINVAL;
			nvgpu_err(g,
				"ADC device entry table wrong size: %d.",
				 adc_desc_table_header.entry_size);
			continue; /* to avoid MISRA violation, exit at top of loop */
		}

		/* Skip VBIOS ADC Device Table Entries Param0 - Param5,
		 * these are not used for device type id ADC_DEVICE_TYPE_V30
		 */

		/* Construct the adc device and insert it into the object list. */
		padc_dev = construct_adc_device(g, padcDeviceV30);
		if (padc_dev == NULL) {
			nvgpu_err(g,
				"Could not construct ADC device object for index: %d.", index);
			status = -EINVAL;
			continue; /* to avoid MISRA violation, exit at top of loop */
		}

		status = boardobjgrp_objinsert(&pAdcObjs->super.super,
						(struct pmu_board_obj *)padc_dev, (u8)index);
		if (status != 0) {
			nvgpu_err(g,
				"Inserting ADC device object to boardObjGrp failed at index: %d",
				index);
			continue; /* to avoid MISRA violation, exit at top of loop */
		}
	}

done:
	nvgpu_kfree(g, padcDeviceV30);
done_no_free:
	nvgpu_log_info(g, " Done. status %d", status);
	return status;
}
