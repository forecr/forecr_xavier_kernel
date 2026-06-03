// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/channel.h>
#include <nvgpu/engines.h>
#include <nvgpu/runlist.h>
#include <nvgpu/tsg.h>
#include <nvgpu/gk20a.h>

#include "hal/fifo/tsg_gv11b.h"
#include "hal/fifo/tsg_gb20c.h"

/* GB20C specific PBDMA assignments - only 7 PBDMAs available (0-6) */
#define GB20C_GR_RUNQUE			0U	/* pbdma 0 */
#define GB20C_ASYNC_CE_RUNQUE		2U	/* pbdma 2 */
#define GB20C_NVENC_RUNQUE		4U	/* pbdma 4 */
#define GB20C_NVDEC_RUNQUE		3U	/* pbdma 3 */
#define GB20C_OFA_RUNQUE		6U	/* pbdma 6 */

void gb20c_tsg_bind_channel_eng_method_buffers(struct nvgpu_tsg *tsg,
		struct nvgpu_channel *ch)
{
	struct gk20a *g = tsg->g;
	u64 gpu_va;
	u32 ce_runlist_id;
	int err;

	nvgpu_assert(tsg->runlist != NULL);

	if (tsg->eng_method_buffers == NULL) {
		nvgpu_log_info(g, "eng method buffer NULL");
		return;
	}

	/* Get fast CE runlist ID for comparison */
	err = nvgpu_engine_get_fast_ce_runlist_id(g,
			tsg->gpu_instance_id,
			&ce_runlist_id);
	if (err != 0) {
		nvgpu_err(g, "failed to get fast ce runlist id");
		return;
	}

	/* Handle all engines with GB20C-specific PBDMA mappings */
	if (tsg->runlist->id == ce_runlist_id) {
		gpu_va = tsg->eng_method_buffers[GB20C_ASYNC_CE_RUNQUE].gpu_va;
	} else if (tsg->runlist->id == nvgpu_engine_get_nvenc_runlist_id(g, 0)) {
		gpu_va = tsg->eng_method_buffers[GB20C_NVENC_RUNQUE].gpu_va;
	} else if (tsg->runlist->id == nvgpu_engine_get_ofa_runlist_id(g, 0)) {
		gpu_va = tsg->eng_method_buffers[GB20C_OFA_RUNQUE].gpu_va;
	} else if (tsg->runlist->id == nvgpu_engine_get_nvdec_runlist_id(g, 0)) {
		gpu_va = tsg->eng_method_buffers[GB20C_NVDEC_RUNQUE].gpu_va;
	} else {
		gpu_va = tsg->eng_method_buffers[GB20C_GR_RUNQUE].gpu_va;
	}

	g->ops.ramin.set_eng_method_buffer(g, &ch->inst_block, gpu_va);
}
