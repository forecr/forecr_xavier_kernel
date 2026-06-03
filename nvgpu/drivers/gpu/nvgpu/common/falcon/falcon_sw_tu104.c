// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/gk20a.h>
#include <nvgpu/falcon.h>

#include "falcon_sw_gk20a.h"
#include "falcon_sw_tu104.h"

void tu104_falcon_engine_dependency_ops(struct nvgpu_falcon *flcn)
{
	struct nvgpu_falcon_engine_dependency_ops *flcn_eng_dep_ops =
			&flcn->flcn_engine_dep_ops;
	struct gk20a *g = flcn->g;

	gk20a_falcon_engine_dependency_ops(flcn);

	switch (flcn->flcn_id) {
	case FALCON_ID_GSPLITE:
		flcn_eng_dep_ops->reset_eng = g->ops.gsp.gsp_reset;
		flcn_eng_dep_ops->setup_bootstrap_config =
			g->ops.gsp.falcon_setup_boot_config;
		break;
	case FALCON_ID_SEC2:
		flcn_eng_dep_ops->reset_eng = g->ops.sec2.sec2_reset;
		flcn_eng_dep_ops->setup_bootstrap_config =
			g->ops.sec2.flcn_setup_boot_config;
		flcn_eng_dep_ops->copy_to_emem = g->ops.sec2.sec2_copy_to_emem;
		flcn_eng_dep_ops->copy_from_emem =
						g->ops.sec2.sec2_copy_from_emem;
		break;
	default:
		flcn_eng_dep_ops->reset_eng = NULL;
		break;
	}
}

void tu104_falcon_sw_init(struct nvgpu_falcon *flcn)
{
	struct gk20a *g = flcn->g;

	switch (flcn->flcn_id) {
	case FALCON_ID_GSPLITE:
		flcn->flcn_base = g->ops.gsp.falcon_base_addr();
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = false;
		break;
	case FALCON_ID_SEC2:
		flcn->flcn_base = g->ops.sec2.falcon_base_addr();
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = true;
		flcn->emem_supported = true;
		break;
	case FALCON_ID_MINION:
		flcn->flcn_base = g->ops.nvlink.minion.base_addr(g);
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = true;
		break;
	case FALCON_ID_NVDEC:
		flcn->flcn_base = g->ops.nvdec.base_addr(0);
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = true;
		break;
	case FALCON_ID_NVENC:
		flcn->flcn_base = g->ops.nvenc.base_addr(0);
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = true;
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
		tu104_falcon_engine_dependency_ops(flcn);
	} else {
		/*
		 * Forward call to previous chip's SW init
		 * to fetch info for requested
		 * falcon as no changes between
		 * current & previous chips.
		 */
		gk20a_falcon_sw_init(flcn);
	}
}
