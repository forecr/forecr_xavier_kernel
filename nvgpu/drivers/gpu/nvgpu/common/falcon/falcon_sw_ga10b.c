// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/gk20a.h>
#include <nvgpu/falcon.h>
#include <nvgpu/gops/gsp.h>

#include "common/falcon/falcon_sw_gk20a.h"
#include "common/falcon/falcon_sw_ga10b.h"

static void check_and_enable_falcon2(struct nvgpu_falcon *flcn,
		unsigned long *fuse_settings)
{
	struct gk20a *g = flcn->g;
	bool is_falcon_enabled = false;
	bool is_falcon2_enabled = false;
	int err = 0;
	bool is_falcon_DCS_supported = false;

	nvgpu_falcon_dbg(g, "Fetch FUSE settings for FALCON - %d *", flcn->flcn_id);

	err = g->ops.fuse.fetch_falcon_fuse_settings(g, flcn->flcn_id,
			fuse_settings);
	if (err != 0) {
		nvgpu_err(g, "Failed to fetch fuse settings for Falcon %d",
				flcn->flcn_id);
		/*  setting default to FALCON until bring-up */
		nvgpu_err(g, " setting default to Falcon");
		flcn->is_falcon2_enabled = false;
		return;
	}

	nvgpu_falcon_dbg(g, "fuse_settings -  %lx", *fuse_settings);

	is_falcon_DCS_supported = nvgpu_falcon_is_feature_supported(flcn, DCS);

	is_falcon_enabled =
		(!(nvgpu_falcon_is_feature_supported(flcn, FCD)) &&
		(!is_falcon_DCS_supported));

	is_falcon2_enabled = !is_falcon_enabled &&
		is_falcon_DCS_supported;

	/* select the FALCON/RISCV core based on fuse */
	if (!is_falcon_enabled && !is_falcon2_enabled) {
		nvgpu_err(g, "Invalid fuse combination, both core disabled");
		nvgpu_err(g, "Further execution will try on FALCON core");
		flcn->is_falcon2_enabled = false;
	} NVGPU_COV_WHITELIST(deviate, NVGPU_MISRA(Rule, 14_3), "SWE-NVGPU-052-SWSADR.docx")
		else if (is_falcon_enabled && !is_falcon2_enabled) {
		nvgpu_falcon_dbg(g, "FALCON is enabled");
		/* FALCON is enabled*/
		flcn->is_falcon2_enabled = false;
	} else {
		nvgpu_falcon_dbg(g, "FALCON/RISCV can be enabled, default RISCV is enabled");
		flcn->is_falcon2_enabled = true;
	}

	if (flcn->is_falcon2_enabled) {
		if (nvgpu_falcon_is_feature_supported(flcn,
				NVRISCV_BRE_EN)) {
			nvgpu_falcon_dbg(g, "BRE info enabled");
		} else {
			nvgpu_falcon_dbg(g, "BRE info not enabled");
		}

		if (nvgpu_falcon_is_feature_supported(flcn, NVRISCV_DEVD)) {
			nvgpu_falcon_dbg(g, "DevD");
		} else {
			nvgpu_falcon_dbg(g, "DevE");
		}

		if (nvgpu_falcon_is_feature_supported(flcn,
			NVRISCV_PLD)) {
			nvgpu_falcon_dbg(g, "PL request disabled");
		} else {
			nvgpu_falcon_dbg(g, "PL request enabled");
		}

		if (nvgpu_falcon_is_feature_supported(flcn, NVRISCV_SEN)) {
			nvgpu_falcon_dbg(g, "S enabled");

			if (nvgpu_falcon_is_feature_supported(flcn,
				NVRISCV_SA)) {
				nvgpu_falcon_dbg(g, "assert enabled");
			} else {
				nvgpu_falcon_dbg(g, "assert disabled");
			}

			if (nvgpu_falcon_is_feature_supported(flcn,
				NVRISCV_SH)) {
				nvgpu_falcon_dbg(g, "HALT enabled");
			} else {
				nvgpu_falcon_dbg(g, "HALT disabled");
			}

			if (nvgpu_falcon_is_feature_supported(flcn,
				NVRISCV_SI)) {
				nvgpu_falcon_dbg(g, "interrupt enabled");
			} else {
				nvgpu_falcon_dbg(g, "interrupt disabled");
			}
		} else {
			nvgpu_falcon_dbg(g, "S not enabled");
		}
	}
}

/**
 * @brief Configures engine-specific operations for the Falcon microcontroller.
 *
 * This function sets up the engine dependency operations for the Falcon microcontroller
 * based on its identifier. It assigns function pointers for reset, bootstrap configuration,
 * and EMEM copy operations specific to the engine that the Falcon is associated with.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the pointer to the GPU driver struct from the Falcon struct.
 * -# Retrieve the pointer to the Falcon engine dependency operations struct from the Falcon struct.
 * -# Based on the Falcon identifier:
 *    -# If the identifier is for the PMU (Power Management Unit), call the function
 *       gk20a_falcon_engine_dependency_ops() to set up the PMU-specific operations.
 *    -# If the identifier is for the GSPLITE (Graphics Scheduler Processor Lite), set the
 *       reset engine operation to the GSPLITE-specific reset function. If the GSP scheduler
 *       is configured (CONFIG_NVGPU_GSP_SCHEDULER), also set up the bootstrap configuration,
 *       copy to EMEM, and copy from EMEM operations to the GSPLITE-specific functions.
 *    -# For all other Falcon identifiers, set the reset engine operation to NULL to ensure
 *       that a CPU hard reset is executed if no specific reset implementation is needed.
 *
 * @param [in] flcn  The Falcon struct associated with the engine.
 */
static void ga10b_falcon_engine_dependency_ops(struct nvgpu_falcon *flcn)
{
	struct gk20a *g = flcn->g;
	struct nvgpu_falcon_engine_dependency_ops *flcn_eng_dep_ops =
			&flcn->flcn_engine_dep_ops;

	switch (flcn->flcn_id) {
	case FALCON_ID_PMU:
		gk20a_falcon_engine_dependency_ops(flcn);
		break;
	case FALCON_ID_GSPLITE:
		flcn_eng_dep_ops->reset_eng = g->ops.gsp.gsp_reset;
#ifdef CONFIG_NVGPU_GSP_SCHEDULER
		flcn_eng_dep_ops->copy_to_emem = g->ops.gsp.gsp_copy_to_emem;
		flcn_eng_dep_ops->copy_from_emem =
						g->ops.gsp.gsp_copy_from_emem;
#endif
		break;
	default:
		/* NULL assignment make sure
		 * CPU hard reset in gk20a_falcon_reset() gets execute
		 * if falcon doesn't need specific reset implementation
		 */
		flcn_eng_dep_ops->reset_eng = NULL;
		break;
	}
}

extern void ga10b_falcon_sw_init(struct nvgpu_falcon *flcn)
{
	struct gk20a *g = flcn->g;

	switch (flcn->flcn_id) {
	case FALCON_ID_PMU:
		flcn->flcn_base = g->ops.pmu.falcon_base_addr();
		flcn->flcn2_base = g->ops.pmu.falcon2_base_addr();
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = true;

		check_and_enable_falcon2(flcn, &flcn->fuse_settings);
		break;
	case FALCON_ID_GSPLITE:
		flcn->flcn_base = g->ops.gsp.falcon_base_addr();
		flcn->flcn2_base = g->ops.gsp.falcon2_base_addr();
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = true;
		flcn->emem_supported = true;

		check_and_enable_falcon2(flcn, &flcn->fuse_settings);
		break;
	default:
		/*
		 * set false to inherit falcon support
		 * from previous chips HAL
		 */
		flcn->is_falcon_supported = false;
		break;
	}

	if (flcn->is_falcon_supported) {
		ga10b_falcon_engine_dependency_ops(flcn);
	} else {
		gk20a_falcon_sw_init(flcn);
	}

}
