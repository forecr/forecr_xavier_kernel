// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/dma.h>
#include <nvgpu/firmware.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>
#include <nvgpu/acr.h>
#include <nvgpu/falcon.h>
#include <nvgpu/gsp.h>

#include "acr_priv.h"
#ifdef CONFIG_NVGPU_ACR_LEGACY
#include "acr_sw_gm20b.h"
#include "acr_sw_gp10b.h"
#endif
#include "acr_sw_gv11b.h"
#include "acr_sw_ga10b.h"
#ifdef CONFIG_NVGPU_DGPU
#include "acr_sw_tu104.h"
#include "acr_sw_ga100.h"
#endif

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
#include "acr_sw_gb10b.h"
#endif

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB20C)
#include "acr_sw_gb20c.h"
#endif

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
#include <nvgpu_next_acr.h>
#endif

bool nvgpu_acr_is_lsb_v3_supported(struct gk20a *g)
{
	return g->acr->is_acr_lsb_v3_enabled;
}

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
static int nvgpu_acr_wait_for_init(struct gk20a *g, signed int timeoutms)
{
	u32 intr_stat = 0;

	nvgpu_acr_dbg(g, " ");

	do {
		if (nvgpu_gsp_is_ready(g)) {
			break;
		}

		if (!g->ops.falcon.is_priv_lockdown(&g->gsp_flcn)) {
			if (g->ops.gsp.gsp_is_interrupted(g, &intr_stat)) {
				g->ops.gsp.gsp_isr(g);
			}
		}

		if (timeoutms <= 0) {
			nvgpu_err(g, "gsp wait for init timedout");
			return -1;
		}

		nvgpu_msleep(10);
		timeoutms -= 10;
	} while (true);

	return 0;
}

int nvgpu_next_acr_wait_for_completion(struct gk20a *g,
				struct hs_acr *acr_desc, u32 timeout)
{
	u32 flcn_id;
	int completion = 0;
	u64 acr_sysmem_desc_addr = 0LL;
	u32 wpr_address_lo, wpr_address_hi = 0;

	nvgpu_acr_dbg(g, " ");

	flcn_id = nvgpu_falcon_get_id(acr_desc->acr_flcn);

	acr_sysmem_desc_addr = nvgpu_mem_get_addr(g,
				&g->acr->acr_asc.acr_falcon2_sysmem_desc);
	wpr_address_lo = u64_lo32(acr_sysmem_desc_addr);
	wpr_address_hi = u64_hi32(acr_sysmem_desc_addr);

	completion = nvgpu_acr_wait_for_init(g, timeout);
	if (completion != 0) {
		nvgpu_err(g, "flcn-%d: HS ucode boot timed out, limit: %d ms",
				flcn_id, timeout);
		completion = ACR_BOOT_TIMEDOUT;
		goto exit;
	}

	completion = nvgpu_acr_lock_wpr(g, wpr_address_lo, wpr_address_hi);
	if (completion != 0) {
		nvgpu_err(g, "flcn-%d: HS ucode boot init failed: %d",
				flcn_id, completion);
		goto exit;
	}

	g->acr->is_cold_boot_done = true;

	/*
	 * TODO: Add acr_engine_bus_err_status
	 * currently disabled as it gives false errors as
	 * PMU is not in a good state
	 */

exit:
	return completion;
}
#endif

/* TODO: Redundant, if no ACR specific suspend functionality */
void nvgpu_acr_suspend(struct gk20a *g)
{
	if (g->acr == NULL) {
		nvgpu_err(g, "ACR GSP not initialized");
		return;
	}
}

void nvgpu_acr_deinit(struct gk20a *g)
{
	g->is_acr_ipc_supported = false;
	if (g->acr != NULL) {
		nvgpu_kfree(g, g->acr);
		g->acr = NULL;
	}
}

#if defined(CONFIG_NVGPU_LS_PMU)
/* ACR public API's */
bool nvgpu_acr_is_lsf_lazy_bootstrap(struct gk20a *g, struct nvgpu_acr *acr,
	u32 falcon_id)
{
	if (acr == NULL) {
		return false;
	}

	if ((falcon_id == FALCON_ID_FECS) || (falcon_id == FALCON_ID_PMU) ||
		(falcon_id == FALCON_ID_GPCCS)) {
		return acr->lsf[falcon_id].is_lazy_bootstrap;
	} else {
		nvgpu_err(g, "Invalid falcon id\n");
		return false;
	}
}
#endif

#ifdef CONFIG_NVGPU_DGPU
int nvgpu_acr_alloc_blob_prerequisite(struct gk20a *g, struct nvgpu_acr *acr,
	size_t size)
{
	if (acr == NULL) {
		return -EINVAL;
	}

	return acr->alloc_blob_space(g, size, &acr->ucode_blob);
}
#endif

/* ACR blob construct & bootstrap */
int nvgpu_acr_bootstrap_hs_acr(struct gk20a *g, struct nvgpu_acr *acr)
{
	int err = 0;

	if (acr == NULL) {
		return -EINVAL;
	}

	err = acr->bootstrap_hs_acr(g, acr);
	if (err != 0) {
		nvgpu_err(g, "ACR bootstrap failed");
	}

	nvgpu_log(g, gpu_dbg_gr, "ACR bootstrap Done");
	return err;
}

int nvgpu_acr_construct_execute(struct gk20a *g)
{
	int err = 0;

	if (g->acr == NULL) {
		return -EINVAL;
	}

	err = g->acr->prepare_ucode_blob(g);
	if (err != 0) {
		nvgpu_err(g, "ACR ucode blob prepare failed");
		goto done;
	}

	err = nvgpu_acr_bootstrap_hs_acr(g, g->acr);
	if (err != 0) {
		nvgpu_err(g, "Bootstrap HS ACR failed");
	}

done:
	return err;
}

/* ACR init */
int nvgpu_acr_init(struct gk20a *g)
{
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch,
					g->params.gpu_impl);
	int err = 0;

	if (g->acr != NULL) {
		/*
		 * Recovery/unrailgate case, we do not need to do ACR init as ACR is
		 * set during cold boot & doesn't execute ACR clean up as part off
		 * sequence, so reuse to perform faster boot.
		 */
		return err;
	}

	g->acr = (struct nvgpu_acr *)nvgpu_kzalloc(g, sizeof(struct nvgpu_acr));
	if (g->acr == NULL) {
		err = -ENOMEM;
		goto deinit;
	}

	/*
	 * Firmware is stored in soc specific path in FMODEL
	 * Hence NVGPU_REQUEST_FIRMWARE_NO_WARN is used instead
	 * of NVGPU_REQUEST_FIRMWARE_NO_SOC
	 */
#ifdef CONFIG_NVGPU_SIM
	if (nvgpu_is_enabled(g, NVGPU_IS_FMODEL)) {
		g->acr->fw_load_flag = NVGPU_REQUEST_FIRMWARE_NO_WARN;
	} else
#endif
	{
		g->acr->fw_load_flag = NVGPU_REQUEST_FIRMWARE_NO_SOC;
	}

	switch (ver) {
#ifdef CONFIG_NVGPU_ACR_LEGACY
	case GK20A_GPUID_GM20B:
	case GK20A_GPUID_GM20B_B:
		nvgpu_gm20b_acr_sw_init(g, g->acr);
		break;
	case NVGPU_GPUID_GP10B:
		nvgpu_gp10b_acr_sw_init(g, g->acr);
		break;
#endif
#if defined(CONFIG_NVGPU_NON_FUSA)
	case NVGPU_GPUID_GV11B:
		nvgpu_gv11b_acr_sw_init(g, g->acr);
		break;
#endif
	case NVGPU_GPUID_GA10B:
		g->acr->is_lsf_encrypt_support = true;
		nvgpu_ga10b_acr_sw_init(g, g->acr);
		break;
#ifdef CONFIG_NVGPU_DGPU
	case NVGPU_GPUID_TU104:
		nvgpu_tu104_acr_sw_init(g, g->acr);
		break;
#if defined(CONFIG_NVGPU_NON_FUSA)
	case NVGPU_GPUID_GA100:
		nvgpu_ga100_acr_sw_init(g, g->acr);
		break;
#endif /* CONFIG_NVGPU_NON_FUSA */
#endif
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
	case NVGPU_GPUID_GB10B:
	case NVGPU_GPUID_GB10B_NEXT:
		/*
		 * Firmware is stored in soc specific path for T264
		 * Hence NVGPU_REQUEST_FIRMWARE_NO_WARN is used instead
		 * of NVGPU_REQUEST_FIRMWARE_NO_SOC
		 */
		g->acr->fw_load_flag = NVGPU_REQUEST_FIRMWARE_NO_WARN;
		nvgpu_gb10b_acr_sw_init(g, g->acr);
		break;
#endif
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB20C)
	case NVGPU_GPUID_GB20C:
		/*
		 * Firmware is stored in soc specific path for T256
		 * Hence NVGPU_REQUEST_FIRMWARE_NO_WARN is used instead
		 * of NVGPU_REQUEST_FIRMWARE_NO_SOC
		 */
		g->acr->fw_load_flag = NVGPU_REQUEST_FIRMWARE_NO_WARN;
		nvgpu_gb20c_acr_sw_init(g, g->acr);
		break;
#endif
	default:
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
		if (nvgpu_next_acr_init(g))
#endif
		{
			nvgpu_kfree(g, g->acr);
			err = -ENODEV;
			nvgpu_err(g, "no support for GPUID %x", ver);
		}
		break;
	}
	return err;

deinit:
	nvgpu_acr_deinit(g);
	return err;
}

int nvgpu_acr_process_message(struct gk20a *g)
{
	return nvgpu_gsp_process_message(g);
}
