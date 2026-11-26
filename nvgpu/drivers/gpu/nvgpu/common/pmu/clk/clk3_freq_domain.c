// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025-2026, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bios.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/pmu/boardobjgrp_classes.h>
#include <nvgpu/boardobjgrpmask.h>
#include <nvgpu/pmu/clk/clk.h>
#include <nvgpu/string.h>

#include "ucode_clk_inf.h"
#include "clk3_freq_domain.h"
#include "clk.h"
#include "clk_domain.h"

// Clock 3.0 : Clock frequency domain objects
struct nvgpu_freq_domain_grp freqDomainGrp = { 0 };

// gb20c iGPU is displayless.
static const u32 clkFreqDomainGrpBitmapDisplayless =
	CTRL_CLK_DOMAIN_GPCCLK   |
	CTRL_CLK_DOMAIN_XBARCLK  |
	CTRL_CLK_DOMAIN_NVDCLK   |
	CTRL_CLK_DOMAIN_UPROCCLK |
	CTRL_CLK_DOMAIN_MCLK     |
	CTRL_CLK_DOMAIN_UTILSCLK;

/*!
 * @brief       Bitmap of clocks domains supported by Clocks 3.x
 *
 * @details     This implementation includes display clocks unless the display
 *              has been floorswept.
 *
 * @param[in]   g     Pointer to GPU object
 *
 */
static u32 clkFreqDomainGrpBitmap_HAL(struct gk20a *g)
{
	(void) g;
	return clkFreqDomainGrpBitmapDisplayless;
}

static s32 clk_freq_domain_pmudata_instget(struct gk20a *g,
		struct nv_pmu_boardobjgrp *pmuboardobjgrp,
		struct nv_pmu_boardobj **pmu_obj, u16 idx)
{
	struct nv_pmu_clk_freq_domain_boardobj_grp_set  *pgrp_set =
		(struct nv_pmu_clk_freq_domain_boardobj_grp_set *)
		pmuboardobjgrp;

	nvgpu_log_info(g, " ");

	/*check whether pmuboardobjgrp has a valid boardobj in index*/
	if (idx >= CTRL_BOARDOBJGRP_E32_MAX_OBJECTS) {
		nvgpu_err(g, "Invalid boardonj index: %d", idx);
		return -EINVAL;
	}

	*pmu_obj = (struct nv_pmu_boardobj *)
		&pgrp_set->objects[idx].data.obj;
	nvgpu_log_info(g, " Done");
	return 0;
}

static s32 clk_freq_domain_pmudatainit(struct gk20a *g,
		struct boardobjgrp *pboardobjgrp,
		struct nv_pmu_boardobjgrp_super *pboardobjgrppmu)
{
	s32 status = 0;
	struct nv_pmu_clk_freq_domain_boardobjgrp_set_header *pFreqDomainNvPmuHeader = NULL;
	struct nvgpu_freq_domain_grp *pFreqDomainGrp = NULL;

	nvgpu_pmu_dbg(g, " ");

	status = boardobjgrp_pmudatainit_e32(g, pboardobjgrp, pboardobjgrppmu);
	if (status != 0) {
		nvgpu_err(g, "Error updating pmu boardobjgrp for Clk_freq_domain. status - %d",
				status);
		goto done;
	}

	// Set FREQ_DOMAIN_GRP-specific data
	pFreqDomainNvPmuHeader =
		(struct nv_pmu_clk_freq_domain_boardobjgrp_set_header *)pboardobjgrppmu;
	pFreqDomainGrp         =
		(struct nvgpu_freq_domain_grp *) pboardobjgrp;

	// Copy init flags in PMU data.
	pFreqDomainNvPmuHeader->initFlags =
		pFreqDomainGrp->initFlags;

	pFreqDomainNvPmuHeader->mclkFreqMHzBootPstate =
		pFreqDomainGrp->mclkFreqMHzBootPstate;

done:
	nvgpu_pmu_dbg(g, " Done");

	return status;
}

static s32 clkFreqDomain_PmuDataInit_SUPER(struct gk20a *g,
	struct pmu_board_obj *pBoardObj,
	struct nv_pmu_boardobj *pPmuData)
{
	struct nv_pmu_clk_freq_domain_boardobj_set *pFreqDomainSet = NULL;
	struct freq_domain *pFreqDomain   = NULL;
	s32 status = 0;

	// Call BOARDOBJ super class.
	status = pmu_board_obj_pmu_data_init_super(g, pBoardObj, pPmuData);
	if (status != 0) {
		nvgpu_err(g,
			"Error initializing BOARDOBJ PMU data: status=0%d",
			status);
		goto clkFreqDomainIfaceModel10PmuDataInit_SUPER_exit;
	}

	pFreqDomainSet = (struct nv_pmu_clk_freq_domain_boardobj_set *)pPmuData;
	pFreqDomain    = (struct freq_domain *)pBoardObj;

	// Set FREQ_DOMAIN-specific data.
	pFreqDomainSet->clkDomain = pFreqDomain->clkDomain;

clkFreqDomainIfaceModel10PmuDataInit_SUPER_exit:
	return status;
}

static struct freq_domain *clkFreqDomainConstruct(struct gk20a *g, void *pArgs)
{
	struct pmu_board_obj *pBoardObj = NULL;
	struct freq_domain *pFreqDomain = NULL;
	struct freq_domain *pFreqDomainTmp =
		(struct freq_domain *) pArgs;
	s32 status;

	// Construct the common part
	pFreqDomain = nvgpu_kzalloc(g, sizeof(struct freq_domain));
	if (pFreqDomain == NULL) {
		nvgpu_err(g, "memory allocation for freq_domain failed.");
		return NULL;
	}
	pBoardObj = (struct pmu_board_obj *)(void *)pFreqDomain;

	status = pmu_board_obj_construct_super(g, pBoardObj, pArgs);
	if (status != 0) {
		nvgpu_err(g,
			"Error constructing FREQ_DOMAIN: status=%d", status);
		nvgpu_kfree(g, pFreqDomain);
		return NULL;
	}

	// Set type-specific parameters.
	pFreqDomain->clkDomain = pFreqDomainTmp->clkDomain;

	// Set BOARDOBJ interfaces.
	pBoardObj->pmudatainit = clkFreqDomain_PmuDataInit_SUPER;

	return (struct freq_domain *)pBoardObj;
}


static s32 clkFreqDomainGrpConstruct(struct gk20a *g,
	u32 domainSet,
	struct nvgpu_freq_domain_grp *pFreqDomainGrp)
{
	struct freq_domain *pFreqDomain;
	struct freq_domain  freqDomainData;
	u32 domain;
	u32 lclMask;
	u8 entry = 0;
	s32 status = 0;

	// For all the supported domains
	lclMask = domainSet;
	for (domain = 0; lclMask != 0U; domain++) {
		if ((BIT32(0) & lclMask) == 0U) {
			lclMask >>= 1U;
			continue;
		}
		// Init fields to zero.
		memset(&freqDomainData, 0x00U, sizeof(freqDomainData));

		// Populate BOARDOBJ specific data
		freqDomainData.super.type = NV2080_CTRL_CLK_FREQ_DOMAIN_SCHEMA_CLK3;

		// Populate FREQ_DOMAIN specific data.
		freqDomainData.clkDomain = BIT32(domain);

		// Construct FREQ_DOMAIN object
		pFreqDomain = clkFreqDomainConstruct(g, (void *) &freqDomainData);
		if (pFreqDomain == NULL) {
			nvgpu_err(g,
				"Could not construct FREQ_DOMAIN.  entry=%u  domain=0x%08x",
				entry, domain);
			status = -EINVAL;
			goto clkFreqDomainGrpConstruct_exit;
		}

		// Insert FREQ_DOMAIN object into the FREQ_DOMAIN_GRP.
		status = boardobjgrp_objinsert(&pFreqDomainGrp->super.super,
					&pFreqDomain->super, entry);
		if (status != 0) {
			nvgpu_err(g,
				"Could not insert FREQ_DOMAIN into FREQ_DOMAIN_GRP.  entry=%d  domain=0x%08x  status=%d",
				entry, BIT32(domain), status);
			goto clkFreqDomainGrpConstruct_exit;
		}

		lclMask >>= 1U;
		// Count the domains
		++entry;
	}

clkFreqDomainGrpConstruct_exit:
	return status;
}

s32 clk3_freq_domain_sw_setup(struct gk20a *g)
{
	s32 status = 0;
	struct boardobjgrp *pboardobjgrp = NULL;
	struct nvgpu_clk_domain *pClkDomain;

	nvgpu_pmu_dbg(g, " ");
	if (g->pmu->clk_pmu->clk_freqDomainGrp == NULL) {
		nvgpu_err(g,
			"clk_freqDomainGrp is NULL");
		status = -EINVAL;
		goto done;
	}

	/* If already constructed, do not re-construct (suspend/resume pattern) */
	pboardobjgrp = &g->pmu->clk_pmu->clk_freqDomainGrp->super.super;
	if (pboardobjgrp->bconstructed) {
		nvgpu_pmu_dbg(g, "clk3_freq_domain boardobjgrp already constructed, skipping reinit");
		return 0;
	}

	nvgpu_pmu_dbg(g, "Constructing clk3_freq_domain boardobjgrp for first time");

	status = nvgpu_boardobjgrp_construct_e32(g,
			&g->pmu->clk_pmu->clk_freqDomainGrp->super);
	if (status != 0) {
		nvgpu_err(g,
			"error creating boardobjgrp for clk freq domain err=%d",
			status);
		goto done;
	}
	g->pmu->clk_pmu->clk_freqDomainGrp->initFlags = 0;

	pClkDomain = clkDomainsFindByApiDomain(g, CTRL_CLK_DOMAIN_MCLK);
	// Todo: Implement MCLK setting if we implement Pstate support?
	(void) pClkDomain;
	// Populate the Boot Pstate MCLK setting.
	// Ok to populate value to zero for no Pstate support
	// and memory clk is controlled by SoC.
	g->pmu->clk_pmu->clk_freqDomainGrp->mclkFreqMHzBootPstate = 0;

	BOARDOBJGRP_PMU_CONSTRUCT(pboardobjgrp, CLK, CLK_FREQ_DOMAIN);

	status = BOARDOBJGRP_PMU_CMD_GRP_SET_CONSTRUCT(g, pboardobjgrp,
			clk, CLK, freq_domain, CLK_FREQ_DOMAIN);
	if (status != 0) {
		nvgpu_err(g,
			"error constructing PMU_BOARDOBJ_CMD_GRP_SET interface - status=%d",
			status);
		goto done;
	}

	pboardobjgrp->pmudatainit = clk_freq_domain_pmudatainit;
	pboardobjgrp->pmudatainstget = clk_freq_domain_pmudata_instget;

	// Construct all FREQ_DOMAIN objects in FREQ_DOMAIN_GRP
	status = clkFreqDomainGrpConstruct(g,
			clkFreqDomainGrpBitmap_HAL(g), g->pmu->clk_pmu->clk_freqDomainGrp);
	if (status != 0) {
		nvgpu_err(g,
			"error constructing FREQ_DOMAIN objects - status=%d",
			status);
		goto done;
	}

done:
	nvgpu_pmu_dbg(g, " done status %x", status);

	return status;
}

s32 clk3_freq_domain_pmu_setup(struct gk20a *g)
{
	s32 status = 0;
	struct boardobjgrp *pboardobjgrp = NULL;

	nvgpu_pmu_dbg(g, " ");

	pboardobjgrp = &g->pmu->clk_pmu->clk_freqDomainGrp->super.super;

	if (!pboardobjgrp->bconstructed) {
		nvgpu_err(g, "FREQ_DOMAIN boardobjgrp not constructed");
		return -EINVAL;
	}

	status = pboardobjgrp->pmuinithandle(g, pboardobjgrp);

	nvgpu_pmu_dbg(g, " done status %d", status);
	return status;
}

s32 clk3_freq_domain_init_pmupstate(struct gk20a *g)
{
	// FREQ_DOMAIN GRP is static structure.
	g->pmu->clk_pmu->clk_freqDomainGrp = &freqDomainGrp;
	return 0;
}

void clk3_freq_domain_free_pmupstate(struct gk20a *g)
{
	// Nothing to do here. FREQ_DOMAIN GRP is static structure.
	(void) g;
}

