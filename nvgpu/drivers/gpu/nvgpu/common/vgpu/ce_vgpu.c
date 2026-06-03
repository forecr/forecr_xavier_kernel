// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>

#include <nvgpu/bug.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/vgpu/ce_vgpu.h>

u32 vgpu_ce_get_num_pce(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);

	return priv->constants.num_pce;
}
