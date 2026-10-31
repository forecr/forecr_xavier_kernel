// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/falcon.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/gsp.h>
#include "common/gsp/ipc/gsp_ipc.h"
#ifdef CONFIG_NVGPU_GSP_STRESS_TEST
#include <nvgpu/gsp/gsp_test.h>
#endif
#ifdef CONFIG_NVGPU_GSP_SCHEDULER
#include <nvgpu/gsp_sched.h>
#endif
#include "common/gsp/ipc/gsp_seq.h"
#include "common/gsp/ipc/gsp_queue.h"
#include "common/gsp/ipc/gsp_msg.h"

void nvgpu_gsp_isr_support(struct gk20a *g, bool enable)
{
	struct nvgpu_falcon *flcn = &g->gsp_flcn;

	nvgpu_log_fn(g, " ");

	/* Enable irq*/
	nvgpu_mutex_acquire(&flcn->isr_mutex);
	if (g->ops.gsp.enable_irq != NULL) {
		g->ops.gsp.enable_irq(g, enable);
	}
	flcn->isr_enabled = enable;
	nvgpu_mutex_release(&flcn->isr_mutex);
}

void nvgpu_gsp_suspend(struct gk20a *g)
{
	nvgpu_gsp_isr_support(g, false);

#if defined(CONFIG_NVGPU_GSP_SCHEDULER) && defined(CONFIG_NVGPU_FALCON_DEBUG)
	nvgpu_falcon_dbg_error_print_enable(&g->gsp_flcn, false);
#endif

	nvgpu_gsp_queues_free(g, g->gsp->gsp_ipc->queues);
	g->gsp->gsp_ipc->gsp_ready = false;
}

void nvgpu_gsp_sw_deinit(struct gk20a *g)
{
	struct nvgpu_falcon *flcn = &g->gsp_flcn;
	struct nvgpu_gsp *gsp = g->gsp;

	nvgpu_gsp_dbg(g, " ");

	if (gsp != NULL) {
		nvgpu_mutex_destroy(&flcn->isr_mutex);
#ifdef CONFIG_NVGPU_FALCON_DEBUG
		nvgpu_falcon_dbg_buf_destroy(gsp->gsp_flcn);
#endif

		if (gsp->gsp_ipc != NULL) {
			if (gsp->gsp_ipc->sequences != NULL) {
				nvgpu_gsp_sequences_free(g, &gsp->gsp_ipc->sequences);
			}

			nvgpu_gsp_queues_free(g, gsp->gsp_ipc->queues);

			nvgpu_mutex_destroy(&g->gsp->gsp_ipc->cmd_lock);

			nvgpu_kfree(g, gsp->gsp_ipc);
		}

		nvgpu_kfree(g, gsp);
		g->gsp = NULL;
	}
}

int nvgpu_gsp_sw_init(struct gk20a *g)
{
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	if (g->gsp != NULL) {
		/* Initialized already, so reuse the same to perform boot faster */
		return err;
	}

	/* Init struct holding the gsp software state */
	g->gsp = (struct nvgpu_gsp *) nvgpu_kzalloc(g, sizeof(struct nvgpu_gsp));
	if (g->gsp == NULL) {
		err = -ENOMEM;
		goto de_init;
	}

	g->gsp->gsp_ipc = (struct nvgpu_gsp_ipc *)nvgpu_kzalloc(g, sizeof(struct nvgpu_gsp_ipc));
	if (g->gsp->gsp_ipc == NULL) {
		err = -ENOMEM;
		goto de_init;
	}

	/* gsp falcon software state */
	g->gsp->gsp_flcn = &g->gsp_flcn;
	g->gsp->g = g;

	/* Init isr mutex */
	nvgpu_mutex_init(&g->gsp->gsp_flcn->isr_mutex);
	/* Init cmd lock mutex */
	nvgpu_mutex_init(&g->gsp->gsp_ipc->cmd_lock);

	err = nvgpu_gsp_sequences_init(g, &g->gsp->gsp_ipc);
	if (err != 0) {
		nvgpu_err(g, "GSP sequences init failed");
		goto de_init;
	}

	nvgpu_gsp_dbg(g, " Done ");
	return err;

de_init:
	nvgpu_gsp_sw_deinit(g);
	return err;
}

int nvgpu_gsp_debug_buf_init(struct gk20a *g, u32 queue_no, u32 buffer_size)
{
	int err = 0;

	nvgpu_log_fn(g, " ");

	/* enable debug buffer support */
#ifdef CONFIG_NVGPU_FALCON_DEBUG
	if ((g->ops.gsp.gsp_get_queue_head != NULL) &&
			(g->ops.gsp.gsp_get_queue_tail != NULL)) {
		err = nvgpu_falcon_dbg_buf_init(
					&g->gsp_flcn, buffer_size,
					g->ops.gsp.gsp_get_queue_head(queue_no),
					g->ops.gsp.gsp_get_queue_tail(queue_no));
		if (err != 0) {
			nvgpu_err(g, "GSP debug init failed");
		}
	}

#else
	(void)queue_no;
	(void)buffer_size;
#endif
	return err;
}

void nvgpu_gsp_isr_mutex_acquire(struct gk20a *g, struct nvgpu_falcon *flcn)
{
	(void)g;
	nvgpu_mutex_acquire(&flcn->isr_mutex);
}

void nvgpu_gsp_isr_mutex_release(struct gk20a *g, struct nvgpu_falcon *flcn)
{
	(void)g;
	nvgpu_mutex_release(&flcn->isr_mutex);
}

bool nvgpu_gsp_is_isr_enable(struct gk20a *g, struct nvgpu_falcon *flcn)
{
	(void)g;
	return flcn->isr_enabled;
}

struct nvgpu_falcon *nvgpu_gsp_falcon_instance(struct gk20a *g)
{
	return &g->gsp_flcn;
}

void nvgpu_gsp_isr(struct gk20a *g)
{
	g->ops.gsp.gsp_isr(g);
	return;
}

u32 nvgpu_gsp_get_last_cmd_id(struct gk20a *g)
{
	(void)g;
	return GSP_NV_CMDQ_LOG_ID__LAST;
}

bool nvgpu_gsp_is_ready(struct gk20a *g)
{
	return g->gsp->gsp_ipc->gsp_ready;
}

int nvgpu_gsp_process_message(struct gk20a *g)
{
	return nvgpu_gsp_process_ipc_message(g);
}
