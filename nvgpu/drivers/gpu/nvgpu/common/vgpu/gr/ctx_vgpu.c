// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/kmem.h>
#include <nvgpu/bug.h>
#include <nvgpu/dma.h>
#include <nvgpu/dma.h>
#include <nvgpu/vgpu/vgpu_ivc.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/gr/global_ctx.h>
#include <nvgpu/gr/ctx.h>
#include <nvgpu/gr/obj_ctx.h>
#include <nvgpu/gr/hwpm_map.h>
#include <nvgpu/gr/gr_utils.h>

#include "common/gr/ctx_priv.h"

#include "ctx_vgpu.h"
#include "common/vgpu/ivc/comm_vgpu.h"

void vgpu_gr_free_gr_ctx(struct gk20a *g,
			 struct nvgpu_gr_ctx *gr_ctx)
{
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_gr_ctx_params *p = &msg.params.gr_ctx;
	int err;

	nvgpu_log_fn(g, " ");

	msg.cmd = TEGRA_VGPU_CMD_GR_CTX_FREE;
	msg.handle = vgpu_get_handle(g);
	p->tsg_id = gr_ctx->tsgid;
	err = vgpu_comm_sendrecv(g, &msg);
	WARN_ON(err || msg.ret);

	(void) memset(gr_ctx, 0, sizeof(*gr_ctx));
}
