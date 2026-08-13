// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/kmem.h>
#include <nvgpu/device.h>
#include <nvgpu/engines.h>

#include <nvgpu/vgpu/vgpu.h>

#include "top_vgpu.h"

/*
 * Similar to how the real HW version works, just read a device out of the vGPU
 * device list one at a time. The core device management code will manage the
 * actual device lists for us.
 */
struct nvgpu_device *vgpu_top_parse_next_dev(struct gk20a *g, u32 *token)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	struct tegra_vgpu_engines_info *engines = &priv->constants.engines_info;
	struct nvgpu_device *dev;

	/*
	 * Check to see if we are done parsing engines.
	 */
	if (*token >= engines->num_engines) {
		return NULL;
	}

	dev = nvgpu_kzalloc(g, sizeof(*dev));
	if (!dev) {
		return NULL;
	}

	/*
	 * Copy the engine data into the device and return it to our caller.
	 */
	dev->type       = engines->info[*token].engine_enum;
	dev->engine_id  = engines->info[*token].engine_id;
	dev->intr_id    = nvgpu_ffs(engines->info[*token].intr_mask) - 1;
	dev->reset_id   = nvgpu_ffs(engines->info[*token].reset_mask) - 1;
	dev->runlist_id = engines->info[*token].runlist_id;
	dev->rleng_id   = engines->info[*token].rleng_id;
	dev->pbdma_id   = engines->info[*token].pbdma_id;
	dev->inst_id    = engines->info[*token].inst_id;
	dev->pri_base   = engines->info[*token].pri_base;
	dev->fault_id   = engines->info[*token].fault_id;

	(*token)++;

	return dev;
}

u32 vgpu_top_get_max_rop_per_gpc(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);

	return priv->constants.max_rop_per_gpc;
}
