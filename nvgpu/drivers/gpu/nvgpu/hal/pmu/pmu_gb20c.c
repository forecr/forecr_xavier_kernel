// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/pmu.h>
#include <nvgpu/io.h>
#include <nvgpu/mm.h>
#include <nvgpu/firmware.h>
#include <nvgpu/pmu/debug.h>
#include <nvgpu/pmu/pmu_pg.h>
#include <nvgpu/soc.h>
#include <nvgpu/device.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/pmu/pmuif/pg.h>
#include <nvgpu/timers.h>

#include "pmu_gb20c.h"

#include <nvgpu/hw/gb20c/hw_pwr_gb20c.h>

int gb20c_pmu_elcg_sub_feature_id_to_engine_id(struct gk20a *g,
								u32 sub_feature_id)
{
	int engine_id = -1;

	nvgpu_log_fn(g, " ");

	switch (sub_feature_id) {

	case CG_SUBFEATURE_ID_ELCG_CTRL_GR:
		engine_id = CG_ENGINE_ID_ELCG_GRAPHICS0_GB20C;
		break;

	case CG_SUBFEATURE_ID_ELCG_CTRL_NVENC0:
		engine_id = CG_ENGINE_ID_ELCG_NVENC0_GB20C;
		break;

	case CG_SUBFEATURE_ID_ELCG_CTRL_NVDEC0:
		engine_id = CG_ENGINE_ID_ELCG_NVDEC0_GB20C;
		break;

	case CG_SUBFEATURE_ID_ELCG_CTRL_OFA0:
		engine_id = CG_ENGINE_ID_ELCG_OFA0_GB20C;
		break;

	default:
		nvgpu_err(g, "Invalid Sub Feature ID %#X", sub_feature_id);
		break;
	}

	return engine_id;
}

u32 gb20c_pmu_elcg_all_engine_mask(void)
{
	return CG_ELCG_ALL_ENGINE_ENABLED_MASK_GB20C;
}

u32 gb20c_pmu_elcg_mm_engine_mask(void)
{
	return CG_ELCG_MM_ENGINE_ENABLED_MASK_GB20C;
}

u32 gb20c_pmu_fgpg_get_all_engine_mask(void)
{
	return PG_FGPG_ALL_ENGINE_ENABLED_BITMASK_GB20C;
}
