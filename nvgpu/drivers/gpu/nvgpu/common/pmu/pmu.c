// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/enabled.h>
#include <nvgpu/bug.h>
#include <nvgpu/utils.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/nvgpu_err.h>
#if defined(CONFIG_NVGPU_DGPU) || defined(CONFIG_NVGPU_NEXT)
#include <nvgpu/boardobjgrp.h>
#endif
#include <nvgpu/pmu.h>

/* PMU H/W error functions */
void nvgpu_pmu_report_bar0_pri_err_status(struct gk20a *g, u32 bar0_status,
	u32 error_type)
{
	nvgpu_report_err_to_sdl(g, NVGPU_ERR_MODULE_PMU,
			GPU_PMU_BAR0_ERROR_TIMEOUT);
	nvgpu_err(g, "PMU falcon bar0 timeout. status(0x%x), "
			"error_type(0x%x)", bar0_status, error_type);
}

/**
 * @brief Enable/Disable PMU hardware
 *
 * @param pmu    [in] The PMU unit struct. This function does
 *                    not perform any validation of this input parameter.
 * @param enable [in] boolean parameter to enable/disable. This function does
 *                    not perform any validation of this input parameter.
 *
 * Assign g = pmu->g.
 * This function is used to enable the PMU by doing engine reset, configuring
 * PMU Engine clock gating and waiting for IMEM/DMEM memory scrubbing for reset
 * request. This function can also be used to disable PMU by keeping the engine
 * in reset state. The parameter \a enable will be used to decide the path.
 * + If parameter \a enable is true do the following
 *   + Reset PMU engine by calling \ref gops_pmu.reset_engine
 *     "g->ops.pmu.reset_engine(g, true)".
 *   + Call \ref nvgpu_cg_slcg_pmu_load_enable "nvgpu_cg_slcg_pmu_load_enable(g)"
 *   + Call \ref nvgpu_cg_blcg_pmu_load_enable "nvgpu_cg_blcg_pmu_load_enable(g)"
 *   + Call \ref nvgpu_falcon_mem_scrub_wait "nvgpu_falcon_mem_scrub_wait(pmu->flcn)".
 *     if return value is not equal to zero then keep the PMU engine in reset by
 *     calling \ref gops_pmu.reset_engine "g->ops.pmu.reset_engine(g, false))"
 *     and return -ETIMEDOUT.
 * + If parameter \a enable is false then keep the PMU engine in reset by calling
 *   \ref gops_pmu.reset_engine "g->ops.pmu.reset_engine(g, false))".
 *
 *@return 0 in case of success. <0 in case of failure.
 *@retval -ETIMEDOUT in case of memory scrub failure.
 */
static int pmu_enable_hw(struct nvgpu_pmu *pmu, bool enable)
{
	struct gk20a *g = pmu->g;
	int err = 0;

	nvgpu_log_fn(g, " %s ", g->name);

	if (enable) {
		/* bring PMU falcon/engine out of reset */
		g->ops.pmu.reset_engine(g, true);

		nvgpu_cg_slcg_pmu_load_enable(g);

		nvgpu_cg_blcg_pmu_load_enable(g);

		if (nvgpu_falcon_mem_scrub_wait(pmu->flcn) != 0) {
			/* keep PMU falcon/engine in reset
			 * if IMEM/DMEM scrubbing fails
			 */
			g->ops.pmu.reset_engine(g, false);
			nvgpu_err(g, "Falcon mem scrubbing timeout");
			err = -ETIMEDOUT;
		}
	} else {
		/* keep PMU falcon/engine in reset */
		g->ops.pmu.reset_engine(g, false);
	}

	nvgpu_log_fn(g, "%s Done, status - %d ", g->name, err);
	return err;
}

void nvgpu_pmu_enable_irq(struct gk20a *g, bool enable)
{
	if ((g->pmu != NULL) && (g->ops.pmu.pmu_enable_irq != NULL)) {
		nvgpu_mutex_acquire(&g->pmu->isr_mutex);
		g->ops.pmu.pmu_enable_irq(g->pmu, enable);
		g->pmu->isr_enabled = enable;
		nvgpu_mutex_release(&g->pmu->isr_mutex);
	}
}

/**
 * @brief Enable/Disable PMU
 *
 * @param pmu    [in] The PMU unit struct. This function does
 *                    not perform any validation of this input parameter.
 * @param enable [in] boolean parameter to enable/disable. This function does
 *                    not perform any validation of this input parameter.
 *
 * Assign g = pmu->g.
 * This function is used to enable/disable the PMU engine and configuring the
 * PMU ECC interrupts.
 * + If parameter \a enable is true do the following, in case of failure return error.
 *   + Call \ref pmu_enable_hw "pmu_enable_hw(pmu, true)".
 *   + Call \ref nvgpu_falcon_wait_idle "nvgpu_falcon_wait_idle(pmu->flcn)".
 *   + Call \ref nvgpu_pmu_enable_irq "nvgpu_pmu_enable_irq(g, true)".
 * + If parameter \a enable is false do the following, in case of failure return error.
 *   + Check if PMU engine is in reset by calling \ref gops_pmu.is_engine_in_reset()
 *     and proceed only if engine not in reset, else return.
 *   + Call \ref nvgpu_pmu_enable_irq "nvgpu_pmu_enable_irq(g, false)".
 *   + Call \ref pmu_enable_hw "pmu_enable_hw(pmu, false)".
 *
 *@return 0 in case of success, < 0 in case of failure.
 */
static int pmu_enable(struct nvgpu_pmu *pmu, bool enable)
{
	struct gk20a *g = pmu->g;
	int err = 0;

	nvgpu_log_fn(g, " ");

	if (!enable) {
		if (!g->ops.pmu.is_engine_in_reset(g)) {
			nvgpu_pmu_enable_irq(g, false);
			err = pmu_enable_hw(pmu, false);
			if (err != 0) {
				goto exit;
			}
		}
	} else {
		err = pmu_enable_hw(pmu, true);
		if (err != 0) {
			goto exit;
		}

		err = nvgpu_falcon_wait_idle(pmu->flcn);
		if (err != 0) {
			goto exit;
		}

#ifndef CONFIG_NVGPU_LS_PMU
		/* Enable PMU ECC interrupts for safety. */
		nvgpu_pmu_enable_irq(g, true);
#endif
	}

exit:
	nvgpu_log_fn(g, "Done, status - %d ", err);
	return err;
}

int nvgpu_pmu_reset(struct gk20a *g, u32 inst_id)
{
	struct nvgpu_pmu *pmu = g->pmu;
	int err = 0;

	nvgpu_log_fn(g, " %s ", g->name);

	nvgpu_assert(inst_id == 0U);

	err = pmu_enable(pmu, false);
	if (err != 0) {
		goto exit;
	}

	err = pmu_enable(pmu, true);
	if (err != 0) {
		goto exit;
	}

exit:
	nvgpu_log_fn(g, " %s Done, status - %d ", g->name, err);
	return err;
}

/* PMU unit deinit */
void nvgpu_pmu_remove_support(struct gk20a *g, struct nvgpu_pmu *pmu)
{
	if(pmu != NULL) {
#ifdef CONFIG_NVGPU_LS_PMU
		if (pmu->remove_support != NULL) {
			pmu->remove_support(g->pmu);
		}
#endif
		nvgpu_mutex_destroy(&pmu->isr_mutex);
		nvgpu_mutex_destroy(&pmu->ptcb_queue_mutex);
		if (g->ops.pmu.ecc_free != NULL) {
			g->ops.pmu.ecc_free(g);
		}
		nvgpu_kfree(g, g->pmu);
		g->pmu = NULL;
	}
}

static void nvgpu_pmu_disable_features(struct gk20a *g)
{
		g->support_ls_pmu = false;

		/* Disable LS PMU global checkers */
#ifdef CONFIG_NVGPU_NON_FUSA
		g->can_elpg = false;
		g->elpg_enabled = false;
		g->aelpg_enabled = false;
		g->elpg_ms_enabled = false;
#endif
		nvgpu_set_enabled(g, NVGPU_PMU_PERFMON, false);
		nvgpu_set_enabled(g, NVGPU_ELPG_MS_ENABLED, false);
#if defined(CONFIG_NVGPU_DGPU) || defined(CONFIG_NVGPU_NEXT)
		nvgpu_set_enabled(g, NVGPU_PMU_PSTATE, false);
#endif
}

/* PMU unit init */
int nvgpu_pmu_early_init(struct gk20a *g)
{
	int err = 0;
	struct nvgpu_pmu *pmu;

	nvgpu_log_fn(g, " ");

	if (g->ops.pmu.is_pmu_supported == NULL) {
		nvgpu_pmu_disable_features(g);
		goto exit;
	}

	if (g->pmu != NULL) {
		/* skip alloc/reinit for unrailgate sequence */
		nvgpu_pmu_dbg(g, "skip pmu init for unrailgate sequence");
		goto exit;
	}

	pmu = (struct nvgpu_pmu *) nvgpu_kzalloc(g, sizeof(struct nvgpu_pmu));
	if (pmu == NULL) {
		err = -ENOMEM;
		goto exit;
	}

	g->pmu = pmu;
	pmu->g = g;
	pmu->flcn = &g->pmu_flcn;

#if defined(CONFIG_NVGPU_NON_FUSA)
	if (nvgpu_falcon_is_falcon2_enabled(&g->pmu_flcn)) {
		nvgpu_set_enabled(g, NVGPU_PMU_NEXT_CORE_ENABLED, true);
	}
#endif

	if ((g->ops.pmu.ecc_init != NULL) && !g->ecc.initialized) {
		err = g->ops.pmu.ecc_init(g);
		if (err != 0) {
			nvgpu_kfree(g, pmu);
			g->pmu = NULL;
			goto exit;
		}
	}

	nvgpu_mutex_init(&pmu->isr_mutex);
	nvgpu_mutex_init(&pmu->ptcb_queue_mutex);

	if (!g->ops.pmu.is_pmu_supported(g)) {
		nvgpu_pmu_disable_features(g);
		goto exit;
	}

#ifdef CONFIG_NVGPU_LS_PMU
	err = nvgpu_pmu_rtos_early_init(g, pmu);
	if (err != 0) {
		nvgpu_mutex_destroy(&pmu->isr_mutex);
		nvgpu_mutex_destroy(&pmu->ptcb_queue_mutex);
		if (g->ops.pmu.ecc_free != NULL) {
			g->ops.pmu.ecc_free(g);
		}
		nvgpu_kfree(g, pmu);
		g->pmu = NULL;
		goto exit;
	}
#endif

exit:
	return err;
}
