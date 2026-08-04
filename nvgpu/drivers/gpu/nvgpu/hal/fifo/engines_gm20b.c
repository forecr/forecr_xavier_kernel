// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/device.h>
#include <nvgpu/engines.h>
#include <nvgpu/log.h>
#include <nvgpu/errno.h>
#include <nvgpu/gk20a.h>

#include <nvgpu/hw/gm20b/hw_fifo_gm20b.h>

#include "engines_gm20b.h"

bool gm20b_is_fault_engine_subid_gpc(struct gk20a *g, u32 engine_subid)
{
	(void)g;
	return (engine_subid == fifo_intr_mmu_fault_info_engine_subid_gpc_v());
}

int gm20b_engine_init_ce_info(struct nvgpu_fifo *f)
{
	struct gk20a *g = f->g;
	u32 i;
	bool found;

	for (i = NVGPU_DEVTYPE_COPY0;  i <= NVGPU_DEVTYPE_COPY2; i++) {
		const struct nvgpu_device *dev;
		struct nvgpu_device *dev_rw;

		dev = (struct nvgpu_device *)nvgpu_device_get(g, i,
							      i - NVGPU_DEVTYPE_COPY0);
		if (dev == NULL) {
			/*
			 * Not an error condition; gm20b has only 1 CE.
			 */
			continue;
		}

		/*
		 * Cast to a non-const version since we have to hack up a few fields for
		 * SW to work.
		 */
		dev_rw = (struct nvgpu_device *)dev;

		found = g->ops.fifo.find_pbdma_for_runlist(g,
							   dev->runlist_id,
							   &dev_rw->pbdma_id);
		if (!found) {
			nvgpu_err(g, "busted pbdma map");
			return -EINVAL;
		}

		f->host_engines[dev->engine_id] = dev;
		f->active_engines[f->num_engines] = dev;
		++f->num_engines;
	}

	return 0;
}
