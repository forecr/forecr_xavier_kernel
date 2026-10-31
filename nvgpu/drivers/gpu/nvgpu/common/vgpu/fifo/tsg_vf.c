// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/tsg.h>
#include <nvgpu/bug.h>

#include <nvgpu/vgpu/tegra_vgpu.h>

#include "tsg_vf.h"

void vf_tsg_set_unserviceable(struct gk20a *g,
				struct tegra_vf_tsg_set_unserviceable *p)
{
	if (p->tsgid >= g->fifo.num_channels) {
		nvgpu_err(g, "invalid tsgid %u", p->tsgid);
		return;
	}

	nvgpu_tsg_set_unserviceable(g, 0U, p->tsgid);
}
