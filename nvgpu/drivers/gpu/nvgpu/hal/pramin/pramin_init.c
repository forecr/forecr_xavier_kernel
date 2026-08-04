// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>

#include "pramin_init.h"
#include "pramin_gp10b.h"
#ifdef CONFIG_NVGPU_DGPU
#include "pramin_gv100.h"
#include "pramin_tu104.h"
#endif

void nvgpu_pramin_ops_init(struct gk20a *g)
{
	u32 ver = g->params.gpu_arch + g->params.gpu_impl;

	switch (ver) {
	case NVGPU_GPUID_GP10B:
		g->ops.pramin.data032_r = gp10b_pramin_data032_r;
		break;
#ifdef CONFIG_NVGPU_DGPU
	case NVGPU_GPUID_GV100:
		g->ops.pramin.data032_r = gv100_pramin_data032_r;
		break;
	case NVGPU_GPUID_TU104:
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	case NVGPU_GPUID_GA100:
#endif
		g->ops.pramin.data032_r = tu104_pramin_data032_r;
		break;
#endif
	default:
		g->ops.pramin.data032_r = NULL;
		break;
	}
}
