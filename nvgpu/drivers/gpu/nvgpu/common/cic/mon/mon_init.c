// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/kmem.h>
#include <nvgpu/log.h>
#include <nvgpu/cic_mon.h>
#include "cic_mon_priv.h"

int nvgpu_cic_mon_setup(struct gk20a *g)
{
	struct nvgpu_cic_mon *cic_mon;
	int err = 0;

	if (g->cic_mon != NULL) {
		cic_dbg(g, "CIC_MON already initialized");
		return 0;
	}

	cic_mon = nvgpu_kzalloc(g, sizeof(*cic_mon));
	if (cic_mon == NULL) {
		nvgpu_err(g, "Failed to allocate memory "
				"for struct nvgpu_cic_mon");
		return -ENOMEM;
	}

	g->cic_mon = cic_mon;

	cic_dbg(g, "CIC_MON unit initialization done.");
	return err;
}

int nvgpu_cic_mon_init(struct gk20a *g)
{
	struct nvgpu_cic_mon *cic_mon;
	int err = 0;

	cic_mon = g->cic_mon;
	if (cic_mon == NULL) {
		nvgpu_err(g, "CIC_MON setup pending");
		return -EINVAL;
	}

#ifdef CONFIG_NVGPU_EPL_INIT
	err = nvgpu_cic_mon_init_epl(g);
	if (err != 0) {
		nvgpu_err(g, "NvEpl init failed: %d", err);
		return -EINVAL;
	}
#endif

#ifdef CONFIG_NVGPU_FSI_ERR_INJECTION
	if (g->ops.cic_mon.reg_errinj_cb != NULL) {
		err = g->ops.cic_mon.reg_errinj_cb(g);
		if (err != 0) {
			nvgpu_err(g,
				"Err inj callback registration failed: %d",
				err);
			/* Continue CIC init despite err inj utility
			 * registration failure, as the err inj support
			 * is meant only for debug purposes.
			 */
			err = 0;
		}
	}
#endif

	if (g->ops.cic_mon.init != NULL) {
		err = g->ops.cic_mon.init(g, cic_mon);
		if (err != 0) {
			nvgpu_err(g, "CIC MON chip specific "
					"initialization failed.");
			goto cleanup;
		}
	} else {
		cic_mon->err_lut = NULL;
		cic_mon->num_hw_modules = 0;
	}

	return 0;
cleanup:
	NVGPU_COV_WHITELIST(deviate, NVGPU_MISRA(Rule, 14_3), "SWE-NVGPU-052-SWSADR.docx")
	if (cic_mon != NULL) {
		nvgpu_kfree(g, cic_mon);
		g->cic_mon = NULL;
	}
	return err;
}

int nvgpu_cic_mon_remove(struct gk20a *g)
{
	struct nvgpu_cic_mon *cic_mon;

	cic_mon = g->cic_mon;

	if (cic_mon == NULL) {
		cic_dbg(g, "CIC_MON already removed");
		return 0;
	}

	nvgpu_kfree(g, cic_mon);
	g->cic_mon = NULL;

	return 0;
}

int nvgpu_cic_mon_deinit_lut(struct gk20a *g)
{
	struct nvgpu_cic_mon *cic_mon;

	cic_mon = g->cic_mon;

	if (cic_mon == NULL) {
		cic_dbg(g, "CIC_MON and LUT removed");
		return 0;
	}

	cic_mon->err_lut = NULL;
	cic_mon->num_hw_modules = 0;

	return 0;
}

int nvgpu_cic_mon_deinit(struct gk20a *g)
{
#ifdef CONFIG_NVGPU_FSI_ERR_INJECTION
	if (g->ops.cic_mon.dereg_errinj_cb != NULL) {
		int err = g->ops.cic_mon.dereg_errinj_cb();
		if (err != 0) {
			nvgpu_err(g,
				"Err inj callback de-registration failed: %d",
				err);
			/* Continue CIC mon deinit despite err inj utility
			 * de-registration failure, as the err inj support
			 * is meant only for debug purposes.
			 */
		}
	}
#endif

	/** More deinit calls might get added here as CIC grows. */
	return nvgpu_cic_mon_deinit_lut(g);
}
