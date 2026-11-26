// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/error_notifier.h>
#include <nvgpu/vgpu/vgpu.h>

#include "channel_vgpu.h"

u32 vgpu_channel_count(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);

	return priv->constants.num_channels;
}

void vgpu_channel_set_ctx_mmu_error(struct gk20a *g, struct nvgpu_channel *ch)
{
	/*
	 * If error code is already set, this mmu fault
	 * was triggered as part of recovery from other
	 * error condition.
	 * Don't overwrite error flag.
	 */
	g->ops.channel.set_error_notifier(ch,
		NVGPU_ERR_NOTIFIER_FIFO_ERROR_MMU_ERR_FLT);

	/* mark channel as faulted */
	nvgpu_channel_set_unserviceable(ch);

	/* unblock pending waits */
	nvgpu_cond_broadcast_interruptible(&ch->semaphore_wq);
	nvgpu_cond_broadcast_interruptible(&ch->notifier_wq);
}

void vgpu_channel_set_error_notifier(struct gk20a *g,
				struct tegra_vgpu_channel_set_error_notifier *p)
{
	struct nvgpu_channel *ch;

	if (p->chid >= g->fifo.num_channels) {
		nvgpu_err(g, "invalid chid %d", p->chid);
		return;
	}

	ch = &g->fifo.channel[p->chid];
	g->ops.channel.set_error_notifier(ch, p->error);
}

void vgpu_channel_abort_cleanup(struct gk20a *g, u32 chid)
{
	struct nvgpu_channel *ch = nvgpu_channel_from_id(g, chid);

	if (ch == NULL) {
		nvgpu_err(g, "invalid channel id %d", chid);
		return;
	}

	nvgpu_channel_set_unserviceable(ch);
	g->ops.channel.abort_clean_up(ch);
	nvgpu_channel_put(ch);
}

int vgpu_channel_get_vmid(struct nvgpu_channel *ch, u32 *vmid)
{
	*vmid = vgpu_get_vmid(ch->g);

	return 0;
}
