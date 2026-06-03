// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/log.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/types.h>
#include <nvgpu/firmware.h>
#include <nvgpu/enabled.h>
#include <nvgpu/bug.h>
#include <nvgpu/dma.h>
#include <nvgpu/grmgr.h>
#include <nvgpu/soc.h>
#include <nvgpu/falcon.h>
#include <nvgpu/riscv.h>
#include <nvgpu/io.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/pmu.h>
#include <nvgpu/acr.h>
#include <nvgpu/gsp.h>
#ifdef CONFIG_NVGPU_LS_PMU
#include <nvgpu/pmu/fw.h>
#endif
#include <nvgpu/hw/gb10b/hw_priscv_gb10b.h>
#include "common/acr/acr_wpr.h"
#include "common/acr/acr_priv.h"
#include "common/acr/acr_blob_alloc.h"
#include "common/acr/acr_blob_construct_v2.h"
#include "common/acr/acr_bootstrap.h"
#include "common/acr/acr_sw_ga10b.h"
#include "common/acr/acr_sw_gb10b.h"
#include "common/acr/acr_priv.h"
#include "acr_sw_gb20c.h"
#define GSPDBG_RISCV_ACR_FW	             "acr_gsp_gb20c_pwm.bin"
#define GSP_BCR_STAGE1_PKCPARAMS_SIZE    0x400U
static u32 gb20c_acr_lsf_config(struct gk20a *g,
	struct nvgpu_acr *acr)
{
	u32 lsf_enable_mask = 0U;
	lsf_enable_mask |= nvgpu_gb10b_acr_lsf_riscv_eb_fecs(g,
					&acr->lsf[FALCON_ID_FECS_RISCV_EB]);
	lsf_enable_mask |= nvgpu_gb10b_acr_lsf_riscv_eb_gpccs(g,
					&acr->lsf[FALCON_ID_GPCCS_RISCV_EB]);
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_MULTIMEDIA)) {
		lsf_enable_mask |= gb10b_acr_lsf_ofa_riscv(g,
						&acr->lsf[FALCON_ID_OFA_RISCV_EB]);
		lsf_enable_mask |= gb10b_acr_lsf_nvenc_riscv(g,
						&acr->lsf[FALCON_ID_NVENC_RISCV_EB]);
		lsf_enable_mask |= gb10b_acr_lsf_nvdec_riscv(g,
						&acr->lsf[FALCON_ID_NVDEC_RISCV]);
	}
	if (g->ops.pmu.is_pmu_supported != NULL) {
		if (g->ops.pmu.is_pmu_supported(g)) {
			lsf_enable_mask |= gb10b_acr_lsf_pmu_riscv_eb(g,
							&acr->lsf[FALCON_ID_PMU_RISCV_EB]);
		}
	}
	return lsf_enable_mask;
}
static void gb20c_acr_sw_init(struct gk20a *g, struct nvgpu_acr *acr)
{
	nvgpu_log_fn(g, " ");
	acr->g = g;
	g->acr->is_acr_lsb_v3_enabled = true;
	acr->bootstrap_owner = FALCON_ID_GSPLITE;
	acr->lsf_enable_mask = gb20c_acr_lsf_config(g, acr);
	gb10b_acr_default_sw_init(g, &acr->acr_asc);
	acr->prepare_ucode_blob = nvgpu_acr_prepare_ucode_blob_v2;
	acr->get_wpr_info = nvgpu_acr_wpr_info_sys;
	acr->alloc_blob_space = nvgpu_acr_alloc_blob_space_sys;
	acr->bootstrap_hs_acr = nvgpu_gb10b_bootstrap_hs_acr;
	acr->patch_wpr_info_to_ucode = nvgpu_gb10b_acr_patch_wpr_info_to_ucode;
}
void nvgpu_gb20c_acr_sw_init(struct gk20a *g, struct nvgpu_acr *acr)
{
	nvgpu_log_fn(g, " ");
	acr->g = g;
	nvgpu_gb10b_acr_sw_ipc_init(g, acr);
	nvgpu_set_enabled(g, NVGPU_ACR_NEXT_CORE_ENABLED, true);
	nvgpu_set_enabled(g, NVGPU_PKC_LS_SIG_ENABLED, true);
	gb20c_acr_sw_init(g, acr);
	return;
}
