/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GB10B_REGOPS_ALLOWLIST_H
#define NVGPU_GB10B_REGOPS_ALLOWLIST_H

#include <nvgpu/types.h>
#include <nvgpu/pm_reservation.h>

struct gk20a;
struct nvgpu_profiler_object;
struct nvgpu_pm_resource_register_range;

bool gb10b_is_hwpm_pma_reg_context_switched(void);

int gb10b_init_reg_op_type_per_register_type(struct gk20a *g, struct nvgpu_profiler_object *prof,
						enum nvgpu_profiler_pm_resource_type pm_resource);
u32
gb10b_add_bitvector_to_reg_ranges(struct gk20a *g, const u32 *bitvector, u32 count,
				 u32 offset, struct nvgpu_pm_resource_register_range *ranges);
const struct nvgpu_pm_resource_register_range
		*gb10b_get_hwpm_perfmon_bitvector_register_ranges(struct gk20a *g,
					struct nvgpu_profiler_object *prof, u32 *count);
const struct nvgpu_pm_resource_register_range
		*gb10b_get_hwpm_router_bitvector_register_ranges(struct gk20a *g,
					struct nvgpu_profiler_object *prof, u32 *count);
const struct nvgpu_pm_resource_register_range
		*gb10b_get_hwpm_pma_channel_bitvector_register_ranges(struct gk20a *g,
					u32 *count);
const struct nvgpu_pm_resource_register_range
		*gb10b_get_hwpm_pma_trigger_bitvector_register_ranges(struct gk20a *g,
					u32 *count);
const struct nvgpu_pm_resource_register_range
		*gb10b_get_hwpm_smpc_bitvector_register_ranges(struct gk20a *g,
					struct nvgpu_profiler_object *prof, u32 *count);
const struct nvgpu_pm_resource_register_range
		*gb10b_get_hwpm_cau_bitvector_register_ranges(struct gk20a *g,
					struct nvgpu_profiler_object *prof, u32 *count);
const struct nvgpu_pm_resource_register_range
		*gb10b_get_hwpm_hes_bitvector_register_ranges(struct gk20a *g,
					u32 *count);

#endif /* NVGPU_GB10B_REGOPS_ALLOWLIST_H */
