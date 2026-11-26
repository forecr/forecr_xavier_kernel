// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bios.h>
#include <nvgpu/bug.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/boardobjgrp_e255.h>
#include <nvgpu/pmu/boardobjgrp_classes.h>
#include <nvgpu/boardobjgrpmask.h>
#include <nvgpu/pmu/clk/clk.h>
#include <nvgpu/pmu/perf.h>
#include <nvgpu/string.h>
#include <nvgpu/pmu/volt.h>
#include <nvgpu/pmu/cmd.h>

#include "ucode_clk_inf.h"
#include "clk_domain.h"
#include "clk_prog.h"
#include "clk.h"

#include "clk_prop_top_rel.h"
#include "clk_vf_rel.h"


static int devinit_get_clocks_table(struct gk20a *g);

// We only support two HAL types.
static struct vbios_clocks_table vbios_clocks_table_hal_clock_entry[] ={
	{
		CLK_TABLE_HAL_ENTRY_GV,
		{
			{ CLKWHICH_GPCCLK,     true,    1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_GRAPHICS, 0x0ffU},
			{ CLKWHICH_XBARCLK,    true,    1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID, 0x0ffU},
			{ CLKWHICH_MCLK,       false,   1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_MEMORY, 0x0ffU},
			{ CLKWHICH_SYSCLK,     true,    1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID, 0x0ffU},
			{ CLKWHICH_HUBCLK,     false,   1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID, 0x0ffU},
			{ CLKWHICH_NVDCLK,     true,    1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID, 0x0ffU},
			{ CLKWHICH_PWRCLK,     false,   1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID, 0x0ffU},
			{ CLKWHICH_DISPCLK,    false,   1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID, 0x0ffU},
			{ CLKWHICH_PCIEGENCLK, false,   1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID, 0x0ffU},
			{ CLKWHICH_HOSTCLK,    true,    1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID, 0x0ffU}
		}
	},
	{
		CLK_TABLE_HAL_ENTRY_GB20Y,
		{
			{ CLKWHICH_GPCCLK,		true,	4, NV2080_CTRL_CLK_PUBLIC_DOMAIN_GRAPHICS, 0},
			{ CLKWHICH_XBARCLK,		true,	1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID, 0x1U},
			{ CLKWHICH_MCLK,		false,	1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID, 0x0ffU},
			{ CLKWHICH_NVDCLK,		true,	1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID, 0x03U},
			{ CLKWHICH_UPROCCLK,	true,	1, NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID, 0x0ffU}
		}
	}
};

static struct nvgpu_clk_vf_points_status *vf_points_status;

s32 devinitClocksTableHALTranslate_1X(struct gk20a *g,
		u8 vbiosClocksHAL,
		struct vbios_clocks_table **pClocksHAL)
{
	s32 status = 0;
	// We only support two HAL types.
	switch (vbiosClocksHAL) {
	case CLK_TABLE_HAL_ENTRY_GV:
	{
		*pClocksHAL = &vbios_clocks_table_hal_clock_entry[0];
		break;
	}
	case CLK_TABLE_HAL_ENTRY_GB20Y:
	{
		*pClocksHAL = &vbios_clocks_table_hal_clock_entry[1];
		break;
	}
	default:
	{
		*pClocksHAL = NULL;
		status = -EINVAL;
		nvgpu_err(g, "Unsupported clock hal id: 0x%x",
				vbiosClocksHAL);
		break;
	}
	}

	return status;
}

u32 clktranslatehalmumsettoapinumset(u32 clkhaldomains)
{
	u32   clkapidomains = 0;

	if ((clkhaldomains & BIT32(CLKWHICH_GPCCLK)) != 0U) {
		clkapidomains |= CTRL_CLK_DOMAIN_GPCCLK;
	}
	if ((clkhaldomains & BIT32(CLKWHICH_XBARCLK)) != 0U) {
		clkapidomains |= CTRL_CLK_DOMAIN_XBARCLK;
	}
	if ((clkhaldomains & BIT32(CLKWHICH_SYSCLK)) != 0U) {
		clkapidomains |= CTRL_CLK_DOMAIN_SYSCLK;
	}
	if ((clkhaldomains & BIT32(CLKWHICH_HUBCLK)) != 0U) {
		clkapidomains |= CTRL_CLK_DOMAIN_HUBCLK;
	}
	if ((clkhaldomains & BIT32(CLKWHICH_HOSTCLK)) != 0U) {
		clkapidomains |= CTRL_CLK_DOMAIN_HOSTCLK;
	}
	if ((clkhaldomains & BIT32(CLKWHICH_GPC2CLK)) != 0U) {
		clkapidomains |= CTRL_CLK_DOMAIN_GPC2CLK;
	}
	if ((clkhaldomains & BIT32(CLKWHICH_XBAR2CLK)) != 0U) {
		clkapidomains |= CTRL_CLK_DOMAIN_XBAR2CLK;
	}
	if ((clkhaldomains & BIT32(CLKWHICH_SYS2CLK)) != 0U) {
		clkapidomains |= CTRL_CLK_DOMAIN_SYS2CLK;
	}
	if ((clkhaldomains & BIT32(CLKWHICH_HUB2CLK)) != 0U) {
		clkapidomains |= CTRL_CLK_DOMAIN_HUB2CLK;
	}
	if ((clkhaldomains & BIT32(CLKWHICH_UPROCCLK)) != 0U) {
		clkapidomains |= CTRL_CLK_DOMAIN_UPROCCLK;
	}
	if ((clkhaldomains & BIT32(CLKWHICH_PCIEGENCLK)) != 0U) {
		clkapidomains |= CTRL_CLK_DOMAIN_PCIEGENCLK;
	}
	if ((clkhaldomains & BIT32(CLKWHICH_MCLK)) != 0U) {
		clkapidomains |= CTRL_CLK_DOMAIN_MCLK;
	}
	if ((clkhaldomains & BIT32(CLKWHICH_NVDCLK)) != 0U) {
		clkapidomains |= CTRL_CLK_DOMAIN_NVDCLK;
	}
	if ((clkhaldomains & BIT32(CLKWHICH_DISPCLK)) != 0U) {
		clkapidomains |= CTRL_CLK_DOMAIN_DISPCLK;
	}

	return clkapidomains;
}

static struct nvgpu_clk_domain *clk_get_clk_domain_from_index(
		struct nvgpu_clk_pmupstate *pclk, u8 idx)
{
	return (struct nvgpu_clk_domain *)(void *)BOARDOBJGRP_OBJ_GET_BY_IDX(
			&(pclk->clk_domainobjs->super.super), idx);
}

static int clk_domains_pmudatainit_3x(struct gk20a *g,
		struct boardobjgrp *pboardobjgrp,
		struct nv_pmu_boardobjgrp_super *pboardobjgrppmu)
{
	struct nv_pmu_clk_clk_domain_10_boardobjgrp_set_header *pset =
		(struct nv_pmu_clk_clk_domain_10_boardobjgrp_set_header *)
		(void *)pboardobjgrppmu;
	struct nvgpu_clk_domains *pdomains =
		(struct nvgpu_clk_domains *)(void *)pboardobjgrp;
	int status = 0;

	status = boardobjgrp_pmudatainit_e32(g, pboardobjgrp, pboardobjgrppmu);
	if (status != 0) {
		nvgpu_err(g,
			  "error updating pmu boardobjgrp for clk domain 0x%x",
			  status);
		goto done;
	}

	pset->vbios_domains = pdomains->vbios_domains;
	pset->cntr_sampling_periodms = pdomains->cntr_sampling_periodms;
	pset->clkmon_refwin_usec = pdomains->clkmon_refwin_usec;
	pset->version = pdomains->version;
	pset->b_override_o_v_o_c = false;
	pset->b_debug_mode = false;
	pset->b_enforce_vf_monotonicity = pdomains->b_enforce_vf_monotonicity;
	pset->b_enforce_vf_smoothening = pdomains->b_enforce_vf_smoothening;
	if (g->ops.clk.split_rail_support) {
		pset->volt_rails_max = 2;
	} else {
		pset->volt_rails_max = 1;
	}
	status = nvgpu_boardobjgrpmask_export(
				&pdomains->master_domains_mask.super,
				pdomains->master_domains_mask.super.bitcount,
				&pset->master_domains_mask.super);
	if (status != 0) {
		nvgpu_err(g, "Error exporting Clk master domains masks");
		return status;
	}

	status = nvgpu_boardobjgrpmask_export(
				&pdomains->prog_domains_mask.super,
				pdomains->prog_domains_mask.super.bitcount,
				&pset->prog_domains_mask.super);
	if (status != 0) {
		nvgpu_err(g, "Error exporting Clk prog domains masks");
		return status;
	}

	status = nvgpu_boardobjgrpmask_export(
				&pdomains->clkmon_domains_mask.super,
				pdomains->clkmon_domains_mask.super.bitcount,
				&pset->clkmon_domains_mask.super);
	if (status != 0) {
		nvgpu_err(g, "Error exporting Clk monitor domains masks");
		return status;
	}
	nvgpu_memcpy((u8 *)&pset->deltas, (u8 *)&pdomains->deltas,
		(sizeof(struct ctrl_clk_clk_delta)));

done:
	return status;
}

s32 clk_domains_pmudata_instget(struct gk20a *g,
		struct nv_pmu_boardobjgrp *pmuboardobjgrp,
		struct nv_pmu_boardobj **pmu_obj, u16 idx)
{
	struct nv_pmu_clk_clk_domain_boardobj_grp_set  *pgrp_set =
		(struct nv_pmu_clk_clk_domain_boardobj_grp_set *)(void *)
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

s32 clk_domain_sw_setup(struct gk20a *g)
{
	s32 status;
	struct boardobjgrp *pboardobjgrp = NULL;
	struct nvgpu_clk_domains *pclkdomainobjs;
	struct nvgpu_clk_domain *pdomain;
	struct clk_domain_35_master *pdomain_master_35;
	struct clk_domain_35_slave *pdomain_slave_35;
	struct clk_domain_35_prog *pdomain_prog_35;
	u16 i;
	u32 ver;

	nvgpu_log_info(g, " ");

	ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);
	if (ver == NVGPU_GPUID_GB20C)
		return clk_domain_sw_setup_50(g);

	/* If already constructed, do not re-construct (suspend/resume pattern) */
	pboardobjgrp = &g->pmu->clk_pmu->clk_domainobjs->super.super;
	if (pboardobjgrp->bconstructed) {
		nvgpu_pmu_dbg(g, "clk_domain boardobjgrp already constructed, skipping reinit");
		return 0;
	}

	nvgpu_pmu_dbg(g, "Constructing clk_domain boardobjgrp for first time");

	status = nvgpu_boardobjgrp_construct_e32(g,
			&g->pmu->clk_pmu->clk_domainobjs->super);
	if (status != 0) {
		nvgpu_err(g,
			"error creating boardobjgrp for clk domain err=%d",
			status);
		goto done;
	}

	pboardobjgrp = &g->pmu->clk_pmu->clk_domainobjs->super.super;
	pclkdomainobjs = g->pmu->clk_pmu->clk_domainobjs;

	BOARDOBJGRP_PMU_CONSTRUCT(pboardobjgrp, CLK, CLK_DOMAIN);

	status = BOARDOBJGRP_PMU_CMD_GRP_SET_CONSTRUCT(g, pboardobjgrp,
			clk, CLK, clk_domain, CLK_DOMAIN);
	if (status != 0) {
		nvgpu_err(g,
		 "error constructing PMU_BOARDOBJ_CMD_GRP_SET interface err=%d",
		 status);
		goto done;
	}

	/* Initialize mask to zero.*/
	status = boardobjgrpmask_e32_init(&pclkdomainobjs->prog_domains_mask,
			NULL);
	if (status != 0) {
		nvgpu_err(g, "boardobjgrpmask_e32_init(prog) failed err=%d",
			status);
		goto done;
	}
	status = boardobjgrpmask_e32_init(&pclkdomainobjs->master_domains_mask,
			NULL);
	if (status != 0) {
		nvgpu_err(g, "boardobjgrpmask_e32_init(master) failed err=%d",
			status);
		goto done;
	}
	status = boardobjgrpmask_e32_init(&pclkdomainobjs->clkmon_domains_mask,
			NULL);
	if (status != 0) {
		nvgpu_err(g, "boardobjgrpmask_e32_init(clkmon) failed err=%d",
			status);
		goto done;
	}

	pclkdomainobjs->b_enforce_vf_monotonicity = true;
	pclkdomainobjs->b_enforce_vf_smoothening = true;

	(void) memset(&pclkdomainobjs->ordered_noise_aware_list, 0,
		sizeof(pclkdomainobjs->ordered_noise_aware_list));

	(void) memset(&pclkdomainobjs->ordered_noise_unaware_list, 0,
		sizeof(pclkdomainobjs->ordered_noise_unaware_list));

	(void) memset(&pclkdomainobjs->deltas, 0,
		sizeof(struct ctrl_clk_clk_delta));

	status = devinit_get_clocks_table(g);
	nvgpu_log_info(g, "Clocks table version: 0x%x", pclkdomainobjs->version);
	if (status != 0)
		goto done;

	if (pclkdomainobjs->version == CLK_DOMAIN_BOARDOBJGRP_VERSION_35) {
		pboardobjgrp->pmudatainit  = clk_domains_pmudatainit_3x;
		pboardobjgrp->pmudatainstget  = clk_domains_pmudata_instget;

		BOARDOBJGRP_FOR_EACH(&(pclkdomainobjs->super.super),
					 struct nvgpu_clk_domain *, pdomain, i) {
			pdomain_master_35 = NULL;

			if (pdomain->super.implements(g, &pdomain->super,
					CTRL_CLK_CLK_DOMAIN_TYPE_35_PROG)) {
				pdomain_prog_35 =
					(struct clk_domain_35_prog *)(void *)pdomain;
				status = nvgpu_boardobjgrpmask_bit_set(
					&pclkdomainobjs->prog_domains_mask.super, i);
				if (status != 0)
					goto done;

				/* Create the mask of clk monitors that are supported */
				if ((pdomain_prog_35->clkmon_info.high_threshold_vfe_idx !=
							CLK_CLKMON_VFE_INDEX_INVALID) ||
					(pdomain_prog_35->clkmon_info.low_threshold_vfe_idx !=
							CLK_CLKMON_VFE_INDEX_INVALID)) {
					status = nvgpu_boardobjgrpmask_bit_set(
							&pclkdomainobjs->clkmon_domains_mask.super, i);
					if (status != 0) {
						nvgpu_err(g,
						"Error setting Clk monitor masks");
						return status;
					}
				}
			}

			if (pdomain->super.implements(g, &pdomain->super,
					CTRL_CLK_CLK_DOMAIN_TYPE_35_MASTER)) {
				status = nvgpu_boardobjgrpmask_bit_set(
					&pclkdomainobjs->master_domains_mask.super, i);
				if (status != 0)
					goto done;

				pdomain_master_35 =
					(struct clk_domain_35_master *)(void *)pdomain;
				status = nvgpu_boardobjgrpmask_bit_set(
					&pdomain_master_35->master_slave_domains_grp_mask.super, i);
				if (status != 0)
					goto done;
			}

			if (pdomain->super.implements(g, &pdomain->super,
					CTRL_CLK_CLK_DOMAIN_TYPE_35_SLAVE)) {
				pdomain_slave_35 =
					(struct clk_domain_35_slave *)(void *)pdomain;
				pdomain_master_35 = (struct clk_domain_35_master *)
					(void *)
					(clk_get_clk_domain_from_index(
					(g->pmu->clk_pmu),
					pdomain_slave_35->slave.master_idx));
				pdomain_master_35->master.slave_idxs_mask |= BIT32(i);
				pdomain_slave_35->super.clk_pos =
					nvgpu_boardobjgrpmask_bit_set_count(
					&pdomain_master_35->master_slave_domains_grp_mask.super);
				status = nvgpu_boardobjgrpmask_bit_set(
					&pdomain_master_35->master_slave_domains_grp_mask.super, i);
				if (status != 0)
					goto done;
			}
		}
	} else {
		status = -EINVAL;
		nvgpu_err(g, "Unsupported version: 0x%x",
				pclkdomainobjs->version);
	}

done:
	nvgpu_log_info(g, " done status %x", status);
	return status;
}

s32 clk_domain_pmu_setup(struct gk20a *g)
{
	s32 status;
	struct boardobjgrp *pboardobjgrp = NULL;
	struct boardobjgrp_e32 *psuper;

	nvgpu_log_info(g, " ");

	if (g->pmu->clk_pmu->clk_domainobjs_50 != NULL) {
		psuper = &g->pmu->clk_pmu->clk_domainobjs_50->super.super;
	} else if (g->pmu->clk_pmu->clk_domainobjs != NULL) {
		psuper = &g->pmu->clk_pmu->clk_domainobjs->super;
	} else {
		status = -EINVAL;
		goto done;
	}

	pboardobjgrp = &psuper->super;

	if (!pboardobjgrp->bconstructed) {
		status = -EINVAL;
		goto done;
	}

	status = pboardobjgrp->pmuinithandle(g, pboardobjgrp);

done:
	nvgpu_log_info(g, "Done");
	return status;
}

static s32 devinit_get_clocks_table_35(struct gk20a *g,
		u8 *clocks_table_ptr)
{
	s32 status = 0;
	struct vbios_clocks_table_35_header clocks_table_header = { 0 };
	struct vbios_clocks_table_35_entry clocks_table_entry = { 0 };
	struct vbios_clocks_table *pClocksHAL;
	struct vbios_clocks_table_1x_hal_clock_entry *vbiosclktbl1xhalentry;
	u8 *clocks_tbl_entry_ptr = NULL;
	u32 index = 0;
	bool done = false;
	struct nvgpu_clk_domain *pclkdomain_dev;
	union {
		struct pmu_board_obj obj;
		struct nvgpu_clk_domain clk_domain;
		struct clk_domain_3x v3x;
		struct clk_domain_3x_fixed v3x_fixed;
		struct clk_domain_35_prog v35_prog;
		struct clk_domain_35_master v35_master;
		struct clk_domain_35_slave v35_slave;
	} clk_domain_data;
	struct nvgpu_clk_domains *pclkdomainobjs = g->pmu->clk_pmu->clk_domainobjs;

	nvgpu_log_info(g, " ");
	pclkdomainobjs->version = CLK_DOMAIN_BOARDOBJGRP_VERSION_35;

	nvgpu_memcpy((u8 *)&clocks_table_header, clocks_table_ptr,
			VBIOS_CLOCKS_TABLE_35_HEADER_SIZE_09);
	if (clocks_table_header.header_size <
			(u8) VBIOS_CLOCKS_TABLE_35_HEADER_SIZE_09) {
		status = -EINVAL;
		goto done;
	}

	if (clocks_table_header.entry_size <
			(u8) VBIOS_CLOCKS_TABLE_35_ENTRY_SIZE_11) {
		status = -EINVAL;
		goto done;
	}

	switch (clocks_table_header.clocks_hal) {
	case CLK_TABLE_HAL_ENTRY_GV:
		devinitClocksTableHALTranslate_1X(g, CLK_TABLE_HAL_ENTRY_GV, &pClocksHAL);
		vbiosclktbl1xhalentry = pClocksHAL->domains;
		break;
	default:
		status = -EINVAL;
		break;
	}

	if (status == -EINVAL)
		goto done;

	pclkdomainobjs->cntr_sampling_periodms =
		(u16)clocks_table_header.cntr_sampling_periodms;
	pclkdomainobjs->clkmon_refwin_usec =
		(u16)clocks_table_header.reference_window;

	/* Read table entries*/
	clocks_tbl_entry_ptr = clocks_table_ptr +
			clocks_table_header.header_size;
	for (index = 0; index < clocks_table_header.entry_count; index++) {
		nvgpu_memcpy((u8 *)&clocks_table_entry,
			clocks_tbl_entry_ptr, clocks_table_header.entry_size);
		clk_domain_data.clk_domain.domain =
				(u8) vbiosclktbl1xhalentry[index].domain;
		clk_domain_data.clk_domain.api_domain =
				clktranslatehalmumsettoapinumset(
				(u32) BIT(clk_domain_data.clk_domain.domain));
		clk_domain_data.v3x.b_noise_aware_capable =
			vbiosclktbl1xhalentry[index].b_noise_aware_capable;

		switch (BIOS_GET_FIELD(u32, clocks_table_entry.flags0,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_FLAGS0_USAGE)) {
		case  NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_FLAGS0_USAGE_FIXED:
		{
			clk_domain_data.obj.type =
				CTRL_CLK_CLK_DOMAIN_TYPE_3X_FIXED;
			clk_domain_data.v3x_fixed.freq_mhz = BIOS_GET_FIELD(u16,
				clocks_table_entry.param1,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM1_FIXED_FREQUENCY_MHZ);
			break;
		}

		case  NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_FLAGS0_USAGE_MASTER:
		{
			clk_domain_data.obj.type =
				CTRL_CLK_CLK_DOMAIN_TYPE_35_MASTER;
			clk_domain_data.v35_prog.super.clk_prog_idx_first =
				BIOS_GET_FIELD(u8, clocks_table_entry.param0,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM0_PROG_CLK_PROG_IDX_FIRST);
			clk_domain_data.v35_prog.super.clk_prog_idx_last =
				BIOS_GET_FIELD(u8, clocks_table_entry.param0,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM0_PROG_CLK_PROG_IDX_LAST);
			clk_domain_data.v35_prog.super.noise_unaware_ordering_index =
				BIOS_GET_FIELD(u8, clocks_table_entry.param2,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM2_PROG_NOISE_UNAWARE_ORDERING_IDX);
			if (clk_domain_data.v3x.b_noise_aware_capable) {
				clk_domain_data.v35_prog.super.b_force_noise_unaware_ordering =
				BIOS_GET_FIELD(bool, clocks_table_entry.param2,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM2_PROG_FORCE_NOISE_UNAWARE_ORDERING);

			} else {
				clk_domain_data.v35_prog.super.noise_aware_ordering_index =
					CTRL_CLK_CLK_DOMAIN_3X_PROG_ORDERING_INDEX_INVALID;
				clk_domain_data.v35_prog.super.b_force_noise_unaware_ordering =
						false;
			}
			clk_domain_data.v35_prog.pre_volt_ordering_index =
				BIOS_GET_FIELD(u8, clocks_table_entry.param2,
				NV_VBIOS_CLOCKS_TABLE_35_ENTRY_PARAM2_PROG_PRE_VOLT_ORDERING_IDX);

			clk_domain_data.v35_prog.post_volt_ordering_index =
				BIOS_GET_FIELD(u8, clocks_table_entry.param2,
				NV_VBIOS_CLOCKS_TABLE_35_ENTRY_PARAM2_PROG_POST_VOLT_ORDERING_IDX);

			clk_domain_data.v35_prog.super.factory_delta.data.delta_khz = 0;
			clk_domain_data.v35_prog.super.factory_delta.type = 0;

			clk_domain_data.v35_prog.super.freq_delta_min_mhz =
				BIOS_GET_FIELD(s16, clocks_table_entry.param1,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM1_MASTER_FREQ_OC_DELTA_MIN_MHZ);

			clk_domain_data.v35_prog.super.freq_delta_max_mhz =
				BIOS_GET_FIELD(s16, clocks_table_entry.param1,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM1_MASTER_FREQ_OC_DELTA_MAX_MHZ);
			clk_domain_data.v35_prog.clk_vf_curve_count =
				vbiosclktbl1xhalentry[index].clk_vf_curve_count;

			clk_domain_data.v35_prog.clkmon_info.low_threshold_vfe_idx =
				BIOS_GET_FIELD(u8, clocks_table_entry.param3,
				NV_VBIOS_CLOCKS_TABLE_35_ENTRY_PARAM3_CLK_MONITOR_THRESHOLD_MIN);
			clk_domain_data.v35_prog.clkmon_info.high_threshold_vfe_idx =
				BIOS_GET_FIELD(u8, clocks_table_entry.param3,
				NV_VBIOS_CLOCKS_TABLE_35_ENTRY_PARAM3_CLK_MONITOR_THRESHOLD_MAX);
			break;
		}

		case  NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_FLAGS0_USAGE_SLAVE:
		{
			clk_domain_data.obj.type =
				CTRL_CLK_CLK_DOMAIN_TYPE_35_SLAVE;
			clk_domain_data.v35_prog.super.clk_prog_idx_first =
				BIOS_GET_FIELD(u8, clocks_table_entry.param0,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM0_PROG_CLK_PROG_IDX_FIRST);
			clk_domain_data.v35_prog.super.clk_prog_idx_last =
				BIOS_GET_FIELD(u8, clocks_table_entry.param0,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM0_PROG_CLK_PROG_IDX_LAST);
			clk_domain_data.v35_prog.super.noise_unaware_ordering_index =
				BIOS_GET_FIELD(u8, clocks_table_entry.param2,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM2_PROG_NOISE_UNAWARE_ORDERING_IDX);

			if (clk_domain_data.v3x.b_noise_aware_capable) {
				clk_domain_data.v35_prog.super.b_force_noise_unaware_ordering =
				BIOS_GET_FIELD(bool, clocks_table_entry.param2,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM2_PROG_FORCE_NOISE_UNAWARE_ORDERING);

			} else {
				clk_domain_data.v35_prog.super.noise_aware_ordering_index =
					CTRL_CLK_CLK_DOMAIN_3X_PROG_ORDERING_INDEX_INVALID;
				clk_domain_data.v35_prog.super.b_force_noise_unaware_ordering =
						false;
			}
			clk_domain_data.v35_prog.pre_volt_ordering_index =
				BIOS_GET_FIELD(u8, clocks_table_entry.param2,
				NV_VBIOS_CLOCKS_TABLE_35_ENTRY_PARAM2_PROG_PRE_VOLT_ORDERING_IDX);

			clk_domain_data.v35_prog.post_volt_ordering_index =
				BIOS_GET_FIELD(u8, clocks_table_entry.param2,
				NV_VBIOS_CLOCKS_TABLE_35_ENTRY_PARAM2_PROG_POST_VOLT_ORDERING_IDX);

			clk_domain_data.v35_prog.super.factory_delta.data.delta_khz = 0;
			clk_domain_data.v35_prog.super.factory_delta.type = 0;
			clk_domain_data.v35_prog.super.freq_delta_min_mhz = 0;
			clk_domain_data.v35_prog.super.freq_delta_max_mhz = 0;
			clk_domain_data.v35_slave.slave.master_idx =
				BIOS_GET_FIELD(u8, clocks_table_entry.param1,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM1_SLAVE_MASTER_DOMAIN);

			clk_domain_data.v35_prog.clkmon_info.low_threshold_vfe_idx =
				BIOS_GET_FIELD(u8, clocks_table_entry.param3,
				NV_VBIOS_CLOCKS_TABLE_35_ENTRY_PARAM3_CLK_MONITOR_THRESHOLD_MIN);
			clk_domain_data.v35_prog.clkmon_info.high_threshold_vfe_idx =
				BIOS_GET_FIELD(u8, clocks_table_entry.param3,
				NV_VBIOS_CLOCKS_TABLE_35_ENTRY_PARAM3_CLK_MONITOR_THRESHOLD_MAX);
			break;
		}

		default:
		{
			nvgpu_err(g,
				  "error reading clock domain entry %d", index);
			status = -EINVAL;
			done = true;
			break;
		}

		}
		/*
		 * Previously we were doing "goto done" from the default case of
		 * the switch-case block above. MISRA however, gets upset about
		 * this because it wants a break statement in the default case.
		 * That's why we had to move the goto statement outside of the
		 * switch-case block.
		 */
		if (done)
			goto done;

		pclkdomain_dev = construct_clk_domain(g,
				(void *)&clk_domain_data);
		if (pclkdomain_dev == NULL) {
			nvgpu_err(g,
			"unable to construct clock domain boardobj for %d",
			index);
			status = -EINVAL;
			goto done;
		}
		status = boardobjgrp_objinsert(&pclkdomainobjs->super.super,
				(struct pmu_board_obj *)(void *)
				pclkdomain_dev, (u16)index);
		if (status != 0) {
			nvgpu_err(g,
			"unable to insert clock domain boardobj for %d", index);
			status = -EINVAL;
			goto done;
		}
		clocks_tbl_entry_ptr += clocks_table_header.entry_size;
	}

done:
	nvgpu_log_info(g, " done status %x", status);
	return status;
}

static s32 devinit_get_clocks_table(struct gk20a *g)
{
	s32 status = 0;
	u8 *clocks_table_ptr = NULL;

	nvgpu_log_info(g, " ");

	clocks_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			nvgpu_bios_get_bit_token(g, NVGPU_BIOS_CLOCK_TOKEN),
							CLOCKS_TABLE);

	if (clocks_table_ptr == NULL) {
		status = -EINVAL;
		goto done;
	}
	status = devinit_get_clocks_table_35(g, clocks_table_ptr);

done:
	return status;
}

s32 clk_domain_construct_super(struct gk20a *g,
					struct pmu_board_obj **obj,
					size_t size, void *pargs)
{
	struct nvgpu_clk_domain *pdomain;
	struct nvgpu_clk_domain *ptmpdomain = (struct nvgpu_clk_domain *)pargs;
	s32 status = 0;

	pdomain = nvgpu_kzalloc(g, size);
	if (pdomain == NULL) {
		return -ENOMEM;
	}

	status = pmu_board_obj_construct_super(g,
			(struct pmu_board_obj *)(void *)pdomain, pargs);
	if (status != 0) {
		return -EINVAL;
	}

	*obj = (struct pmu_board_obj *)(void *)pdomain;

	pdomain->super.pmudatainit =
			clk_domain_pmudatainit_super;

	pdomain->api_domain = ptmpdomain->api_domain;
	pdomain->domain = ptmpdomain->domain;
	pdomain->perf_domain_grp_idx =
		ptmpdomain->perf_domain_grp_idx;

	return status;
}

static s32 clk_domain_pmudatainit_3x(struct gk20a *g,
					struct pmu_board_obj *obj,
					struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;
	struct clk_domain_3x *pclk_domain_3x;
	struct nv_pmu_clk_clk_domain_3x_boardobj_set *pset;

	nvgpu_log_info(g, " ");

	status = clk_domain_pmudatainit_super(g, obj, pmu_obj);
	if (status != 0) {
		return status;
	}

	pclk_domain_3x = (struct clk_domain_3x *)(void *)obj;

	pset = (struct nv_pmu_clk_clk_domain_3x_boardobj_set *)(void *)pmu_obj;

	pset->b_noise_aware_capable = pclk_domain_3x->b_noise_aware_capable;

	return status;
}

static s32 clk_domain_construct_3x(struct gk20a *g,
				   struct pmu_board_obj **obj,
				   size_t size, void *pargs)
{
	struct pmu_board_obj *obj_tmp = (struct pmu_board_obj *)pargs;
	struct clk_domain_3x *pdomain;
	struct clk_domain_3x *ptmpdomain =
			(struct clk_domain_3x *)pargs;
	s32 status = 0;

	obj_tmp->type_mask = BIT32(CTRL_CLK_CLK_DOMAIN_TYPE_3X);
	status = clk_domain_construct_super(g, obj,
					size, pargs);
	if (status != 0) {
		return -EINVAL;
	}

	pdomain = (struct clk_domain_3x *)(void *)*obj;

	pdomain->super.super.pmudatainit =
			clk_domain_pmudatainit_3x;

	pdomain->b_noise_aware_capable = ptmpdomain->b_noise_aware_capable;

	return status;
}

static s32 clkdomainclkproglink_3x_prog(struct gk20a *g,
					struct nvgpu_clk_pmupstate *pclk,
					struct nvgpu_clk_domain *pdomain)
{
	s32 status = 0;
	struct clk_domain_3x_prog *p3xprog =
		(struct clk_domain_3x_prog *)(void *)pdomain;
	struct clk_prog *pprog = NULL;
	u8 i;

	nvgpu_log_info(g, " ");

	for (i = p3xprog->clk_prog_idx_first;
			i <= p3xprog->clk_prog_idx_last;
			i++) {
		pprog = CLK_CLK_PROG_GET(pclk, i);
		if (pprog == NULL)
			status = -EINVAL;
	}
	return status;
}

static s32 clkdomaingetslaveclk(struct gk20a *g,
				struct nvgpu_clk_pmupstate *pclk,
				struct nvgpu_clk_domain *pdomain,
				u16 *pclkmhz,
				u16 masterclkmhz)
{
	s32 status = 0;
	struct clk_prog *pprog = NULL;
	struct clk_prog_1x_master *pprog1xmaster = NULL;
	u8 slaveidx;
	struct clk_domain_35_master *p35master;
	nvgpu_log_info(g, " ");

	if (pclkmhz == NULL)
		return -EINVAL;

	if (masterclkmhz == 0U)
		return -EINVAL;

	slaveidx = pmu_board_obj_get_idx(pdomain);
	p35master = (struct clk_domain_35_master *)(void *)
		clk_get_clk_domain_from_index(pclk,
		((struct clk_domain_35_slave *)
		(void *)pdomain)->slave.master_idx);
	pprog = CLK_CLK_PROG_GET(pclk, p35master->master.super.clk_prog_idx_first);
	pprog1xmaster = (struct clk_prog_1x_master *)(void *)pprog;

	status = pprog1xmaster->getslaveclk(g, pclk, pprog1xmaster,
			slaveidx, pclkmhz, masterclkmhz, &pdomain->ratio_domain);

	return status;
}

s32 clkdomainvfsearch(struct gk20a *g,
				struct nvgpu_clk_pmupstate *pclk,
				struct nvgpu_clk_domain *pdomain,
				u16 *pclkmhz,
				u32 *pvoltuv,
				u8 rail)
{
	s32 status = 0;
	struct clk_domain_3x_master *p3xmaster  =
		(struct clk_domain_3x_master *)(void *)pdomain;
	struct clk_prog *pprog = NULL;
	struct clk_prog_1x_master *pprog1xmaster = NULL;
	u8 i;
	u8 *pslaveidx = NULL;
	u8 slaveidx;
	u16 clkmhz;
	u32 voltuv;
	u16 bestclkmhz;
	u32 bestvoltuv;

	nvgpu_log_info(g, " ");

	if ((pclkmhz == NULL) || (pvoltuv == NULL))
		return -EINVAL;

	if ((*pclkmhz != 0U) && (*pvoltuv != 0U))
		return -EINVAL;

	bestclkmhz = *pclkmhz;
	bestvoltuv = *pvoltuv;

	if (pdomain->super.implements(g, &pdomain->super,
			CTRL_CLK_CLK_DOMAIN_TYPE_3X_SLAVE)) {
		slaveidx = pmu_board_obj_get_idx(pdomain);
		pslaveidx = &slaveidx;
		p3xmaster = (struct clk_domain_3x_master *)(void *)
				clk_get_clk_domain_from_index(pclk,
				((struct clk_domain_3x_slave *)(void *)
					pdomain)->master_idx);
	}
	/* Iterate over the set of CLK_PROGs pointed at by this domain.*/
	for (i = p3xmaster->super.clk_prog_idx_first;
			i <= p3xmaster->super.clk_prog_idx_last;
			i++) {
		clkmhz = *pclkmhz;
		voltuv = *pvoltuv;
		pprog = CLK_CLK_PROG_GET(pclk, i);

		/* MASTER CLK_DOMAINs must point to MASTER CLK_PROGs.*/
		if (!pprog->super.implements(g, &pprog->super,
				CTRL_CLK_CLK_PROG_TYPE_1X_MASTER)) {
			status = -EINVAL;
			goto done;
		}

		pprog1xmaster = (struct clk_prog_1x_master *)(void *)pprog;
		status = pprog1xmaster->vflookup(g, pclk, pprog1xmaster,
				pslaveidx, &clkmhz, &voltuv, rail);
		/* if look up has found the V or F value matching to other
		 exit */
		if (status == 0) {
			if (*pclkmhz == 0U) {
				bestclkmhz = clkmhz;
			} else {
				bestvoltuv = voltuv;
				break;
			}
		}
	}
	/* clk and volt sent as zero to print vf table */
	if ((*pclkmhz == 0U) && (*pvoltuv == 0U)) {
		status = 0;
		goto done;
	}
	/* atleast one search found a matching value? */
	if ((bestvoltuv != 0U) && (bestclkmhz != 0U)) {
		*pclkmhz = bestclkmhz;
		*pvoltuv = bestvoltuv;
		status = 0;
		goto done;
	}
done:
	nvgpu_log_info(g, "done status %x", status);
	return status;
}

s32 clkdomaingetfpoints
(
	struct gk20a *g,
	struct nvgpu_clk_pmupstate *pclk,
	struct nvgpu_clk_domain *pdomain,
	u32 *pfpointscount,
	u16 *pfreqpointsinmhz,
	u8 rail
)
{
	s32 status = 0;
	struct clk_domain_3x_master *p3xmaster  =
		(struct clk_domain_3x_master *)(void *)pdomain;
	struct clk_prog *pprog = NULL;
	struct clk_prog_1x_master *pprog1xmaster = NULL;
	u32 fpointscount = 0;
	u32 remainingcount;
	u32 totalcount;
	u16 *freqpointsdata;
	u8 i;

	nvgpu_log_info(g, " ");

	if (pfpointscount == NULL)
		return -EINVAL;

	if ((pfreqpointsinmhz == NULL) && (*pfpointscount != 0U))
		return -EINVAL;

	if (pdomain->super.implements(g, &pdomain->super,
			CTRL_CLK_CLK_DOMAIN_TYPE_3X_SLAVE))
		return -EINVAL;

	freqpointsdata = pfreqpointsinmhz;
	totalcount = 0;
	fpointscount = *pfpointscount;
	remainingcount = fpointscount;
	/* Iterate over the set of CLK_PROGs pointed at by this domain.*/
	for (i = p3xmaster->super.clk_prog_idx_first;
			i <= p3xmaster->super.clk_prog_idx_last;
			i++) {
		pprog = CLK_CLK_PROG_GET(pclk, i);
		pprog1xmaster = (struct clk_prog_1x_master *)(void *)pprog;
		status = pprog1xmaster->getfpoints(g, pclk, pprog1xmaster,
				&fpointscount, &freqpointsdata, rail);
		if (status != 0) {
			*pfpointscount = 0;
			goto done;
		}
		totalcount += fpointscount;
		if (*pfpointscount != 0U) {
			remainingcount -= fpointscount;
			fpointscount = remainingcount;
		} else {
			fpointscount = 0;
		}

	}

	*pfpointscount = totalcount;
done:
	nvgpu_log_info(g, "done status %x", status);
	return status;
}

static s32 clk_domain_pmudatainit_35_prog(struct gk20a *g,
			struct pmu_board_obj *obj,
			struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;
	struct clk_domain_35_prog *pclk_domain_35_prog;
	struct clk_domain_3x_prog *pclk_domain_3x_prog;
	struct nv_pmu_clk_clk_domain_35_prog_boardobj_set *pset;
	struct nvgpu_clk_domains *pdomains = g->pmu->clk_pmu->clk_domainobjs;

	nvgpu_log_info(g, " ");

	status = clk_domain_pmudatainit_3x(g, obj, pmu_obj);
	if (status != 0)
		return status;

	pclk_domain_35_prog = (struct clk_domain_35_prog *)(void *)obj;
	pclk_domain_3x_prog = &pclk_domain_35_prog->super;

	pset = (struct nv_pmu_clk_clk_domain_35_prog_boardobj_set *)
		(void *)pmu_obj;

	pset->super.clk_prog_idx_first =
			pclk_domain_3x_prog->clk_prog_idx_first;
	pset->super.clk_prog_idx_last = pclk_domain_3x_prog->clk_prog_idx_last;
	pset->super.b_force_noise_unaware_ordering =
		pclk_domain_3x_prog->b_force_noise_unaware_ordering;
	pset->super.factory_delta = pclk_domain_3x_prog->factory_delta;
	pset->super.freq_delta_min_mhz =
			pclk_domain_3x_prog->freq_delta_min_mhz;
	pset->super.freq_delta_max_mhz =
			pclk_domain_3x_prog->freq_delta_max_mhz;
	nvgpu_memcpy((u8 *)&pset->super.deltas, (u8 *)&pdomains->deltas,
		(sizeof(struct ctrl_clk_clk_delta)));
	pset->pre_volt_ordering_index =
			pclk_domain_35_prog->pre_volt_ordering_index;
	pset->post_volt_ordering_index =
			pclk_domain_35_prog->post_volt_ordering_index;
	pset->clk_pos = pclk_domain_35_prog->clk_pos;
	pset->clk_vf_curve_count = pclk_domain_35_prog->clk_vf_curve_count;
	pset->clkmon_info.high_threshold_vfe_idx =
			pclk_domain_35_prog->clkmon_info.high_threshold_vfe_idx;
	pset->clkmon_info.low_threshold_vfe_idx =
			pclk_domain_35_prog->clkmon_info.low_threshold_vfe_idx;
	pset->clkmon_ctrl.high_threshold_override =
			pclk_domain_35_prog->clkmon_ctrl.high_threshold_override;
	pset->clkmon_ctrl.low_threshold_override =
			pclk_domain_35_prog->clkmon_ctrl.low_threshold_override;

	return status;
}

static s32 clk_domain_construct_35_prog(struct gk20a *g,
					struct pmu_board_obj **obj,
					size_t size, void *pargs)
{
	struct pmu_board_obj *obj_tmp = (struct pmu_board_obj *)pargs;
	struct clk_domain_35_prog *pdomain;
	struct clk_domain_35_prog *ptmpdomain =
			(struct clk_domain_35_prog *)pargs;
	s32 status = 0;

	obj_tmp->type_mask |= BIT32(CTRL_CLK_CLK_DOMAIN_TYPE_35_PROG);
	status = clk_domain_construct_3x(g, obj, size, pargs);
	if (status != 0)
		return -EINVAL;

	pdomain = (struct clk_domain_35_prog *)(void *) *obj;

	pdomain->super.super.super.super.type_mask |=
		BIT32(CTRL_CLK_CLK_DOMAIN_TYPE_35_PROG);

	pdomain->super.super.super.super.pmudatainit =
				clk_domain_pmudatainit_35_prog;

	pdomain->super.super.super.clkdomainclkproglink =
				clkdomainclkproglink_3x_prog;

	pdomain->super.super.super.clkdomainclkvfsearch =
				clkdomainvfsearch;

	pdomain->super.super.super.clkdomainclkgetfpoints =
				clkdomaingetfpoints;

	pdomain->super.clk_prog_idx_first =
			ptmpdomain->super.clk_prog_idx_first;
	pdomain->super.clk_prog_idx_last = ptmpdomain->super.clk_prog_idx_last;
	pdomain->super.noise_unaware_ordering_index =
		ptmpdomain->super.noise_unaware_ordering_index;
	pdomain->super.noise_aware_ordering_index =
		ptmpdomain->super.noise_aware_ordering_index;
	pdomain->super.b_force_noise_unaware_ordering =
		ptmpdomain->super.b_force_noise_unaware_ordering;
	pdomain->super.factory_delta = ptmpdomain->super.factory_delta;
	pdomain->super.freq_delta_min_mhz =
			ptmpdomain->super.freq_delta_min_mhz;
	pdomain->super.freq_delta_max_mhz =
			ptmpdomain->super.freq_delta_max_mhz;
	pdomain->pre_volt_ordering_index = ptmpdomain->pre_volt_ordering_index;
	pdomain->post_volt_ordering_index =
			ptmpdomain->post_volt_ordering_index;
	pdomain->clk_pos = ptmpdomain->clk_pos;
	pdomain->clk_vf_curve_count = ptmpdomain->clk_vf_curve_count;
	pdomain->clkmon_info.high_threshold_vfe_idx =
			ptmpdomain->clkmon_info.high_threshold_vfe_idx;
	pdomain->clkmon_info.low_threshold_vfe_idx =
			ptmpdomain->clkmon_info.low_threshold_vfe_idx;

	return status;
}

static s32 clk_domain_pmudatainit_35_slave(struct gk20a *g,
			struct pmu_board_obj *obj,
			struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;
	struct clk_domain_35_slave *pclk_domain_35_slave;
	struct nv_pmu_clk_clk_domain_35_slave_boardobj_set *pset;

	nvgpu_log_info(g, " ");

	status = clk_domain_pmudatainit_35_prog(g, obj, pmu_obj);
	if (status != 0)
		return status;

	pclk_domain_35_slave = (struct clk_domain_35_slave *)
					(void *)obj;

	pset = (struct nv_pmu_clk_clk_domain_35_slave_boardobj_set *)
		(void *)pmu_obj;

	pset->slave.master_idx = pclk_domain_35_slave->slave.master_idx;

	return status;
}

static s32 clk_domain_construct_35_slave(struct gk20a *g,
			struct pmu_board_obj **obj,
			size_t size, void *pargs)
{
	struct pmu_board_obj *obj_tmp = (struct pmu_board_obj *)pargs;
	struct clk_domain_35_slave *pdomain;
	struct clk_domain_35_slave *ptmpdomain =
			(struct clk_domain_35_slave *)pargs;
	s32 status = 0;

	if (pmu_board_obj_get_type(pargs) !=
			(u8) CTRL_CLK_CLK_DOMAIN_TYPE_35_SLAVE) {
		return -EINVAL;
	}

	obj_tmp->type_mask |= BIT32(CTRL_CLK_CLK_DOMAIN_TYPE_35_SLAVE);
	status = clk_domain_construct_35_prog(g, obj, size, pargs);
	if (status != 0)
		return -EINVAL;

	pdomain = (struct clk_domain_35_slave *)(void *)*obj;

	pdomain->super.super.super.super.super.pmudatainit =
			clk_domain_pmudatainit_35_slave;

	pdomain->slave.master_idx = ptmpdomain->slave.master_idx;

	pdomain->slave.clkdomainclkgetslaveclk =
				clkdomaingetslaveclk;

	return status;
}

static s32 clkdomainclkproglink_3x_master(struct gk20a *g,
			struct nvgpu_clk_pmupstate *pclk,
			struct nvgpu_clk_domain *pdomain)
{
	s32 status = 0;
	struct clk_domain_3x_master *p3xmaster  =
		(struct clk_domain_3x_master *)(void *)pdomain;
	struct clk_prog *pprog = NULL;
	struct clk_prog_1x_master *pprog1xmaster = NULL;
	u16 freq_max_last_mhz = 0;
	u8 i;

	nvgpu_log_info(g, " ");

	status = clkdomainclkproglink_3x_prog(g, pclk, pdomain);
	if (status != 0)
		goto done;

	/* Iterate over the set of CLK_PROGs pointed at by this domain.*/
	for (i = p3xmaster->super.clk_prog_idx_first;
			i <= p3xmaster->super.clk_prog_idx_last;
			i++) {
		pprog = CLK_CLK_PROG_GET(pclk, i);

		/* MASTER CLK_DOMAINs must point to MASTER CLK_PROGs.*/
		if (!pprog->super.implements(g, &pprog->super,
				CTRL_CLK_CLK_PROG_TYPE_1X_MASTER)) {
			status = -EINVAL;
			goto done;
		}

		pprog1xmaster = (struct clk_prog_1x_master *)(void *)pprog;
		status = pprog1xmaster->vfflatten(g, pclk, pprog1xmaster,
			pmu_board_obj_get_idx(p3xmaster), &freq_max_last_mhz);
		if (status != 0) {
			goto done;
		}
	}
done:
	nvgpu_log_info(g, "done status %x", status);
	return status;
}

static s32 clk_domain_pmudatainit_35_master(struct gk20a *g,
			struct pmu_board_obj *obj,
			struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;
	struct clk_domain_35_master *pclk_domain_35_master;
	struct nv_pmu_clk_clk_domain_35_master_boardobj_set *pset;

	nvgpu_log_info(g, " ");

	status = clk_domain_pmudatainit_35_prog(g, obj, pmu_obj);
	if (status != 0) {
		return status;
	}

	pclk_domain_35_master = (struct clk_domain_35_master *)
		(void *)obj;

	pset = (struct nv_pmu_clk_clk_domain_35_master_boardobj_set *)
		(void *)pmu_obj;

	pset->master.slave_idxs_mask =
			pclk_domain_35_master->master.slave_idxs_mask;

	status = nvgpu_boardobjgrpmask_export(
		&pclk_domain_35_master->master_slave_domains_grp_mask.super,
		pclk_domain_35_master->master_slave_domains_grp_mask.super.bitcount,
		&pset->master_slave_domains_grp_mask.super);

	return status;
}

static s32 clk_domain_construct_35_master(struct gk20a *g,
			struct pmu_board_obj **obj,
			size_t size, void *pargs)
{
	struct pmu_board_obj *obj_tmp = (struct pmu_board_obj *)pargs;
	struct clk_domain_35_master *pdomain;
	s32 status = 0;

	if (pmu_board_obj_get_type(pargs) !=
			(u8) CTRL_CLK_CLK_DOMAIN_TYPE_35_MASTER)
		return -EINVAL;

	obj_tmp->type_mask |= BIT32(CTRL_CLK_CLK_DOMAIN_TYPE_35_MASTER);
	status = clk_domain_construct_35_prog(g, obj, size, pargs);
	if (status != 0) {
		return -EINVAL;
	}

	pdomain = (struct clk_domain_35_master *)(void *) *obj;

	pdomain->super.super.super.super.super.pmudatainit =
			clk_domain_pmudatainit_35_master;
	pdomain->super.super.super.super.clkdomainclkproglink =
				clkdomainclkproglink_3x_master;

	pdomain->master.slave_idxs_mask = 0;
	pdomain->super.clk_pos = 0;

	status = boardobjgrpmask_e32_init(
			&pdomain->master_slave_domains_grp_mask, NULL);

	return status;
}

s32 clkdomainclkproglink_fixed(struct gk20a *g,
			struct nvgpu_clk_pmupstate *pclk,
			struct nvgpu_clk_domain *pdomain)
{
	(void)pclk;
	(void)pdomain;
	nvgpu_log_info(g, " ");
	return 0;
}

static s32 clk_domain_pmudatainit_3x_fixed(struct gk20a *g,
					struct pmu_board_obj *obj,
					struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;
	struct clk_domain_3x_fixed *pclk_domain_3x_fixed;
	struct nv_pmu_clk_clk_domain_3x_fixed_boardobj_set *pset;

	nvgpu_log_info(g, " ");

	status = clk_domain_pmudatainit_3x(g, obj, pmu_obj);
	if (status != 0)
		return status;

	pclk_domain_3x_fixed = (struct clk_domain_3x_fixed *)
					(void *)obj;

	pset = (struct nv_pmu_clk_clk_domain_3x_fixed_boardobj_set *)
		(void *)pmu_obj;

	pset->freq_mhz = pclk_domain_3x_fixed->freq_mhz;

	return status;
}

static s32 clk_domain_construct_3x_fixed(struct gk20a *g,
			struct pmu_board_obj **obj,
			size_t size, void *pargs)
{
	struct pmu_board_obj *obj_tmp = (struct pmu_board_obj *)pargs;
	struct clk_domain_3x_fixed *pdomain;
	struct clk_domain_3x_fixed *ptmpdomain =
			(struct clk_domain_3x_fixed *)pargs;
	s32 status = 0;

	if (pmu_board_obj_get_type(pargs) != CTRL_CLK_CLK_DOMAIN_TYPE_3X_FIXED)
		return -EINVAL;

	obj_tmp->type_mask |= BIT32(CTRL_CLK_CLK_DOMAIN_TYPE_3X_FIXED);
	status = clk_domain_construct_3x(g, obj, size, pargs);
	if (status != 0)
		return -EINVAL;

	pdomain = (struct clk_domain_3x_fixed *)(void *)*obj;

	pdomain->super.super.super.pmudatainit =
			clk_domain_pmudatainit_3x_fixed;

	pdomain->super.super.clkdomainclkproglink =
			clkdomainclkproglink_fixed;

	pdomain->freq_mhz = ptmpdomain->freq_mhz;

	return status;
}

struct nvgpu_clk_domain *construct_clk_domain(struct gk20a *g,
		void *pargs)
{
	struct pmu_board_obj *obj = NULL;
	int status;

	nvgpu_log_info(g, " %d", (pmu_board_obj_get_type(pargs)));
	switch (pmu_board_obj_get_type(pargs)) {
	case CTRL_CLK_CLK_DOMAIN_TYPE_3X_FIXED:
		status = clk_domain_construct_3x_fixed(g, &obj,
			sizeof(struct clk_domain_3x_fixed), pargs);
		break;

	case CTRL_CLK_CLK_DOMAIN_TYPE_35_MASTER:
		status = clk_domain_construct_35_master(g, &obj,
			sizeof(struct clk_domain_35_master), pargs);
		break;

	case CTRL_CLK_CLK_DOMAIN_TYPE_35_SLAVE:
		status = clk_domain_construct_35_slave(g, &obj,
			sizeof(struct clk_domain_35_slave), pargs);
		break;

	case CTRL_CLK_CLK_DOMAIN_TYPE_50_FIXED:
		status = clk_domain_construct_50_fixed(g, &obj,
			sizeof(struct clk_domain_50_fixed), pargs);
		break;

	case CTRL_CLK_CLK_DOMAIN_TYPE_50_PROG_PHYSICAL:
		status = clk_domain_construct_50_physical(g, &obj,
			sizeof(struct clk_domain_50_prog_physical), pargs);
		break;

	default:
		nvgpu_err(g, "Unsupported Clk domain type");
		status = -EINVAL;
		break;
	}

	if (status != 0) {
		obj = NULL;
		nvgpu_err(g, "Error constructing CLK_DOMAIN: status=%d.", status);
	}

	nvgpu_log_info(g, " Done");

	return (struct nvgpu_clk_domain *)(void *)obj;
}

s32 clk_domain_pmudatainit_super(struct gk20a *g,
			struct pmu_board_obj *obj,
			struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;
	struct nvgpu_clk_domain *pclk_domain;
	struct nv_pmu_clk_clk_domain_boardobj_set *pset;

	nvgpu_log_info(g, " ");

	status = pmu_board_obj_pmu_data_init_super(g, obj, pmu_obj);
	if (status != 0)
		return status;

	pclk_domain = (struct nvgpu_clk_domain *)(void *)obj;

	pset = (struct nv_pmu_clk_clk_domain_boardobj_set *)(void *)pmu_obj;

	pset->domain = pclk_domain->domain;
	pset->api_domain = pclk_domain->api_domain;
	pset->perf_domain_grp_idx = pclk_domain->perf_domain_grp_idx;
	pset->b_noise_aware_capable = pclk_domain->b_noise_aware_capable;
	pset->perf_domain_grp_idx = pclk_domain->perf_domain_grp_idx;

	return status;
}

s32 clk_domain_clk_prog_link(struct gk20a *g,
		struct nvgpu_clk_pmupstate *pclk)
{
	s32 status = 0;
	struct nvgpu_clk_domain *pdomain;
	u16 i;
	struct boardobjgrp_e32 *psuper;

	if (g->pmu->clk_pmu->clk_domainobjs_50 != NULL) {
		psuper = &g->pmu->clk_pmu->clk_domainobjs_50->super.super;
	} else if (g->pmu->clk_pmu->clk_domainobjs != NULL) {
		psuper = &g->pmu->clk_pmu->clk_domainobjs->super;
	} else {
		status = -EINVAL;
		goto done;
	}

	/* Iterate over all CLK_DOMAINs and flatten their VF curves.*/
	BOARDOBJGRP_FOR_EACH(&(psuper->super),
			struct nvgpu_clk_domain *, pdomain, i) {
		status = pdomain->clkdomainclkproglink(g, pclk, pdomain);
		if (status != 0) {
			nvgpu_err(g,
				  "error flattening VF for CLK DOMAIN - 0x%x - index: %d",
				  pdomain->domain, i);
			goto done;
		}
	}

done:
	return status;
}

s32 clk_pmu_clk_domains_load(struct gk20a *g)
{
	s32 status;
	struct nvgpu_pmu *pmu = g->pmu;
	struct nv_pmu_rpc_struct_clk_load clk_load_rpc;

	(void) memset(&clk_load_rpc, 0,
			sizeof(struct nv_pmu_rpc_struct_clk_load));

	clk_load_rpc.clk_load.feature = NV_NV_PMU_CLK_LOAD_FEATURE_CLK_DOMAIN;

	/* Continue with PMU setup, assume FB map is done  */
	PMU_RPC_EXECUTE_CPB(status, pmu, CLK, LOAD, &clk_load_rpc, 0);
	if (status != 0) {
		nvgpu_err(g,
			"Failed to execute Clock domain Load RPC status=0x%x",
			status);
	}

	return status;
}

#ifdef CONFIG_NVGPU_CLK_ARB
s32 clk_get_fll_clks_per_clk_domain(struct gk20a *g,
		struct nvgpu_clk_slave_freq *setfllclk)
{
	s32 status = -EINVAL;
	struct nvgpu_clk_domain *pdomain;
	u16 i;
	struct nvgpu_clk_pmupstate *pclk = g->pmu->clk_pmu;
	unsigned long bit;
	u16 clkmhz = 0;
	struct clk_domain_35_master *p35master;
	struct clk_domain_35_slave *p35slave;
	unsigned long slaveidxmask;

	if (setfllclk->gpc_mhz == 0U)
		return -EINVAL;

	BOARDOBJGRP_FOR_EACH(&(pclk->clk_domainobjs->super.super),
			struct nvgpu_clk_domain *, pdomain, i) {

		if (pdomain->api_domain == CTRL_CLK_DOMAIN_GPCCLK) {
			if (!pdomain->super.implements(g, &pdomain->super,
				CTRL_CLK_CLK_DOMAIN_TYPE_35_MASTER)) {
				status = -EINVAL;
				goto done;
			}
			p35master = (struct clk_domain_35_master *)
				(void *)pdomain;
			slaveidxmask = p35master->master.slave_idxs_mask;
			for_each_set_bit(bit, &slaveidxmask, 32U) {
				i = (u8)bit;
				p35slave = (struct clk_domain_35_slave *)
					(void *)
					clk_get_clk_domain_from_index(pclk, i);

				clkmhz = 0;
				status = p35slave->slave.clkdomainclkgetslaveclk(g,
					pclk, (struct nvgpu_clk_domain *)
					(void *)p35slave,
					&clkmhz, setfllclk->gpc_mhz);
				if (status != 0) {
					status = -EINVAL;
					goto done;
				}
				if (p35slave->super.super.super.super.
					api_domain == CTRL_CLK_DOMAIN_XBARCLK) {
					setfllclk->xbar_mhz = clkmhz;
				}
				if (p35slave->super.super.super.super.
					api_domain == CTRL_CLK_DOMAIN_SYSCLK) {
					setfllclk->sys_mhz = clkmhz;
				}
				if (p35slave->super.super.super.super.
					api_domain == CTRL_CLK_DOMAIN_NVDCLK) {
					setfllclk->nvd_mhz = clkmhz;
				}
				if (p35slave->super.super.super.super.
					api_domain == CTRL_CLK_DOMAIN_HOSTCLK) {
					setfllclk->host_mhz = clkmhz;
				}
			}
		}
	}
done:
	return status;
}

/* _mg_ Todo:
 * Need to enable clock rate setting for P0 and P8 pstates
 * once we have both pstates implements.
 * This is called by change sequencer to set clocks: nvgpu_pmu_perf_changeseq_set_clks()
 */
void clk_set_p0_clk_per_domain(struct gk20a *g, u8 *gpcclk_domain,
		u32 *gpcclk_clkmhz,
		struct nvgpu_clk_slave_freq *vf_point,
		struct nvgpu_pmu_perf_change_input_clk_info *change_input)
{
	struct nvgpu_clk_domain *pclk_domain;
	struct nvgpu_pmu_perf_pstate_clk_info *p0_info;
	u32 max_clkmhz;
	u16 max_ratio;
	u16 i = 0;
	struct boardobjgrp_e32 *psuper;

	if (g->pmu->clk_pmu->clk_domainobjs_50 != NULL) {
		psuper = &g->pmu->clk_pmu->clk_domainobjs_50->super.super;
	} else if (g->pmu->clk_pmu->clk_domainobjs != NULL) {
		psuper = &g->pmu->clk_pmu->clk_domainobjs->super;
	} else {
		return;
	}

	BOARDOBJGRP_FOR_EACH(&(psuper->super),
			struct nvgpu_clk_domain *, pclk_domain, i) {

		switch (pclk_domain->api_domain) {
		case CTRL_CLK_DOMAIN_GPCCLK:
			*gpcclk_domain = i;
			*gpcclk_clkmhz = vf_point->gpc_mhz;

			p0_info = nvgpu_pmu_perf_pstate_get_clk_set_info(g,
					CTRL_PERF_PSTATE_P0, CLKWHICH_GPCCLK);
			if (p0_info == NULL) {
				nvgpu_err(g, "failed to get GPCCLK P0 info");
				break;
			}
			if (vf_point->gpc_mhz < p0_info->min_mhz) {
				vf_point->gpc_mhz = p0_info->min_mhz;
			}
			if (vf_point->gpc_mhz > p0_info->max_mhz) {
				vf_point->gpc_mhz = p0_info->max_mhz;
			}
			change_input->clk[i].clk_freq_khz =
					(u32)vf_point->gpc_mhz * 1000U;
			change_input->clk_domains_mask.super.data[0] |=
					(u32) BIT(i);
			break;
		case CTRL_CLK_DOMAIN_XBARCLK:
			p0_info = nvgpu_pmu_perf_pstate_get_clk_set_info(g,
					CTRL_PERF_PSTATE_P0, CLKWHICH_XBARCLK);
			if (p0_info == NULL) {
				nvgpu_err(g, "failed to get XBARCLK P0 info");
				break;
			}
			max_ratio = pclk_domain->ratio_domain;

			if (vf_point->xbar_mhz < p0_info->min_mhz) {
				vf_point->xbar_mhz = p0_info->min_mhz;
			}
			if (vf_point->xbar_mhz > p0_info->max_mhz) {
				vf_point->xbar_mhz = p0_info->max_mhz;
			}
			change_input->clk[i].clk_freq_khz =
					(u32)vf_point->xbar_mhz * 1000U;
			change_input->clk_domains_mask.super.data[0] |=
					(u32) BIT(i);
			if (vf_point->gpc_mhz < vf_point->xbar_mhz) {
				max_clkmhz = (((u32)vf_point->xbar_mhz * 100U) /
						(u32)max_ratio);
				if (*gpcclk_clkmhz < max_clkmhz) {
					*gpcclk_clkmhz = max_clkmhz;
				}
			}
			break;
		case CTRL_CLK_DOMAIN_SYSCLK:
			p0_info = nvgpu_pmu_perf_pstate_get_clk_set_info(g,
					CTRL_PERF_PSTATE_P0, CLKWHICH_SYSCLK);
			if (p0_info == NULL) {
				nvgpu_err(g, "failed to get SYSCLK P0 info");
				break;
			}
			max_ratio = pclk_domain->ratio_domain;
			if (vf_point->sys_mhz < p0_info->min_mhz) {
				vf_point->sys_mhz = p0_info->min_mhz;
			}
			if (vf_point->sys_mhz > p0_info->max_mhz) {
				vf_point->sys_mhz = p0_info->max_mhz;
			}
			change_input->clk[i].clk_freq_khz =
					(u32)vf_point->sys_mhz * 1000U;
			change_input->clk_domains_mask.super.data[0] |=
					(u32) BIT(i);
			if (vf_point->gpc_mhz < vf_point->sys_mhz) {
				max_clkmhz = (((u32)vf_point->sys_mhz * 100U) /
						(u32)max_ratio);
				if (*gpcclk_clkmhz < max_clkmhz) {
					*gpcclk_clkmhz = max_clkmhz;
				}
			}
			break;
		case CTRL_CLK_DOMAIN_NVDCLK:
			p0_info = nvgpu_pmu_perf_pstate_get_clk_set_info(g,
					CTRL_PERF_PSTATE_P0, CLKWHICH_NVDCLK);
			if (p0_info == NULL) {
				nvgpu_err(g, "failed to get NVDCLK P0 info");
				break;
			}
			max_ratio = pclk_domain->ratio_domain;
			if (vf_point->nvd_mhz < p0_info->min_mhz) {
				vf_point->nvd_mhz = p0_info->min_mhz;
			}
			if (vf_point->nvd_mhz > p0_info->max_mhz) {
				vf_point->nvd_mhz = p0_info->max_mhz;
			}
			change_input->clk[i].clk_freq_khz =
					(u32)vf_point->nvd_mhz * 1000U;
			change_input->clk_domains_mask.super.data[0] |=
					(u32) BIT(i);
			if (vf_point->gpc_mhz < vf_point->nvd_mhz) {
				max_clkmhz = (((u32)vf_point->nvd_mhz * 100U) /
						(u32)max_ratio);
				if (*gpcclk_clkmhz < max_clkmhz) {
					*gpcclk_clkmhz = max_clkmhz;
				}
			}
			break;
		case CTRL_CLK_DOMAIN_HOSTCLK:
			p0_info = nvgpu_pmu_perf_pstate_get_clk_set_info(g,
					CTRL_PERF_PSTATE_P0, CLKWHICH_HOSTCLK);
			if (p0_info == NULL) {
				nvgpu_err(g, "failed to get HOSTCLK P0 info");
				break;
			}
			max_ratio = pclk_domain->ratio_domain;
			if (vf_point->host_mhz < p0_info->min_mhz) {
				vf_point->host_mhz = p0_info->min_mhz;
			}
			if (vf_point->host_mhz > p0_info->max_mhz) {
				vf_point->host_mhz = p0_info->max_mhz;
			}
			change_input->clk[i].clk_freq_khz =
					(u32)vf_point->host_mhz * 1000U;
			change_input->clk_domains_mask.super.data[0] |=
					(u32) BIT(i);
			if (vf_point->gpc_mhz < vf_point->host_mhz) {
				max_clkmhz = (((u32)vf_point->host_mhz * 100U) /
						(u32)max_ratio);
				if (*gpcclk_clkmhz < max_clkmhz) {
					*gpcclk_clkmhz = max_clkmhz;
				}
			}
			break;
		default:
			nvgpu_pmu_dbg(g, "Fixed clock domain");
			break;
		}
	}
}
#endif

s32 clk_domain_init_pmupstate(struct gk20a *g)
{
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);
	if (ver == NVGPU_GPUID_GB20C) {
		/* If already allocated, do not re-allocate */
		if (g->pmu->clk_pmu->clk_domainobjs_50 != NULL) {
			return 0;
		}

		g->pmu->clk_pmu->clk_domainobjs_50 = nvgpu_kzalloc(g,
				sizeof(*g->pmu->clk_pmu->clk_domainobjs_50));
		if (g->pmu->clk_pmu->clk_domainobjs_50 == NULL) {
			return -ENOMEM;
		}
	} else {
		/* If already allocated, do not re-allocate */
		if (g->pmu->clk_pmu->clk_domainobjs != NULL) {
			return 0;
		}

		g->pmu->clk_pmu->clk_domainobjs = nvgpu_kzalloc(g,
				sizeof(*g->pmu->clk_pmu->clk_domainobjs));
		if (g->pmu->clk_pmu->clk_domainobjs == NULL) {
			return -ENOMEM;
		}
	}

	vf_points_status = NULL;
	return 0;
}

void clk_domain_free_pmupstate(struct gk20a *g)
{
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);
	if (ver == NVGPU_GPUID_GB20C) {
		nvgpu_kfree(g, g->pmu->clk_pmu->clk_domainobjs_50);
		g->pmu->clk_pmu->clk_domainobjs_50 = NULL;
	} else {
		nvgpu_kfree(g, g->pmu->clk_pmu->clk_domainobjs);
		g->pmu->clk_pmu->clk_domainobjs = NULL;
	}

	clk_domain_free_vf_tables(g);
}

s32 nvgpu_pmu_clk_domain_get_from_index(struct gk20a *g, u32 *domain, u32 index)
{
	struct nvgpu_clk_domain *clk_domain;
	struct boardobjgrp_e32 *psuper;

	if (g->pmu->clk_pmu->clk_domainobjs_50 != NULL) {
		psuper = &g->pmu->clk_pmu->clk_domainobjs_50->super.super;
	} else if (g->pmu->clk_pmu->clk_domainobjs != NULL) {
		psuper = &g->pmu->clk_pmu->clk_domainobjs->super;
	} else {
		return -EINVAL;
	}
	clk_domain = (struct nvgpu_clk_domain *) BOARDOBJGRP_OBJ_GET_BY_IDX(
		&(psuper->super), (u8)index);
	if (clk_domain == NULL)
		return -EINVAL;

	*domain = clk_domain->domain;
	return 0;
}

s32 nvgpu_pmu_clk_domain_get_f_points(struct gk20a *g,
		u32 clkapidomain,
		u32 *pfpointscount,
		u16 *pfreqpointsinmhz)
{
	s32 status = -EINVAL;
	struct nvgpu_clk_domain *pdomain;
	struct nvgpu_clk_pmupstate *pclk = g->pmu->clk_pmu;
	u16 i;
	struct boardobjgrp_e32 *psuper;

	if (g->pmu->clk_pmu->clk_domainobjs_50 != NULL) {
		psuper = &g->pmu->clk_pmu->clk_domainobjs_50->super.super;
	} else if (g->pmu->clk_pmu->clk_domainobjs != NULL) {
		psuper = &g->pmu->clk_pmu->clk_domainobjs->super;
	} else {
		return -EINVAL;
	}

	BOARDOBJGRP_FOR_EACH(&(psuper->super),
			struct nvgpu_clk_domain *, pdomain, i) {
		if (pdomain->api_domain == clkapidomain) {
			status = pdomain->clkdomainclkgetfpoints(g, pclk,
				pdomain, pfpointscount,
				pfreqpointsinmhz,
				CLK_PROG_VFE_ENTRY_LOGIC);
			return status;
		}
	}
	return status;
}

u8 nvgpu_pmu_clk_domain_update_clk_info(struct gk20a *g,
		struct ctrl_clk_clk_domain_list *clk_list)
{
	struct nvgpu_pmu_perf_pstate_clk_info *p0_info;
	struct nvgpu_clk_domain *pdomain;
	u16 i = 0U, num_domains = 0U;
	struct boardobjgrp_e32 *psuper;

	if (g->pmu->clk_pmu->clk_domainobjs_50 != NULL) {
		psuper = &g->pmu->clk_pmu->clk_domainobjs_50->super.super;
	} else if (g->pmu->clk_pmu->clk_domainobjs != NULL) {
		psuper = &g->pmu->clk_pmu->clk_domainobjs->super;
	} else {
		return 0;
	}

	BOARDOBJGRP_FOR_EACH(&(psuper->super),
		struct nvgpu_clk_domain *, pdomain, i) {

		p0_info = nvgpu_pmu_perf_pstate_get_clk_set_info(g,
				CTRL_PERF_PSTATE_P0, pdomain->domain);
		if (p0_info == NULL) {
			nvgpu_err(g, "Unable to get P0 info");
			return num_domains;
		}
		clk_list->clk_domains[i].clk_domain =
			pdomain->api_domain;

		clk_list->clk_domains[i].clk_freq_khz =
			p0_info->nominal_mhz * 1000U;

		/* VBIOS always boots with FFR*/
		clk_list->clk_domains[i].regime_id =
			CTRL_CLK_FLL_REGIME_ID_FFR;

		num_domains =
		nvgpu_safe_cast_u32_to_u8(nvgpu_safe_add_u32(num_domains, 1U));

		nvgpu_pmu_dbg(g, "Domain %x, Nom Freq = %d Max Freq =%d, regime %d",
			pdomain->api_domain, p0_info->nominal_mhz,
			p0_info->max_mhz, CTRL_CLK_FLL_REGIME_ID_FFR);
	}
	return num_domains;
}


s32 clk_domain_get_vf_tables(struct gk20a *g)
{
	s32 status = 0;

	nvgpu_log_info(g, " ");

	// release any previous VF tables
	if (vf_points_status != NULL) {
		nvgpu_kfree(g, vf_points_status);
		vf_points_status = NULL;
	}

	vf_points_status = nvgpu_kzalloc(g, sizeof(*vf_points_status));
	if (vf_points_status == NULL) {
		status = -ENOMEM;
		nvgpu_err(g, "Failed to allocate memory for VF points status");
		goto done;
	}

	// fetch all VF tables from PMU
	status = nvgpu_clk_vf_points_get_status(g, vf_points_status);
	if (status != 0) {
		nvgpu_err(g, "Failed to get VF points status");
		nvgpu_kfree(g, vf_points_status);
		vf_points_status = NULL;
	}

done:
	nvgpu_log_info(g, "Done.");
	return status;
}

void clk_domain_free_vf_tables(struct gk20a *g)
{
	nvgpu_kfree(g, vf_points_status);
	vf_points_status = NULL;
}

s32 nvgpu_pmu_clk_domain_freq_to_volt(struct gk20a *g,
	u16 clkdomain_idx,
	u32 *pclkmhz, u32 *pvoltuv)
{
	struct boardobjgrp_e32 *psuper;
	struct pmu_board_obj *obj;
	struct clk_domain_50_prog_physical *pDomain50ProgPhysical;
	struct clk_domain_50_physical_rail_vf_primary *pVfPrimary;
	struct nv_pmu_clk_clk_vf_rel_vf_entry_pri *pVfEntryPri;
	struct nvgpu_clk_vf_point_status vfPoint;
	struct clk_vf_rel *pVfRel = NULL;
	s32 status = -EINVAL;
	u16 index;
	u32 input_freq;
	struct nvgpu_clk_vf_point_status best_vfPoint;
	u16 best_index;
	u32 best_freq;
	bool found_any;
	u8 railidx;

	nvgpu_pmu_dbg(g, " ");

	if (vf_points_status == NULL) {
		nvgpu_err(g, "VF points status is NULL");
		goto done;
	}

	psuper = &g->pmu->clk_pmu->clk_domainobjs_50->super.super;

	obj = BOARDOBJGRP_OBJ_GET_BY_IDX(&(psuper->super), (u16)clkdomain_idx);
	if (obj == NULL) {
		nvgpu_err(g, "No clkDomain Obj for index: %d", clkdomain_idx);
		goto done;
	}

	if (pmu_board_obj_get_type(obj) != CTRL_CLK_CLK_DOMAIN_TYPE_50_PROG_PHYSICAL) {
		nvgpu_err(g, "clkDomain %d type 0x%x is not PROG_PHYSICAL",
				pmu_board_obj_get_type(obj), clkdomain_idx);
		goto done;
	}

	pDomain50ProgPhysical = (struct clk_domain_50_prog_physical *) obj;
	//
	// We expect all programmable domains on GB20C to have
	// exactly one voltage rail powering the clock domain.
	// Thus no need to iterate over a railmask.
	//
	railidx = clkDomainProgVoltRailIdxGet_50_PROG_PHYSICAL(g, (void *)pDomain50ProgPhysical);
	pVfPrimary = &pDomain50ProgPhysical->rail_vf_item[railidx].data.primary;
	if (pVfPrimary == NULL) {
		nvgpu_err(g, "Invalid rail VF item for clock domain %d - rail %d",
			clkdomain_idx, railidx);
		goto done;
	}

	pVfRel = CLK_CLK_VF_REL_GET(g->pmu->clk_pmu, pVfPrimary->clk_vf_rel_idx_first);

	if (pVfRel == NULL) {
		nvgpu_err(g, "Unable to obtain VF_REL object for clock domain %d", clkdomain_idx);
		goto done;
	}

	pVfEntryPri = &pVfRel->vf_entry_pri;

	status = 0;

	// Search over the relevant block of VF POINT board objects with floor logic
	input_freq = *pclkmhz;
	best_index = 0;
	best_freq = 0;
	found_any = false;

	for (index = pVfEntryPri->vfPointIdxFirst; index <= pVfEntryPri->vfPointIdxLast; index++) {

		// Check bounds
		if (index >= vf_points_status->super.objCount) {
			nvgpu_err(g, "VF Point index %d out of bounds (max %d)",
				  index, vf_points_status->super.objCount);
			continue;
		}

		// Use the VF point data directly from the cached structure (NO PMU CALL!)
		vfPoint = vf_points_status->vf_points[index];

		/* Only consider VF points with frequency <= target */
		if (vfPoint.freq_mhz <= input_freq) {
			/* Update best match if this is higher frequency but still <= target */
			if (vfPoint.freq_mhz > best_freq) {
				best_freq = vfPoint.freq_mhz;
				best_vfPoint = vfPoint;
				best_index = index;
				found_any = true;
			}
		}
	}

	if (found_any) {
		*pvoltuv = best_vfPoint.voltage_uv;
		*pclkmhz = best_vfPoint.freq_mhz;
		nvgpu_pmu_dbg(g, "F->V Floor: Domain %d, VF index %d, %u MHz (<= target %u MHz) -> %u uV",
			   clkdomain_idx, best_index, *pclkmhz, input_freq, *pvoltuv);
	} else {
		/* No VF point found <= target, use minimum frequency */

		if (pVfEntryPri->vfPointIdxFirst < vf_points_status->super.objCount) {
			vfPoint = vf_points_status->vf_points[pVfEntryPri->vfPointIdxFirst];
			*pvoltuv = vfPoint.voltage_uv;
			*pclkmhz = vfPoint.freq_mhz;
			nvgpu_pmu_dbg(g, "F->V Floor: Domain %d, VF index %d, %u MHz (minimum, target %u MHz too low) -> %u uV",
				   clkdomain_idx, pVfEntryPri->vfPointIdxFirst,
				   *pclkmhz, input_freq, *pvoltuv);
		} else {
			nvgpu_err(g, "No valid VF points found for domain %d", clkdomain_idx);
			status = -EINVAL;
		}
	}

done:
	nvgpu_pmu_dbg(g, "Done.");
	return status;
}


s32 nvgpu_pmu_clk_domain_get_index_by_domain(struct gk20a *g,
		u32 domain,
		u32 *idx)
{
	s32 status = -EINVAL;
	struct nvgpu_clk_domain *pdomain;
	struct nvgpu_clk_pmupstate *pclk = g->pmu->clk_pmu;
	u16 domainIndex;

	// Only need to support v5 domains
	BOARDOBJGRP_FOR_EACH(&(pclk->clk_domainobjs_50->super.super.super),
			struct nvgpu_clk_domain *, pdomain, domainIndex) {
		if (pdomain->domain == domain) {
			status = 0;
			*idx = domainIndex;
			return status;
		}
	}
	return status;
}

s32 clkClkDomainGetApiDomainByIndex(struct gk20a *g, u32 *api_domain, u32 index)
{
	struct nvgpu_clk_domain *clk_domain;
	struct boardobjgrp_e32 *psuper;
	s32 status = 0;

	if (g->pmu->clk_pmu->clk_domainobjs_50 != NULL) {
		psuper = &g->pmu->clk_pmu->clk_domainobjs_50->super.super;
	} else if (g->pmu->clk_pmu->clk_domainobjs != NULL) {
		psuper = &g->pmu->clk_pmu->clk_domainobjs->super;
	} else {
		status = -EINVAL;
		goto done;
	}
	clk_domain = (struct nvgpu_clk_domain *) BOARDOBJGRP_OBJ_GET_BY_IDX(
		&(psuper->super), (u16)index);
	if (clk_domain == NULL) {
		status = -EINVAL;
		goto done;
	}
	*api_domain = clk_domain->api_domain;

done:
	return status;
}

s32 nvgpu_clk_domain_volt_to_freq(struct gk20a *g, u16 clkdomain_idx,
	u32 *pclkmhz, u32 *pvoltuv)
{
	struct boardobjgrp_e32 *psuper;
	struct pmu_board_obj *obj;
	struct clk_domain_50_prog_physical *pDomain50ProgPhysical;
	struct clk_domain_50_physical_rail_vf_primary *pVfPrimary;
	struct nv_pmu_clk_clk_vf_rel_vf_entry_pri *pVfEntryPri;
	struct nvgpu_clk_vf_point_status vfPoint;
	struct clk_vf_rel *pVfRel = NULL;
	s32 status = -EINVAL;
	u16 index;
	u32 input_volt;
	struct nvgpu_clk_vf_point_status best_vfPoint;
	u16 best_index;
	u32 best_volt;
	bool found_any;
	u8 railidx;

	nvgpu_pmu_dbg(g, " ");

	if (vf_points_status == NULL) {
		nvgpu_err(g, "VF points status is NULL");
		goto done;
	}

	psuper = &g->pmu->clk_pmu->clk_domainobjs_50->super.super;

	obj = BOARDOBJGRP_OBJ_GET_BY_IDX(&(psuper->super), (u16)clkdomain_idx);
	if (obj == NULL) {
		nvgpu_err(g, "No clkDomain Obj for index: %d", clkdomain_idx);
		goto done;
	}

	if (pmu_board_obj_get_type(obj) != CTRL_CLK_CLK_DOMAIN_TYPE_50_PROG_PHYSICAL) {
		nvgpu_err(g, "clkDomain %d type 0x%x is not PROG_PHYSICAL",
				pmu_board_obj_get_type(obj), clkdomain_idx);
		goto done;
	}

	pDomain50ProgPhysical = (struct clk_domain_50_prog_physical *) obj;
	//
	// We expect all programmable domains on GB20C to have
	// exactly one voltage rail powering the clock domain.
	// Thus no need to iterate over a railmask.
	//
	railidx = clkDomainProgVoltRailIdxGet_50_PROG_PHYSICAL(g, (void *)pDomain50ProgPhysical);
	pVfPrimary = &pDomain50ProgPhysical->rail_vf_item[railidx].data.primary;
	if (pVfPrimary == NULL) {
		nvgpu_err(g, "Invalid rail VF item for clock domain %d - rail %d",
			clkdomain_idx, railidx);
		goto done;
	}
	pVfRel = CLK_CLK_VF_REL_GET(g->pmu->clk_pmu, pVfPrimary->clk_vf_rel_idx_first);
	if (pVfRel == NULL) {
		nvgpu_err(g, "Unable to obtain VF_REL object for clock domain %d", clkdomain_idx);
		goto done;
	}

	pVfEntryPri = &pVfRel->vf_entry_pri;

	status = 0;

	// Search over the relevant block of VF POINT board objects with floor logic
	input_volt = *pvoltuv;
	best_index = 0;
	best_volt = 0;
	found_any = false;

	for (index = pVfEntryPri->vfPointIdxFirst; index <= pVfEntryPri->vfPointIdxLast; index++) {

		// Check bounds
		if (index >= vf_points_status->super.objCount) {
			nvgpu_err(g, "VF Point index %d out of bounds (max %d)",
				  index, vf_points_status->super.objCount);
			continue;
		}

		// Use the VF point data directly from the cached structure (NO PMU CALL!)
		vfPoint = vf_points_status->vf_points[index];

		/* Only consider VF points with voltage <= target */
		if (vfPoint.voltage_uv <= input_volt) {
			/* Update best match if this is higher voltage but still <= target */
			if (vfPoint.voltage_uv > best_volt) {
				best_volt = vfPoint.voltage_uv;
				best_vfPoint = vfPoint;
				best_index = index;
				found_any = true;
			}
		}
	}

	if (found_any) {
		*pclkmhz = best_vfPoint.freq_mhz;
		*pvoltuv = best_vfPoint.voltage_uv;
		nvgpu_pmu_dbg(g, "V->F Floor: Domain %d, VF index %d, %u uV (<= target %u uV) -> %u MHz",
			   clkdomain_idx, best_index, *pvoltuv, input_volt, *pclkmhz);
	} else {
		/* No VF point found <= target, use minimum voltage */

		if (pVfEntryPri->vfPointIdxFirst < vf_points_status->super.objCount) {
			vfPoint = vf_points_status->vf_points[pVfEntryPri->vfPointIdxFirst];
			*pclkmhz = vfPoint.freq_mhz;
			*pvoltuv = vfPoint.voltage_uv;
			nvgpu_pmu_dbg(g, "V->F Floor: Domain %d, VF index %d, %u uV (minimum, target %u uV too low) -> %u MHz",
				   clkdomain_idx, pVfEntryPri->vfPointIdxFirst,
				   *pvoltuv, input_volt, *pclkmhz);
		} else {
			nvgpu_err(g, "No valid VF points found for domain %d", clkdomain_idx);
			status = -EINVAL;
		}
	}

done:
	nvgpu_pmu_dbg(g, "Done.");
	return status;
}

int nvgpu_clk_find_vf_point_for_gpc_rate(struct gk20a *g, u32 gpc_freq_mhz,
				       struct nvgpu_clk_vf_point_5x *vf_point)
{
	u32 gpc_volt_uv = 0;
	u32 gpm_volt_uv = 0;
	u32 sys_gpm_volt_uv, nvd_gpm_volt_uv, uproc_gpm_volt_uv;
	u32 temp_freq, temp_volt;
	u32 sys_ratio_percent;
	u16 gpc_domain_idx, xbar_domain_idx, nvd_domain_idx, uproc_domain_idx;
	int ret = 0;

	if (!vf_point) {
		nvgpu_err(g, "Invalid VF point pointer");
		ret = -EINVAL;
		goto done;
	}

	/* Initialize VF point structure */
	memset(vf_point, 0, sizeof(*vf_point));

	/* Get domain indices for all clock domains */
	ret = nvgpu_pmu_clk_domain_get_index_by_domain(g, CLKWHICH_GPCCLK, (u32 *)&gpc_domain_idx);
	if (ret != 0) {
		nvgpu_err(g, "Failed to get GPC domain index");
		goto done;
	}

	ret = nvgpu_pmu_clk_domain_get_index_by_domain(g, CLKWHICH_XBARCLK, (u32 *)&xbar_domain_idx);
	if (ret != 0) {
		nvgpu_err(g, "Failed to get XBAR domain index");
		goto done;
	}

	ret = nvgpu_pmu_clk_domain_get_index_by_domain(g, CLKWHICH_NVDCLK, (u32 *)&nvd_domain_idx);
	if (ret != 0) {
		nvgpu_err(g, "Failed to get NVD domain index");
		goto done;
	}

	ret = nvgpu_pmu_clk_domain_get_index_by_domain(g, CLKWHICH_UPROCCLK, (u32 *)&uproc_domain_idx);
	if (ret != 0) {
		nvgpu_err(g, "Failed to get UPROC domain index");
		goto done;
	}

	/* Step 0: Fetch all VF points ONCE to eliminate redundant RPC to PMU
	 * in each of the F->V and V->F steps below.
	 */
	ret = clk_domain_get_vf_tables(g);
	if (ret != 0) {
		nvgpu_err(g, "Failed to get VF Curves from PMU");
		goto done;
	}

	nvgpu_pmu_dbg(g, "=== VF Point Lookup Request ===");
	nvgpu_pmu_dbg(g, "Requested GPC Frequency: %u MHz", gpc_freq_mhz);
	nvgpu_pmu_dbg(g, "Domain indices: GPC=%d, XBAR=%d, NVD=%d, UPROC=%d",
		   gpc_domain_idx, xbar_domain_idx, nvd_domain_idx, uproc_domain_idx);

	/* Step 1: Set GPC frequency */
	vf_point->gpc_mhz = gpc_freq_mhz;

	/* Step 2: Get GPC voltage from GPC frequency using F→V */
	temp_freq = gpc_freq_mhz;
	ret = nvgpu_pmu_clk_domain_freq_to_volt(g, gpc_domain_idx, &temp_freq, &gpc_volt_uv);
	if (ret != 0) {
		nvgpu_err(g, "Failed to get GPC voltage for freq %u MHz", gpc_freq_mhz);
		goto cleanup;
	}
	/* Use the actual frequency and voltage from VF point */
	vf_point->gpc_mhz = temp_freq;
	vf_point->gpc_volt_uv = gpc_volt_uv;

	/* Step 3: Calculate SYS frequency (% of actual GPC) and get GPM voltage */

	/* Get actual ratio percentage from parsed VBIOS data */
	ret = get_gpc_to_sys_clk_freq_ratio_percent(g, &sys_ratio_percent);
	if (ret != 0) {
		nvgpu_err(g, "Failed to get GPC to SYS frequency ratio from clk_top_relobjs");
		goto cleanup;
	}

	temp_freq = (vf_point->gpc_mhz * sys_ratio_percent) / 100U;
	nvgpu_pmu_dbg(g, "DEBUG: Calculated SYS freq = %u MHz (%d%% of GPC %u MHz)",
		   temp_freq, sys_ratio_percent, vf_point->gpc_mhz);

	ret = nvgpu_pmu_clk_domain_freq_to_volt(g, xbar_domain_idx, &temp_freq, &gpm_volt_uv);
	if (ret != 0) {
		nvgpu_err(g, "Failed to get GPM voltage for SYS freq %u MHz", temp_freq);
		goto cleanup;
	}

	/* Use the actual values from VF point */
	nvgpu_pmu_dbg(g, "DEBUG: Final SYS freq=%u MHz, GPM volt=%u uV", temp_freq, gpm_volt_uv);
	vf_point->sys_mhz = temp_freq;
	sys_gpm_volt_uv = gpm_volt_uv;

	/* Step 4: Get NVD frequency from GPM voltage using V→F on NVD domain */
	temp_volt = sys_gpm_volt_uv;
	ret = nvgpu_clk_domain_volt_to_freq(g, nvd_domain_idx, &temp_freq, &temp_volt);
	if (ret != 0) {
		nvgpu_err(g, "Failed to get NVD frequency for voltage %u uV", sys_gpm_volt_uv);
		goto cleanup;
	}
	/* Use the actual values from VF point */
	vf_point->nvd_mhz = temp_freq;
	nvd_gpm_volt_uv = temp_volt;

	/* Step 5: Get UPROC frequency from GPM voltage using V→F on UPROC domain */
	temp_volt = sys_gpm_volt_uv;
	temp_freq = 0;
	ret = nvgpu_clk_domain_volt_to_freq(g, uproc_domain_idx, &temp_freq, &temp_volt);
	if (ret != 0) {
		nvgpu_err(g, "Failed to get UPROC frequency for voltage %u uV", sys_gpm_volt_uv);
		goto cleanup;
	}
	/* Use the actual values from VF point */
	vf_point->uproc_mhz = temp_freq;
	uproc_gpm_volt_uv = temp_volt;

	/* Step 6: Set GPM voltage to maximum of all three domains */
	vf_point->gpm_volt_uv = max(max(sys_gpm_volt_uv, nvd_gpm_volt_uv), uproc_gpm_volt_uv);

	/* Step 7: Set clock domain mask using domain indices */
	vf_point->clk_domain_mask = BIT(gpc_domain_idx) | BIT(xbar_domain_idx) |
				    BIT(nvd_domain_idx) | BIT(uproc_domain_idx);

	nvgpu_pmu_dbg(g, "Clock domain mask: 0x%x (GPC:%d XBAR:%d NVD:%d UPROC:%d)",
		   vf_point->clk_domain_mask, gpc_domain_idx, xbar_domain_idx,
		   nvd_domain_idx, uproc_domain_idx);

	nvgpu_pmu_dbg(g, "=== FINAL VF POINT RESULT ===");
	nvgpu_pmu_dbg(g, "Expected SYS freq should be: %u MHz (90%% of GPC)",
		   (vf_point->gpc_mhz * 90) / 100);
	nvgpu_pmu_dbg(g, "SYS constraint check: SYS %u MHz %s GPC %u MHz",
		   vf_point->sys_mhz,
		   (vf_point->sys_mhz <= vf_point->gpc_mhz) ? "<=" : ">",
		   vf_point->gpc_mhz);
	nvgpu_pmu_dbg(g,
		   "Auto VF Point: GPC=%u MHz/%u uV, SYS=%u MHz, NVD=%u MHz, UPROC=%u MHz,",
		   vf_point->gpc_mhz, vf_point->gpc_volt_uv, vf_point->sys_mhz,
		   vf_point->nvd_mhz, vf_point->uproc_mhz);
	nvgpu_pmu_dbg(g,
		  "Auto VF Point: GPM=%u uV (SYS:%u NVD:%u UPROC:%u)",
		   vf_point->gpm_volt_uv, sys_gpm_volt_uv, nvd_gpm_volt_uv, uproc_gpm_volt_uv);

cleanup:
	clk_domain_free_vf_tables(g);

done:
	return ret;
}
