// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/timers.h>
#include <nvgpu/pmu.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/bug.h>
#include <nvgpu/pmu/cmd.h>
#include <nvgpu/pmu/lsfm.h>

#include "lsfm_sw_gv100.h"
#include "lsfm_sw_tu104.h"

void nvgpu_tu104_lsfm_sw_init(struct gk20a *g, struct nvgpu_pmu_lsfm *lsfm)
{
	nvgpu_log_fn(g, " ");

	lsfm->is_wpr_init_done = false;

	/* LSF is not handled by PMU on this chip */
	lsfm->init_wpr_region = NULL;
	lsfm->bootstrap_ls_falcon = NULL;

	lsfm->ls_pmu_cmdline_args_copy = gv100_update_lspmu_cmdline_args_copy;
}
