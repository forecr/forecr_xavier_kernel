// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/tsg_subctx.h>
#include <nvgpu/gr/subctx.h>
#include <nvgpu/channel.h>
#include <nvgpu/log.h>

#include "subctx_vgpu.h"

void vgpu_gr_setup_free_subctx(struct nvgpu_channel *c)
{
	nvgpu_log(c->g, gpu_dbg_gr, " ");

	if (!nvgpu_is_enabled(c->g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return;
	}

	nvgpu_gr_subctx_free(c->g, c->subctx, c->vm, false);

	nvgpu_log(c->g, gpu_dbg_gr, "done");
}
