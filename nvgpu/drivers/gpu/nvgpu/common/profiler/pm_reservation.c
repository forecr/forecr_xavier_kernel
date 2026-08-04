// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/pm_reservation.h>
#include <nvgpu/log.h>
#include <nvgpu/mc.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/kmem.h>
#include <nvgpu/lock.h>
#include <nvgpu/gr/gr_instances.h>
#include <nvgpu/gr/hwpm_map.h>

#include "common/gr/gr_priv.h"

static void prepare_resource_reservation(struct gk20a *g,
		enum nvgpu_profiler_pm_resource_type pm_resource, bool acquire)
{
	int err;

	if ((pm_resource != NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY) &&
	    (pm_resource != NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM) &&
	    (pm_resource != NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES) &&
	    (pm_resource != NVGPU_PROFILER_PM_RESOURCE_TYPE_CCU_PROF)) {
		return;
	}

	if (acquire) {
		nvgpu_atomic_inc(&g->hwpm_refcount);
		nvgpu_log(g, gpu_dbg_prof, "HWPM refcount acquired %u, resource %u",
			nvgpu_atomic_read(&g->hwpm_refcount), pm_resource);

		if (nvgpu_atomic_read(&g->hwpm_refcount) == 1) {
			nvgpu_log(g, gpu_dbg_prof,
				"Trigger HWPM system reset, disable perf SLCG");
			err = nvgpu_mc_reset_units(g, NVGPU_UNIT_PERFMON);
			if (err != 0) {
				nvgpu_err(g, "Failed to reset PERFMON unit");
			}
			nvgpu_cg_slcg_perf_load_enable(g, false);
#ifdef CONFIG_NVGPU_NON_FUSA
			/*
			 * By default, disable the PMASYS legacy mode for
			 * NVGPU_NEXT.
			 */
			if (g->ops.perf.enable_pmasys_legacy_mode != NULL) {
				g->ops.perf.enable_pmasys_legacy_mode(g, false);
			}
#endif
		}
	} else {
		nvgpu_atomic_dec(&g->hwpm_refcount);
		nvgpu_log(g, gpu_dbg_prof, "HWPM refcount released %u, resource %u",
			nvgpu_atomic_read(&g->hwpm_refcount), pm_resource);

		if (nvgpu_atomic_read(&g->hwpm_refcount) == 0) {
			nvgpu_log(g, gpu_dbg_prof,
				"Trigger HWPM system reset, re-enable perf SLCG");
			err = nvgpu_mc_reset_units(g, NVGPU_UNIT_PERFMON);
			if (err != 0) {
				nvgpu_err(g, "Failed to reset PERFMON unit");
			}
			nvgpu_cg_slcg_perf_load_enable(g, true);
		}
	}
}

bool check_pm_resource_existing_reservation_locked(
		struct nvgpu_pm_resource_reservations *reservations,
		u32 reservation_id, u32 vmid)
{
	struct nvgpu_pm_resource_reservation_entry *reservation_entry;
	bool reserved = false;

	nvgpu_list_for_each_entry(reservation_entry,
			&reservations->head,
			nvgpu_pm_resource_reservation_entry,
			entry) {
		if ((reservation_entry->reservation_id == reservation_id) &&
		    (reservation_entry->vmid == vmid )) {
			reserved = true;
			break;
		}
	}

	return reserved;
}

static bool check_pm_resource_reservation_allowed_locked(
		struct nvgpu_pm_resource_reservations *reservations,
		enum nvgpu_profiler_pm_reservation_scope scope,
		enum nvgpu_profiler_pm_resource_type pm_resource,
		u32 reservation_id, u32 vmid)
{
	struct nvgpu_pm_resource_reservation_entry *reservation_entry;
	bool allowed = false;

	switch (scope) {
	case NVGPU_PROFILER_PM_RESERVATION_SCOPE_DEVICE:
		/*
		 * At DEVICE SCOPE, any resource group will be reserved only once and
		 * if that group is not reserved for CONTEXT SCOPE.
		 * But multiple PMA stream can be reserved if the streams are available
		 * for reservation.
		 */
		if (reservations->count == 0U ||
			pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM) {
			allowed = true;
		}
		break;

	case NVGPU_PROFILER_PM_RESERVATION_SCOPE_CONTEXT:
		/*
		 * Reservation of SCOPE_CONTEXT is allowed only if -
		 * 1. There is no current SCOPE_DEVICE reservation by any other profiler
		 *    object.
		 * 2. Requesting profiler object does not already have the reservation.
		 */

		if (!nvgpu_list_empty(&reservations->head)) {
			reservation_entry = nvgpu_list_first_entry(
				&reservations->head,
				nvgpu_pm_resource_reservation_entry,
				entry);
			if (reservation_entry->scope ==
					NVGPU_PROFILER_PM_RESERVATION_SCOPE_DEVICE) {
				break;
			}
		}

		if (check_pm_resource_existing_reservation_locked(reservations,
				reservation_id, vmid)) {
			break;
		}

		allowed = true;
		break;
	}

	return allowed;
}

/* input pma_channel_id is valid for the pm_resource type of PMA_STREAM only */
int nvgpu_pm_reservation_acquire(struct gk20a *g, u32 gpu_instance_id,
		u32 reservation_id, enum nvgpu_profiler_pm_resource_type pm_resource,
		enum nvgpu_profiler_pm_reservation_scope scope,
		u32 vmid, u32 *pma_channel_id)
{
	struct nvgpu_pm_resource_reservations *reservations =
			g->pm_reservations[gpu_instance_id] + pm_resource;
	struct nvgpu_pm_resource_reservation_entry *reservation_entry;
	bool channel_available = false;
	int err = 0;
	u32 i;

	nvgpu_mutex_acquire(&reservations->lock);

	if (!check_pm_resource_reservation_allowed_locked(reservations, scope,
			pm_resource, reservation_id, vmid)) {
		err = -EBUSY;
		goto done;
	}

	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM) {
		for (i = 0; i < g->num_pma_channels; i++) {
			if (g->pma_channel_reserved[i] ==  false) {
				*pma_channel_id = i;
				channel_available = true;
				break;
			}
		}

		if (channel_available != true) {
			err = -EBUSY;
			nvgpu_err(g, "No pma channel is free for allocation. %d", err);
			goto done;
		}
		nvgpu_log(g, gpu_dbg_prof, "pma_stream idx %u is getting reserved",
									*pma_channel_id);
	}

	reservation_entry = nvgpu_kzalloc(g, sizeof(*reservation_entry));
	if (reservation_entry == NULL) {
		err = -ENOMEM;
		goto done;
	}

	nvgpu_init_list_node(&reservation_entry->entry);

	reservation_entry->reservation_id = reservation_id;
	reservation_entry->scope = scope;
	reservation_entry->vmid = vmid;
	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM) {
		reservation_entry->pma_channel_id = *pma_channel_id;
		g->pma_channel_reserved[*pma_channel_id] = true;
	}

	nvgpu_list_add(&reservation_entry->entry, &reservations->head);
	reservations->count++;

	prepare_resource_reservation(g, pm_resource, true);

done:
	nvgpu_mutex_release(&reservations->lock);

	return err;
}

int nvgpu_pm_reservation_release(struct gk20a *g, u32 gpu_instance_id,
		u32 reservation_id, enum nvgpu_profiler_pm_resource_type pm_resource,
		u32 vmid, u32 pma_channel_id)
{
	struct nvgpu_pm_resource_reservations *reservations =
				g->pm_reservations[gpu_instance_id] + pm_resource;
	struct nvgpu_pm_resource_reservation_entry *reservation_entry, *n;
	bool was_reserved = false;
	int err = 0;

	nvgpu_mutex_acquire(&reservations->lock);

	nvgpu_list_for_each_entry_safe(reservation_entry, n,
			&reservations->head,
			nvgpu_pm_resource_reservation_entry,
			entry) {
		if ((reservation_entry->reservation_id == reservation_id) &&
		    (reservation_entry->vmid == vmid) &&
		    (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM ?
		    pma_channel_id == reservation_entry->pma_channel_id : true)) {
			was_reserved = true;
			nvgpu_list_del(&reservation_entry->entry);
			reservations->count--;
			nvgpu_kfree(g, reservation_entry);
			break;
		}
	}

	if (was_reserved) {
		prepare_resource_reservation(g, pm_resource, false);

		if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM) {
			g->pma_channel_reserved[pma_channel_id] = false;
		}
	} else {
		err = -EINVAL;
	}

	nvgpu_mutex_release(&reservations->lock);

	return err;
}

void nvgpu_pm_reservation_release_all_per_vmid(struct gk20a *g,
					u32 gpu_instance_id, u32 vmid)
{
	struct nvgpu_pm_resource_reservations *reservations;
	struct nvgpu_pm_resource_reservation_entry *reservation_entry, *n;
	int i;

	for (i = 0; i < NVGPU_PROFILER_PM_RESOURCE_TYPE_COUNT; i++) {
		reservations = g->pm_reservations[gpu_instance_id] + i;

		nvgpu_mutex_acquire(&reservations->lock);
		nvgpu_list_for_each_entry_safe(reservation_entry, n,
				&reservations->head,
				nvgpu_pm_resource_reservation_entry,
				entry) {
			if (reservation_entry->vmid == vmid) {
				nvgpu_list_del(&reservation_entry->entry);
				reservations->count--;
				nvgpu_kfree(g, reservation_entry);
				prepare_resource_reservation(g,
					(enum nvgpu_profiler_pm_resource_type)i,
					false);
			}
		}
		nvgpu_mutex_release(&reservations->lock);
	}
}

static void nvgpu_pm_free_pmabuf_vas(struct gk20a *g)
{
	u32 i;

	if (g->mm.perfbuf == NULL) {
		return;
	}

	for (i = 0; i < g->mm.num_of_perfbuf; i++) {
		nvgpu_kfree(g, g->mm.perfbuf[i].pma_bytes_available_buffer_gpu_va);
		nvgpu_kfree(g, g->mm.perfbuf[i].pma_buffer_gpu_va);
		nvgpu_mutex_destroy(&g->mm.perfbuf[i].lock);
	}
}

int nvgpu_pm_reservation_init(struct gk20a *g)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr(g);
	struct nvgpu_pm_resource_reservations *reservations;
	u32 perfbuf_count, i, j;
	u64 *va;
	int err = 0;

	nvgpu_log(g, gpu_dbg_prof, " ");

	if (g->support_gpu_tools == 0U) {
		nvgpu_info(g, "Profiler support is disabled");
		return 0;
	}

	if (g->pm_reservations) {
		return 0;
	}

	g->pm_reservations = nvgpu_kzalloc(g, sizeof(struct nvgpu_pm_resource_reservations *) *
						g->mig.num_gpu_instances);
	if (g->pm_reservations == NULL) {
		return -ENOMEM;
	}

	for (i = 0; i < g->mig.num_gpu_instances; i++) {
		reservations = nvgpu_kzalloc(g, sizeof(*reservations) *
				NVGPU_PROFILER_PM_RESOURCE_TYPE_COUNT);
		if (reservations == NULL) {
			err = -ENOMEM;
			goto pm_reservation_failed;
		}

		for (j = 0; j < NVGPU_PROFILER_PM_RESOURCE_TYPE_COUNT; j++) {
			nvgpu_init_list_node(&reservations[j].head);
			nvgpu_mutex_init(&reservations[j].lock);
		}

		g->pm_reservations[i] = reservations;
	}

	/* Create map of pri address and pm offset if necessary */
	if (!gr->hwpm_map->init) {
		err = nvgpu_gr_exec_with_ret_for_each_instance(g,
				nvgpu_gr_hwpm_map_create(g, gr->hwpm_map,
								gr->config));
		if (err != 0) {
			goto pm_reservation_failed;
		}
	}

	if ((g->num_sys_perfmon == 0U) &&
			(g->ops.perf.get_num_hwpm_perfmon != NULL)) {
		g->ops.perf.get_num_hwpm_perfmon(g, &g->num_sys_perfmon,
				&g->num_fbp_perfmon, &g->num_gpc_perfmon);
		nvgpu_log(g, gpu_dbg_prof | gpu_dbg_gpu_dbg,
			"num_sys_perfmon[%u] num_fbp_perfmon[%u] "
				"num_gpc_perfmon[%u] ",
			g->num_sys_perfmon, g->num_fbp_perfmon,
			g->num_gpc_perfmon);
		nvgpu_assert((g->num_sys_perfmon != 0U) &&
			(g->num_fbp_perfmon != 0U) &&
			(g->num_gpc_perfmon != 0U));
	}

	if (g->ops.perf.get_pma_channel_count != NULL)
		g->num_pma_channels = g->ops.perf.get_pma_channel_count(g);
	else
		g->num_pma_channels = 1;

	g->pma_channel_reserved = nvgpu_kzalloc(g,
			sizeof(*(g->pma_channel_reserved)) *
				g->num_pma_channels);
	if (g->pma_channel_reserved == NULL) {
		err = -ENOMEM;
		goto pm_reservation_failed;
	}

	if (g->ops.perf.get_pma_channels_per_cblock != NULL) {
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
		goto perfbuf_alloc_failed;
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
		nvgpu_mutex_init(&g->mm.perfbuf[i].lock);
	}

	if (g->ops.perf.alloc_mem_for_sys_partition_dg_map != NULL) {
		err = g->ops.perf.alloc_mem_for_sys_partition_dg_map(g);
		if (err != 0) {
			goto va_alloc_failed;
		}
	}

	g->hs_credits = nvgpu_kzalloc(g,
				nvgpu_safe_mult_u32(sizeof(struct nvgpu_pma_stream_hs_credits),
						   g->num_pma_channels));
	if (g->hs_credits == NULL) {
		err = -ENOMEM;
		goto hs_credit_alloc_failed;
	}

	nvgpu_atomic_set(&g->hwpm_refcount, 0);

	nvgpu_log(g, gpu_dbg_prof, "initialized");

	return 0;

hs_credit_alloc_failed:
	for (i = 0; i < g->mig.num_gpu_instances; i++) {
		if (g->sys_partition_dg_map != NULL) {
			nvgpu_kfree(g, g->sys_partition_dg_map[i]);
		}
	}
	nvgpu_kfree(g, g->sys_partition_dg_map);

va_alloc_failed:
	nvgpu_pm_free_pmabuf_vas(g);
	g->mm.num_of_perfbuf = 0;
	nvgpu_kfree(g, g->mm.perfbuf);
perfbuf_alloc_failed:
	nvgpu_kfree(g, g->pma_channel_reserved);
pm_reservation_failed:
	for (i = 0; i < g->mig.num_gpu_instances; i++) {
		if (g->pm_reservations[i] == NULL) {
			continue;
		}
		for (j = 0; j < NVGPU_PROFILER_PM_RESOURCE_TYPE_COUNT; j++) {
			nvgpu_mutex_destroy(&g->pm_reservations[i][j].lock);
		}
		nvgpu_kfree(g, g->pm_reservations[i]);
	}
	nvgpu_kfree(g, g->pm_reservations);

	return err;
}

void nvgpu_pm_reservation_deinit(struct gk20a *g)
{
	u32 num_gpu_instances = g->mig.num_gpu_instances;
	u32 i, j;

	if (g->support_gpu_tools == 0U) {
		return;
	}

	nvgpu_kfree(g, g->hs_credits);

	for (i = 0; i < num_gpu_instances; i++) {
		if (g->sys_partition_dg_map != NULL) {
			nvgpu_kfree(g, g->sys_partition_dg_map[i]);
		}
	}
	nvgpu_kfree(g, g->sys_partition_dg_map);

	nvgpu_pm_free_pmabuf_vas(g);
	g->mm.num_of_perfbuf = 0;
	nvgpu_kfree(g, g->mm.perfbuf);

	nvgpu_kfree(g, g->pma_channel_reserved);

	for (i = 0; i < num_gpu_instances; i++) {
		if (g->pm_reservations != NULL && g->pm_reservations[i] != NULL) {
			for (j = 0; j < NVGPU_PROFILER_PM_RESOURCE_TYPE_COUNT; j++) {
				nvgpu_mutex_destroy(&g->pm_reservations[i][j].lock);
			}
			nvgpu_kfree(g, g->pm_reservations[i]);
		}
	}
	nvgpu_kfree(g, g->pm_reservations);
}
