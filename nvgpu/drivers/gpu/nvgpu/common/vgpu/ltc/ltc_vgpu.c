// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/vgpu/vgpu.h>

#include "ltc_vgpu.h"

u64 vgpu_determine_L2_size_bytes(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);

	nvgpu_log_fn(g, " ");

	return priv->constants.l2_size;
}

void vgpu_ltc_init_fs_state(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	struct nvgpu_ltc *ltc = g->ltc;

	nvgpu_log_fn(g, " ");

	ltc->ltc_count = priv->constants.ltc_count;
	ltc->cacheline_size = priv->constants.cacheline_size;
	ltc->slices_per_ltc = priv->constants.slices_per_ltc;
}

#ifdef CONFIG_NVGPU_DEBUGGER

#endif
