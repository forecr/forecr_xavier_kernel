// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "acr_sw_gp10b.h"

#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/pmu.h>

#include "acr_blob_construct_v0.h"
#include "acr_priv.h"

#include "acr_sw_gm20b.h"
#include "acr_sw_gp10b.h"

/* LSF static config functions */
static u32 gp10b_acr_lsf_gpccs(struct gk20a *g,
		struct acr_lsf_config *lsf)
{
	(void)g;
	/* GPCCS LS falcon info */
	lsf->falcon_id = FALCON_ID_GPCCS;
	lsf->falcon_dma_idx = GK20A_PMU_DMAIDX_UCODE;
	lsf->is_lazy_bootstrap = true;
	lsf->is_priv_load = true;
	lsf->get_lsf_ucode_details = nvgpu_acr_lsf_gpccs_ucode_details_v0;
	lsf->get_cmd_line_args_offset = NULL;

	return BIT32(lsf->falcon_id);
}

static void gp10b_acr_default_sw_init(struct gk20a *g, struct hs_acr *hs_acr)
{
	nvgpu_log_fn(g, " ");

	/* ACR HS ucode type & f/w name*/
	hs_acr->acr_type = ACR_DEFAULT;

	if (!g->ops.pmu.is_debug_mode_enabled(g)) {
		hs_acr->acr_fw_name = HSBIN_ACR_PROD_UCODE;
	} else {
		hs_acr->acr_fw_name = HSBIN_ACR_DBG_UCODE;
	}

	/* set on which falcon ACR need to execute*/
	hs_acr->acr_flcn = g->pmu->flcn;
	hs_acr->acr_engine_bus_err_status =
		g->ops.pmu.bar0_error_status;
}

void nvgpu_gp10b_acr_sw_init(struct gk20a *g, struct nvgpu_acr *acr)
{
	nvgpu_log_fn(g, " ");

	/* inherit the gm20b config data */
	nvgpu_gm20b_acr_sw_init(g, acr);
	gp10b_acr_default_sw_init(g, &acr->acr);

	/* gp10b supports LSF gpccs bootstrap */
	acr->lsf_enable_mask |= gp10b_acr_lsf_gpccs(g,
		&acr->lsf[FALCON_ID_GPCCS]);
}
