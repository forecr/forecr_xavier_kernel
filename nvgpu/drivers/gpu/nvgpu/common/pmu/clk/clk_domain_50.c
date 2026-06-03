// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

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

#include "common/pmu/volt/volt.h"
#include "common/pmu/clk/clk_vf_rel.h"



static s32 devinitClocksTableHALGetClientDomainsMask_1X(
	struct vbios_clocks_table_1x_hal_clock_entry *pClocksHAL,
	u8 numEntries,
	struct boardobjgrpmask_e32 *pClientDomainsMask)
{
	s32 status = -EINVAL;
	u8 i;

	if (numEntries <= NV_VBIOS_CLK_MAX_DEVICES) {
		status = boardobjgrpmask_e32_init(pClientDomainsMask, NULL);
		if (status == 0) {
			for (i = 0; i < numEntries; i++) {
				if (pClocksHAL[i].public_domain !=
					NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID) {
					status = nvgpu_boardobjgrpmask_bit_set(&pClientDomainsMask->super, i);
					if (status != 0)
						break;
				}
			}
		}
	}

	return status;
}

struct nvgpu_clk_domain *clkDomainsFindByApiDomain(struct gk20a *g,
	u32 apiDomain)
{
	struct nvgpu_clk_domains_5x *pClkDomains = &g->pmu->clk_pmu->clk_domainobjs_50->super;
	struct nvgpu_clk_domain *pDomain;
	u16 i;

	// Search through the CLK_DOMAIN objects.
	BOARDOBJGRP_FOR_EACH(&(pClkDomains->super.super),
		struct nvgpu_clk_domain *, pDomain, i) {
		if (pDomain->api_domain == apiDomain)
			return pDomain;
	}

	// Not found.
	return NULL;
}

static s32 devinit_get_clocks_table_50(struct gk20a *g,
		u8 *clocks_table_ptr)
{
	s32 status = 0;
	struct vbios_clocks_table_5x_header clocks_table_header = { 0 };
	struct vbios_clocks_table_5x_entry clocks_table_entry = { 0 };
	struct vbios_clocks_table_50_vf_item_entry vfItemEntry = { 0 };
	struct vbios_clocks_table *pClocksHAL = NULL;
	const u8 *clocks_tbl_entry_ptr = NULL;
	u32 index = 0;
	u32 railIdx;
	struct nvgpu_clk_domain *pclkdomain_dev;
	struct nvgpu_clk_domains_5x *pclkdomainobjs = &g->pmu->clk_pmu->clk_domainobjs_50->super;
	/*
	 * misra_c_2012_rule_19_2_violation
	 * MISRA C-2012 Rule 19.2 a typedef Union is used to reduce
	 * the stackframe memory footprint by allocating the memory for
	 * the union of boardobjs.
	 * This prevents a "frame-larger-than 2048 bytes" compile error.
	 */
	typedef union {
		struct pmu_board_obj obj;
		struct nvgpu_clk_domain clk_domain;
		struct clk_domain_50 v5x;
		struct clk_domain_50_fixed v5x_fixed;
		struct clk_domain_50_prog_physical v50_prog_phy;
	} u_clk_domain_data;
	u_clk_domain_data *pclk_domain_data = NULL;

	nvgpu_log_info(g, " ");

	pclk_domain_data = (u_clk_domain_data *) nvgpu_kzalloc(g, sizeof(u_clk_domain_data));
	if (pclk_domain_data == NULL) {
		nvgpu_err(g, "Unable to allocate memory.");
		status = -ENOMEM;
		goto done_no_free;
	}

	nvgpu_memcpy((u8 *)&clocks_table_header, clocks_table_ptr,
			VBIOS_CLOCKS_TABLE_5x_HEADER_SIZE_13);
	if (clocks_table_header.header_size <
			(u8) VBIOS_CLOCKS_TABLE_5x_HEADER_SIZE_13) {
		nvgpu_err(g, "Unrecognized Clocks Table header size - size=0x%02x",
			clocks_table_header.header_size);
		status = -EINVAL;
		goto done;
	}

	if (clocks_table_header.base_entry_size <
			(u8) VBIOS_CLOCKS_TABLE_5x_ENTRY_SIZE_15) {
		status = -EINVAL;
		nvgpu_err(g, "Unrecognized Clocks Table entry size - size=0x%02x",
			clocks_table_header.base_entry_size);
		goto done;
	}

	if (clocks_table_header.vf_item_entry_size <
		(u8) VBIOS_CLOCKS_TABLE_50_VF_ITEM_ENTRY_SIZE_03) {
		nvgpu_err(g, "Unrecognized Clocks Table VF Item entry size - size=0x%02x",
			clocks_table_header.vf_item_entry_size);
		status = -EINVAL;
		goto done;
	}

	// Cache clocksHAL value for parsing FactoryOC table
	pclkdomainobjs->vbiosClocksHAL = clocks_table_header.clocks_hal;

	// Initialize global state from the Clocks Table Header.
	status = devinitClocksTableHALTranslate_1X(g, pclkdomainobjs->vbiosClocksHAL, &pClocksHAL);
	if (status == -EINVAL) {
		nvgpu_err(g, "Error translating Clocks HAL - status=%d", status);
		goto done;
	}
	pclkdomainobjs->clocksHAL = pClocksHAL->nvClocksHAL;

	status = devinitClocksTableHALGetClientDomainsMask_1X(pClocksHAL->domains,
			clocks_table_header.base_entry_count,
			&pclkdomainobjs->clientDomainsMask);

	if (status != 0) {
		nvgpu_err(g,
				"Error requesting client clocks masks.");
		goto done;
	}

	pclkdomainobjs->cntrSamplingPeriodms = (u16)clocks_table_header.clock_counter_sample_period;
	pclkdomainobjs->clkMonRefWinUsec     = (u16)clocks_table_header.fmon_ref_window;
	pclkdomainobjs->xbarBoostVfeIdx      = (u16)clocks_table_header.xbar_boost_vfe_idx;

	clocks_tbl_entry_ptr = clocks_table_ptr +
			clocks_table_header.header_size;

	/* Read table entries*/
	for (index = 0; index < clocks_table_header.base_entry_count; index++) {

		/* To avoid MISRA violation,
		 * this is a single point of exit
		 * from this for loop in the case
		 * of an error is encountered.
		 */
		if (status != 0)
			break;

		clocks_tbl_entry_ptr = (clocks_table_ptr + clocks_table_header.header_size +
				(index * (clocks_table_header.base_entry_size +
				(clocks_table_header.vf_item_entry_count *
				clocks_table_header.vf_item_entry_size))));

		nvgpu_memcpy((u8 *)&clocks_table_entry,
			clocks_tbl_entry_ptr, sizeof(clocks_table_entry));

		// Populate the domain values. Only 2 types for GB20C.
		pclk_domain_data->clk_domain.domain =
			(u8) pClocksHAL->domains[index].domain;
		pclk_domain_data->clk_domain.api_domain =
			clktranslatehalmumsettoapinumset(
			BIT32(pclk_domain_data->clk_domain.domain));

		// Set 2X legacy fields.
		pclk_domain_data->clk_domain.perf_domain_grp_idx = 0;
		pclk_domain_data->clk_domain.ratio_domain = CLKWHICH_DEFAULT;
		pclk_domain_data->clk_domain.usage = 0;
		pclk_domain_data->clk_domain.b_noise_aware_capable =
			pClocksHAL->domains[index].b_noise_aware_capable;

		// _50-specific super class parameters:
		switch (BIOS_GET_FIELD(u32, clocks_table_entry.flags0,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_FLAGS0_USAGE)) {
		case  NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_FLAGS0_USAGE_FIXED:
		{
			pclk_domain_data->obj.type =
				CTRL_CLK_CLK_DOMAIN_TYPE_50_FIXED;
			// _FIXED-specific class parameters
			pclk_domain_data->v5x_fixed.freq_mhz = BIOS_GET_FIELD(u16,
				clocks_table_entry.param1,
				NV_VBIOS_CLOCKS_TABLE_50_ENTRY_PARAM1_FIXED_FREQUENCY_MHZ);
			break;
		}
		case  NV_VBIOS_CLOCKS_TABLE_5X_ENTRY_FLAGS0_USAGE_PROG_PHYSICAL:
		{
			s16 grdOffsetMHz;

			pclk_domain_data->obj.type =
				CTRL_CLK_CLK_DOMAIN_TYPE_50_PROG_PHYSICAL;
			pclk_domain_data->v50_prog_phy.clk_vf_curve_count =
				pClocksHAL->domains[index].clk_vf_curve_count;

			pclk_domain_data->v50_prog_phy.logical_domain_idx =
				VBIOS_CLOCKS_TABLE_1X_HAL_CLK_DOMAIN_IDX_INVALID;

			// _PROG-specific super-class parameters
			pclk_domain_data->v50_prog_phy.clk_enum_idx_first =
				BIOS_GET_FIELD(u8, clocks_table_entry.param0,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM0_PROG_CLK_PROG_IDX_FIRST);
			pclk_domain_data->v50_prog_phy.clk_enum_idx_last  =
				BIOS_GET_FIELD(u8, clocks_table_entry.param0,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM0_PROG_CLK_PROG_IDX_LAST);

			pclk_domain_data->v50_prog_phy.freq_delta_min_mhz =
				BIOS_GET_FIELD(s16, clocks_table_entry.param1,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM1_MASTER_FREQ_OC_DELTA_MIN_MHZ);
			pclk_domain_data->v50_prog_phy.freq_delta_max_mhz =
				BIOS_GET_FIELD(s16, clocks_table_entry.param1,
				NV_VBIOS_CLOCKS_TABLE_1X_ENTRY_PARAM1_MASTER_FREQ_OC_DELTA_MAX_MHZ);

			// Init factory offset to default.
			pclk_domain_data->v50_prog_phy.factory_delta.data.delta_khz = 0;
			pclk_domain_data->v50_prog_phy.factory_delta.type = 0;

			grdOffsetMHz = (s32)BIOS_GET_FIELD(s16, clocks_table_entry.param2,
				NV_VBIOS_CLOCKS_TABLE_5x_ENTRY_PARAM2_GRD_FREQUENCY_OFFSET);

			// Init GRD offset.
			pclk_domain_data->v50_prog_phy.grd_freq_delta.type = 0;
			pclk_domain_data->v50_prog_phy.grd_freq_delta.data.delta_khz =
				((s32)grdOffsetMHz) * 1000;

			pclk_domain_data->v50_prog_phy.pre_volt_ordering_index =
				BIOS_GET_FIELD(u8, clocks_table_entry.param3,
				NV_VBIOS_CLOCKS_TABLE_5x_ENTRY_PARAM3_PROG_PRE_VOLT_ORDERING_IDX);

			pclk_domain_data->v50_prog_phy.post_volt_ordering_index =
				BIOS_GET_FIELD(u8, clocks_table_entry.param3,
				NV_VBIOS_CLOCKS_TABLE_5x_ENTRY_PARAM3_PROG_POST_VOLT_ORDERING_IDX);

			pclk_domain_data->v50_prog_phy.fbvdd_data.valid =
				(BIOS_GET_FIELD(u32, clocks_table_entry.param3,
				NV_VBIOS_CLOCKS_TABLE_5x_ENTRY_PARAM3_FBVDD_DATA_SUPPORT) ==
				NV_VBIOS_CLOCKS_TABLE_5x_ENTRY_PARAM3_FBVDD_DATA_SUPPORT_YES);

			pclk_domain_data->v50_prog_phy.cpmu_clk_id =
				(u8) pClocksHAL->domains[index].cpmuClkId;

			// _PROG specific CLK_MON parameters
			pclk_domain_data->v50_prog_phy.clkmon_info.low_threshold_vfe_idx =
				BIOS_GET_FIELD(u16, clocks_table_entry.param4,
				NV_VBIOS_CLOCKS_TABLE_5x_ENTRY_PARAM4_CLK_MONITOR_THRESHOLD_MIN);
			pclk_domain_data->v50_prog_phy.clkmon_info.high_threshold_vfe_idx =
				BIOS_GET_FIELD(u16, clocks_table_entry.param4,
				NV_VBIOS_CLOCKS_TABLE_5x_ENTRY_PARAM4_CLK_MONITOR_THRESHOLD_MAX);

			// Init mask
			pclk_domain_data->v50_prog_phy.rail_mask = 0;

			// Copy out rail specific params.
			for (railIdx = 0; railIdx < clocks_table_header.vf_item_entry_count; railIdx++) {
				u8 data8;
				u8 *vfEntry_ptr;

				vfEntry_ptr = (u8*)(clocks_tbl_entry_ptr +
					clocks_table_header.base_entry_size +
					(railIdx * clocks_table_header.vf_item_entry_size));

				nvgpu_memcpy((u8 *)&vfItemEntry,
					vfEntry_ptr,
					sizeof(vfItemEntry));

				data8 = BIOS_GET_FIELD(u8, vfItemEntry.flags0,
					NV_VBIOS_CLOCKS_TABLE_50_VF_ITEM_ENTRY_FLAGS0_USAGE);
				switch(data8) {
				case NV_VBIOS_CLOCKS_TABLE_50_VF_ITEM_ENTRY_FLAGS0_USAGE_NONE:
				{
					pclk_domain_data->v50_prog_phy.rail_vf_item[railIdx].type =
					CTRL_CLK_CLK_DOMAIN_50_PROG_PHYSICAL_RAIL_VF_TYPE_NONE;
					break;
				}
				case NV_VBIOS_CLOCKS_TABLE_50_VF_ITEM_ENTRY_FLAGS0_USAGE_PRIMARY:
				{
					pclk_domain_data->v50_prog_phy.rail_mask |= BIT32(railIdx);

					pclk_domain_data->v50_prog_phy.rail_vf_item[railIdx].type =
						CTRL_CLK_CLK_DOMAIN_50_PROG_PHYSICAL_RAIL_VF_TYPE_PRIMARY;

					pclk_domain_data->v50_prog_phy.rail_vf_item[railIdx].data.primary.clk_vf_rel_idx_first =
						BIOS_GET_FIELD(u8, vfItemEntry.param0,
							NV_VBIOS_CLOCKS_TABLE_50_VF_ITEM_ENTRY_PARAM0_PRIMARY_CLK_VF_REL_IDX_FIRST);
					pclk_domain_data->v50_prog_phy.rail_vf_item[railIdx].data.primary.clk_vf_rel_idx_last =
						BIOS_GET_FIELD(u8, vfItemEntry.param0,
							NV_VBIOS_CLOCKS_TABLE_50_VF_ITEM_ENTRY_PARAM0_PRIMARY_CLK_VF_REL_IDX_LAST);
					break;
				}
				case NV_VBIOS_CLOCKS_TABLE_50_VF_ITEM_ENTRY_FLAGS0_USAGE_SECONDARY:
				{
					pclk_domain_data->v50_prog_phy.rail_mask |= BIT32(railIdx);

					pclk_domain_data->v50_prog_phy.rail_vf_item[railIdx].type   =
						CTRL_CLK_CLK_DOMAIN_50_PROG_PHYSICAL_RAIL_VF_TYPE_SECONDARY;

					pclk_domain_data->v50_prog_phy.rail_vf_item[railIdx].data.secondary.primary_idx =
						BIOS_GET_FIELD(u8, vfItemEntry.param0,
							NV_VBIOS_CLOCKS_TABLE_50_VF_ITEM_ENTRY_PARAM0_SECONDARY_PRIMARY_DOMAIN);

					// Sanity check the PRIMARY domain index.
					if ((pclk_domain_data->v50_prog_phy.rail_vf_item[railIdx].data.secondary.primary_idx >=
						VBIOS_CLOCKS_TABLE_1X_HAL_NUM_CLOCK_ENTRIES) ||
						(pClocksHAL->domains[pclk_domain_data->v50_prog_phy.rail_vf_item[railIdx].data.secondary.primary_idx].domain ==
						CLKWHICH_DEFAULT)) {
						status = -EINVAL;
						nvgpu_err(g,
							"Clock Domain %d Invalid Primary Domain Index: %d.",
							index,
							pclk_domain_data->v50_prog_phy.rail_vf_item[railIdx].data.secondary.primary_idx);
						/* Will exit at top of outer for loop. */
					}
					break;
				}
				default:
				{
					status = -EINVAL;
					nvgpu_err(g,
						"Error reading rail specific type (primary | secondary). type = 0x%x.",
						data8);
					/* Will exit at top of outer for loop. */
					break;
				}
				}
				if (status == -EINVAL)
					break; /* to avoid MISRA violation, single exit point at top of OUTER loop */
			}
			break;
		}
		default:
		{
			nvgpu_err(g,
				  "Invalid clock domain type at entry %d", index);
			status = -EINVAL;
			break;
		}
		}

		if (status == -EINVAL)
			continue; /* to avoid MISRA violation, single exit point at top of loop */

		// Construct CLK_DOMAIN object for this CLK_DOMAIN
		pclkdomain_dev = construct_clk_domain(g, (void *)pclk_domain_data);
		if (pclkdomain_dev == NULL) {
			nvgpu_err(g,
			"Unable to construct clock domain boardobj for entry %d",
			index);
			status = -EINVAL;
			continue; /* to avoid MISRA violation, single exit point at top of loop */
		}

		// Insert the CLK_DOMAIN into the CLK_DOMAINS group.
		status = boardobjgrp_objinsert(&pclkdomainobjs->super.super,
				(struct pmu_board_obj *)
				pclkdomain_dev, (u16)index);
		if (status != 0) {
			nvgpu_err(g,
			"unable to insert clock domain boardobj for %d", index);
			status = -EINVAL;
			continue; /* to avoid MISRA violation, single exit point at top of loop */
		}

		if (pmu_board_obj_get_type(pclkdomain_dev) == CTRL_CLK_CLK_DOMAIN_TYPE_50_PROG_PHYSICAL) {
			struct clk_domain_50_prog_physical *pDomain50ProgPhysical = (struct clk_domain_50_prog_physical *)pclkdomain_dev;

			status = nvgpu_boardobjgrpmask_bit_set(
				&g->pmu->clk_pmu->clk_domainobjs_50->super.progDomainsMask.super,
				pclkdomain_dev->super.idx);
			if (status == 0) {
				status = nvgpu_boardobjgrpmask_bit_set(
					&g->pmu->clk_pmu->clk_domainobjs_50->progPhysicalDomainsMask.super,
					pclkdomain_dev->super.idx);
			}
			if (status == 0) {
				status = nvgpu_boardobjgrpmask_bit_set(
					&g->pmu->clk_pmu->clk_domainobjs_50->progClientVisibleDomainsMask.super,
					pclkdomain_dev->super.idx);
			}
			if (status == 0) {
				if (pDomain50ProgPhysical->cpmu_clk_id != NV2080_CTRL_CLK_CLK_DOMAIN_CPMU_CLOCK_ID_INVALID) {
					status = nvgpu_boardobjgrpmask_bit_set(
						&g->pmu->clk_pmu->clk_domainobjs_50->progPhysicalCpmuDomainsMask.super,
						pclkdomain_dev->super.idx);
				}
			}

			// Create the mask of clock monitors
			if (status == 0 &&
				((pDomain50ProgPhysical->clkmon_info.low_threshold_vfe_idx  != NV2080_CTRL_PERF_VFE_EQU_INDEX_INVALID) ||
				(pDomain50ProgPhysical->clkmon_info.high_threshold_vfe_idx != NV2080_CTRL_PERF_VFE_EQU_INDEX_INVALID))) {
					status = nvgpu_boardobjgrpmask_bit_set(
						&g->pmu->clk_pmu->clk_domainobjs_50->super.clkMonDomainsMask.super,
						pclkdomain_dev->super.idx);
			}
			if (status != 0) {
				nvgpu_err(g, "failed to set domains mask");
				continue; /* to avoid MISRA violation, single exit point at top of loop */
			}

			// Init per volt rail params.
			BOARDOBJGRP_FOR_EACH_INDEX_IN_MASK(32, railIdx, pDomain50ProgPhysical->rail_mask) {

				/* To avoid MISRA violation,
				* this is a single point of exit
				* from this for loop in the case
				* of an error is encountered.
				*/
				if (status != 0)
					break;

				if (pDomain50ProgPhysical->rail_vf_item[railIdx].type ==
					CTRL_CLK_CLK_DOMAIN_50_PROG_PHYSICAL_RAIL_VF_TYPE_PRIMARY) {
						struct clk_domain_50_physical_rail_vf_primary *pVfPrimary =
						&pDomain50ProgPhysical->rail_vf_item[railIdx].data.primary;

					//
					// Update the clock domain position in tightly packed VF tuples.
					// Primary clock domain position is fixed at index ZERO.
					//
					pDomain50ProgPhysical->rail_vf_item[railIdx].clk_position = 0;

					// Register this PRIMARY object in the mask.
					status = nvgpu_boardobjgrpmask_bit_set(&g->pmu->clk_pmu->clk_domainobjs_50->super.primaryDomainsMask.super,
								pDomain50ProgPhysical->super.super.super.super.idx);
					if (status != 0) {
						nvgpu_err(g,
							"Error setting primaryDomainsMask: idx=%d, status=%d",
							pDomain50ProgPhysical->super.super.super.super.idx, status);
						/* to avoid MISRA violation, single exit point at top of loop */
						continue;
					}

					status = nvgpu_boardobjgrpmask_bit_set(&pVfPrimary->primary_secondary_domain_mask.super,
								pDomain50ProgPhysical->super.super.super.super.idx);
					if (status != 0) {
						nvgpu_err(g,
							"Error setting primary_secondary_domain_mask, idx=%d, status=%d",
							pDomain50ProgPhysical->super.super.super.super.idx, status);
						/* to avoid MISRA violation, single exit point at top of loop */
						continue;
					}
				} else if (pDomain50ProgPhysical->rail_vf_item[railIdx].type ==
					CTRL_CLK_CLK_DOMAIN_50_PROG_PHYSICAL_RAIL_VF_TYPE_SECONDARY) {
					u8 primaryIdx =
						pDomain50ProgPhysical->rail_vf_item[railIdx].data.secondary.primary_idx;
					struct clk_domain_50_prog_physical *pDomain50ProgPhysicalPrimary =
						(struct clk_domain_50_prog_physical *) BOARDOBJGRP_OBJ_GET_BY_IDX(
						&g->pmu->clk_pmu->clk_domainobjs_50->super.super.super, (u8) primaryIdx);
					struct clk_domain_50_physical_rail_vf_primary  *pVfPrimaryLocal =
						&pDomain50ProgPhysicalPrimary->rail_vf_item[railIdx].data.primary;

					status = nvgpu_boardobjgrpmask_bit_set(&pVfPrimaryLocal->secondary_domain_mask.super,
								pDomain50ProgPhysical->super.super.super.super.idx);
					if (status != 0) {
						nvgpu_err(g,
							"Error setting secondary_domain_mask, idx=%d, status=%d",
							pDomain50ProgPhysical->super.super.super.super.idx, status);
						/* to avoid MISRA violation, single exit point at top of loop */
						continue;
					}

					//
					// Update the clock domain position in tightly packed VF tuples.
					// MUST be done before updating the MASK.
					//
					pDomain50ProgPhysical->rail_vf_item[railIdx].clk_position =
					nvgpu_boardobjgrpmask_bit_set_count(&pVfPrimaryLocal->primary_secondary_domain_mask.super);

					//
					// @note PRIMARY clock domain position is fixed at index 0. So if primary clock
					// domain is not already set, add +1 to account for primary clock domain.
					//
					if (!nvgpu_boardobjgrpmask_bit_get(&pVfPrimaryLocal->primary_secondary_domain_mask.super,
							primaryIdx)) {
						pDomain50ProgPhysical->rail_vf_item[railIdx].clk_position =
							pDomain50ProgPhysical->rail_vf_item[railIdx].clk_position + 1;
					}

					status = nvgpu_boardobjgrpmask_bit_set(&pVfPrimaryLocal->primary_secondary_domain_mask.super,
								pDomain50ProgPhysical->super.super.super.super.idx);
					if (status != 0) {
						nvgpu_err(g,
							"Error setting primary_secondary_domain_mask, idx=%d, status=%d",
							pDomain50ProgPhysical->super.super.super.super.idx, status);
						/* to avoid MISRA violation, single exit point at top of loop */
						continue;
					}
				}
			}
			BOARDOBJGRP_FOR_EACH_INDEX_IN_MASK_END;

			if (status != 0) {
				nvgpu_err(g, "volt rail param init failed.");
				/* to avoid MISRA violation, single exit point at top of loop */
				continue;
			}
		}
	}

done:
	nvgpu_kfree(g, pclk_domain_data);
done_no_free:
	nvgpu_log_info(g, " done status %x", status);
	return status;
}

static s32 clk_domain_pmudatainit_50(struct gk20a *g,
	struct pmu_board_obj *obj,
	struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;

	if (obj == NULL || pmu_obj == NULL) {
		status = -EINVAL;
		nvgpu_err(g, "an input obj pointer is NULL");
		goto done;
	}

	// Call CLK_DOMAIN super class.
	status = clk_domain_pmudatainit_super(g, obj, pmu_obj);

done:
	return status;
}

static s32 clkDomains_PmuDataInit_SUPER(struct gk20a *g,
	struct boardobjgrp *pboardobjgrp,
	struct nv_pmu_boardobjgrp_super *pBoardObjGrpPMU)
{
	s32 status = 0;
	u32 i;
	u16 clientClkIdx = 0;
	struct nv_pmu_clk_clk_domain_boardobjgrp_set_header *pSet =
		(struct nv_pmu_clk_clk_domain_boardobjgrp_set_header *)pBoardObjGrpPMU;
	struct nvgpu_clk_domains_5x *pDomains = (struct nvgpu_clk_domains_5x *)pboardobjgrp;
	struct client_clk_domain  *pClientDomain   = NULL;
	struct nvgpu_pmu_volt *pVolt = g->pmu->volt;

	status = boardobjgrp_pmudatainit_e32(g, pboardobjgrp, pBoardObjGrpPMU);
	if (status != 0) {
		nvgpu_err(g, "Error updating pmu boardobjgrp for Clk_domain. status - %d",
			status);
		goto done;
	}

	// Set CLK_DOMAINS-specific data
	pSet->version                = pDomains->version;
	pSet->cntrSamplingPeriodms   = pDomains->cntrSamplingPeriodms;
	pSet->bOverrideOVOC          = pDomains->bOverrideOVOC;
	pSet->bDebugMode             = pDomains->bDebugMode;
	pSet->bEnforceVfMonotonicity = pDomains->bEnforceVfMonotonicity;
	pSet->bEnforceVfSmoothening  = pDomains->bEnforceVfSmoothening;
	pSet->bInsAwareVFDisable     = pDomains->bInsAwareVFDisable;
	pSet->bSkipMclkSwitch        = pDomains->bSkipMclkSwitch;
	pSet->bGrdFreqOCEnabled      = pDomains->bGrdFreqOCEnabled;
	pSet->voltRailsMax           =
		BOARDOBJGRP_PMU_SLOTS_GET(&(pVolt->volt_metadata->volt_rail_metadata.volt_rails.super));
	pSet->bClkMonEnabled         = pDomains->bClkMonEnabled;
	pSet->clkMonRefWinUsec       = pDomains->clkMonRefWinUsec;
	pSet->xbarBoostVfeIdx        = pDomains->xbarBoostVfeIdx;

	status = nvgpu_boardobjgrpmask_export(
		&pDomains->progDomainsMask.super,
		pDomains->progDomainsMask.super.bitcount,
		&pSet->progDomainsMask.super);
	if (status != 0) {
		nvgpu_err(g,
			"Error exporting the PRIMARY mask (status=%d).", status);
		goto done;
	}

	status = nvgpu_boardobjgrpmask_export(
		&pDomains->clkMonDomainsMask.super,
		pDomains->clkMonDomainsMask.super.bitcount,
		&pSet->clkMonDomainsMask.super);
	if (status != 0) {
		nvgpu_err(g,
			"Error exporting the Clock Monitor mask (status=%d).", status);
		goto done;
	}

	// Copy out the value of global delta.
	nvgpu_memcpy((u8 *)&pSet->deltas, (u8 *)&pDomains->deltas,
		(sizeof(struct ctrl_clk_clk_delta))
	);

	for (i = 0;
		i < ((sizeof(pSet->clientToInternalIdxMap)/sizeof(pSet->clientToInternalIdxMap[0])));
		i++) {
		pSet->clientToInternalIdxMap[i] =
			CTRL_CLK_CLK_DOMAIN_INDEX_INVALID;
	}
	pSet->clientToInternalIdxMap[0] = 0x0;// _mg Remove.  Test if index 0 is aligned. Yes it is. But should be 00 not FF. If we have PSTATE parsing, see below.

	BOARDOBJGRP_ITERATOR(&(pDomains->clientDomains.super.super), struct client_clk_domain *,
		pClientDomain, clientClkIdx, &(pDomains->clientDomains.super.mask.super)) {
		pSet->clientToInternalIdxMap[clientClkIdx] = pClientDomain->domainIdx;
	}

done:
	return status;
}

static s32 clkDomains_PmuDataInit_50(struct gk20a *g,
struct boardobjgrp *pboardobjgrp,
struct nv_pmu_boardobjgrp_super *pBoardObjGrpPMU)
{
	s32 status = 0;
	struct nv_pmu_clk_clk_domain_boardobjgrp_set_header *pSet50 =
		(struct nv_pmu_clk_clk_domain_boardobjgrp_set_header *)pBoardObjGrpPMU;
	struct nvgpu_clk_domains_50  *pDomains50  = (struct nvgpu_clk_domains_50 *)pboardobjgrp;

	// init super class
	status = clkDomains_PmuDataInit_SUPER(g, pboardobjgrp, pBoardObjGrpPMU);
	if (status != 0) {
		nvgpu_err(g,
			"pmu datainit super failed (status=%d).", status);
		goto done;
	}

	// mask exports
	status = nvgpu_boardobjgrpmask_export(
		&pDomains50->progClientVisibleDomainsMask.super,
		pDomains50->progClientVisibleDomainsMask.super.bitcount,
		&pSet50->data.v50.progClientVisibleDomainsMask.super);
	if (status != 0) {
		nvgpu_err(g,
			"Could not export progClientVisibleDomainsMask (status=%d).", status);
		goto done;
	}

	status = nvgpu_boardobjgrpmask_export(
		&pDomains50->progPhysicalDomainsMask.super,
		pDomains50->progPhysicalDomainsMask.super.bitcount,
		&pSet50->data.v50.progPhysicalDomainsMask.super);
	if (status != 0) {
		nvgpu_err(g,
			"Could not export progPhysicalDomainsMask (status=%d).", status);
		goto done;
	}

	status = nvgpu_boardobjgrpmask_export(
		&pDomains50->progPhysicalCpmuDomainsMask.super,
		pDomains50->progPhysicalCpmuDomainsMask.super.bitcount,
		&pSet50->data.v50.progPhysicalCpmuDomainsMask.super);
	if (status != 0) {
		nvgpu_err(g,
			"Could not export progPhysicalCpmuDomainsMask (status=%d).", status);
		goto done;
	}

done:

	return status;
}

static s32 clkDomainCopy_SUPER(struct gk20a *g,
	struct nvgpu_clk_domain *pDomainDst,
	struct nvgpu_clk_domain *pDomainSrc)
{
	s32 status = -EINVAL;
	(void) g;

	if (pDomainDst->super.type == pDomainSrc->super.type) {
		status = 0;

		pDomainDst->api_domain            = pDomainSrc->api_domain;
		pDomainDst->part_mask             = pDomainSrc->part_mask;
		pDomainDst->domain                = pDomainSrc->domain;
		pDomainDst->b_noise_aware_capable = pDomainSrc->b_noise_aware_capable;
		pDomainDst->public_domain         = pDomainSrc->public_domain;

		pDomainDst->perf_domain_grp_idx   = pDomainSrc->perf_domain_grp_idx;
		pDomainDst->ratio_domain          = pDomainSrc->ratio_domain;
		pDomainDst->usage                 = pDomainSrc->usage;
	}

	return status;
}

static s32 clkDomainCopy_50_PROG_PHYSICAL(struct gk20a *g,
	struct nvgpu_clk_domain *pDomainDst,
	struct nvgpu_clk_domain *pDomainSrc)
{
	s32 status;
	struct clk_domain_50_prog_physical *pDomDst = (struct clk_domain_50_prog_physical *)pDomainDst;
	struct clk_domain_50_prog_physical *pDomSrc = (struct clk_domain_50_prog_physical *)pDomainSrc;
	(void) g;

	// copy SUPER
	status = clkDomainCopy_SUPER(g, pDomainDst, pDomainSrc);

	if (status == 0) {
		// Copy CLK_DOMAIN_50_PROG_PHYSICAL parameters.
		pDomDst->clk_enum_idx_first        = pDomSrc->clk_enum_idx_first;
		pDomDst->clk_enum_idx_last         = pDomSrc->clk_enum_idx_last;
		pDomDst->pre_volt_ordering_index   = pDomSrc->pre_volt_ordering_index;
		pDomDst->post_volt_ordering_index  = pDomSrc->post_volt_ordering_index;
		pDomDst->clk_vf_curve_count        = pDomSrc->clk_vf_curve_count;
		pDomDst->logical_domain_idx        = pDomSrc->logical_domain_idx;
		pDomDst->factory_delta             = pDomSrc->factory_delta;
		pDomDst->grd_freq_delta            = pDomSrc->grd_freq_delta;
		pDomDst->freq_delta_min_mhz        = pDomSrc->freq_delta_min_mhz;
		pDomDst->freq_delta_max_mhz        = pDomSrc->freq_delta_max_mhz;
		pDomDst->clkmon_info               = pDomSrc->clkmon_info;
		pDomDst->clkmon_ctrl               = pDomSrc->clkmon_ctrl;
		pDomDst->fbvdd_data                = pDomSrc->fbvdd_data;
		pDomDst->rail_mask                 = pDomSrc->rail_mask;
		pDomDst->cpmu_clk_id               = pDomSrc->cpmu_clk_id;

		// Copy rail specific params.
		nvgpu_memcpy((u8 *)pDomDst->rail_vf_item, (u8 *)pDomSrc->rail_vf_item,
				sizeof(struct ctrl_clk_domain_info_50_prog_physical_rail_vf_item)
				* NV_CTRL_CLK_CLK_DOMAIN_PROG_RAIL_VF_ITEM_MAX);
	}

	return status;
}

static s32 clk_domain_construct_50(struct gk20a *g,
					struct pmu_board_obj **obj,
					size_t size, void *pargs)
{
	struct pmu_board_obj *obj_tmp = (struct pmu_board_obj *)pargs;
	struct clk_domain_50 *pDomain;
	struct clk_domain_50 *ptmpdomain =
			(struct clk_domain_50 *)pargs;
	s32 status = 0;

	obj_tmp->type_mask |= BIT32(CTRL_CLK_CLK_DOMAIN_TYPE_50);
	status = clk_domain_construct_super(g, obj,
					size, pargs);
	if (status != 0) {
		nvgpu_err(g, "Error constructing super: status=%d", status);
		goto done;
	}

	pDomain = (struct clk_domain_50 *)*obj;

	// Set type-specific parameters
	status = clkDomainCopy_SUPER(g, &pDomain->super, &ptmpdomain->super);
	if (status != 0) {
		nvgpu_err(g, "Error copying in pTmpDomain: status=%d", status);
		goto done;
	}
	//
	// Initialize CLIENT_CLK_DOMAIN index to _INVALID.  If this domain has a
	// corresponding CLIENT_CLK_DOMAIN, it will be assigned later.
	//
	pDomain->clientDomainIdx = NV_CTRL_CLK_CLIENT_CLK_DOMAIN_IDX_INVALID;

done:
	return status;
}

static s32 clk_domain_pmudatainit_50_fixed(struct gk20a *g,
					struct pmu_board_obj *obj,
					struct nv_pmu_boardobj *pmu_obj)
{
	s32 status = 0;
	struct nv_pmu_clk_clk_domain_50_fixed_boardobj_set *pSet =
		(struct nv_pmu_clk_clk_domain_50_fixed_boardobj_set *)pmu_obj;
	struct clk_domain_50_fixed *pclk_domain_50_fixed =
		(struct clk_domain_50_fixed *)obj;

	nvgpu_log_info(g, " ");

	// Call CLK_DOMAIN_50 super class.
	status = clk_domain_pmudatainit_50(g, obj, pmu_obj);
	if (status != 0) {
		nvgpu_err(g,
			"clk_domain_pmudatainit_50 returned error: status=%d",
			status);
		goto done;
	}

	pSet->freqMHz = pclk_domain_50_fixed->freq_mhz;

done:
	return status;
}

s32 clk_domain_construct_50_fixed(struct gk20a *g,
					struct pmu_board_obj **obj,
					size_t size, void *pargs)
{
	struct pmu_board_obj *obj_tmp = (struct pmu_board_obj *)pargs;
	struct clk_domain_50_fixed *pdomain;
	struct clk_domain_50_fixed *ptmpdomain =
			(struct clk_domain_50_fixed *)pargs;
	s32 status = 0;

	if (pmu_board_obj_get_type(pargs) != CTRL_CLK_CLK_DOMAIN_TYPE_50_FIXED) {
		return -EINVAL;
	}

	obj_tmp->type_mask |= BIT32(CTRL_CLK_CLK_DOMAIN_TYPE_50_FIXED);
	status = clk_domain_construct_50(g, obj, size, pargs);
	if (status != 0)
		return -EINVAL;

	pdomain = (struct clk_domain_50_fixed *)*obj;

	// Set BOARDOBJ interfaces
	pdomain->super.super.super.pmudatainit =
			clk_domain_pmudatainit_50_fixed;

	// Override CLK_DOMAIN super class interfaces
	pdomain->super.super.clkdomainclkproglink =
			clkdomainclkproglink_fixed;

	// Set type-specific parameters by abusing the copy interface.
	pdomain->freq_mhz = ptmpdomain->freq_mhz;

	return status;
}

static s32 clk_domain_construct_50_prog(struct gk20a *g,
					struct pmu_board_obj **obj,
					size_t size, void *pargs)
{
	struct pmu_board_obj *obj_tmp = (struct pmu_board_obj *)pargs;
	struct clk_domain_50_prog *pdomain;
	s32 status = 0;

	nvgpu_log_info(g, " ");

	obj_tmp->type_mask |= BIT32(CTRL_CLK_CLK_DOMAIN_TYPE_50_PROG);
	status = clk_domain_construct_50(g, obj, size, pargs);
	if (status != 0) {
		nvgpu_err(g, "Error constructing super: status=%d", status);
		goto done;
	}

	pdomain = (struct clk_domain_50_prog *) *obj;

	// Construct CLK_DOMAIN_PROG interface
	pdomain->super.super.clkdomainclkvfsearch =
				clkdomainvfsearch;

	pdomain->super.super.clkdomainclkgetfpoints =
				clkdomaingetfpoints;

done:
	return status;
}

static s32 clk_domain_pmudatainit_50_physical(struct gk20a *g,
			struct pmu_board_obj *pBoardObj,
			struct nv_pmu_boardobj *pPmuData)
{
	s32 status = 0;
	struct nv_pmu_clk_clk_domain_50_prog_physical_boardobj_set *pSet =
		(struct nv_pmu_clk_clk_domain_50_prog_physical_boardobj_set *)pPmuData;
	struct clk_domain_50_prog_physical *pDomain = (struct clk_domain_50_prog_physical *)pBoardObj;
	u8 railIdx;

	nvgpu_log_info(g, " ");

	// Call CLK_DOMAIN_50 super class.
	status = clk_domain_pmudatainit_50_prog(g, pBoardObj, pPmuData);
	if (status != 0) {
		nvgpu_err(g, "CLK_DOMAIN_50 PMU_DATA_INIT returned error: status=%d",
			status);
		goto done;
	}

	// Set CLK_DOMAIN_50_PROG_PHYSICAL-specific data.
	pSet->clkEnumIdxFirst       = pDomain->clk_enum_idx_first;
	pSet->clkEnumIdxLast        = pDomain->clk_enum_idx_last;
	pSet->preVoltOrderingIndex  = pDomain->pre_volt_ordering_index;
	pSet->postVoltOrderingIndex = pDomain->post_volt_ordering_index;
	pSet->clkVFCurveCount       = pDomain->clk_vf_curve_count;
	pSet->logicalDomainIdx      = pDomain->logical_domain_idx;
	pSet->factoryDelta          = pDomain->factory_delta;
	pSet->grdFreqDelta          = pDomain->grd_freq_delta;
	pSet->freqDeltaMinMHz       = pDomain->freq_delta_min_mhz;
	pSet->freqDeltaMaxMHz       = pDomain->freq_delta_max_mhz;
	pSet->clkMonInfo            = pDomain->clkmon_info;
	pSet->clkMonCtrl            = pDomain->clkmon_ctrl;
	pSet->fbvddData             = pDomain->fbvdd_data;
	pSet->railMask              = pDomain->rail_mask;
	pSet->cpmuClkId             = pDomain->cpmu_clk_id;

	// Copy out the value of global delta.
	nvgpu_memcpy((u8 *)&pSet->deltas, (u8 *)&pDomain->deltas,
		(sizeof(struct ctrl_clk_clk_delta)));

	// Init per volt rail params.
	for (railIdx = 0; railIdx < NV_CTRL_CLK_CLK_DOMAIN_PROG_RAIL_VF_ITEM_MAX; railIdx++) {
		pSet->railVfItem[railIdx].type = pDomain->rail_vf_item[railIdx].type;
		pSet->railVfItem[railIdx].clk_position = pDomain->rail_vf_item[railIdx].clk_position;

		if (pDomain->rail_vf_item[railIdx].type ==
			CTRL_CLK_CLK_DOMAIN_50_PROG_PHYSICAL_RAIL_VF_TYPE_PRIMARY) {
			pSet->railVfItem[railIdx].data.primary.clk_vf_rel_idx_first =
				pDomain->rail_vf_item[railIdx].data.primary.clk_vf_rel_idx_first;
			pSet->railVfItem[railIdx].data.primary.clk_vf_rel_idx_last =
				pDomain->rail_vf_item[railIdx].data.primary.clk_vf_rel_idx_last;

			// Export the mask of the primary_secondary clock domain indexes
			status = nvgpu_boardobjgrpmask_export(
						&pDomain->rail_vf_item[railIdx].data.primary.primary_secondary_domain_mask.super,
						pDomain->rail_vf_item[railIdx].data.primary.primary_secondary_domain_mask.super.bitcount,
						&pSet->railVfItem[railIdx].data.primary.primary_secondary_domain_mask.super);
			if (status != 0) {
				nvgpu_err(g,
					"Could not export primarySecondaryDomainsMask (status=%d", status);
				goto done;
			}

			// Export the mask of the secondary clock domain indexes
			status = nvgpu_boardobjgrpmask_export(
						&pDomain->rail_vf_item[railIdx].data.primary.secondary_domain_mask.super,
						pDomain->rail_vf_item[railIdx].data.primary.secondary_domain_mask.super.bitcount,
						&pSet->railVfItem[railIdx].data.primary.secondary_domain_mask.super);
			if (status != 0) {
				nvgpu_err(g,
					"Could not export secondaryDomainsMask (status=%d", status);
				goto done;
			}
		} else if (pDomain->rail_vf_item[railIdx].type ==
			CTRL_CLK_CLK_DOMAIN_50_PROG_PHYSICAL_RAIL_VF_TYPE_SECONDARY) {
			pSet->railVfItem[railIdx].data.secondary.primary_idx =
				pDomain->rail_vf_item[railIdx].data.secondary.primary_idx;
		} else {
			continue;
		}
	}

done:
	return status;
}

static s32 clkdomainclkproglink_50_physical(struct gk20a *g,
					struct nvgpu_clk_pmupstate *pClk,
					struct nvgpu_clk_domain *pDomain)
{
	struct clk_domain_50_prog_physical *pDomain50ProgPhysical =
		(struct clk_domain_50_prog_physical *)pDomain;
	struct clk_vf_rel *pVfRel = NULL;
	u8 railIdx;
	u8 i;
	s32 status = 0;

	nvgpu_log_info(g, " ");

	// Update total supported VF curve counts
	if ((pDomain50ProgPhysical->clk_vf_curve_count - NV2080_CTRL_CLK_CLK_VF_REL_VF_CURVE_IDX_SEC_0)
			> pClk->clk_vfrelobjs->vf_entry_count_sec) {
		pDomain50ProgPhysical->clk_vf_curve_count =
			(pClk->clk_vfrelobjs->vf_entry_count_sec +
			NV2080_CTRL_CLK_CLK_VF_REL_VF_CURVE_IDX_SEC_0);
	}
	pDomain50ProgPhysical->super.b_fav_point_available = false;

	// Link per volt rail params.
	BOARDOBJGRP_FOR_EACH_INDEX_IN_MASK(32, railIdx, pDomain50ProgPhysical->rail_mask) {
		if (pDomain50ProgPhysical->rail_vf_item[railIdx].type ==
			CTRL_CLK_CLK_DOMAIN_50_PROG_PHYSICAL_RAIL_VF_TYPE_PRIMARY) {
			struct clk_domain_50_physical_rail_vf_primary *pVfPrimary =
				&pDomain50ProgPhysical->rail_vf_item[railIdx].data.primary;

			// TODO: Temporarily disable FAV point available check
			// This is a workaround to disable FAV point available check
			// until the FAV point available check dependencies are implemented.
			// According to PMU team, PERF CONTROLLER is the dependency.

			//if (pVfPrimary->clk_vf_rel_idx_first == pVfPrimary->clk_vf_rel_idx_last)
				//pDomain50ProgPhysical->super.b_fav_point_available = true;
			pDomain50ProgPhysical->super.b_fav_point_available = false;

			//
			// Iterate over the set of CLK_VF_RELs pointed at by this domain
			// and ensure that they are all valid indexes.
			//
			for (i = pVfPrimary->clk_vf_rel_idx_first; i <= pVfPrimary->clk_vf_rel_idx_last; i++) {
				pVfRel = CLK_CLK_VF_REL_GET(pClk, i);
				if (NULL == pVfRel) {
					status = -EINVAL;
					nvgpu_err(g,
						"Points at invalid/unspecified CLK_VF_REL index - index=%d.",
						i);
					goto clkDomainClkProgLink_50_PROG_PHYSICAL_exit;
				}

				// Update the VF Rel :: Volt rail index.
				pVfRel->rail_idx = railIdx;

				// Flatten the VF curves.
				status = pVfRel->vfFlatten(g, pVfRel,
					pDomain50ProgPhysical->super.super.super.super.idx);
				if (status != 0) {
					nvgpu_err(g,
						"Error flattening CLK_VF_REL VF curve - idx=%d, status=%d",
						i, status);
					goto clkDomainClkProgLink_50_PROG_PHYSICAL_exit;
				}

				if (pVfRel->super.type == NV2080_CTRL_CLK_CLK_VF_REL_TYPE_50_RATIO_VOLT) {
					struct clk_vf_rel_50_ratio_volt *pVfRel50RatioVolt =
						(struct clk_vf_rel_50_ratio_volt *)pVfRel;
					if (pVfRel50RatioVolt->favDeratedSubCurveVFEEqnIdx ==
						NV2080_CTRL_PERF_VFE_EQU_INDEX_INVALID) {
						pDomain50ProgPhysical->super.b_fav_point_available = false;
					}
				}
			}
		}
	}
	BOARDOBJGRP_FOR_EACH_INDEX_IN_MASK_END;

clkDomainClkProgLink_50_PROG_PHYSICAL_exit:
	return status;
}

s32 clk_domain_construct_50_physical(struct gk20a *g,
					struct pmu_board_obj **obj,
					size_t size, void *pargs)
{
	struct pmu_board_obj *obj_tmp = (struct pmu_board_obj *)pargs;
	struct clk_domain_50_prog_physical *pdomain;
	u8 railIdx;
	s32 status = 0;

	if (pmu_board_obj_get_type(pargs) !=
			(u8) CTRL_CLK_CLK_DOMAIN_TYPE_50_PROG_PHYSICAL)
		return -EINVAL;

	// Call super-class constructor.
	obj_tmp->type_mask |= BIT32(CTRL_CLK_CLK_DOMAIN_TYPE_50_PROG_PHYSICAL);
	status = clk_domain_construct_50_prog(g, obj, size, pargs);
	if (status != 0)
		return -EINVAL;

	pdomain = (struct clk_domain_50_prog_physical *) *obj;

	// Set BOARDOBJ interfaces
	pdomain->super.super.super.super.pmudatainit =
			clk_domain_pmudatainit_50_physical;

	// Set CLK_DOMAIN_50 interfaces
	pdomain->super.super.super.clkdomainclkproglink =
			clkdomainclkproglink_50_physical;

	// Init PCLK_DOMAIN_50_PROG_PHYSICAL factory and frequency deltas to default type
	pdomain->factory_delta.type = CTRL_CLK_CLK_FREQ_DELTA_TYPE_STATIC;
	pdomain->factory_delta.data.delta_khz = 0;
	pdomain->deltas.freq_delta.type = CTRL_CLK_CLK_FREQ_DELTA_TYPE_STATIC;
	pdomain->deltas.freq_delta.data.delta_khz = 0;
	pdomain->grd_freq_delta.type = CTRL_CLK_CLK_FREQ_DELTA_TYPE_STATIC;
	pdomain->grd_freq_delta.data.delta_khz = 0;

	// Set type-specific parameters by using the copy interface.
	status = clkDomainCopy_50_PROG_PHYSICAL(g, (struct nvgpu_clk_domain *)pdomain,
					(struct nvgpu_clk_domain *)pargs);

	// Reset for all voltage rails.
	for (railIdx = 0; railIdx < NV_CTRL_CLK_CLK_DOMAIN_PROG_RAIL_VF_ITEM_MAX; railIdx++) {
		//
		// Reset the secondary clock domains mask.
		// This is a drawback of COPY interface which internally calls all its super
		// and the super class will copy source MASK which is ZERO at construct.
		//
		status = boardobjgrpmask_e32_init(
					&pdomain->rail_vf_item[railIdx].data.primary.secondary_domain_mask, NULL);
		if (status != 0) {
			nvgpu_err(g,
				"Error constructing secondary domain mask - status =%d.",
				status);
			break;
		}

		status = boardobjgrpmask_e32_init(
					&pdomain->rail_vf_item[railIdx].data.primary.primary_secondary_domain_mask, NULL);
		if (status != 0) {
			nvgpu_err(g,
				"Error constructing primary-secondary domain group mask- status = %d.",
				status);
			break;
		}
	}

	return status;
}

s32 clk_domain_pmudatainit_50_prog(struct gk20a *g,
			struct pmu_board_obj *pBoardObj,
			struct nv_pmu_boardobj *pPmuData)
{
	struct nv_pmu_clk_clk_domain_50_prog_boardobj_set *pSet =
		(struct nv_pmu_clk_clk_domain_50_prog_boardobj_set *)pPmuData;
	struct clk_domain_50_prog *pDomain = (struct clk_domain_50_prog *)pBoardObj;
	struct clk_domain_50_prog_physical *pDomainPhy = (struct clk_domain_50_prog_physical *)pBoardObj;
	struct boardobjgrpmask_e32 voltRailVminMask;
	s32 status = 0;

	nvgpu_log_info(g, " ");

	// Call CLK_DOMAIN_50 super class.
	status = clk_domain_pmudatainit_50(g, pBoardObj, pPmuData);
	if (status != 0) {
		nvgpu_err(g,
			"CLK_DOMAIN_50 PMU_DATA_INIT returned error: status=%d",
			status);
		goto clkDomainIfaceModel10PmuDataInit_50_PROG_exit;
	}

	// Init CLK_DOMAIN_PROG data.
	status = boardobjgrpmask_e32_init(&voltRailVminMask, NULL);
	if (status != 0)
		goto clkDomainIfaceModel10PmuDataInit_50_PROG_exit;

	// Fetch VOLT_RAIL Vmin Mask from CLK_DOMAIN_PROG iface.
	voltRailVminMask.super.data[0] = pDomainPhy->rail_mask;

	status = nvgpu_boardobjgrpmask_export(&voltRailVminMask.super,
				CTRL_BOARDOBJGRP_E32_MAX_OBJECTS,
				&pSet->prog.voltRailVminMask.super);
	if (status != 0)
		goto clkDomainIfaceModel10PmuDataInit_50_PROG_exit;

	pSet->bFAVPointAvailable = pDomain->b_fav_point_available;

clkDomainIfaceModel10PmuDataInit_50_PROG_exit:

	return status;
}

s32 clk_domain_sw_setup_50(struct gk20a *g)
{
	s32 status;
	struct boardobjgrp *pboardobjgrp = NULL;
	struct nvgpu_clk_domains_5x *pclkdomainobjs;
	struct nvgpu_clk_domains_50 *pDomains50;
	u8 *clocks_table_ptr = NULL;

	nvgpu_log_info(g, " ");

	/* If already constructed, do not re-construct (suspend/resume pattern) */
	pboardobjgrp = &g->pmu->clk_pmu->clk_domainobjs_50->super.super.super;
	if (pboardobjgrp->bconstructed) {
		nvgpu_pmu_dbg(g, "clk_domain_50 boardobjgrp already constructed, skipping reinit");
		return 0;
	}

	nvgpu_pmu_dbg(g, "Constructing clk_domain_50 boardobjgrp for first time");

	status = nvgpu_boardobjgrp_construct_e32(g,
			&g->pmu->clk_pmu->clk_domainobjs_50->super.super);
	if (status != 0) {
		nvgpu_err(g,
			"error creating boardobjgrp for clk domain err=%d",
			status);
		goto done;
	}
	pclkdomainobjs = &g->pmu->clk_pmu->clk_domainobjs_50->super;
	pDomains50 = g->pmu->clk_pmu->clk_domainobjs_50;

	BOARDOBJGRP_PMU_CONSTRUCT(pboardobjgrp, CLK, CLK_DOMAIN);

	status = BOARDOBJGRP_PMU_CMD_GRP_SET_CONSTRUCT(g, pboardobjgrp,
			clk, CLK, clk_domain, CLK_DOMAIN);
	if (status != 0) {
		nvgpu_err(g,
		 "error constructing PMU_BOARDOBJ_CMD_GRP_SET interface err=%d",
		 status);
		goto done;
	}

	/* Initialize masks to zero.*/
	status = boardobjgrpmask_e32_init(&pclkdomainobjs->progDomainsMask,
			NULL);
	if (status != 0) {
		nvgpu_err(g, "boardobjgrpmask_e32_init(prog) failed err=%d",
			status);
		goto done;
	}
	status = boardobjgrpmask_e32_init(&pclkdomainobjs->primaryDomainsMask,
			NULL);
	if (status != 0) {
		nvgpu_err(g, "boardobjgrpmask_e32_init(primary) failed err=%d",
			status);
		goto done;
	}
	status = boardobjgrpmask_e32_init(&pclkdomainobjs->clientDomainsMask,
			NULL);
	if (status != 0) {
		nvgpu_err(g, "boardobjgrpmask_e32_init(client) failed err=%d",
			status);
		goto done;
	}
	status = boardobjgrpmask_e32_init(&pclkdomainobjs->clkMonDomainsMask,
			NULL);
	if (status != 0) {
		nvgpu_err(g, "boardobjgrpmask_e32_init(clkmon) failed err=%d",
			status);
		goto done;
	}

	// Init to default values.
	(void) memset(&pclkdomainobjs->orderedNoiseAwareList, 0,
		sizeof(pclkdomainobjs->orderedNoiseAwareList));

	(void) memset(&pclkdomainobjs->orderedNoiseUnawareList, 0,
		sizeof(pclkdomainobjs->orderedNoiseUnawareList));

	pclkdomainobjs->bEnforceVfMonotonicity = true;
	pclkdomainobjs->bEnforceVfSmoothening = true;

	pboardobjgrp->pmudatainit = clkDomains_PmuDataInit_50;
	pboardobjgrp->pmudatainstget = clk_domains_pmudata_instget;

	// Construct the CLK_DOMAINS_50 object.
	// Init the various BOARDOBJGRPMASKs so they can be set later.
	status = boardobjgrpmask_e32_init(&pDomains50->progClientVisibleDomainsMask,
		NULL);
	if (status != 0) {
	nvgpu_err(g, "boardobjgrpmask_e32_init(progClient) failed err=%d",
		status);
		goto done;
	}
	status = boardobjgrpmask_e32_init(&pDomains50->progPhysicalDomainsMask,
		NULL);
	if (status != 0) {
	nvgpu_err(g, "boardobjgrpmask_e32_init(progPhysical) failed err=%d",
		status);
		goto done;
	}
	status = boardobjgrpmask_e32_init(&pDomains50->progPhysicalCpmuDomainsMask,
		NULL);
	if (status != 0) {
	nvgpu_err(g, "boardobjgrpmask_e32_init(progPhysicalCpmu) failed err=%d",
		status);
		goto done;
	}

	// Set CLK_DOMAIN version to 50.
	pclkdomainobjs->version = CLK_DOMAIN_BOARDOBJGRP_VERSION_50;

	clocks_table_ptr = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
			nvgpu_bios_get_bit_token(g, NVGPU_BIOS_CLOCK_TOKEN),
							CLOCKS_TABLE);
	status = devinit_get_clocks_table_50(g, clocks_table_ptr);

	if (status != 0) {
		nvgpu_err(g,
			"Error parsing Clocks Table 50 - status=%d", status);
	}

done:
	return status;
}

u8 clkDomainProgVoltRailIdxGet_50_PROG_PHYSICAL(struct gk20a *g,
	void* pDomainProg)
{
	struct clk_domain_50_prog_physical  *pDomain50ProgPhysical =
		(struct clk_domain_50_prog_physical  *)(pDomainProg);
	u32 railMask  = pDomain50ProgPhysical->rail_mask;
	u32 railCount = railMask;

	(void)g;
	NUMSETBITS_32(railCount);
	//
	// We expect all programmable domain on GB20C to have
	// exactly one voltage rail powering the clock domain.
	//
	if ((railCount == 0U) ||
		(railCount >  1U)) {
		return CTRL_BOARDOBJ_IDX_INVALID;
	}
	LOWESTBITIDX_32(railMask);

	return (u8)railMask;
}

s32 clk_domains_set_offset_50(struct gk20a *g,
	s32 offset_khz, u32 api_domain)
{
	s32 status = -EINVAL;
	struct nvgpu_clk_domain *pDomain = NULL;
	struct clk_domain_50_prog_physical *pDomProg = NULL;
	struct nvgpu_clk_pmupstate *pclk = g->pmu->clk_pmu;
	u16 i;

	// Sanity checks
	if (g == NULL) {
		goto done;
	}
	nvgpu_log_info(g, " ");
	if (pclk == NULL) {
		nvgpu_err(g, "g->pmu->clk_pmu is NULL");
		goto done;
	}
	if (pclk->clk_domainobjs_50 == NULL) {
		nvgpu_err(g, "clk_domainobjs_50 is NULL");
		goto done;
	}
	// search for board obj that api_domain matches
	BOARDOBJGRP_FOR_EACH(&(pclk->clk_domainobjs_50->super.super.super),
		struct nvgpu_clk_domain *, pDomain, i) {

		if (pDomain != NULL && pDomain->api_domain == api_domain) {
			if (pmu_board_obj_get_type(pDomain) !=
				(u8) CTRL_CLK_CLK_DOMAIN_TYPE_50_PROG_PHYSICAL) {
					nvgpu_err(g, "Clk Domain idx %d is not PROG_PHYSICAL type", i);
					break;
				}

			// Check if the offset is within the range of the
			// frequency delta min and max in mhz
			pDomProg = (struct clk_domain_50_prog_physical *) pDomain;
			if (((s32)pDomProg->freq_delta_min_mhz * 1000) > offset_khz
				|| ((s32)pDomProg->freq_delta_max_mhz * 1000) < offset_khz) {
				nvgpu_err(g, "Frequency offset %d out side of range [%d : %d]",
					offset_khz,
					pDomProg->freq_delta_min_mhz, pDomProg->freq_delta_max_mhz);
				break;
			}
			// update board obj with khz delta
			pclk->clk_domainobjs_50->super.bOverrideOVOC = true;
			pDomProg->deltas.freq_delta.type = CTRL_CLK_CLK_FREQ_DELTA_TYPE_STATIC;
			pDomProg->deltas.freq_delta.data.delta_khz = offset_khz;
			// good to go
			status = 0;
			break;
		}
	}
	if (status != 0 || pDomProg == NULL) {
		nvgpu_err(g, "Failed to change frequency offset");
		goto done;
	}
	//
	// Resend clock domain board obj to PMU this will
	// trigger VF curve regeneration in the PMU
	nvgpu_log_info(g, "Sending CLK_DOMAIN board objs to PMU to change offset to %d KHz", offset_khz);
	status = clk_domain_pmu_setup(g);

done:
	return status;
}

s32 clk_domain_get_gpc_sentry_mode_freq(struct gk20a *g, u32 *freq)
{
	s32 status = 0;
	u16 i;
	u8 *perf_table_prt = NULL;
	u8 *entry_ptr = NULL;
	bool found_p8 = false;
	struct vbios_pstate_header_7x header = { 0 };
	struct vbios_pstate_entry_7x entry = { 0 };

	nvgpu_log_info(g, " ");
	// Sanity check
	if (freq == NULL) {
		nvgpu_err(g, "freq pointer is NULL");
		status = -EINVAL;
		goto done;
	}

	perf_table_prt = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
				nvgpu_bios_get_bit_token(g, NVGPU_BIOS_PERF_TOKEN),
				PERFORMANCE_TABLE);
	if (perf_table_prt == NULL) {
		nvgpu_err(g, "VBIOS PState Table not found");
		status = -EINVAL;
		goto done;
	}

	nvgpu_memcpy((u8 *)&header, perf_table_prt,
				sizeof(struct vbios_pstate_header_7x));

	if (header.version != VBIOS_PSTATE_TABLE_VERSION_7X) {
		nvgpu_err(g, "Unsupported PState header version: %d", header.version);
		status = -EINVAL;
		goto done;
	}

	/* iterate the VBIOS table for P8 entry (Sentry mode) */
	for (i = 0; i < header.base_entry_count; i++) {
		entry_ptr = (perf_table_prt + header.header_size +
				(i * (header.base_entry_size +
				(header.clock_entry_size * header.clock_entry_count))));

		nvgpu_memcpy((u8 *)&entry, entry_ptr,
				sizeof(struct vbios_pstate_entry_7x));

		if (entry.pstate_level == VBIOS_PSTATE_7X_PERFLEVEL_PSTATE_P8) {
			/* this is the pstate we are looking for.
			 * Get the GPC max frequency.
			 */
			found_p8 = true;
			*freq = entry.nvgpu_clockEntry[0].max_frequency;
			nvgpu_info(g, "VBIOS: Sentry GPC Max frequency: %d MHz", *freq);
			break;
		}
	}

	if (!found_p8) {
		nvgpu_err(g, "VBIOS PState P8 entry not found");
		status = -EINVAL;
	}

done:
	return status;
}


s32 clk_domain_get_gpc_drive_mode_freq(struct gk20a *g, u32 *freq)
{
	s32 status = 0;
	u8 drive_idx = 0;
	u8 *perf_table_prt = NULL;
	u8 *entry_ptr = NULL;
	struct vbios_vpstate_header_2x header = { 0 };
	struct vbios_vpstate_entry_2x_base_entry entry = { 0 };
	u32 target_frequency = 0;

	nvgpu_log_info(g, " ");
	// Sanity check
	if (freq == NULL) {
		nvgpu_err(g, "freq pointer is NULL");
		status = -EINVAL;
		goto done;
	}

	perf_table_prt = (u8 *)nvgpu_bios_get_perf_table_ptrs(g,
				nvgpu_bios_get_bit_token(g, NVGPU_BIOS_PERF_TOKEN),
				VIRTUAL_PSTATE_TABLE);
	if (perf_table_prt == NULL) {
		nvgpu_err(g, "VBIOS VPState Table not found");
		status = -EINVAL;
		goto done;
	}

	nvgpu_memcpy((u8 *)&header, perf_table_prt,
				sizeof(struct vbios_vpstate_header_2x));

	// sanity check the VBIOS table
	if (header.version != VBIOS_VPSTATE_HEADER_2X_VERSION) {
		nvgpu_err(g, "Unsupported VPState header version: %d", header.version);
		status = -EINVAL;
		goto done;
	}
	if (header.header_size < VBIOS_VPSTATE_HEADER_2X_SIZE_23) {
		nvgpu_err(g, "Unsupported VPState header size: %d", header.header_size);
		status = -EINVAL;
		goto done;
	}

	/* Drive mode clock index is in Rated TDP Index entry */
	drive_idx = header.RatedTDP_VPstateIdx;
	if (drive_idx == VBIOS_VPSTATE_IDX_INVALID) {
		nvgpu_err(g, "VBIOS: Rated TDP Index is invalid");
		status = -EINVAL;
		goto done;
	}

	entry_ptr = (perf_table_prt + header.header_size +
		(drive_idx * (header.base_entry_size +
		(header.clock_entry_size * header.clock_entry_count))));


	nvgpu_memcpy((u8 *)&entry, entry_ptr,
		sizeof(struct vbios_vpstate_entry_2x_base_entry));

	/* Sanity check the Pstate level of the Rated DTP Entry */
	if (entry.pstate != VBIOS_PSTATE_7X_PERFLEVEL_PSTATE_P0) {
		nvgpu_err(g, "VBIOS: Rated TDP PState is not P0: 0x%x",
			entry.pstate);
		status = -EINVAL;
		goto done;
	}

	/* This is the pstate we are looking for.
	 * Check that target frequency is valid.
	 */
	target_frequency = entry.nvgpu_clockEntry[0].frequencies &
		VBIOS_VPSTATE_ENTRY_2X_CLK_TARGET_F_MASK;
	if (target_frequency == 0) {
		// A Target Frequency value of zero (0 MHz)
		// indicates no specified frequency for the given Clock Domain.
		nvgpu_err(g, "VBIOS: Drive mode Target Frequency is unspecified.");
		status = -EINVAL;
		goto done;
	}

	*freq = target_frequency;
	nvgpu_info(g, "VBIOS: Drive mode GPC frequency: %d MHz", *freq);

done:
	return status;
}

