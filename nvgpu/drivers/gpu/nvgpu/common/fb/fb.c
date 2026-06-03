// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/fb.h>

int nvgpu_init_fb_support(struct gk20a *g)
{
	if (g->ops.mc.fb_reset != NULL) {
		g->ops.mc.fb_reset(g);
	}

	nvgpu_cg_slcg_fb_load_enable(g);

	nvgpu_cg_blcg_fb_load_enable(g);

	if (g->ops.fb.init_fs_state != NULL) {
		g->ops.fb.init_fs_state(g);
	}
	return 0;
}

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_HAL_NON_FUSA)
int nvgpu_fb_vab_init_hal(struct gk20a *g)
{
	int err = 0;

	if (g->ops.fb.vab.init != NULL) {
		err = g->ops.fb.vab.init(g);
	}
	return err;
}

int nvgpu_fb_vab_teardown_hal(struct gk20a *g)
{
	int err = 0;

	if (g->ops.fb.vab.teardown != NULL)  {
		err = g->ops.fb.vab.teardown(g);
	}
	return err;
}
#endif
