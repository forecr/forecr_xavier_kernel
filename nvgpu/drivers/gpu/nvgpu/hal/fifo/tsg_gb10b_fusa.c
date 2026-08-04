// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/channel.h>
#include <nvgpu/engines.h>
#include <nvgpu/runlist.h>
#include <nvgpu/tsg.h>
#include <nvgpu/gk20a.h>

#include "hal/fifo/tsg_gv11b.h"
#include "hal/fifo/tsg_gb10b.h"

/* can be removed after runque support is added */
#define NVENC_RUNQUE			11U	/* pbdma 11 */
#define NVENC1_RUNQUE			12U	/* pbdma 12 */
#define OFA_RUNQUE			10U	/* pbdma 10 */
#define NVDEC_RUNQUE			3U	/* pbdma 3 */
#define NVDEC1_RUNQUE			5U	/* pbdma 5 */
#define NVJPG_RUNQUE			6U	/* pbdma 6 */
#define NVJPG1_RUNQUE			13U	/* pbdma 13 */

void gb10b_tsg_bind_channel_eng_method_buffers(struct nvgpu_tsg *tsg,
		struct nvgpu_channel *ch)
{
	struct gk20a *g = tsg->g;
	u64 gpu_va = 0U;

	nvgpu_assert(tsg->runlist != NULL);

	if (tsg->eng_method_buffers == NULL) {
		nvgpu_log_info(g, "eng method buffer NULL");
		return;
	}

	if (nvgpu_engine_is_multimedia_runlist_id(g, tsg->runlist->id)) {
		if (tsg->runlist->id == nvgpu_engine_get_nvenc_runlist_id(g, 0)) {
			gpu_va = tsg->eng_method_buffers[NVENC_RUNQUE].gpu_va;
		} else if (tsg->runlist->id == nvgpu_engine_get_nvenc_runlist_id(g, 1)) {
			gpu_va = tsg->eng_method_buffers[NVENC1_RUNQUE].gpu_va;
		} else if (tsg->runlist->id == nvgpu_engine_get_ofa_runlist_id(g, 0)) {
			gpu_va = tsg->eng_method_buffers[OFA_RUNQUE].gpu_va;
		} else if (tsg->runlist->id == nvgpu_engine_get_nvdec_runlist_id(g, 0)) {
			gpu_va = tsg->eng_method_buffers[NVDEC_RUNQUE].gpu_va;
		} else if (tsg->runlist->id == nvgpu_engine_get_nvdec_runlist_id(g, 1)) {
			gpu_va = tsg->eng_method_buffers[NVDEC1_RUNQUE].gpu_va;
		} else if (tsg->runlist->id == nvgpu_engine_get_nvjpg_runlist_id(g, 0)) {
			gpu_va = tsg->eng_method_buffers[NVJPG_RUNQUE].gpu_va;
		} else if (tsg->runlist->id == nvgpu_engine_get_nvjpg_runlist_id(g, 1)) {
			gpu_va = tsg->eng_method_buffers[NVJPG1_RUNQUE].gpu_va;
		}
	}

	if (gpu_va != 0U) {
		g->ops.ramin.set_eng_method_buffer(g, &ch->inst_block, gpu_va);
	} else {
		gv11b_tsg_bind_channel_eng_method_buffers(tsg, ch);
	}
}
