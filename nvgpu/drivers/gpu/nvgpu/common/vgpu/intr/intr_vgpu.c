// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/gk20a.h>

#include <nvgpu/vgpu/vgpu_ivc.h>

#include "intr_vgpu.h"
#include "common/vgpu/gr/fecs_trace_vgpu.h"
#include "common/vgpu/fifo/fifo_vgpu.h"
#include "common/vgpu/fifo/fifo_vf.h"
#include "common/vgpu/fifo/channel_vgpu.h"
#include "common/vgpu/fifo/tsg_vgpu.h"
#include "common/vgpu/fifo/tsg_vf.h"
#include "common/vgpu/mm/mm_vgpu.h"
#include "common/vgpu/gr/gr_vgpu.h"

int vgpu_intr_thread(void *dev_id)
{
	struct gk20a *g = dev_id;
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);

	while (true) {
		struct tegra_vgpu_intr_msg msg;
		int err;

		err = vgpu_ivc_recv(&priv->ivc_intr, (void *)&msg, sizeof(msg));
		if (err == -ETIMEDOUT) {
			continue;
		}
		if (nvgpu_thread_should_stop(&priv->intr_handler)) {
			break;
		}
		if (err != 0) {
			nvgpu_do_assert_print(g,
				"Unexpected vgpu_ivc_recv err=%d", err);
			continue;
		}

		switch (msg.event) {
		case TEGRA_VGPU_EVENT_INTR:
			if (msg.unit == TEGRA_VGPU_INTR_GR) {
				vgpu_gr_isr(g, &msg.info.gr_intr);
			} else if (msg.unit == TEGRA_VGPU_INTR_FIFO) {
				vgpu_fifo_isr(g, &msg.info.fifo_intr);
			}
			break;
#ifdef CONFIG_NVGPU_FECS_TRACE
		case TEGRA_VGPU_EVENT_FECS_TRACE:
			vgpu_fecs_trace_data_update(g);
			break;
#endif
		case TEGRA_VGPU_EVENT_CHANNEL:
			vgpu_tsg_handle_event(g, &msg.info.channel_event);
			break;
		case TEGRA_VGPU_EVENT_SM_ESR:
			vgpu_gr_handle_sm_esr_event(g, &msg.info.sm_esr);
			break;
		case TEGRA_VGPU_EVENT_SEMAPHORE_WAKEUP:
			g->ops.semaphore_wakeup(g,
					!!msg.info.sem_wakeup.post_events);
			break;
		case TEGRA_VGPU_EVENT_CHANNEL_CLEANUP:
			vgpu_channel_abort_cleanup(g,
					msg.info.ch_cleanup.chid);
			break;
		case TEGRA_VGPU_EVENT_SET_ERROR_NOTIFIER:
			vgpu_channel_set_error_notifier(g,
						&msg.info.set_error_notifier);
			break;
		case TEGRA_VGPU_EVENT_TSG_SET_ERROR_NOTIFIER:
			vgpu_tsg_set_error_notifier(g,
						&msg.info.tsg_set_error_notifier);
			break;
		case TEGRA_VF_EVENT_TSG_SET_UNSERVICEABLE:
			vf_tsg_set_unserviceable(g,
						&msg.info.tsg_set_unserviceable);
			break;
		case TEGRA_VF_EVENT_RUNLIST_DISABLED_FOR_RC:
			vf_fifo_handle_runlist_disabled_for_rc(g,
						&msg.info.runlist_disabled);
			break;
		case TEGRA_VF_EVENT_RUNLIST_DISABLED_FOR_CILP:
			vf_fifo_handle_runlist_disabled_for_cilp(g,
						&msg.info.runlist_disabled);
			break;
		case TEGRA_VF_EVENT_CILP_SAVE_COMPLETED:
			g->ops.gr.intr.handle_cilp_save_completed(g, 0U,
						msg.info.cilp_save_completed.tsgid);
			break;
		default:
			nvgpu_err(g, "unknown event %u", msg.event);
			break;
		}
	}

	while (!nvgpu_thread_should_stop(&priv->intr_handler)) {
		nvgpu_msleep(10);
	}
	return 0;
}
