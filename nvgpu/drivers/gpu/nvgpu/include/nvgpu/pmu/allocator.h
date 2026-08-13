/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_ALLOCATOR_H
#define NVGPU_PMU_ALLOCATOR_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_pmu;
struct nvgpu_mem;
struct nvgpu_allocator;
union pmu_init_msg_pmu;
struct flcn_mem_desc_v0;

void nvgpu_pmu_allocator_dmem_init(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_allocator *dmem,
	union pmu_init_msg_pmu *init);
void nvgpu_pmu_allocator_dmem_destroy(struct nvgpu_allocator *dmem);

void nvgpu_pmu_allocator_surface_free(struct gk20a *g, struct nvgpu_mem *mem);
void nvgpu_pmu_allocator_surface_describe(struct gk20a *g, struct nvgpu_mem *mem,
		struct flcn_mem_desc_v0 *fb);
int nvgpu_pmu_allocator_sysmem_surface_alloc(struct gk20a *g,
		struct nvgpu_mem *mem, u32 size);
#endif /* NVGPU_PMU_ALLOCATOR_H */
