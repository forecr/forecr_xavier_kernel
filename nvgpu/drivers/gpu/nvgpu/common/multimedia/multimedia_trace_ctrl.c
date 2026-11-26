// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/log2.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/periodic_timer.h>
#include <nvgpu/multimedia_trace.h>
#include <nvgpu/multimedia_trace_ctrl.h>
#include "multimedia_trace_priv.h"

struct nvgpu_multimedia_trace_ctrl_timer_args {
	struct gk20a *g;
	u32 multimedia_id;
};

struct nvgpu_multimedia_trace_ctrl {
	struct nvgpu_mutex poll_lock;
	struct nvgpu_periodic_timer poll_timer;
	struct nvgpu_multimedia_trace_ctrl_timer_args timer_args;

	struct nvgpu_mutex enable_lock;
	u32 enable_count;
	bool initialized;
};

struct nvgpu_multimedia_trace_ctrls {
	struct nvgpu_multimedia_trace_ctrl ctrls[NVGPU_MULTIMEDIA_ENGINE_MAX];
};

static void nvgpu_multimedia_trace_ctrl_periodic_polling(void *arg);

int nvgpu_multimedia_trace_ctrl_init(struct gk20a *g)
{
	struct nvgpu_multimedia_trace_ctrls *trace_ctrls;
	struct nvgpu_multimedia_trace_ctrl *ctrl;
	int err;

	if (!is_power_of_2((u32)MULTIMEDIA_TRACE_NUM_RECORDS)) {
		nvgpu_err(g, "invalid NUM_RECORDS chosen");
		nvgpu_set_enabled(g, NVGPU_SUPPORT_MULTIMEDIA_TRACE, false);
		return -EINVAL;
	}

	trace_ctrls = nvgpu_kzalloc(g, sizeof(*trace_ctrls));
	if (unlikely(!trace_ctrls)) {
		return -ENOMEM;
	}

	g->multimedia_trace_ctrls = trace_ctrls;

	ctrl = trace_ctrls->ctrls;

	for (u32 i = 0; i < NVGPU_MULTIMEDIA_ENGINE_MAX; i++) {
		nvgpu_mutex_init(&ctrl->poll_lock);
		nvgpu_mutex_init(&ctrl->enable_lock);

		ctrl->timer_args.g = g;
		ctrl->timer_args.multimedia_id = i;
		err = nvgpu_periodic_timer_init(&ctrl->poll_timer,
				nvgpu_multimedia_trace_ctrl_periodic_polling, &ctrl->timer_args);
		if (err != 0) {
			nvgpu_err(g, "failed to create multimedia %u trace timer err=%d", i, err);
			goto fail;
		}

		ctrl->enable_count = 0;
		ctrl->initialized = true;
		ctrl++;
	}

	return 0;

fail:
	nvgpu_multimedia_trace_ctrl_deinit(g);
	return err;
}

int nvgpu_multimedia_trace_ctrl_deinit(struct gk20a *g)
{
	struct nvgpu_multimedia_trace_ctrls *trace_ctrls = g->multimedia_trace_ctrls;
	struct nvgpu_multimedia_trace_ctrl *ctrl;

	if (trace_ctrls == NULL) {
		return 0;
	}

	ctrl = trace_ctrls->ctrls;

	for (u32 i = 0; i < NVGPU_MULTIMEDIA_ENGINE_MAX; i++) {
		if (!ctrl->initialized) {
			continue;
		}
		/*
		 * Check if tracer was enabled before attempting to stop the
		 * tracer timer.
		 */
		if (ctrl->enable_count > 0) {
			nvgpu_periodic_timer_stop(&ctrl->poll_timer);
		}
		nvgpu_periodic_timer_destroy(&ctrl->poll_timer);

		nvgpu_mutex_destroy(&ctrl->poll_lock);
		nvgpu_mutex_destroy(&ctrl->enable_lock);

		ctrl++;
	}

	nvgpu_kfree(g, trace_ctrls);
	g->multimedia_trace_ctrls = NULL;
	return 0;
}

int nvgpu_multimedia_trace_ctrl_enable(struct gk20a *g, u32 multimedia_id)
{
	struct nvgpu_multimedia_trace_ctrls *trace_ctrls = g->multimedia_trace_ctrls;
	struct nvgpu_multimedia_trace_ctrl *ctrl;
	int err = 0;

	if (unlikely(!trace_ctrls)) {
		return -ENODEV;
	}

	if (multimedia_id >= NVGPU_MULTIMEDIA_ENGINE_MAX) {
		return -EINVAL;
	}

	ctrl = &trace_ctrls->ctrls[multimedia_id];

	nvgpu_mutex_acquire(&ctrl->enable_lock);
	ctrl->enable_count++;

	if (ctrl->enable_count == 1U) {
		/* Enable in the buffer */
		err = nvgpu_multimedia_trace_enable(g,
				nvgpu_multimedia_get_dev(g, multimedia_id), true);
		if (err != 0) {
			goto done;
		}

		err = nvgpu_periodic_timer_start(&ctrl->poll_timer,
				MULTIMEDIA_TRACE_FRAME_PERIOD_NS);
		if (err != 0) {
			nvgpu_err(g, "failed to start multimedia %u polling timer", multimedia_id);
			goto done;
		}
	}

done:
	nvgpu_mutex_release(&ctrl->enable_lock);
	return err;
}

int nvgpu_multimedia_trace_ctrl_disable(struct gk20a *g, u32 multimedia_id)
{
	struct nvgpu_multimedia_trace_ctrls *trace_ctrls = g->multimedia_trace_ctrls;
	struct nvgpu_multimedia_trace_ctrl *ctrl;
	int err = 0;

	if (unlikely(!trace_ctrls)) {
		return -ENODEV;
	}

	if (multimedia_id >= NVGPU_MULTIMEDIA_ENGINE_MAX) {
		return -EINVAL;
	}

	ctrl = &trace_ctrls->ctrls[multimedia_id];

	nvgpu_mutex_acquire(&ctrl->enable_lock);
	if (ctrl->enable_count <= 0U) {
		nvgpu_mutex_release(&ctrl->enable_lock);
		return 0;
	}

	ctrl->enable_count--;
	if (ctrl->enable_count == 0U) {
		nvgpu_periodic_timer_stop(&ctrl->poll_timer);

		/* Disable in the buffer */
		err = nvgpu_multimedia_trace_enable(g,
				nvgpu_multimedia_get_dev(g, multimedia_id), false);
	}
	nvgpu_mutex_release(&ctrl->enable_lock);

	return err;
}

int nvgpu_multimedia_trace_ctrl_poll(struct gk20a *g, u32 multimedia_id)
{
	struct nvgpu_multimedia_trace_ctrls *trace_ctrls = g->multimedia_trace_ctrls;
	struct nvgpu_multimedia_trace_ctrl *ctrl;
	int cnt;
	int err = 0;

	if (unlikely(!trace_ctrls)) {
		return -ENODEV;
	}

	if (multimedia_id >= NVGPU_MULTIMEDIA_ENGINE_MAX) {
		return -EINVAL;
	}

	ctrl = &trace_ctrls->ctrls[multimedia_id];

	nvgpu_mutex_acquire(&ctrl->poll_lock);
	if (ctrl->enable_count == 0) {
		goto done_unlock;
	}

	err = gk20a_busy(g);
	if (err) {
		goto done_unlock;
	}

	/* Ensure all trace buffer writes have made it to SYSMEM */
	err = g->ops.mm.cache.fb_flush(g);
	if (err != 0) {
		nvgpu_err(g, "mm.cache.fb_flush() failed err=%d", err);
		goto done;
	}

	cnt = nvgpu_multimedia_trace_ctrl_ring_read(g,
			nvgpu_multimedia_get_dev(g, multimedia_id), multimedia_id);
	if (cnt <= 0) {
		err = cnt;
	} else {
		nvgpu_log(g, gpu_dbg_mme, "multimedia %u trace_ctrl_poll read entries %d",
				multimedia_id, cnt);
	}

	/* Ensure trace records have been updated */
	nvgpu_wmb();

done:
	gk20a_idle(g);
done_unlock:
	nvgpu_mutex_release(&ctrl->poll_lock);
	return err;
}

static void nvgpu_multimedia_trace_ctrl_periodic_polling(void *arg)
{
	struct nvgpu_multimedia_trace_ctrl_timer_args *timer_args =
       		(struct nvgpu_multimedia_trace_ctrl_timer_args *)arg;

	nvgpu_multimedia_trace_ctrl_poll(timer_args->g, timer_args->multimedia_id);
}
