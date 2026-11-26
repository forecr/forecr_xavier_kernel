// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bug.h>
#include <nvgpu/vidmem.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/nvgpu_sgt.h>
#include <nvgpu/page_allocator.h>
#include <nvgpu/posix/posix_vidmem.h>

bool nvgpu_addr_is_vidmem_page_alloc(u64 addr)
{
	return ((addr & 1ULL) != 0ULL);
}

void nvgpu_vidmem_set_page_alloc(struct nvgpu_mem_sgl *sgl, u64 addr)
{
	/* set bit 0 to indicate vidmem allocation */
	sgl->dma = (addr | 1ULL);
	sgl->phys = (addr | 1ULL);
}

struct nvgpu_page_alloc *nvgpu_vidmem_get_page_alloc(struct nvgpu_mem_sgl *sgl)
{
	u64 addr;

	addr = sgl->dma;

	if (nvgpu_addr_is_vidmem_page_alloc(addr))
		addr = addr & ~1ULL;
	else
		WARN_ON(true);

	return (struct nvgpu_page_alloc *)(uintptr_t)addr;
}


void nvgpu_mem_free_vidmem_alloc(struct gk20a *g, struct nvgpu_mem *vidmem)
{
	BUG();
}
