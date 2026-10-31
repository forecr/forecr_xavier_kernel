// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/device.h>
#include <nvgpu/engines.h>
#include <nvgpu/log.h>
#include <nvgpu/errno.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/static_analysis.h>

#include <nvgpu/hw/gp10b/hw_fifo_gp10b.h>

#include "engines_gp10b.h"

int gp10b_engine_init_ce_info(struct nvgpu_fifo *f)
{
	struct gk20a *g = f->g;
	bool found;
	const struct nvgpu_device *dev;

	nvgpu_device_for_each(g, dev, NVGPU_DEVTYPE_LCE) {
		struct nvgpu_device *dev_rw = (struct nvgpu_device *)dev;

		/*
		 * vGPU consideration. Not present in older chips. See
		 * nvgpu_engine_init_from_device_info() for more details in the
		 * comments.
		 */
		if (g->ops.fifo.find_pbdma_for_runlist != NULL) {
			found =	g->ops.fifo.find_pbdma_for_runlist(g,
							   dev->runlist_id,
							   &dev_rw->pbdma_id);
			if (!found) {
				nvgpu_err(g, "busted pbdma map");
				return -EINVAL;
			}
		}

#if defined(CONFIG_NVGPU_HAL_NON_FUSA)
		{
			/*
			 * Fill Ampere+ device fields.
			 */
			int err = nvgpu_engine_init_one_dev_extra(g, dev);
			if (err != 0) {
				return err;
			}
		}
#endif

		f->host_engines[dev->engine_id] = dev;
		f->active_engines[f->num_engines] = dev;
		f->num_engines = nvgpu_safe_add_u32(f->num_engines, 1U);
	}

	return 0;
}
