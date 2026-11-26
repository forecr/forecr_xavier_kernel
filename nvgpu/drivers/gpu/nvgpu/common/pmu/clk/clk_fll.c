// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/hw/gb20c/hw_fuse_gb20c.h>
#include <nvgpu/bios.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/pmu/boardobjgrp_classes.h>
#include <nvgpu/boardobjgrpmask.h>
#include <nvgpu/string.h>
#include <nvgpu/pmu/clk/clk.h>

#include "common/pmu/volt/volt.h"
#include "ucode_clk_inf.h"
#include "clk_fll.h"
#include "clk_vin.h"
#include "clk.h"
#include "clk_domain.h"

#define NV_PERF_DOMAIN_4X_CLOCK_DOMAIN_SKIP	0x10U
#define NV_PERF_DOMAIN_4X_CLOCK_DOMAIN_MASK	0x1FU

static s32 devinit_get_fll_device_table(struct gk20a *g,
		struct clk_avfs_fll_objs *pfllobjs);
static struct fll_device *construct_fll_device(struct gk20a *g,
		void *pargs);
static s32 fll_device_init_pmudata_super(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj);

static u32 clk_get_vbios_clk_domain(u32 vbios_domain);
static s32 devinit_get_fll_device_table_v20(struct gk20a *g,
		struct clk_avfs_fll_objs *pfllobjs);
static struct nafll_device_v35 *construct_nafll_device_v35(struct gk20a *g,
		void *pargs);
static s32 nafll_v35_device_init_pmudata_super(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj);
static s32 nafllLutProgMaskSet_V35(struct gk20a *g,
		struct nafll_device *pNafll);

u8 clk_get_fll_lut_vf_num_entries(struct nvgpu_clk_pmupstate *pclk)
{
	return ((pclk)->avfs_fllobjs->lut_num_entries);
}

u32 nvgpu_pmu_clk_fll_get_lut_min_volt(struct nvgpu_clk_pmupstate *pclk)
{
	return ((pclk)->avfs_fllobjs->lut_min_voltage_uv);
}

u32 nvgpu_pmu_clk_fll_get_lut_step_size(struct nvgpu_clk_pmupstate *pclk)
{
	return ((pclk)->avfs_fllobjs->lut_step_size_uv);
}

static s32 clkNafllDeviceStateLoad(struct gk20a *g,
	struct nafll_device *pDev)
{
	s32 status = 0;
	struct nvgpu_clk_domain *pClkDomain;

	if (pDev == NULL) {
		status = -EINVAL;
		goto done;
	}

	// Calculate the number of LUT entries & LUT stride
	// From Ampere and onwards only 6.25 mv step sizes are supported
	g->pmu->clk_pmu->avfs_fllobjs->lut_num_entries = ((CTRL_CLK_LUT_MAX_VOLTAGE_UV -
		CTRL_CLK_LUT_MIN_VOLTAGE_UV) / CTRL_CLK_VIN_STEP_SIZE_UV);

	pDev->regime_desc.regime_id = CTRL_CLK_FLL_REGIME_ID_FFR;

	pClkDomain =
		clkDomainsFindByApiDomain(g, pDev->clk_domain);

	if (pClkDomain == NULL) {
		status = -EINVAL;
		goto done;
	}
	pDev->rail_idx_for_lut = clkDomainProgVoltRailIdxGet_50_PROG_PHYSICAL(g, pClkDomain);

	status = nafllLutProgMaskSet_V35(g, pDev);

done:
	return status;
}

static s32 nafllLutBroadcastSlaveRegister(struct gk20a *g,
	struct nafll_device *pfll,
	struct nafll_device *pfll_slave)
{
	(void)g;

	if (pfll->clk_domain != pfll_slave->clk_domain)
		return -EINVAL;

	return nvgpu_boardobjgrpmask_bit_set(&pfll->
		lut_prog_bcast_secndary_mask.super,
		pmu_board_obj_get_idx(&pfll_slave->super));
}

static s32 nafllLutProgMaskSet_V35(struct gk20a *g,
	struct nafll_device *pNafll)
{
	s32 status = 0;

	// Early return with no action taken if not in broadcast NAFLL mask
	if (!nvgpu_boardobjgrpmask_bit_get(
		&g->pmu->clk_pmu->avfs_fllobjs->broadcast_nafll_dev_mask.super,
		pNafll->super.idx))
		goto done;

	// Always set its bit in lut_prog_master_mask
	status = nvgpu_boardobjgrpmask_bit_set(
		&g->pmu->clk_pmu->avfs_fllobjs->lut_prog_master_mask.super,
		pNafll->super.idx);
	if (status != 0) {
		nvgpu_err(g, "Could not set bit in master mask: status=%d, idx=%d.",
			status, pNafll->super.idx);
	}

	// Always self-register to set the bit in the SECONDARY mask.
	status = nafllLutBroadcastSlaveRegister(g, pNafll, pNafll);
	if (status != 0) {
		nvgpu_err(g, "Could not register broadcast slave with master: status=%d, nafllIdx=%d.",
			status, pNafll->super.idx);
	}

done:
	return status;
}

static s32 clk_fll_devgrp_pmudatainit_super(struct gk20a *g,
		struct boardobjgrp *pboardobjgrp,
		struct nv_pmu_boardobjgrp_super *pboardobjgrppmu)
{
	struct nv_pmu_clk_clk_fll_device_boardobjgrp_set_header *pset =
		(struct nv_pmu_clk_clk_fll_device_boardobjgrp_set_header *)
		pboardobjgrppmu;
	struct clk_avfs_fll_objs *pfll_objs = (struct clk_avfs_fll_objs *)
		pboardobjgrp;
	s32 status = 0;

	nvgpu_log_info(g, " ");

	status = boardobjgrp_pmudatainit_e32(g, pboardobjgrp, pboardobjgrppmu);
	if (status != 0) {
		nvgpu_err(g, "failed to init fll pmuobjgrp");
		return status;
	}
	pset->lut_num_entries = pfll_objs->lut_num_entries;
	pset->lut_step_size_uv = pfll_objs->lut_step_size_uv;
	pset->lut_min_voltage_uv = pfll_objs->lut_min_voltage_uv;
	pset->max_min_freq_mhz = pfll_objs->max_min_freq_mhz;

	status = nvgpu_boardobjgrpmask_export(
		&pfll_objs->lut_prog_master_mask.super,
		pfll_objs->lut_prog_master_mask.super.bitcount,
		&pset->lut_prog_master_mask.super);

	nvgpu_log_info(g, " Done");
	return status;
}

static s32 clk_fll_devgrp_pmudatainit_super_v35(struct gk20a *g,
		struct boardobjgrp *pboardobjgrp,
		struct nv_pmu_boardobjgrp_super *pboardobjgrppmu)
{
	struct nv_pmu_clk_clk_nafll_device_boardobjgrp_set_header *pset =
		(struct nv_pmu_clk_clk_nafll_device_boardobjgrp_set_header *)
		pboardobjgrppmu;
	struct clk_avfs_fll_objs *pfll_objs = (struct clk_avfs_fll_objs *)
		pboardobjgrp;
	s32 status = 0;

	nvgpu_log_info(g, " ");

	status = boardobjgrp_pmudatainit_e32(g, pboardobjgrp, pboardobjgrppmu);
	if (status != 0) {
		nvgpu_err(g, "failed to init nafll pmuobjgrp");
		return status;
	}

	pset->lut_num_entries = pfll_objs->lut_num_entries;
	pset->lut_step_size_uv = pfll_objs->lut_step_size_uv;
	pset->lut_min_voltage_uv = pfll_objs->lut_min_voltage_uv;
	pset->max_min_freq_mhz = pfll_objs->max_min_freq_mhz;
	pset->b_die1_all_gpc_floorswept = pfll_objs->b_die1_all_gpc_floorswept;

	status = nvgpu_boardobjgrpmask_export(
		&pfll_objs->unicast_nafll_dev_mask.super,
		pfll_objs->unicast_nafll_dev_mask.super.bitcount,
		&pset->unicast_nafll_dev_mask.super);
	if (status != 0) {
		nvgpu_err(g, "export mask failed.");
		goto done;
	}

	status = nvgpu_boardobjgrpmask_export(
		&pfll_objs->broadcast_nafll_dev_mask.super,
		pfll_objs->broadcast_nafll_dev_mask.super.bitcount,
		&pset->broadcast_nafll_dev_mask.super);
	if (status != 0) {
		nvgpu_err(g, "export mask failed.");
		goto done;
	}

	status = nvgpu_boardobjgrpmask_export(
		&pfll_objs->lut_prog_master_mask.super,
		pfll_objs->lut_prog_master_mask.super.bitcount,
		&pset->lut_prog_master_mask.super);
	if (status != 0) {
		nvgpu_err(g, "export mask failed.");
		goto done;
	}

done:
	nvgpu_log_info(g, " Done");
	return status;
}

static s32 clk_fll_devgrp_pmudata_instget_v35(struct gk20a *g,
		struct nv_pmu_boardobjgrp *pmuboardobjgrp,
		struct nv_pmu_boardobj **pmu_obj, u16 idx)
{
	struct nv_pmu_clk_clk_nafll_device_boardobj_grp_set  *pgrp_set =
		(struct nv_pmu_clk_clk_nafll_device_boardobj_grp_set *)
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

static s32 clk_fll_devgrp_pmustatus_instget_v35(struct gk20a *g,
		void *pboardobjgrppmu,
		struct nv_pmu_boardobj_query **obj_pmu_status, u16 idx)
{
	struct nv_pmu_clk_clk_nafll_device_v2_boardobj_grp_get_status
	*pgrp_get_status =
		(struct nv_pmu_clk_clk_nafll_device_v2_boardobj_grp_get_status *)
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

static s32 clk_fll_devgrp_pmudata_instget(struct gk20a *g,
		struct nv_pmu_boardobjgrp *pmuboardobjgrp,
		struct nv_pmu_boardobj **pmu_obj, u16 idx)
{
	struct nv_pmu_clk_clk_fll_device_boardobj_grp_set  *pgrp_set =
		(struct nv_pmu_clk_clk_fll_device_boardobj_grp_set *)
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

static s32 clk_fll_devgrp_pmustatus_instget(struct gk20a *g,
		void *pboardobjgrppmu,
		struct nv_pmu_boardobj_query **obj_pmu_status, u16 idx)
{
	struct nv_pmu_clk_clk_fll_device_boardobj_grp_get_status
	*pgrp_get_status =
		(struct nv_pmu_clk_clk_fll_device_boardobj_grp_get_status *)
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

s32 clk_fll_sw_setup(struct gk20a *g)
{
	s32 status;
	struct boardobjgrp *pboardobjgrp = NULL;
	struct clk_avfs_fll_objs *pfllobjs;

	nvgpu_log_info(g, " ");

	/* If already constructed, do not re-construct (suspend/resume pattern) */
	pboardobjgrp = &(g->pmu->clk_pmu->avfs_fllobjs->super.super);
	if (pboardobjgrp->bconstructed) {
		nvgpu_pmu_dbg(g, "clk_fll boardobjgrp already constructed, skipping reinit");
		return 0;
	}

	nvgpu_pmu_dbg(g, "Constructing clk_fll boardobjgrp for first time");

	status = nvgpu_boardobjgrp_construct_e32(g,
			&g->pmu->clk_pmu->avfs_fllobjs->super);
	if (status != 0) {
		nvgpu_err(g,
		"error creating boardobjgrp for fll, status - 0x%x", status);
		goto done;
	}
	pfllobjs = g->pmu->clk_pmu->avfs_fllobjs;

	/* construct and set the boardobj class ID */
	BOARDOBJGRP_PMU_CONSTRUCT(pboardobjgrp, CLK, CLK_NAFLL_DEVICE);

	status = BOARDOBJGRP_PMU_CMD_GRP_SET_CONSTRUCT(g, pboardobjgrp,
			clk, CLK, clk_fll_device, CLK_NAFLL_DEVICE);
	if (status != 0) {
		nvgpu_err(g,
			  "error constructing PMU_BOARDOBJ_CMD_GRP_SET interface - 0x%x",
			  status);
		goto done;
	}

	pfllobjs = (struct clk_avfs_fll_objs *)pboardobjgrp;
	pfllobjs->lut_num_entries = g->ops.clk.lut_num_entries;
	pfllobjs->lut_step_size_uv = CTRL_CLK_VIN_STEP_SIZE_UV;
	pfllobjs->lut_min_voltage_uv = CTRL_CLK_LUT_MIN_VOLTAGE_UV;

	/* Initialize lut prog master mask to zero.*/
	status = boardobjgrpmask_e32_init(&pfllobjs->lut_prog_master_mask,
			NULL);
	if (status != 0) {
		nvgpu_err(g, "boardobjgrpmask_e32_init failed err=%d", status);
		goto done;
	}

	/* Initialize unicast nafll mask to zero.*/
	status = boardobjgrpmask_e32_init(&pfllobjs->unicast_nafll_dev_mask,
		NULL);
	if (status != 0) {
		nvgpu_err(g, "boardobjgrpmask_e32_init failed err=%d", status);
		goto done;
	}

	/* Initialize broadcast nafll mask to zero.*/
	status = boardobjgrpmask_e32_init(&pfllobjs->broadcast_nafll_dev_mask,
		NULL);
	if (status != 0) {
		nvgpu_err(g, "boardobjgrpmask_e32_init failed err=%d", status);
		goto done;
	}

	status = devinit_get_fll_device_table(g, pfllobjs);
	if (status != 0) {
		nvgpu_err(g, "devinit_get_fll_device_table failed err=%d", status);
		goto done;
	}

	status = BOARDOBJGRP_PMU_CMD_GRP_GET_STATUS_CONSTRUCT(g,
				&g->pmu->clk_pmu->avfs_fllobjs->super.super,
				clk, CLK, clk_fll_device, CLK_NAFLL_DEVICE);
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

s32 clk_fll_pmu_setup(struct gk20a *g)
{
	s32 status;
	struct boardobjgrp *pboardobjgrp = NULL;

	nvgpu_log_info(g, " ");
	pboardobjgrp = &g->pmu->clk_pmu->avfs_fllobjs->super.super;

	if (!pboardobjgrp->bconstructed) {
		return -EINVAL;
	}

	status = pboardobjgrp->pmuinithandle(g, pboardobjgrp);

	nvgpu_log_info(g, "Done");
	return status;
}

static s32 devinit_get_fll_device_table(struct gk20a *g,
		struct clk_avfs_fll_objs *pfllobjs)
{
	s32 status = 0;
	u8 *fll_table_ptr = NULL;
	struct fll_descriptor_header fll_desc_table_header_sz = { 0 };
	struct fll_descriptor_header_10 fll_desc_table_header = { 0 };
	struct fll_descriptor_entry_10 fll_desc_table_entry = { 0 };
	u8 *fll_tbl_entry_ptr = NULL;
	u32 index = 0;
	struct fll_device fll_dev_data;
	struct fll_device *pfll_dev;
	struct clk_vin_device *pvin_dev;
	u32 desctablesize;
	u32 vbios_domain = NV_PERF_DOMAIN_4X_CLOCK_DOMAIN_SKIP;
	struct nvgpu_avfsvinobjs *pvinobjs = g->pmu->clk_pmu->avfs_vinobjs;
	struct boardobjgrp *pboardobjgrp = &(g->pmu->clk_pmu->avfs_fllobjs->super.super);

	nvgpu_log_info(g, " ");

	fll_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			  nvgpu_bios_get_bit_token(g, NVGPU_BIOS_CLOCK_TOKEN),
							FLL_TABLE);
	if (fll_table_ptr == NULL) {
		status = -1;
		goto done;
	}

	nvgpu_memcpy((u8 *)&fll_desc_table_header_sz, fll_table_ptr,
			sizeof(struct fll_descriptor_header));

	if (fll_desc_table_header_sz.version == FLL_DESCRIPTOR_HEADER_20_VERSION) {
		status = devinit_get_fll_device_table_v20(g, pfllobjs);
		goto done;
	}

	pboardobjgrp->pmudatainit  = clk_fll_devgrp_pmudatainit_super;
	pboardobjgrp->pmudatainstget  = clk_fll_devgrp_pmudata_instget;
	pboardobjgrp->pmustatusinstget  = clk_fll_devgrp_pmustatus_instget;

	if (fll_desc_table_header_sz.size >= FLL_DESCRIPTOR_HEADER_10_SIZE_7) {
		desctablesize = FLL_DESCRIPTOR_HEADER_10_SIZE_7;
	} else {
		if (fll_desc_table_header_sz.size ==
				FLL_DESCRIPTOR_HEADER_10_SIZE_6) {
			desctablesize = FLL_DESCRIPTOR_HEADER_10_SIZE_6;
		} else {
			nvgpu_err(g, "Invalid FLL_DESCRIPTOR_HEADER size");
			return -EINVAL;
		}
	}

	nvgpu_memcpy((u8 *)&fll_desc_table_header, fll_table_ptr,
		desctablesize);

	pfllobjs->max_min_freq_mhz =
			fll_desc_table_header.max_min_freq_mhz;
	pfllobjs->freq_margin_vfe_idx =
			fll_desc_table_header.freq_margin_vfe_idx;

	/* Read table entries*/
	fll_tbl_entry_ptr = fll_table_ptr + desctablesize;
	for (index = 0; index < fll_desc_table_header.entry_count; index++) {
		u32 fll_id;

		nvgpu_memcpy((u8 *)&fll_desc_table_entry, fll_tbl_entry_ptr,
				sizeof(struct fll_descriptor_entry_10));
		if (fll_desc_table_entry.fll_device_type ==
				CTRL_CLK_FLL_TYPE_DISABLED)
			continue;

		fll_id = fll_desc_table_entry.fll_device_id;

		if ((u8)fll_desc_table_entry.vin_idx_logic !=
				CTRL_CLK_VIN_ID_UNDEFINED) {
			pvin_dev = clk_get_vin_from_index(pvinobjs,
					(u8)fll_desc_table_entry.vin_idx_logic);
			if (pvin_dev == NULL) {
				return -EINVAL;
			} else {
				pvin_dev->flls_shared_mask |= BIT32(fll_id);
			}
		} else {
			nvgpu_err(g, "Invalid Logic ID");
			return -EINVAL;
		}

		fll_dev_data.lut_device.vselect_mode =
			BIOS_GET_FIELD(u8, fll_desc_table_entry.lut_params,
				NV_FLL_DESC_LUT_PARAMS_VSELECT);

		if ((u8)fll_desc_table_entry.vin_idx_sram !=
				CTRL_CLK_VIN_ID_UNDEFINED) {
			pvin_dev = clk_get_vin_from_index(pvinobjs,
					(u8)fll_desc_table_entry.vin_idx_sram);
			if (pvin_dev == NULL) {
				return -EINVAL;
			} else {
				pvin_dev->flls_shared_mask |= BIT32(fll_id);
			}
		} else {
			/* Make sure VSELECT mode is set correctly to _LOGIC*/
			if (fll_dev_data.lut_device.vselect_mode !=
					CTRL_CLK_FLL_LUT_VSELECT_LOGIC) {
				return -EINVAL;
			}
		}

		fll_dev_data.super.type =
			(u8)fll_desc_table_entry.fll_device_type;
		fll_dev_data.id = (u8)fll_desc_table_entry.fll_device_id;
		fll_dev_data.mdiv = BIOS_GET_FIELD(u8,
			fll_desc_table_entry.fll_params,
				NV_FLL_DESC_FLL_PARAMS_MDIV);
		fll_dev_data.input_freq_mhz =
			(u16)fll_desc_table_entry.ref_freq_mhz;
		fll_dev_data.min_freq_vfe_idx =
			(u8)fll_desc_table_entry.min_freq_vfe_idx;
		fll_dev_data.freq_ctrl_idx = CTRL_BOARDOBJ_IDX_INVALID;

		vbios_domain = U32(fll_desc_table_entry.clk_domain) &
				U32(NV_PERF_DOMAIN_4X_CLOCK_DOMAIN_MASK);
		fll_dev_data.clk_domain =
				clk_get_vbios_clk_domain(vbios_domain);

		fll_dev_data.rail_idx_for_lut = 0;
		fll_dev_data.vin_idx_logic =
			(u8)fll_desc_table_entry.vin_idx_logic;
		fll_dev_data.vin_idx_sram =
			(u8)fll_desc_table_entry.vin_idx_sram;
		fll_dev_data.b_skip_pldiv_below_dvco_min =
			BIOS_GET_FIELD(bool, fll_desc_table_entry.fll_params,
			NV_FLL_DESC_FLL_PARAMS_SKIP_PLDIV_BELOW_DVCO_MIN);
		fll_dev_data.lut_device.hysteresis_threshold =
			BIOS_GET_FIELD(u16, fll_desc_table_entry.lut_params,
			NV_FLL_DESC_LUT_PARAMS_HYSTERISIS_THRESHOLD);
		fll_dev_data.regime_desc.regime_id =
			CTRL_CLK_FLL_REGIME_ID_FFR;
		fll_dev_data.regime_desc.fixed_freq_regime_limit_mhz =
			(u16)fll_desc_table_entry.ffr_cutoff_freq_mhz;
		if (fll_desc_table_entry.fll_device_type == 0x1U) {
			fll_dev_data.regime_desc.target_regime_id_override = 0U;
			fll_dev_data.b_dvco_1x = false;
		} else {
			fll_dev_data.regime_desc.target_regime_id_override =
				CTRL_CLK_FLL_REGIME_ID_FFR;
			fll_dev_data.b_dvco_1x = true;
		}

		/*construct fll device*/
		pfll_dev = construct_fll_device(g, (void *)&fll_dev_data);

		status = boardobjgrp_objinsert(&pfllobjs->super.super,
				(struct pmu_board_obj *)pfll_dev, (u8)index);
		fll_tbl_entry_ptr += fll_desc_table_header.entry_size;
	}

done:
	nvgpu_log_info(g, " done status %x", status);
	return status;
}

static u16 clkNafllGetRefClkDivider(
		struct gk20a *g, u8 nafllId)
{
	u16 pdiv;

	switch (nafllId) {
	case NV2080_CTRL_CLK_NAFLL_ID_SYS:
	case NV2080_CTRL_CLK_NAFLL_ID_NVD:
	case NV2080_CTRL_CLK_NAFLL_ID_UPROC:
	case NV2080_CTRL_CLK_NAFLL_ID_GPC0:
	case NV2080_CTRL_CLK_NAFLL_ID_GPC1:
	case NV2080_CTRL_CLK_NAFLL_ID_GPC2:
	case NV2080_CTRL_CLK_NAFLL_ID_GPC3:
	case NV2080_CTRL_CLK_NAFLL_ID_GPCS:
	{
		pdiv = 1U;
		break;
	}
	default:
	{
		nvgpu_err(g, "Unsupported NAFLL ID = %u\n", nafllId);
		pdiv = 0;
		break;
	}
	}
	return pdiv;
}

/* VBIOS NAFLL HAL ID to API ID lookup structure */
static const struct
{
	u8 vbiosNafllId;
	u8 apiNafllId;
} vbiosToApiNafllIdHals[NV_VBIOS_NAFLL_MAX_DEVICES] = {
	{NV_VBIOS_HAL_V3_NAFLL_DEVICE_ID_SYS,     NV2080_CTRL_CLK_NAFLL_ID_SYS},
	{NV_VBIOS_HAL_V3_NAFLL_DEVICE_ID_GPC0,    NV2080_CTRL_CLK_NAFLL_ID_GPC0},
	{NV_VBIOS_HAL_V3_NAFLL_DEVICE_ID_GPC1,    NV2080_CTRL_CLK_NAFLL_ID_GPC1},
	{NV_VBIOS_HAL_V3_NAFLL_DEVICE_ID_GPC2,    NV2080_CTRL_CLK_NAFLL_ID_GPC2},
	{NV_VBIOS_HAL_V3_NAFLL_DEVICE_ID_GPC3,    NV2080_CTRL_CLK_NAFLL_ID_GPC3},
	{NV_VBIOS_HAL_V3_NAFLL_DEVICE_ID_GPCS,    NV2080_CTRL_CLK_NAFLL_ID_GPCS},
	{NV_VBIOS_HAL_V3_NAFLL_DEVICE_ID_NVD,     NV2080_CTRL_CLK_NAFLL_ID_NVD},
	{NV_VBIOS_HAL_V3_NAFLL_DEVICE_ID_UPROC,   NV2080_CTRL_CLK_NAFLL_ID_UPROC},
};

static s32 devinitTranslateNafllIdVbiosToApi(struct gk20a *g,
				u8 vbiosNafllId, u8 *pApiNafllId)
{
	if (vbiosNafllId >= NV_VBIOS_NAFLL_MAX_DEVICES) {
		nvgpu_err(g,
			"Invalid VBIOS NAFLL ID 0x%02x.", vbiosNafllId);
		return -EINVAL;
	}
	*pApiNafllId = vbiosToApiNafllIdHals[vbiosNafllId].apiNafllId;

	return 0;
}

static s32 devinit_get_fll_device_table_v20(struct gk20a *g,
		struct clk_avfs_fll_objs *pfllobjs)
{
	s32 status = 0;
	u8 *fll_table_ptr = NULL;
	struct fll_descriptor_header_20 fll_desc_table_header = { 0 };
	struct fll_descriptor_entry_20 fll_desc_table_entry = { 0 };
	u8 *fll_tbl_entry_ptr = NULL;
	u32 index = 0;
	struct clk_avfs_fll_objs *pnafll_grp = pfllobjs;
	struct nafll_device_v35 *pfll_dev = NULL;
	struct clk_adc_device *padc_dev = NULL;
	u32 gpcMask;
	bool bSkip;
	u32 gpcSkip;
	u32 api_clk_domain = CTRL_CLK_CLK_DOMAIN_INDEX_INVALID;
	struct nvgpu_avfsvinobjs *pvinobjs = g->pmu->clk_pmu->avfs_vinobjs;
	struct boardobjgrp *pboardobjgrp = &(g->pmu->clk_pmu->avfs_fllobjs->super.super);
	union {
		struct pmu_board_obj			boardObj;
		struct nafll_device				nafllDevice;
		struct nafll_device_v35			nafllDeviceV35;
	} nafllDescData = { 0 };

	nvgpu_pmu_dbg(g, " ");

	fll_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			  nvgpu_bios_get_bit_token(g, NVGPU_BIOS_CLOCK_TOKEN),
							FLL_TABLE);
	if (fll_table_ptr == NULL) {
		status = -1;
		goto done;
	}
	/* Sanity checks:
	 * We have a dependency on
	 * a Clock Domain boardobj callback,
	 * and an ADC boardobj callback,
	 * ensure they are not invalid.
	 */
	if (g->pmu->clk_pmu->clk_domainobjs_50 == NULL ||
		g->pmu->clk_pmu->clk_domainobjs_50->super.super.super.objgetbyidx == NULL) {
		nvgpu_err(g, "Clock domain object invalid. Abort VBIOS parsing.");
		status = -EINVAL;
		goto done;
	}
	if (pvinobjs == NULL ||
		pvinobjs->super.super.objgetbyidx == NULL) {
		nvgpu_err(g, "ADC object invalid. Abort VBIOS parsing.");
		status = -EINVAL;
		goto done;
	}

	pboardobjgrp->classType = 1;

	pboardobjgrp->pmudatainit  = clk_fll_devgrp_pmudatainit_super_v35;
	pboardobjgrp->pmudatainstget  = clk_fll_devgrp_pmudata_instget_v35;
	pboardobjgrp->pmustatusinstget  = clk_fll_devgrp_pmustatus_instget_v35;

	nvgpu_memcpy((u8 *)&fll_desc_table_header, fll_table_ptr,
		sizeof(struct fll_descriptor_header_20));

	/* sanity tests of v20 header */
	if (fll_desc_table_header.version != FLL_DESCRIPTOR_HEADER_20_VERSION) {
		nvgpu_err(g,
			"NAFLL version 0x%02x VBIOS Table header not supported.",
			fll_desc_table_header.version);
		status = -EINVAL;
		goto done;
	}
	if (fll_desc_table_header.header_size < FLL_DESCRIPTOR_HEADER_20_SIZE_7) {
		nvgpu_err(g,
			"NAFLL version 20 VBIOS Header size %d not supported, too small.",
			fll_desc_table_header.header_size);
		status = -EINVAL;
		goto done;
	}
	if (fll_desc_table_header.entry_size < FLL_DESCRIPTOR_ENTRY_20_SIZE) {
		nvgpu_err(g,
			"NAFLL version 20 VBIOS Entry size %d not supported, too small.",
			fll_desc_table_header.header_size);
		status = -EINVAL;
		goto done;
	}
	if (fll_desc_table_header.fll_dev_id != NV_FLL_DEVICE_TABLE_HAL_V3_GB20Y) {
		nvgpu_err(g,
			"NAFLL HAL ID 0x%02x in VBIOS Table Header not supported.",
			fll_desc_table_header.fll_dev_id);
		status = -EINVAL;
		goto done;
	}

	/* unicast & broadcast masks to zero.*/
	status = boardobjgrpmask_e32_init(&pnafll_grp->unicast_nafll_dev_mask,
			NULL);
	if (status != 0) {
		nvgpu_err(g, "boardobjgrpmask_e32_init failed err=%d", status);
		goto done;
	}
	status = boardobjgrpmask_e32_init(&pnafll_grp->broadcast_nafll_dev_mask,
			NULL);
	if (status != 0) {
		nvgpu_err(g, "boardobjgrpmask_e32_init failed err=%d", status);
		goto done;
	}

	/* VBIOS deprecated the maxDVCOMinFreqMHz.
	 * There is no more worst-case DVCO Min frequency being tracked anymore,
	 * this is done using VFE equation taking PVT into account.
	 */
	pnafll_grp->max_min_freq_mhz = 0;

	pnafll_grp->b_die1_all_gpc_floorswept = false;

	/*
	 * Get GPC Floorsweeping bit map.
	 * Need to skip GPC NAFLL VBIOS entries
	 * for floorswept GPCs.
	 */
	gpcMask = ~nvgpu_readl(g, fuse_status_opt_gpc_r());
	nvgpu_pmu_dbg(g, "GPC Floorsweep mask - 0x%x", gpcMask);

	fll_tbl_entry_ptr = fll_table_ptr + fll_desc_table_header.header_size;
	/* Parse each of the table entries */
	for (index = 0; index < fll_desc_table_header.entry_count; index++,
			fll_tbl_entry_ptr += fll_desc_table_header.entry_size) {
		u8 api_nafll_id = NV2080_CTRL_CLK_NAFLL_ID_UNDEFINED;
		/* To avoid MISRA violation,
		 * this is a single point of exit
		 * from this for loop in the case
		 * of an error is encountered.
		 */
		if (status != 0)
			break;

		nvgpu_memcpy((u8 *)&fll_desc_table_entry, fll_tbl_entry_ptr,
			sizeof(struct fll_descriptor_entry_20));

		if (fll_desc_table_entry.fll_device_type == FLL_DEVICE_TYPE_DISABLED)
			continue;

		/* Translate VBIOS device ID (index) to API device ID */
		status = devinitTranslateNafllIdVbiosToApi(g, index, &api_nafll_id);
		if (status != 0)
			continue; /* to avoid MISRA violation, exit at top of loop */

		/* Skip Floorswept GPCs */
		bSkip = false;
		gpcSkip = 0xffffffff;
		if (api_nafll_id >= NV2080_CTRL_CLK_NAFLL_ID_GPC0 &&
			api_nafll_id <= NV2080_CTRL_CLK_NAFLL_ID_GPC3) {
			switch (api_nafll_id) {
			case NV2080_CTRL_CLK_NAFLL_ID_GPC0:
			{
				bSkip = ((gpcMask & BIT32(0)) == 0);
				gpcSkip = 0;
				break;
			}
			case NV2080_CTRL_CLK_NAFLL_ID_GPC1:
			{
				bSkip = ((gpcMask & BIT32(1)) == 0);
				gpcSkip = 1;
				break;
			}
			case NV2080_CTRL_CLK_NAFLL_ID_GPC2:
			{
				bSkip = ((gpcMask & BIT32(2)) == 0);
				gpcSkip = 2;
				break;
			}
			case NV2080_CTRL_CLK_NAFLL_ID_GPC3:
			{
				bSkip = ((gpcMask & BIT32(3)) == 0);
				gpcSkip = 3;
				break;
			}
			/* No default - so that coverity will not complain about dead code. */
			}
			/* Skip the construction of device entry
			 * if GPC is floorswept.
			 */
			if (bSkip) {
				nvgpu_pmu_dbg(g, "GPC %d is Floorswept - skip.", gpcSkip);
				continue;
			}
		}
		/*
		 * Sanity check that the LOGIC ADC device is valid for the
		 * unicast NAFLL devices, and invalid for BCAST-type NAFLL devices.
		 * Update the mask of NAFLL devices sharing that ADC as
		 * appropriate. The GPCS NAFLL is the only BCAST-type on GB20Y.
		 */
		if (api_nafll_id == NV2080_CTRL_CLK_NAFLL_ID_GPCS) {
			if (fll_desc_table_entry.adc_idx != NV_ADC_DEVICE_ID_INVALID) {
				nvgpu_err(g,
					"NAFLL ID 0x%02x should have invalid ADC object idx, but valid found. VBIOS Entry: %d. Bad ADC idx=%d",
					api_nafll_id, index, fll_desc_table_entry.adc_idx);
				status = -EINVAL;
				/* to avoid MISRA violation, exit at top of loop */
				continue;
			}
		} else if (fll_desc_table_entry.adc_idx != NV_ADC_DEVICE_ID_INVALID) {
			padc_dev = (struct clk_adc_device *)clk_get_vin_from_index(pvinobjs,
					fll_desc_table_entry.adc_idx);
			if (padc_dev == NULL) {
				nvgpu_err(g, "ADC object for VBIOS NAFLL entry %d not found!",
							index);
				status = -EINVAL;
				/* to avoid MISRA violation, exit at top of loop */
				continue;
			} else {
				padc_dev->naflls_shared_mask |= BIT32(api_nafll_id);
			}
		}

		switch (fll_desc_table_entry.fll_device_type) {
		case FLL_DEVICE_TYPE_V35:
		{
			nafllDescData.boardObj.type = NV2080_CTRL_CLK_NAFLL_DEVICE_TYPE_V35;
			nafllDescData.nafllDevice.b_skip_pldiv_below_dvco_min =
				((fll_desc_table_entry.dvco_params
				& NV_FLL_DESC_V20_DVCO_PARAMS_SKIP_PLDIV_MASK) != 0U);

			nafllDescData.nafllDevice.dvco_min_freq_vfe_idx = BIOS_GET_FIELD(
				u16, fll_desc_table_entry.dvco_params,
				NV_FLL_DESC_V20_DVCO_PARAMS_F_MIN_VFE_IDX);

			nafllDescData.nafllDeviceV35.b_hw_pldiv_en =
				((fll_desc_table_entry.dvco_params
				& NV_FLL_DESC_V20_DVCO_PARAMS_F_ACCURACY_MASK) != 0U);

			break;
		}
		case FLL_DEVICE_TYPE_V35_SECURE_V10:
		{
			nafllDescData.boardObj.type =
				NV2080_CTRL_CLK_CLK_NAFLL_DEVICE_TYPE_V35_SECURE_V10;

			nafllDescData.nafllDevice.b_skip_pldiv_below_dvco_min = false;

			nafllDescData.nafllDevice.dvco_min_freq_vfe_idx = BIOS_GET_FIELD(
				u16, fll_desc_table_entry.dvco_params,
				NV_FLL_DESC_V20_DVCO_PARAMS_F_MIN_VFE_IDX);

			break;
		}
		default:
		{
			nvgpu_err(g,
			"Unsupported NAFLL device type: 0x%x",
			fll_desc_table_entry.fll_device_type);
			status = -EINVAL;
			continue; /* to avoid MISRA violation, exit at top of loop */
		}
		}

		// Common parameters across all types of NAFLL
		nafllDescData.nafllDevice.id = api_nafll_id;
		nafllDescData.nafllDevice.mdiv = BIOS_GET_FIELD(u8,
					fll_desc_table_entry.fll_params,
					NV_FLL_DESC_V20_FLL_PARAMS_MDIV);
		nafllDescData.nafllDevice.input_ref_freq_mhz =
					BIOS_GET_FIELD(u16,
					fll_desc_table_entry.fll_params,
					NV_FLL_DESC_V20_FLL_PARAMS_PDIV_REF_CLK_F_MHZ);
		nafllDescData.nafllDevice.input_ref_div_val =
					clkNafllGetRefClkDivider(g, api_nafll_id);
		nafllDescData.nafllDevice.b_multistep_pldiv_switch_en = true;

		/* Convert the VBIOS clock domain index to API clock domain*/
		status = clkClkDomainGetApiDomainByIndex(g,
					&api_clk_domain, fll_desc_table_entry.clk_domain);

		if (status != 0) {
			nvgpu_err(g,
			"Clock domain object not found. Clk index: 0x%x",
			fll_desc_table_entry.clk_domain);
			continue; /* to avoid MISRA violation, exit at top of loop */
		}
		nafllDescData.nafllDevice.clk_domain = api_clk_domain;
		nafllDescData.nafllDevice.adc_idx_logic = fll_desc_table_entry.adc_idx;

		/* Initialize to _INVALID rail index since this will be set
		 * correctly during Nafll state load
		 */
		nafllDescData.nafllDevice.rail_idx_for_lut = CTRL_VOLT_DEVICE_INDEX_INVALID;

		/* SRAM ADC does not apply for V35 NAFLL and later */
		nafllDescData.nafllDevice.adc_idx_sram = CTRL_BOARDOBJ_IDX_INVALID;

		/* VBIOS no longer initializes the CLFC index for NAFLL,
		 * it will come indirectly from the clock domain.
		 */
		nafllDescData.nafllDevice.freq_ctrl_idx = CTRL_BOARDOBJ_IDX_INVALID;

		/* Initialize the regime desc params.
		 * Hardcode the regime id to _INVALID for now, since this is
		 * going to be read from the HW state as part of
		 * the state load code.
		 */
		nafllDescData.nafllDevice.regime_desc.regime_id = CTRL_BOARDOBJ_IDX_INVALID;
		nafllDescData.nafllDevice.regime_desc.fixed_freq_regime_limit_mhz =
					fll_desc_table_entry.ffr_cutoff_freq_mhz;

		// DVCO1X is true by default from HOPPER+.
		nafllDescData.nafllDevice.b_dvco_1x = true;

		/* construct nafll device and insert into GRP */
		pfll_dev = construct_nafll_device_v35(g, (void *)&nafllDescData);
		if (pfll_dev == NULL) {
			nvgpu_err(g,
			"Unable to construct nafll boardobj for Entry %d",
			index);
			status = -EINVAL;
			/* to avoid MISRA violation, exit at top of loop */
			continue;
		}
		status = boardobjgrp_objinsert(&pfllobjs->super.super,
				(struct pmu_board_obj *)pfll_dev, (u8)index);
		if (status != 0) {
			nvgpu_err(g,
				"Inserting nafll device object to boardObjGrp failed for Entry: %d",
				index);
			/* to avoid MISRA violation, exit at top of loop */
			continue;
		}

		/* Set the object within the GRP mask
		 * of unicast or broadcast NAFLLs.
		 */
		if ((api_nafll_id >= NV2080_CTRL_CLK_NAFLL_ID_GPC0) &&
			(api_nafll_id <= NV2080_CTRL_CLK_NAFLL_ID_GPC3)) {
			/* These NAFLLs are unicast */
			status = nvgpu_boardobjgrpmask_bit_set(
				&pnafll_grp->unicast_nafll_dev_mask.super,
				(u8)index);
			if (status != 0) {
				nvgpu_err(g,
				"Setting unicast mask failed. Entry index: 0x%x",
				index);
				continue; /* to avoid MISRA violation, exit at top of loop */
			}
		} else if (api_nafll_id == NV2080_CTRL_CLK_NAFLL_ID_GPCS) {
			/* This NAFLL is broadcast */
			status = nvgpu_boardobjgrpmask_bit_set(
				&pnafll_grp->broadcast_nafll_dev_mask.super,
				(u8)index);
			if (status != 0) {
				nvgpu_err(g,
				"Setting broadcast mask failed. Entry index: 0x%x",
				index);
				continue; /* to avoid MISRA violation, exit at top of loop */
			}
		} else {
			/* These NAFLLs are simply 1:1. They are added to both masks */
			status = nvgpu_boardobjgrpmask_bit_set(
				&pnafll_grp->unicast_nafll_dev_mask.super,
				(u8)index);
			if (status != 0) {
				nvgpu_err(g,
				"Setting unicast mask failed. Entry index: 0x%x",
				index);
				continue; /* to avoid MISRA violation, exit at top of loop */
			}
			status = nvgpu_boardobjgrpmask_bit_set(
				&pnafll_grp->broadcast_nafll_dev_mask.super,
				(u8)index);
			if (status != 0) {
				nvgpu_err(g,
				"Setting broadcast mask failed. Entry index: 0x%x",
				index);
				continue; /* to avoid MISRA violation, exit at top of loop */
			}
		}
		status = clkNafllDeviceStateLoad(g, (struct nafll_device *)pfll_dev);
	}

done:
	nvgpu_pmu_dbg(g, " Done status %x", status);
	return status;
}

static u32 clk_get_vbios_clk_domain(u32 vbios_domain)
{
	if (vbios_domain == 0U) {
		return CTRL_CLK_DOMAIN_GPCCLK;
	} else if (vbios_domain == 1U) {
		return CTRL_CLK_DOMAIN_XBARCLK;
	} else if (vbios_domain == 3U) {
		return CTRL_CLK_DOMAIN_SYSCLK;
	} else if (vbios_domain == 5U) {
		return CTRL_CLK_DOMAIN_NVDCLK;
	} else if (vbios_domain == 9U) {
		return CTRL_CLK_DOMAIN_HOSTCLK;
	} else {
		return 0;
	}
}

static int lutbroadcastslaveregister(struct gk20a *g,
		struct clk_avfs_fll_objs *pfllobjs, struct fll_device *pfll,
		struct fll_device *pfll_slave)
{
	(void)g;
	(void)pfllobjs;

	if (pfll->clk_domain != pfll_slave->clk_domain) {
		return -EINVAL;
	}

	return nvgpu_boardobjgrpmask_bit_set(&pfll->
		lut_prog_broadcast_slave_mask.super,
		pmu_board_obj_get_idx(pfll_slave));
}

static struct fll_device *construct_fll_device(struct gk20a *g,
		void *pargs)
{
	struct pmu_board_obj *obj = NULL;
	struct fll_device *pfll_dev;
	struct fll_device *board_obj_fll_ptr = NULL;
	int status;

	nvgpu_log_info(g, " ");

	board_obj_fll_ptr = nvgpu_kzalloc(g, sizeof(struct fll_device));
	if (board_obj_fll_ptr == NULL) {
		return NULL;
	}
	obj = (struct pmu_board_obj *)(void *)board_obj_fll_ptr;

	status = pmu_board_obj_construct_super(g, obj, pargs);
	if (status != 0) {
		return NULL;
	}

	pfll_dev = (struct fll_device *)pargs;
	obj->pmudatainit  = fll_device_init_pmudata_super;
	board_obj_fll_ptr->lut_broadcast_slave_register =
		lutbroadcastslaveregister;
	board_obj_fll_ptr->id = pfll_dev->id;
	board_obj_fll_ptr->mdiv = pfll_dev->mdiv;
	board_obj_fll_ptr->rail_idx_for_lut = pfll_dev->rail_idx_for_lut;
	board_obj_fll_ptr->input_freq_mhz = pfll_dev->input_freq_mhz;
	board_obj_fll_ptr->clk_domain = pfll_dev->clk_domain;
	board_obj_fll_ptr->vin_idx_logic = pfll_dev->vin_idx_logic;
	board_obj_fll_ptr->vin_idx_sram = pfll_dev->vin_idx_sram;
	board_obj_fll_ptr->min_freq_vfe_idx =
		pfll_dev->min_freq_vfe_idx;
	board_obj_fll_ptr->freq_ctrl_idx = pfll_dev->freq_ctrl_idx;
	board_obj_fll_ptr->b_skip_pldiv_below_dvco_min =
		pfll_dev->b_skip_pldiv_below_dvco_min;
	nvgpu_memcpy((u8 *)&board_obj_fll_ptr->lut_device,
		(u8 *)&pfll_dev->lut_device,
		sizeof(struct nv_pmu_clk_lut_device_desc));
	nvgpu_memcpy((u8 *)&board_obj_fll_ptr->regime_desc,
		(u8 *)&pfll_dev->regime_desc,
		sizeof(struct nv_pmu_clk_regime_desc));
	board_obj_fll_ptr->b_dvco_1x=pfll_dev->b_dvco_1x;

	status = boardobjgrpmask_e32_init(
		&board_obj_fll_ptr->lut_prog_broadcast_slave_mask, NULL);
	if (status != 0) {
		nvgpu_err(g, "boardobjgrpmask_e32_init failed err=%d", status);
		status = obj->destruct(obj);
		if (status != 0)
			nvgpu_err(g, "destruct failed err=%d", status);

		return NULL;
	}

	nvgpu_log_info(g, " Done");

	return (struct fll_device *)(void *)obj;
}

static s32 fll_device_init_pmudata_super(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;
	struct fll_device *pfll_dev;
	struct nv_pmu_clk_clk_fll_device_boardobj_set *perf_pmu_data;

	nvgpu_log_info(g, " ");

	status = pmu_board_obj_pmu_data_init_super(g, obj, pmu_obj);
	if (status != 0) {
		return status;
	}

	pfll_dev = (struct fll_device *)(void *)obj;
	perf_pmu_data = (struct nv_pmu_clk_clk_fll_device_boardobj_set *)
		pmu_obj;

	perf_pmu_data->id = pfll_dev->id;
	perf_pmu_data->mdiv = pfll_dev->mdiv;
	perf_pmu_data->rail_idx_for_lut = pfll_dev->rail_idx_for_lut;
	perf_pmu_data->input_freq_mhz = pfll_dev->input_freq_mhz;
	perf_pmu_data->vin_idx_logic = pfll_dev->vin_idx_logic;
	perf_pmu_data->vin_idx_sram = pfll_dev->vin_idx_sram;
	perf_pmu_data->clk_domain = pfll_dev->clk_domain;
	perf_pmu_data->min_freq_vfe_idx =
		pfll_dev->min_freq_vfe_idx;
	perf_pmu_data->freq_ctrl_idx = pfll_dev->freq_ctrl_idx;
	perf_pmu_data->b_skip_pldiv_below_dvco_min =
			pfll_dev->b_skip_pldiv_below_dvco_min;
	perf_pmu_data->b_dvco_1x = pfll_dev->b_dvco_1x;
	nvgpu_memcpy((u8 *)&perf_pmu_data->lut_device,
		(u8 *)&pfll_dev->lut_device,
		sizeof(struct nv_pmu_clk_lut_device_desc));
	nvgpu_memcpy((u8 *)&perf_pmu_data->regime_desc,
		(u8 *)&pfll_dev->regime_desc,
		sizeof(struct nv_pmu_clk_regime_desc));

	status = nvgpu_boardobjgrpmask_export(
		&pfll_dev->lut_prog_broadcast_slave_mask.super,
		pfll_dev->lut_prog_broadcast_slave_mask.super.bitcount,
		&perf_pmu_data->lut_prog_broadcast_slave_mask.super);

	nvgpu_log_info(g, " Done");

	return status;
}

static struct nafll_device_v35 *construct_nafll_device_v35(struct gk20a *g,
		void *pargs)
{
	struct pmu_board_obj *obj = NULL;
	struct nafll_device_v35 *pfll_dev_v35;
	struct nafll_device_v35 *board_obj_fll_ptr = NULL;
	struct nafll_device *pnafll_dev = NULL;
	struct nafll_device *pobj_nafll = NULL;
	s32 status = 0;

	nvgpu_log_info(g, " ");

	board_obj_fll_ptr = nvgpu_kzalloc(g, sizeof(struct nafll_device_v35));
	if (board_obj_fll_ptr == NULL) {
		nvgpu_err(g, "Unable to allocate memory.");
		return NULL;
	}

	obj = (struct pmu_board_obj *)board_obj_fll_ptr;
	/* construct super class and insert in the object list. */
	status = pmu_board_obj_construct_super(g, obj, pargs);
	if (status != 0) {
		nvgpu_kfree(g, board_obj_fll_ptr);
		return NULL;
	}

	pfll_dev_v35 = (struct nafll_device_v35 *)pargs;
	obj->pmudatainit = nafll_v35_device_init_pmudata_super;

	/* copy boardobj data */
	pnafll_dev = &pfll_dev_v35->super.super.super;
	pobj_nafll = &board_obj_fll_ptr->super.super.super;

	pobj_nafll->id = pnafll_dev->id;
	pobj_nafll->mdiv = pnafll_dev->mdiv;
	pobj_nafll->input_ref_freq_mhz = pnafll_dev->input_ref_freq_mhz;
	pobj_nafll->input_ref_div_val = pnafll_dev->input_ref_div_val;
	pobj_nafll->clk_domain = pnafll_dev->clk_domain;
	pobj_nafll->adc_idx_logic = pnafll_dev->adc_idx_logic;
	pobj_nafll->adc_idx_sram = pnafll_dev->adc_idx_sram;
	pobj_nafll->rail_idx_for_lut = pnafll_dev->rail_idx_for_lut;
	pobj_nafll->lut_device = pnafll_dev->lut_device;
	pobj_nafll->regime_desc = pnafll_dev->regime_desc;
	pobj_nafll->dvco_min_freq_vfe_idx = pnafll_dev->dvco_min_freq_vfe_idx;
	pobj_nafll->freq_ctrl_idx = pnafll_dev->freq_ctrl_idx;
	pobj_nafll->b_skip_pldiv_below_dvco_min = pnafll_dev->b_skip_pldiv_below_dvco_min;
	pobj_nafll->b_multistep_pldiv_switch_en = pnafll_dev->b_multistep_pldiv_switch_en;
	pobj_nafll->b_dvco_1x = pnafll_dev->b_dvco_1x;

	board_obj_fll_ptr->b_hw_pldiv_en  = pfll_dev_v35->b_hw_pldiv_en;


	status = boardobjgrpmask_e32_init(
		&pobj_nafll->lut_prog_bcast_secndary_mask, NULL);
	if (status != 0) {
		nvgpu_err(g, "boardobjgrpmask_e32_init failed err=%d", status);
		status = obj->destruct(obj);

		if (status != 0)
			nvgpu_err(g, "destruct failed err=%d", status);

		return NULL;
	}

	nvgpu_log_info(g, " Done.");

	return (struct nafll_device_v35 *)obj;
}

static s32 nafll_v35_device_init_pmudata_super(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;
	struct nafll_device *pfll_dev;
	struct nv_pmu_clk_clk_nafll_device_boardobj_set *perf_pmu_data;
	struct nv_pmu_clk_clk_fll_device_v35_boardobj_set *perf_pmu_data_v35;
	struct nafll_device_v35 *pfll_dev_v35;

	nvgpu_log_info(g, " ");

	status = pmu_board_obj_pmu_data_init_super(g, obj, pmu_obj);
	if (status != 0)
		return status;

	pfll_dev = (struct nafll_device *)obj;
	perf_pmu_data = (struct nv_pmu_clk_clk_nafll_device_boardobj_set *)
		pmu_obj;
	perf_pmu_data_v35 = (struct nv_pmu_clk_clk_fll_device_v35_boardobj_set *)
		pmu_obj;
	pfll_dev_v35 = (struct nafll_device_v35 *)obj;

	perf_pmu_data->id = pfll_dev->id;
	perf_pmu_data->mdiv = pfll_dev->mdiv;
	perf_pmu_data->vin_idx_logic = pfll_dev->adc_idx_logic;
	perf_pmu_data->vin_idx_sram = pfll_dev->adc_idx_sram;
	perf_pmu_data->rail_idx_for_lut = pfll_dev->rail_idx_for_lut;
	perf_pmu_data->input_ref_freq_mhz = pfll_dev->input_ref_freq_mhz;
	perf_pmu_data->input_ref_div_value = pfll_dev->input_ref_div_val;
	perf_pmu_data->clk_domain = pfll_dev->clk_domain;
	perf_pmu_data->dvco_min_freq_vfe_idx =
		pfll_dev->dvco_min_freq_vfe_idx;
	perf_pmu_data->freq_ctrl_idx = pfll_dev->freq_ctrl_idx;
	perf_pmu_data->b_skip_pldiv_below_dvco_min =
			pfll_dev->b_skip_pldiv_below_dvco_min;
	perf_pmu_data->b_mulltistep_pldiv_switch_enable =
			pfll_dev->b_multistep_pldiv_switch_en;
	perf_pmu_data->b_dvco_1x = pfll_dev->b_dvco_1x;

	perf_pmu_data->lut_device = pfll_dev->lut_device;
	perf_pmu_data->regime_desc = pfll_dev->regime_desc;

	perf_pmu_data_v35->b_hw_pldiv_enable = pfll_dev_v35->b_hw_pldiv_en;

	status = nvgpu_boardobjgrpmask_export(
		&pfll_dev->lut_prog_bcast_secndary_mask.super,
		pfll_dev->lut_prog_bcast_secndary_mask.super.bitcount,
		&perf_pmu_data->lut_prog_bcast_secndary_mask.super);

	nvgpu_log_info(g, " Done");

	return status;
}

u8 nvgpu_pmu_clk_fll_get_fmargin_idx(struct gk20a *g)
{
	struct clk_avfs_fll_objs *pfllobjs =  g->pmu->clk_pmu->avfs_fllobjs;
	u8 fmargin_idx;

	fmargin_idx = pfllobjs->freq_margin_vfe_idx;
	if (fmargin_idx == 255U) {
		return 0;
	}
	return fmargin_idx;
}

u16 nvgpu_pmu_clk_fll_get_min_max_freq(struct gk20a *g)
{
	if ((g->pmu->clk_pmu != NULL) &&
			(g->pmu->clk_pmu->avfs_fllobjs != NULL)) {
		return (g->pmu->clk_pmu->avfs_fllobjs->max_min_freq_mhz);
	}
	return 0;
}

s32 nvgpu_pmu_clk_domains_fll_set_regime_50(struct gk20a *g,
	u8 regime, u32 api_clk_domain)
{
	struct nafll_device *pNafllDevice;
	struct nvgpu_clk_pmupstate *pclk;
	u16 idx;
	s32 status = -EINVAL;
	bool regime_changed = false;

	// Sanity checks
	if (g == NULL) {
		nvgpu_err(g, "g is NULL");
		goto done;
	}

	nvgpu_log_info(g, " ");

	pclk = g->pmu->clk_pmu;
	if (pclk == NULL) {
		nvgpu_err(g, "g->pmu->clk_pmu is NULL");
		goto done;
	}
	if (pclk->clk_domainobjs_50 == NULL) {
		nvgpu_err(g, "clk_domainobjs_50 is NULL");
		goto done;
	}
	if (regime >= CTRL_CLK_FLL_REGIME_ID_MAX) {
		nvgpu_err(g, "regime %d out of range", regime);
		goto done;
	}

	//
	// Loop through the NAFLL devices and check if regime needs to change
	// on NAFLL devices that belong to the api_clk_domain
	BOARDOBJGRP_FOR_EACH(&(pclk->avfs_fllobjs->super.super),
		struct nafll_device *, pNafllDevice, idx) {

		if (pNafllDevice->clk_domain == api_clk_domain) {
			// Check if regime is actually changing
			u8 current_regime = pNafllDevice->regime_desc.target_regime_id_override;

			if (current_regime != regime) {
				pNafllDevice->regime_desc.target_regime_id_override = regime;
				regime_changed = true;

			} else {
				nvgpu_log_info(g, "NAFLL %d regime already set to %d, no change needed",
						idx, regime);
			}
			//
			// we have at least one NAFLL found for the
			// target clkDomain, set status good.
			status = 0;
		}
	}
	if (status != 0) {
		nvgpu_err(g, "Failed to find a NAFLL device for api clkDomain 0x%x",
			api_clk_domain);
		goto done;
	}

	// Only resend to PMU if regime actually changed
	if (regime_changed) {
		// Resend the NAFLL board objects to PMU
		// to set the regime
		nvgpu_log_info(g, "Sending NAFLL board objs to PMU to change regime to %d",
			regime);

			status = clk_fll_pmu_setup(g);
	} else {
		nvgpu_log_info(g, "Regime %d already active for domain 0x%x, skipping PMU setup",
			       regime, api_clk_domain);
	}

	// Todo: Once we have an valid value, end change_seq RPC here.
	// Use same GPC clock freq and voltage as current setting
	// (pto_counter reading).

done:
	return status;
}

s32 clk_fll_init_pmupstate(struct gk20a *g)
{
	/* If already allocated, do not re-allocate */
	if (g->pmu->clk_pmu->avfs_fllobjs != NULL) {
		return 0;
	}

	g->pmu->clk_pmu->avfs_fllobjs = nvgpu_kzalloc(g,
			sizeof(struct clk_avfs_fll_objs));
	if (g->pmu->clk_pmu->avfs_fllobjs == NULL) {
		return -ENOMEM;
	}

	return 0;
}

void clk_fll_free_pmupstate(struct gk20a *g)
{
	/* Clean up PMU memory allocations before freeing the object */
	if (g->pmu->clk_pmu->avfs_fllobjs != NULL)
		nvgpu_boardobjgrp_pmu_cleanup(g, &g->pmu->clk_pmu->avfs_fllobjs->super.super);

	nvgpu_kfree(g, g->pmu->clk_pmu->avfs_fllobjs);
	g->pmu->clk_pmu->avfs_fllobjs = NULL;
}
