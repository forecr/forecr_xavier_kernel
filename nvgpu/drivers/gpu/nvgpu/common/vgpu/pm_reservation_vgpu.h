/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PM_RESERVATION_VGPU_H
#define NVGPU_PM_RESERVATION_VGPU_H

#include <nvgpu/types.h>

struct gk20a;
enum nvgpu_profiler_pm_resource_type;
enum nvgpu_profiler_pm_reservation_scope;

int vgpu_pm_reservation_init(struct gk20a *g);

int vgpu_pm_reservation_acquire(struct gk20a *g, u32 gpu_instance_id, u32 reservation_id,
	enum nvgpu_profiler_pm_resource_type pm_resource,
	enum nvgpu_profiler_pm_reservation_scope scope,
	u32 vmid, u32 *pma_channel_id);
int vgpu_pm_reservation_release(struct gk20a *g, u32 gpu_instance_id, u32 reservation_id,
	enum nvgpu_profiler_pm_resource_type pm_resource,
	u32 vmid, u32 pma_channel_id);

#endif /* NVGPU_PM_RESERVATION_VGPU_H */
