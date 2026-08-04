// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/vgpu/vgpu_ivc.h>
#include <nvgpu/vgpu/tegra_vgpu.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/pm_reservation.h>

#include "pm_reservation_vgpu.h"
#include "common/vgpu/ivc/comm_vgpu.h"

int vgpu_pm_reservation_init(struct gk20a *g)
{
	u32 perfbuf_count, i;
	u64 *va;
	int err = 0;

	if (g->mm.perfbuf != NULL) {
		return 0;
	}

	if (g->ops.perf.get_pma_channel_count != NULL)
		g->num_pma_channels = g->ops.perf.get_pma_channel_count(g);
	else
		g->num_pma_channels = 1;

	if (g->ops.perf.get_pma_channels_per_cblock) {
		g->channels_per_cblock =
			g->ops.perf.get_pma_channels_per_cblock();
	} else {
		g->channels_per_cblock = 1;
	}
	perfbuf_count = g->num_pma_channels / g->channels_per_cblock;
	nvgpu_log(g, gpu_dbg_prof,
		"num_pma_channels: %u channels_per_cblock: %u perfbuf count %u",
		g->num_pma_channels, g->channels_per_cblock, perfbuf_count);

	nvgpu_assert((g->channels_per_cblock <= g->num_pma_channels) &&
			(g->num_pma_channels % g->channels_per_cblock == 0U));

	g->mm.perfbuf = nvgpu_kzalloc(g, sizeof(*g->mm.perfbuf) * perfbuf_count);
	if (g->mm.perfbuf == NULL) {
		err = -ENOMEM;
		return err;
	}

	g->mm.num_of_perfbuf = perfbuf_count;

	for (i = 0; i < perfbuf_count; i++) {
		va = nvgpu_kzalloc(g, sizeof(*va) * g->channels_per_cblock);
		if (va == NULL) {
			err = -ENOMEM;
			goto va_alloc_failed;
		}
		g->mm.perfbuf[i].pma_bytes_available_buffer_gpu_va = va;

		va = nvgpu_kzalloc(g, sizeof(*va) * g->channels_per_cblock);
		if (va == NULL) {
			err = -ENOMEM;
			goto va_alloc_failed;
		}
		g->mm.perfbuf[i].pma_buffer_gpu_va = va;
		nvgpu_atomic_set(&g->mm.perfbuf[i].buf_refcount, 0);
	}

	g->hs_credits = nvgpu_kzalloc(g,
				nvgpu_safe_mult_u32(
					sizeof(struct nvgpu_pma_stream_hs_credits),
						g->num_pma_channels));
	if (g->hs_credits == NULL) {
		err = -ENOMEM;
		goto va_alloc_failed;
	}

	nvgpu_atomic_set(&g->hwpm_refcount, 0);

	nvgpu_log(g, gpu_dbg_prof, "initialized");

	return 0;

va_alloc_failed:
	for (i = 0; i < perfbuf_count; i++) {
		nvgpu_kfree(g, g->mm.perfbuf[i].pma_bytes_available_buffer_gpu_va);
		nvgpu_kfree(g, g->mm.perfbuf[i].pma_buffer_gpu_va);
	}
	nvgpu_kfree(g, g->mm.perfbuf);
	g->mm.perfbuf = NULL;

	return err;
}

int vgpu_pm_reservation_acquire(struct gk20a *g, u32 gpu_instance_id, u32 reservation_id,
	enum nvgpu_profiler_pm_resource_type pm_resource,
	enum nvgpu_profiler_pm_reservation_scope scope,
	u32 vmid, u32 *pma_channel_id)
{
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_prof_mgt_params *p = &msg.params.prof_management;
	int err = 0;
	(void)gpu_instance_id;

	msg.cmd = TEGRA_VGPU_CMD_PROF_MGT;
	msg.handle = vgpu_get_handle(g);

	p->mode = TEGRA_VGPU_PROF_PM_RESERVATION_ACQUIRE;
	p->reservation_id = reservation_id;
	p->pm_resource = pm_resource;
	p->scope = scope;

	err = vgpu_comm_sendrecv(g, &msg);

	err = err ? err : msg.ret;
	// pma channel id is allocated only when resource type is pma stream
	if (err == 0 && pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM) {
		*pma_channel_id = p->pma_channel_id;
	}
	return err;
}

int vgpu_pm_reservation_release(struct gk20a *g, u32 gpu_instance_id, u32 reservation_id,
	enum nvgpu_profiler_pm_resource_type pm_resource,
	u32 vmid, u32 pma_channel_id)
{
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_prof_mgt_params *p = &msg.params.prof_management;
	int err = 0;
	(void)gpu_instance_id;

	msg.cmd = TEGRA_VGPU_CMD_PROF_MGT;
	msg.handle = vgpu_get_handle(g);

	p->mode = TEGRA_VGPU_PROF_PM_RESERVATION_RELEASE;
	p->reservation_id = reservation_id;
	p->pm_resource = pm_resource;
	p->pma_channel_id = pma_channel_id;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	return err;
}
