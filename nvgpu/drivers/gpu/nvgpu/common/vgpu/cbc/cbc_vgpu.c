// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/vgpu/vgpu.h>

#include "cbc_vgpu.h"

int vgpu_cbc_alloc_comptags(struct gk20a *g, struct nvgpu_cbc *cbc)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	u32 max_comptag_lines = 0;
	int err;

	nvgpu_log_fn(g, " ");

	cbc->comptags_per_cacheline = priv->constants.comptags_per_cacheline;
	max_comptag_lines = priv->constants.comptag_lines;

	if (max_comptag_lines < 2) {
		return -ENXIO;
	}

	err = gk20a_comptag_allocator_init(g, &cbc->comp_tags, max_comptag_lines);
	if (err) {
		return err;
	}

	cbc->max_comptag_lines = max_comptag_lines;

	return 0;
}
