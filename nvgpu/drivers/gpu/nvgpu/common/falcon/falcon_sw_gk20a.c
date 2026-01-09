// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/gk20a.h>
#include <nvgpu/falcon.h>

#include "falcon_sw_gk20a.h"

void gk20a_falcon_engine_dependency_ops(struct nvgpu_falcon *flcn)
{
	struct gk20a *g = flcn->g;
	struct nvgpu_falcon_engine_dependency_ops *flcn_eng_dep_ops =
			&flcn->flcn_engine_dep_ops;

	switch (flcn->flcn_id) {
	case FALCON_ID_PMU:
		flcn_eng_dep_ops->reset_eng = g->ops.pmu.pmu_reset;
		flcn_eng_dep_ops->setup_bootstrap_config =
			g->ops.pmu.flcn_setup_boot_config;
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

void gk20a_falcon_sw_init(struct nvgpu_falcon *flcn)
{
	struct gk20a *g = flcn->g;

	switch (flcn->flcn_id) {
	case FALCON_ID_PMU:
		flcn->flcn_base = g->ops.pmu.falcon_base_addr();
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = true;
		break;
	case FALCON_ID_FECS:
		flcn->flcn_base = g->ops.gr.falcon.fecs_base_addr();
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = false;
		break;
	case FALCON_ID_GPCCS:
		flcn->flcn_base = g->ops.gr.falcon.gpccs_base_addr();
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = false;
		break;
	default:
		flcn->is_falcon_supported = false;
		break;
	}

	if (flcn->is_falcon_supported) {
		gk20a_falcon_engine_dependency_ops(flcn);
	} else {
		nvgpu_log_info(g, "falcon 0x%x not supported on %s",
			flcn->flcn_id, g->name);
	}
}
