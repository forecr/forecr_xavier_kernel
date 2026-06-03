// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/pmu/allocator.h>
#include <nvgpu/allocator.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/pmu.h>
#include <nvgpu/pmu/fw.h>
#include <nvgpu/dma.h>

void nvgpu_pmu_allocator_dmem_init(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_allocator *dmem,
	union pmu_init_msg_pmu *init)
{
	struct pmu_fw_ver_ops *fw_ops = &pmu->fw->ops;

	if (!nvgpu_alloc_initialized(dmem)) {
		/* Align start and end addresses */
		u32 start =
			NVGPU_ALIGN(U32(fw_ops->get_init_msg_sw_mngd_area_off(init)),
			PMU_DMEM_ALLOC_ALIGNMENT);
		u32 end = (U32(fw_ops->get_init_msg_sw_mngd_area_off(init)) +
			U32(fw_ops->get_init_msg_sw_mngd_area_size(init))) &
			~(PMU_DMEM_ALLOC_ALIGNMENT - 1U);
		u32 size = end - start;

		if (size != 0U) {
			nvgpu_allocator_init(g, dmem, NULL, "gk20a_pmu_dmem",
				start, size, PMU_DMEM_ALLOC_ALIGNMENT, 0ULL, 0ULL,
				BITMAP_ALLOCATOR);
		} else {
			dmem->priv = NULL;
		}
	}
}

void nvgpu_pmu_allocator_dmem_destroy(struct nvgpu_allocator *dmem)
{
	if (nvgpu_alloc_initialized(dmem)) {
		nvgpu_alloc_destroy(dmem);
	}
}

void nvgpu_pmu_allocator_surface_free(struct gk20a *g, struct nvgpu_mem *mem)
{
	if (nvgpu_mem_is_valid(mem)) {
		nvgpu_dma_free(g, mem);
	}
}

void nvgpu_pmu_allocator_surface_describe(struct gk20a *g, struct nvgpu_mem *mem,
		struct flcn_mem_desc_v0 *fb)
{
	u64 vaddr;

	(void)g;
	vaddr = nvgpu_safe_add_u64(NV_NEXT_CORE_AMAP_EXTMEM3_START,
			mem->gpu_va);

	fb->address.lo = u64_lo32(vaddr);
	fb->address.hi = u64_hi32(vaddr);
	fb->params = ((u32)mem->size & 0x1FFFFFFFU);
	fb->params |= (GK20A_PMU_DMAIDX_VIRT << 29U);
}

int nvgpu_pmu_allocator_sysmem_surface_alloc(struct gk20a *g,
		struct nvgpu_mem *mem, u32 size)
{
	struct mm_gk20a *mm = &g->mm;
	struct vm_gk20a *vm = mm->pmu.vm;
	int err;

	err = nvgpu_dma_alloc_map_sys(vm, size, mem);
	if (err != 0) {
		nvgpu_err(g, "failed to allocate memory\n");
		return -ENOMEM;
	}

	return 0;
}
