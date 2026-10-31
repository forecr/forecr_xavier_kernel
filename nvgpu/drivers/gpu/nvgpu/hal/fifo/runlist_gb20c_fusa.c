// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/fifo.h>
#include <nvgpu/pbdma.h>

#include "runlist_gb20c.h"

#include <nvgpu/hw/gb20c/hw_runlist_gb20c.h>

u32 gb20c_runlist_get_runlist_id(struct gk20a *g, u32 runlist_pri_base)
{
	u32 doorbell_config = nvgpu_readl(g, nvgpu_safe_add_u32(
			runlist_pri_base, runlist_doorbell_config_r()));

	return runlist_doorbell_config_id_v(doorbell_config);
}

u32 gb20c_runlist_get_engine_id_from_rleng_id(struct gk20a *g,
					u32 rleng_id, u32 runlist_pri_base)
{
	u32 engine_status_debug = nvgpu_readl(g, nvgpu_safe_add_u32(
			runlist_pri_base,
			runlist_engine_status_debug_r(rleng_id)));

	return runlist_engine_status_debug_engine_id_v(engine_status_debug);
}

u32 gb20c_runlist_get_chram_bar0_offset(struct gk20a *g, u32 runlist_pri_base)
{
	u32 channel_config = nvgpu_readl(g, nvgpu_safe_add_u32(
			runlist_pri_base, runlist_channel_config_r()));

	return (runlist_channel_config_chram_bar0_offset_v(channel_config)
			<< runlist_channel_config_chram_bar0_offset_b());
}

/*
 * Use u32 runlist_pri_base instead of struct nvgpu_runlist *runlist
 * as input paramter, because by the time this hal is called, runlist_info
 * is not populated.
 */
void gb20c_runlist_get_pbdma_info(struct gk20a *g, u32 runlist_pri_base,
			struct nvgpu_pbdma_info *pbdma_info)
{
	u32 i, pbdma_config;

	if (runlist_pbdma_config__size_1_v() != PBDMA_PER_RUNLIST_SIZE) {
		nvgpu_warn(g, "mismatch: h/w & s/w for pbdma_per_runlist_size");
	}
	for (i = 0U; i < runlist_pbdma_config__size_1_v(); i++) {
		pbdma_config = nvgpu_readl(g, nvgpu_safe_add_u32(
				runlist_pri_base, runlist_pbdma_config_r(i)));
		if (runlist_pbdma_config_valid_v(pbdma_config) ==
				runlist_pbdma_config_valid_true_v()) {
			pbdma_info->pbdma_pri_base[i] =
			runlist_pbdma_config_pbdma_bar0_offset_v(pbdma_config) << PBDMA_BAR0_OFFSET_SHIFT;
			pbdma_info->pbdma_id[i] =
				runlist_pbdma_config_id_v(pbdma_config);
		} else {
			pbdma_info->pbdma_pri_base[i] =
						NVGPU_INVALID_PBDMA_PRI_BASE;
			pbdma_info->pbdma_id[i] = NVGPU_INVALID_PBDMA_ID;
		}
	}
}

u32 gb20c_runlist_get_esched_fb_thread_id(struct gk20a *g, u32 runlist_pri_base)
{
	u32 esched_fb_config = nvgpu_readl(g, nvgpu_safe_add_u32(
			runlist_pri_base, runlist_fb_config_r()));

	return runlist_fb_config_fb_thread_id_v(esched_fb_config);
}
