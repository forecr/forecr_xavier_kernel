/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved. */

#ifndef NVGPU_GB20C_REGOPS_ALLOWLIST_H
#define NVGPU_GB20C_REGOPS_ALLOWLIST_H

#include <nvgpu/types.h>
#include <nvgpu/pm_reservation.h>

struct gk20a;
struct nvgpu_profiler_object;
struct nvgpu_pm_resource_register_range_map;

int gb20c_init_reg_op_type_per_register_type(struct gk20a *g, struct nvgpu_profiler_object *prof,
						enum nvgpu_profiler_pm_resource_type pm_resource);
const struct nvgpu_pm_resource_register_range
		*gb20c_get_hwpm_ccuprof_bitvector_register_ranges(struct gk20a *g,
					struct nvgpu_profiler_object *prof, u32 *count);
void
gb20c_allowlist_misc_register_ranges(struct gk20a *g, struct nvgpu_profiler_object *prof,
				    struct nvgpu_pm_resource_register_range_map *map,
				    u32 *map_index, u32 *count);

#endif /* NVGPU_GB20C_REGOPS_ALLOWLIST_H */
