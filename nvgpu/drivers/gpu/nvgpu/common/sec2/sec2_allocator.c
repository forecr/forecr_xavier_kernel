// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/sec2/allocator.h>
#include <nvgpu/allocator.h>
#include <nvgpu/sec2/msg.h>

int nvgpu_sec2_dmem_allocator_init(struct gk20a *g,
				struct nvgpu_allocator *dmem,
				struct sec2_init_msg_sec2_init *sec2_init)
{
	int err = 0;
	if (!nvgpu_alloc_initialized(dmem)) {
		/* Align start and end addresses */
		u32 start = NVGPU_ALIGN(sec2_init->nv_managed_area_offset,
			PMU_DMEM_ALLOC_ALIGNMENT);

		u32 end = (sec2_init->nv_managed_area_offset +
			sec2_init->nv_managed_area_size) &
			~(PMU_DMEM_ALLOC_ALIGNMENT - 1U);
		u32 size = end - start;

		err = nvgpu_allocator_init(g, dmem, NULL, "sec2_dmem", start,
				size, PMU_DMEM_ALLOC_ALIGNMENT, 0ULL, 0ULL,
				BITMAP_ALLOCATOR);
		if (err != 0) {
			nvgpu_err(g, "Couldn't init sec2_dmem allocator\n");
		}
	}
	return err;
}

void nvgpu_sec2_dmem_allocator_destroy(struct nvgpu_allocator *dmem)
{
	if (nvgpu_alloc_initialized(dmem)) {
		nvgpu_alloc_destroy(dmem);
	}
}
