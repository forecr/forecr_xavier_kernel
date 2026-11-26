// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/fuse.h>
#include <nvgpu/string.h>
#include <nvgpu/gr/gr_ecc.h>
#include <hal/gr/ecc/ecc_gv11b.h>

#include "ecc_ga10b.h"

static void ga10b_ecc_enable_sm_lrf(struct gk20a *g,
	struct nvgpu_fuse_feature_override_ecc *ecc_feature, bool opt_ecc_en)
{
	if (ecc_feature->sm_lrf_override) {
		if (ecc_feature->sm_lrf_enable) {
			nvgpu_set_enabled(g, NVGPU_ECC_ENABLED_SM_LRF, true);
		}
	} else {
		if (opt_ecc_en) {
			nvgpu_set_enabled(g, NVGPU_ECC_ENABLED_SM_LRF, true);
		}
	}
}

static void ga10b_ecc_enable_sm_l1_data(struct gk20a *g,
	struct nvgpu_fuse_feature_override_ecc *ecc_feature, bool opt_ecc_en)
{
	if (ecc_feature->sm_l1_data_override) {
		if (ecc_feature->sm_l1_data_enable) {
		  nvgpu_set_enabled(g, NVGPU_ECC_ENABLED_SM_L1_DATA, true);
		}
	} else {
		if (opt_ecc_en) {
		  nvgpu_set_enabled(g, NVGPU_ECC_ENABLED_SM_L1_DATA, true);
		}
	}
}

static void ga10b_ecc_enable_sm_l1_tag(struct gk20a *g,
	struct nvgpu_fuse_feature_override_ecc *ecc_feature, bool opt_ecc_en)
{
	if (ecc_feature->sm_l1_tag_override) {
		if (ecc_feature->sm_l1_tag_enable) {
		       nvgpu_set_enabled(g, NVGPU_ECC_ENABLED_SM_L1_TAG, true);
		}
	} else {
		if (opt_ecc_en) {
		       nvgpu_set_enabled(g, NVGPU_ECC_ENABLED_SM_L1_TAG, true);
		}
	}
}

static void ga10b_ecc_enable_sm_icache(struct gk20a *g,
	struct nvgpu_fuse_feature_override_ecc *ecc_feature, bool opt_ecc_en)
{
	if (ecc_feature->sm_l0_icache_override &&
	    ecc_feature->sm_l1_icache_override) {
		if (ecc_feature->sm_l0_icache_enable &&
		    ecc_feature->sm_l1_icache_enable) {
		       nvgpu_set_enabled(g, NVGPU_ECC_ENABLED_SM_ICACHE, true);
		}
	} else {
		if (opt_ecc_en) {
		       nvgpu_set_enabled(g, NVGPU_ECC_ENABLED_SM_ICACHE, true);
		}
	}
}

static void ga10b_ecc_enable_ltc(struct gk20a *g,
	struct nvgpu_fuse_feature_override_ecc *ecc_feature, bool opt_ecc_en)
{
	if (ecc_feature->ltc_override) {
		if (ecc_feature->ltc_enable) {
			nvgpu_set_enabled(g, NVGPU_ECC_ENABLED_LTC, true);
		}
	} else {
		if (opt_ecc_en) {
			nvgpu_set_enabled(g, NVGPU_ECC_ENABLED_LTC, true);
		}
	}
}

static void ga10b_ecc_enable_sm_cbu(struct gk20a *g,
	struct nvgpu_fuse_feature_override_ecc *ecc_feature, bool opt_ecc_en)
{
	if (ecc_feature->sm_cbu_override) {
		if (ecc_feature->sm_cbu_enable) {
			nvgpu_set_enabled(g, NVGPU_ECC_ENABLED_SM_CBU, true);
		}
	} else {
		if (opt_ecc_en) {
			nvgpu_set_enabled(g, NVGPU_ECC_ENABLED_SM_CBU, true);
		}
	}
}

void ga10b_ecc_detect_enabled_units(struct gk20a *g)
{
	struct nvgpu_fuse_feature_override_ecc ecc_feature;
	bool opt_ecc_en = g->ops.fuse.is_opt_ecc_enable(g);
	bool opt_feature_fuses_override_disable =
		g->ops.fuse.is_opt_feature_override_disable(g);

	(void) memset(&ecc_feature, 0,
			sizeof(struct nvgpu_fuse_feature_override_ecc));
	g->ops.fuse.read_feature_override_ecc(g, &ecc_feature);

	if (opt_feature_fuses_override_disable) {
		if (opt_ecc_en) {
			nvgpu_set_enabled(g,
					NVGPU_ECC_ENABLED_SM_LRF, true);
			nvgpu_set_enabled(g,
					NVGPU_ECC_ENABLED_SM_L1_DATA, true);
			nvgpu_set_enabled(g,
					NVGPU_ECC_ENABLED_SM_L1_TAG, true);
			nvgpu_set_enabled(g,
					NVGPU_ECC_ENABLED_SM_ICACHE, true);
			nvgpu_set_enabled(g, NVGPU_ECC_ENABLED_LTC, true);
			nvgpu_set_enabled(g, NVGPU_ECC_ENABLED_SM_CBU, true);
		}
	} else {
		/* SM LRF */
		ga10b_ecc_enable_sm_lrf(g, &ecc_feature, opt_ecc_en);
		/* SM L1 DATA*/
		ga10b_ecc_enable_sm_l1_data(g, &ecc_feature, opt_ecc_en);
		/* SM L1 TAG*/
		ga10b_ecc_enable_sm_l1_tag(g, &ecc_feature, opt_ecc_en);
		/* SM ICACHE*/
		ga10b_ecc_enable_sm_icache(g, &ecc_feature, opt_ecc_en);
		/* LTC */
		ga10b_ecc_enable_ltc(g, &ecc_feature, opt_ecc_en);
		/* SM CBU */
		ga10b_ecc_enable_sm_cbu(g, &ecc_feature, opt_ecc_en);
		/*
		 * Further feature override checking tracked by
		 * https:jirasw.nvidia.com/browse/NVGPU-5284
		 */
	}
}

static int ga10b_gr_gpc_tpc_ecc_init_sm_rams(struct gk20a *g)
{
	NVGPU_ECC_COUNTER_INIT_PER_TPC_OR_RETURN(sm_rams_ecc_corrected_err_count);
	NVGPU_ECC_COUNTER_INIT_PER_TPC_OR_RETURN(sm_rams_ecc_uncorrected_err_count);

	return 0;
}

int ga10b_gr_gpc_tpc_ecc_init(struct gk20a *g)
{
	int err;

	err = gv11b_gr_gpc_tpc_ecc_init(g);
	if (err != 0) {
		goto done;
	}

	err = ga10b_gr_gpc_tpc_ecc_init_sm_rams(g);
	if (err != 0) {
		goto done;
	}

done:
	if (err != 0) {
		nvgpu_err(g, "ecc counter allocate failed, err=%d", err);
		ga10b_gr_gpc_tpc_ecc_deinit(g);
	}

	return err;
}

void ga10b_gr_gpc_tpc_ecc_deinit(struct gk20a *g)
{
	gv11b_gr_gpc_tpc_ecc_deinit(g);
	NVGPU_ECC_COUNTER_DEINIT_PER_TPC(sm_rams_ecc_corrected_err_count);
	NVGPU_ECC_COUNTER_DEINIT_PER_TPC(sm_rams_ecc_uncorrected_err_count);
}
