// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/trace.h>
#include <nvgpu/kmem.h>
#include <nvgpu/dma.h>
#include <nvgpu/atomic.h>
#include <nvgpu/bug.h>
#include <nvgpu/barrier.h>
#include <nvgpu/io.h>
#include <nvgpu/error_notifier.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>
#include <nvgpu/fifo.h>
#include <nvgpu/runlist.h>
#include <nvgpu/string.h>
#include <nvgpu/vm_area.h>
#include <nvgpu/vgpu/vgpu_ivc.h>
#include <nvgpu/vgpu/vgpu.h>

#include <hal/fifo/tsg_gk20a.h>

#include "fifo_vgpu.h"
#include "channel_vgpu.h"
#include "tsg_vgpu.h"

void vgpu_fifo_cleanup_sw(struct gk20a *g)
{
	u32 i;
	struct nvgpu_fifo *f = &g->fifo;

	for (i = 0U; i < f->max_engines; i++) {
		if (f->host_engines[i] == NULL) {
			continue;
		}

		/*
		 * Cast to (void *) to get rid of the constness.
		 */
		nvgpu_kfree_impl(g, (void *)f->host_engines[i]);
	}
	nvgpu_fifo_cleanup_sw_common(g);
}

int vgpu_fifo_setup_sw(struct gk20a *g)
{
	struct nvgpu_fifo *f = &g->fifo;
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	int err = 0;

	nvgpu_log_fn(g, " ");

	if (f->sw_ready) {
		nvgpu_log_fn(g, "skip init");
		return 0;
	}

	err = nvgpu_fifo_setup_sw_common(g);
	if (err != 0) {
		nvgpu_err(g, "fifo sw setup failed, err=%d", err);
		return err;
	}

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
	err = nvgpu_channel_worker_init(g);
	if (err) {
		goto clean_up;
	}
#endif

	f->channel_base = priv->constants.channel_base;

	f->sw_ready = true;

	nvgpu_log_fn(g, "done");
	return 0;

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
clean_up:
	nvgpu_fifo_cleanup_sw_common(g);
#endif

	return err;
}

int vgpu_fifo_isr(struct gk20a *g, struct tegra_vgpu_fifo_intr_info *info)
{
	struct nvgpu_tsg *tsg = NULL;

	nvgpu_log_fn(g, " ");

	tsg = nvgpu_tsg_check_and_get_from_id(g, info->tsgid);
	if (tsg == NULL) {
		nvgpu_err(g, "invalid tsgid %u", info->tsgid);
		return 0;
	}

	nvgpu_err(g, "fifo intr (%d) on tsg %u",
		info->type, info->tsgid);

	switch (info->type) {
	case TEGRA_VGPU_FIFO_INTR_CTXSW_TIMEOUT:
		nvgpu_tsg_set_error_notifier(g, 0U, tsg->tsgid,
			NVGPU_ERR_NOTIFIER_FIFO_ERROR_IDLE_TIMEOUT);
		break;
	default:
		WARN_ON(1);
		break;
	}

	return 0;
}
