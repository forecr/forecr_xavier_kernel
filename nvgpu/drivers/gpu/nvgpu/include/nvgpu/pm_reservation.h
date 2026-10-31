/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PM_RESERVATION_H
#define NVGPU_PM_RESERVATION_H

#ifdef CONFIG_NVGPU_PROFILER

#include <nvgpu/list.h>
#include <nvgpu/lock.h>

struct gk20a;

enum nvgpu_profiler_pm_reservation_scope {
	NVGPU_PROFILER_PM_RESERVATION_SCOPE_DEVICE,
	NVGPU_PROFILER_PM_RESERVATION_SCOPE_CONTEXT,
};

enum nvgpu_profiler_pm_resource_type {
	NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY,
	NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC,
	NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM,
	NVGPU_PROFILER_PM_RESOURCE_TYPE_PC_SAMPLER,
	NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES,
	NVGPU_PROFILER_PM_RESOURCE_TYPE_CCU_PROF,
	NVGPU_PROFILER_PM_RESOURCE_TYPE_COUNT,
};

struct nvgpu_pm_resource_reservation_entry {
	struct nvgpu_list_node entry;

	u32 reservation_id;
	u32 vmid;
	/* valid only for the PMA_STREAM_TYPE reservation entries */
	u32 pma_channel_id;
	enum nvgpu_profiler_pm_reservation_scope scope;
};

static inline struct nvgpu_pm_resource_reservation_entry *
nvgpu_pm_resource_reservation_entry_from_entry(struct nvgpu_list_node *node)
{
	return (struct nvgpu_pm_resource_reservation_entry *)
		((uintptr_t)node - offsetof(struct nvgpu_pm_resource_reservation_entry, entry));
}

struct nvgpu_pm_resource_reservations {
	struct nvgpu_list_node head;
	u32 count;
	struct nvgpu_mutex lock;
};

int nvgpu_pm_reservation_init(struct gk20a *g);
void nvgpu_pm_reservation_deinit(struct gk20a *g);

int nvgpu_pm_reservation_acquire(struct gk20a *g, u32 gpu_instance_id,
	u32 reservation_id, enum nvgpu_profiler_pm_resource_type pm_resource,
	enum nvgpu_profiler_pm_reservation_scope scope,
	u32 vmid, u32 *pma_channel_id);
int nvgpu_pm_reservation_release(struct gk20a *g, u32 gpu_instance_id,
	u32 reservation_id, enum nvgpu_profiler_pm_resource_type pm_resource,
	u32 vmid, u32 pma_channel_id);
void nvgpu_pm_reservation_release_all_per_vmid(struct gk20a *g,
					u32 gpu_instance_id, u32 vmid);
bool check_pm_resource_existing_reservation_locked(
		struct nvgpu_pm_resource_reservations *reservations,
		u32 reservation_id, u32 vmid);

#endif /* CONFIG_NVGPU_PROFILER */
#endif /* NVGPU_PM_RESERVATION_H */
