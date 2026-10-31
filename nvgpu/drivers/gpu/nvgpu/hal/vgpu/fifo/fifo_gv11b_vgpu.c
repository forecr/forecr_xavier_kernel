// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/vgpu/vgpu.h>

#include "fifo_gv11b_vgpu.h"

int vgpu_gv11b_init_fifo_setup_hw(struct gk20a *g)
{
	struct nvgpu_fifo *f = &g->fifo;
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);

	f->max_subctx_count = priv->constants.max_subctx_count;

	return 0;
}
