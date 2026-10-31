// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

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
#include <nvgpu/nvgpu_gb10b_firmware.h>
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
#include "common/acr/acr_priv.h"
#include "acr_sw_gb10b.h"

#define GSP_BCR_STAGE1_PKCPARAMS_SIZE    0x400U

u32 gb10b_acr_lsf_pmu_riscv_eb(struct gk20a *g,
		struct acr_lsf_config *lsf)
{
	nvgpu_log_fn(g, " ");

	/* PMU riscv eb info */
	lsf->falcon_id = FALCON_ID_PMU_RISCV_EB;
	lsf->falcon_dma_idx = GK20A_PMU_DMAIDX_UCODE;
	lsf->is_lazy_bootstrap = false;
	lsf->is_priv_load = false;
	lsf->get_lsf_ucode_details = nvgpu_gb10b_acr_lsf_pmu_riscv_eb_ucode_details;
	lsf->get_cmd_line_args_offset = NULL;

	return BIT32(lsf->falcon_id);
}

u32 nvgpu_gb10b_acr_lsf_riscv_eb_fecs(struct gk20a *g,
		struct acr_lsf_config *lsf)
{
	nvgpu_log_fn(g, " ");

	/* FECS RISCV EB info */
	lsf->falcon_id = FALCON_ID_FECS_RISCV_EB;
	lsf->falcon_dma_idx = GK20A_PMU_DMAIDX_UCODE;
	lsf->is_lazy_bootstrap = false;
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_MIG)) {
		lsf->is_priv_load = true;
	} else {
		lsf->is_priv_load = false;
	}
	lsf->get_lsf_ucode_details = nvgpu_gb10b_acr_lsf_fecs_riscv_eb_ucode_details;
	lsf->get_cmd_line_args_offset = NULL;

	return BIT32(lsf->falcon_id);
}

u32 nvgpu_gb10b_acr_lsf_riscv_eb_gpccs(struct gk20a *g,
		struct acr_lsf_config *lsf)
{
	nvgpu_log_fn(g, " ");

	/* GPCCS RISCV EB info */
	lsf->falcon_id = FALCON_ID_GPCCS_RISCV_EB;
	lsf->falcon_dma_idx = GK20A_PMU_DMAIDX_UCODE;
	lsf->is_lazy_bootstrap = false;
	lsf->is_priv_load = true;
	lsf->get_lsf_ucode_details = nvgpu_gb10b_acr_lsf_gpccs_riscv_eb_ucode_details;
	lsf->get_cmd_line_args_offset = NULL;

	return BIT32(lsf->falcon_id);
}

u32 gb10b_acr_lsf_nvdec_riscv(struct gk20a *g,
		struct acr_lsf_config *lsf)
{
	nvgpu_log_fn(g, " ");

	/* NVDEC riscv  info */
	lsf->falcon_id = FALCON_ID_NVDEC_RISCV;
	lsf->falcon_dma_idx = GK20A_PMU_DMAIDX_UCODE;
	lsf->is_lazy_bootstrap = false;
	lsf->is_priv_load = false;
	lsf->get_lsf_ucode_details = nvgpu_gb10b_acr_lsf_nvdec_riscv_ucode_details;
	lsf->get_cmd_line_args_offset = NULL;

	return BIT32(lsf->falcon_id);
}

u32 gb10b_acr_lsf_nvenc_riscv(struct gk20a *g,
		struct acr_lsf_config *lsf)
{
	nvgpu_log_fn(g, " ");

	/* NVENC riscv eb info */
	lsf->falcon_id = FALCON_ID_NVENC_RISCV_EB;
	lsf->falcon_dma_idx = GK20A_PMU_DMAIDX_UCODE;
	lsf->is_lazy_bootstrap = false;
	lsf->is_priv_load = false;
	lsf->get_lsf_ucode_details = nvgpu_gb10b_acr_lsf_nvenc_riscv_eb_ucode_details;
	lsf->get_cmd_line_args_offset = NULL;

	return BIT32(lsf->falcon_id);
}

u32 gb10b_acr_lsf_ofa_riscv(struct gk20a *g,
		struct acr_lsf_config *lsf)
{
	nvgpu_log_fn(g, " ");

	/* OFA riscv eb info */
	lsf->falcon_id = FALCON_ID_OFA_RISCV_EB;
	lsf->falcon_dma_idx = GK20A_PMU_DMAIDX_UCODE;
	lsf->is_lazy_bootstrap = false;
	lsf->is_priv_load = false;
	lsf->get_lsf_ucode_details = nvgpu_gb10b_acr_lsf_ofa_riscv_eb_ucode_details;
	lsf->get_cmd_line_args_offset = NULL;

	return BIT32(lsf->falcon_id);
}

static u32 gb10b_acr_lsf_nvjpg_riscv(struct gk20a *g,
		struct acr_lsf_config *lsf)
{
	nvgpu_log_fn(g, " ");

	/* NVJPG riscv eb info */
	lsf->falcon_id = FALCON_ID_NVJPG_RISCV_EB;
	lsf->falcon_dma_idx = GK20A_PMU_DMAIDX_UCODE;
	lsf->is_lazy_bootstrap = false;
	lsf->is_priv_load = false;
	lsf->get_lsf_ucode_details = nvgpu_gb10b_acr_lsf_nvjpg_riscv_eb_ucode_details;
	lsf->get_cmd_line_args_offset = NULL;

	return BIT32(lsf->falcon_id);
}

int nvgpu_gb10b_acr_patch_wpr_info_to_ucode(struct gk20a *g,
	struct nvgpu_acr *acr, struct hs_acr *acr_desc, bool is_recovery)
{
	int err = 0;
#ifdef CONFIG_NVGPU_MIG
	bool swizzle_id_checks;
	u32 swizzle_id = 0;
#endif
	struct nvgpu_mem *acr_falcon2_sysmem_desc =
					&acr_desc->acr_falcon2_sysmem_desc;
	RM_RISCV_ACR_DESC_WRAPPER  *acr_sysmem_desc = &acr_desc->acr_sysmem_desc_v1;

	(void)acr;
	(void)is_recovery;

	nvgpu_log_fn(g, " ");

	if (is_recovery) {
		/*
		 * In case of recovery ucode blob size is 0 as it has already
		 * been authenticated during cold boot.
		 */
		if (!nvgpu_mem_is_valid(&acr_desc->acr_falcon2_sysmem_desc)) {
			nvgpu_err(g, "invalid mem acr_falcon2_sysmem_desc");
			return -EINVAL;
                }
	} else
    {
		/*
		 * Alloc space for  sys mem space to which interface struct is
		 * copied.
		 */
		if (!nvgpu_mem_is_valid(acr_falcon2_sysmem_desc)) {
			err = nvgpu_dma_alloc_flags_sys(g,
				NVGPU_DMA_PHYSICALLY_ADDRESSED,
				sizeof(RM_RISCV_ACR_DESC_WRAPPER ),
				acr_falcon2_sysmem_desc);
			if (err != 0) {
				nvgpu_err(g, "alloc for sysmem desc failed");
				goto end;
			}
		}

		/*
		 * Generic header info for ACR descriptor.
		 */
		acr_sysmem_desc->genericHdr.size =
			nvgpu_safe_cast_u64_to_u32(RM_RISCV_ACR_DESC_V1_WRAPPER_SIZE_BYTE);
		acr_sysmem_desc->genericHdr.version = ACR_DESC_GENERIC_HEADER_VERSION_1;
		acr_sysmem_desc->genericHdr.identifier = WPR_GENERIC_HEADER_ID_ACR_DESC_HEADER;

		/*
		 * Start address of non wpr sysmem region holding ucode blob.
		 */
		acr_sysmem_desc->acrDesc.riscvAcrDescV1.regionDesc[0].nonWprBlobStart =
			nvgpu_mem_get_addr(g, &g->acr->ucode_blob);
		/*
		 * LS ucode blob size.
		 */
		nvgpu_assert(g->acr->ucode_blob.size <= U32_MAX);
		acr_sysmem_desc->acrDesc.riscvAcrDescV1.regionDesc[0].nonWprBlobSize =
			(u32)g->acr->ucode_blob.size;

		/*
		 * Set the WPR1 region id.
		 */
		acr_sysmem_desc->acrDesc.riscvAcrDescV1.regionDesc[0].regionID = 0x1U;
		acr_sysmem_desc->acrDesc.riscvAcrDescV1.mode = 0U;
		if (nvgpu_is_enabled(g, NVGPU_SUPPORT_EMULATE_MODE) &&
				(g->emulate_mode < EMULATE_MODE_MAX_CONFIG)) {
			acr_sysmem_desc->acrDesc.riscvAcrDescV1.mode &= (~EMULATE_MODE_MASK);
			acr_sysmem_desc->acrDesc.riscvAcrDescV1.mode |= g->emulate_mode;
		} else {
			acr_sysmem_desc->acrDesc.riscvAcrDescV1.mode &= (~EMULATE_MODE_MASK);
		}

#ifdef CONFIG_NVGPU_MIG
		if (nvgpu_is_enabled(g, NVGPU_SUPPORT_MIG)) {
			acr_sysmem_desc->acrDesc.riscvAcrDescV1.mode |=
				nvgpu_safe_cast_u64_to_u32(MIG_MODE);
			if (g->ops.grmgr.choose_mig_vpr_setting != NULL) {
				err = g->ops.grmgr.choose_mig_vpr_setting(g,
						&swizzle_id_checks, &swizzle_id);
				if (err != 0) {
					nvgpu_err(g, "Failed to choose mig config. Error: %d", err);
					goto end;
				}
			}
			/*
		 	* Pass VPR swizzle id in Bits 24:31
		 	*/
			acr_sysmem_desc->acrDesc.riscvAcrDescV1.mode |=
						((nvgpu_safe_cast_u32_to_u8(swizzle_id)) << 24);
		} else {
			acr_sysmem_desc->acrDesc.riscvAcrDescV1.mode &= (u32)(~MIG_MODE);
		}
#endif
	}

	/*
	 * Push the acr descriptor data to sysmem.
	 */
	nvgpu_mem_wr_n(g, acr_falcon2_sysmem_desc, 0U,
				acr_sysmem_desc,
					sizeof(RM_RISCV_ACR_DESC_WRAPPER ));

end:
	return err;
}

static int acr_bootstrap_gsp_dma_pwm(struct gk20a *g, struct nvgpu_falcon *flcn)
{
	int err = 0;
	u32 val;
	u64 fmc_base_addr;
	struct nvgpu_firmware *gsp_fw;
	static struct nvgpu_mem ucode;
	struct mm_gk20a *mm = &g->mm;
	struct vm_gk20a *vm = mm->pmu.vm;

	nvgpu_log_fn(g, " ");

	gsp_fw = nvgpu_request_gsp_firmware(g);

	if (gsp_fw == NULL)
	{
		nvgpu_err(g, "ucode get fail for GSP FMC FW");
		return -ENOENT;
	}

	if (!nvgpu_mem_is_valid(&ucode)) {
		err = nvgpu_dma_alloc_map_sys(vm, gsp_fw->size, &ucode);
		if(err != 0) {
			nvgpu_err(g, "GSP ALLOC fail [%lu]", gsp_fw->size);
			goto free_gsp_fw;
		}
	}

	nvgpu_mem_wr_n(g, &ucode, 0, gsp_fw->data, gsp_fw->size);

	/* Skip the first 1KB as it holds the STAGE1 PKCPARAMS which BR doesn't process */
	fmc_base_addr = nvgpu_mem_get_addr(g, &ucode) - GSP_BCR_STAGE1_PKCPARAMS_SIZE;
	fmc_base_addr = fmc_base_addr >> 8U;

	/* core reset */
	err = nvgpu_falcon_reset(flcn);
	if (err != 0) {
		nvgpu_err(g, "GSP core reset failed err=%d", err);
		goto free_gsp_fw;
	}

	nvgpu_riscv_writel(flcn, priscv_priscv_bcr_ctrl_r(), 0x110U);

	/* Pass the fmc base to bcr dma cfg */
	nvgpu_riscv_writel(flcn,
		priscv_priscv_bcr_dmaaddr_stage2_pkcparam_lo_r(),
		(u32)(fmc_base_addr & ~(u32)0));

	nvgpu_riscv_writel(flcn,
		priscv_priscv_bcr_dmaaddr_stage2_pkcparam_hi_r(),
		(u32)(fmc_base_addr >> 32));

	/* Set pointer walking mode parameters for dma cfg */
	val = priscv_priscv_bcr_dmacfg_target_coherent_system_f() |
			priscv_priscv_bcr_dmacfg_pointer_walking_true_f() |
			priscv_priscv_bcr_dmacfg_lock_locked_f();

	nvgpu_riscv_writel(flcn, priscv_priscv_bcr_dmacfg_r(), val);

	g->ops.falcon.bootstrap(flcn, 0x0);

	err = nvgpu_falcon_wait_for_nvriscv_brom_completion(flcn);
	if (err != 0) {
		nvgpu_err(g, "GSP RISCV BROM FAILURE");
	}

free_gsp_fw:
	nvgpu_release_firmware(g, gsp_fw);
	return err;
}

static int nvgpu_gb10b_acr_bootstrap_hs_ucode_riscv
(
	struct gk20a *g,
	struct nvgpu_acr *acr
)
{
	int err = 0;
	u32 timeout = 0;
	struct nvgpu_falcon *flcn = NULL;

	flcn = acr->acr_asc.acr_flcn;
	if (acr->is_cold_boot_done) {
		err = acr->patch_wpr_info_to_ucode(g, acr, &acr->acr_asc, true);
		if (err != 0) {
			nvgpu_err(g, "RISCV ucode patch wpr info failed");
			return err;
		}
	} else {
		err = acr->patch_wpr_info_to_ucode(g, acr, &acr->acr_asc, false);
		if (err != 0) {
			nvgpu_err(g, "RISCV ucode patch wpr info failed");
			return err;
		}
	}

	/* Enable required interrupts support and isr */
	g->is_acr_ipc_supported = true;
	nvgpu_gsp_isr_support(g, true);

	/*
	* For other platforms SEC2 will bootstrap GSP
	*/
	if (!nvgpu_platform_is_silicon(g)) {
		err = acr_bootstrap_gsp_dma_pwm(g, flcn);
		if (err != 0) {
			nvgpu_err(g, "GSP bootstrap failed, err=%d", err);
			return err;
		}
	}

	if (nvgpu_platform_is_silicon(g)) {
		timeout = ACR_COMPLETION_TIMEOUT_SILICON_MS;
	} else {
		timeout = ACR_COMPLETION_TIMEOUT_NON_SILICON_MS;
	}

	err = nvgpu_next_acr_wait_for_completion(g, &acr->acr_asc, timeout);
	return err;
}

int nvgpu_gb10b_bootstrap_hs_acr(struct gk20a *g, struct nvgpu_acr *acr)
{
	int err = 0;

	(void)acr;

	nvgpu_log_fn(g, " ");

	err = nvgpu_gb10b_acr_bootstrap_hs_ucode_riscv(g, g->acr);
	if (err != 0) {
		nvgpu_err(g, "ACR bootstrap failed");
	}

	return err;
}

void gb10b_acr_default_sw_init(struct gk20a *g, struct hs_acr *riscv_hs)
{
	nvgpu_log_fn(g, " ");

	riscv_hs->acr_type = ACR_DEFAULT;

	riscv_hs->acr_flcn = &g->gsp_flcn;
	riscv_hs->report_acr_engine_bus_err_status =
		nvgpu_pmu_report_bar0_pri_err_status;
	riscv_hs->acr_engine_bus_err_status =
		g->ops.pmu.bar0_error_status;
	riscv_hs->acr_validate_mem_integrity = NULL;
}

static u32 gb10b_acr_lsf_config(struct gk20a *g,
	struct nvgpu_acr *acr)
{
	u32 lsf_enable_mask = 0U;

	/*
	 * For simulation boot falcon due to bug in fmodel
	 * regarding ctxsw eb recovery
	 * TODO: Enable CTXSW EB RISCV in SCSIM
	 */
	if (nvgpu_platform_is_simulation(g)) {
		if (!g->fecs_flcn.is_falcon2_enabled) {
			lsf_enable_mask |= nvgpu_ga10b_acr_lsf_fecs(g,
							&acr->lsf[FALCON_ID_FECS]);
			lsf_enable_mask |= nvgpu_ga10b_acr_lsf_gpccs(g,
							&acr->lsf[FALCON_ID_GPCCS]);
		} else {
			lsf_enable_mask |= nvgpu_gb10b_acr_lsf_riscv_eb_fecs(g,
							&acr->lsf[FALCON_ID_FECS_RISCV_EB]);
			lsf_enable_mask |= nvgpu_gb10b_acr_lsf_riscv_eb_gpccs(g,
							&acr->lsf[FALCON_ID_GPCCS_RISCV_EB]);
		}
	} else {
		lsf_enable_mask |= nvgpu_gb10b_acr_lsf_riscv_eb_fecs(g,
						&acr->lsf[FALCON_ID_FECS_RISCV_EB]);
		lsf_enable_mask |= nvgpu_gb10b_acr_lsf_riscv_eb_gpccs(g,
						&acr->lsf[FALCON_ID_GPCCS_RISCV_EB]);
	}
	lsf_enable_mask |= gb10b_acr_lsf_nvjpg_riscv(g,
					&acr->lsf[FALCON_ID_NVJPG_RISCV_EB]);
	lsf_enable_mask |= gb10b_acr_lsf_ofa_riscv(g,
					&acr->lsf[FALCON_ID_OFA_RISCV_EB]);
	lsf_enable_mask |= gb10b_acr_lsf_nvenc_riscv(g,
					&acr->lsf[FALCON_ID_NVENC_RISCV_EB]);
	lsf_enable_mask |= gb10b_acr_lsf_nvdec_riscv(g,
					&acr->lsf[FALCON_ID_NVDEC_RISCV]);
	if (g->ops.pmu.is_pmu_supported != NULL) {
		if (g->ops.pmu.is_pmu_supported(g)) {
			lsf_enable_mask |= gb10b_acr_lsf_pmu_riscv_eb(g,
							&acr->lsf[FALCON_ID_PMU_RISCV_EB]);
		}
	}

	return lsf_enable_mask;
}

static void gb10b_acr_sw_init(struct gk20a *g, struct nvgpu_acr *acr)
{
	nvgpu_log_fn(g, " ");

	acr->g = g;

	acr->bootstrap_owner = FALCON_ID_GSPLITE;

	acr->lsf_enable_mask = gb10b_acr_lsf_config(g, acr);

	gb10b_acr_default_sw_init(g, &acr->acr_asc);

	acr->prepare_ucode_blob = nvgpu_acr_prepare_ucode_blob_v2;

	acr->get_wpr_info = nvgpu_acr_wpr_info_sys;
	acr->alloc_blob_space = nvgpu_acr_alloc_blob_space_sys;
	acr->bootstrap_hs_acr = nvgpu_gb10b_bootstrap_hs_acr;

	acr->patch_wpr_info_to_ucode = nvgpu_gb10b_acr_patch_wpr_info_to_ucode;
}

void nvgpu_gb10b_acr_sw_ipc_init(struct gk20a *g, struct nvgpu_acr *acr)
{
	(void) acr;
	if (!nvgpu_falcon_is_falcon2_enabled(&g->gsp_flcn)) {
		nvgpu_err(g, "GSP RISCV Peregrine core not enabled");
		goto deinit;
	}
	return;

deinit:
	nvgpu_acr_deinit(g);
	return;
}

void nvgpu_gb10b_acr_sw_init(struct gk20a *g, struct nvgpu_acr *acr)
{
	nvgpu_log_fn(g, " ");
	acr->g = g;
	acr->gsp = g->gsp;

	nvgpu_gb10b_acr_sw_ipc_init(g, acr);

	nvgpu_set_enabled(g, NVGPU_ACR_NEXT_CORE_ENABLED, true);
	nvgpu_set_enabled(g, NVGPU_PKC_LS_SIG_ENABLED, true);

	gb10b_acr_sw_init(g, acr);

	return;
}
