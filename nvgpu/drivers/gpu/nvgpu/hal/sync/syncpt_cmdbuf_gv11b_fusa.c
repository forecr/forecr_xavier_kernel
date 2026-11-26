// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/mm.h>
#include <nvgpu/vm.h>
#include <nvgpu/gmmu.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/dma.h>
#include <nvgpu/lock.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>
#include <nvgpu/nvhost.h>
#include <nvgpu/channel_sync_syncpt.h>

#include <nvgpu/nvgpu_sgt_os.h>
#include "syncpt_cmdbuf_gv11b.h"

/**
 * @brief Maps the syncpoint buffer as read-only in the GPU virtual address space.
 *
 * This function performs the following steps:
 * -# Retrieve the GPU context from the provided VM context using gk20a_from_vm().
 * -# Check if the syncpoint read-only map GPU virtual address is already set.
 *    If it is, the function returns success immediately.
 * -# If the syncpoint read-only map GPU virtual address is not set, attempt to
 *    map the syncpoint memory as read-only using the GPU's MMU by calling
 *    nvgpu_gmmu_map_partial().
 * -# If the mapping fails, log an error message using nvgpu_err() and return
 *    an out-of-memory error code indicated by the macro -#ENOMEM.
 * -# If the mapping is successful, return success.
 *
 * @param [in] vm  The VM context for which the syncpoint buffer is to be mapped.
 *
 * @return 0 if the mapping is successful, or an error code if the mapping fails.
 */
static int set_syncpt_ro_map_gpu_va_locked(struct vm_gk20a *vm)
{
	struct gk20a *g = gk20a_from_vm(vm);
	u32 i = 0U;

	for (i = 0U; i < g->num_nvhost; i++) {
		if (vm->syncpt_ro_map_gpu_va[i] != 0ULL) {
			continue;
		}

		vm->syncpt_ro_map_gpu_va[i] = nvgpu_gmmu_map_partial(vm,
			&g->syncpt_mem[i], g->syncpt_unit_size,
			0, gk20a_mem_flag_read_only,
			false, APERTURE_SYSMEM);

		if (vm->syncpt_ro_map_gpu_va[i] == 0ULL) {
			nvgpu_err(g, "failed to ro map syncpt buffer");
			return -ENOMEM;
		}
	}
	return 0;
}

int gv11b_syncpt_alloc_buf(struct nvgpu_channel *c,
		u32 syncpt_id, struct nvgpu_mem *syncpt_buf)
{
	u64 nr_pages;
	int err = 0;
	struct gk20a *g = c->g;
	u64 syncpt_addr;

	/*
	 * Add ro map for complete sync point shim range in vm
	 * All channels sharing same vm will share same ro mapping.
	 * Create rw map for current channel sync point
	 */
	nvgpu_mutex_acquire(&c->vm->syncpt_ro_map_lock);
	err = set_syncpt_ro_map_gpu_va_locked(c->vm);
	nvgpu_mutex_release(&c->vm->syncpt_ro_map_lock);
	if (err != 0) {
		return err;
	}

	syncpt_addr = nvgpu_nvhost_compute_os_specific_syncpt_addr(g,
			syncpt_id);
	nr_pages = DIV_ROUND_UP(g->syncpt_size, NVGPU_CPU_PAGE_SIZE);
	if (nvgpu_iommuable(g)) {
		err = nvgpu_sgt_os_init_mem_from_iova(g, syncpt_buf,
				syncpt_addr, nr_pages);
		if (err < 0) {
			nvgpu_err(g, "failed to create mem from iova");
			return err;
		}
	} else {
		err = nvgpu_mem_create_from_phys(g, syncpt_buf, syncpt_addr,
				nr_pages);

		if (err < 0) {
			nvgpu_err(g, "failed to create mem from physical addr");
			return err;
		}
	}
	syncpt_buf->gpu_va = nvgpu_gmmu_map_partial(c->vm, syncpt_buf,
			g->syncpt_size, 0, gk20a_mem_flag_none,
			false, APERTURE_SYSMEM);

	if (syncpt_buf->gpu_va == 0ULL) {
		nvgpu_err(g, "failed to map syncpt buffer");
		nvgpu_dma_free(g, syncpt_buf);
		err = -ENOMEM;
	}
	return err;
}

void gv11b_syncpt_free_buf(struct nvgpu_channel *c,
		struct nvgpu_mem *syncpt_buf)
{
	nvgpu_dma_unmap_free(c->vm, syncpt_buf);
}

int gv11b_syncpt_get_sync_ro_map(struct vm_gk20a *vm,
		u64 *base_gpuva, u32 *sync_size, u32 *num_syncpoints)
{
	struct gk20a *g = gk20a_from_vm(vm);
	int err;
	size_t tmp;

	nvgpu_mutex_acquire(&vm->syncpt_ro_map_lock);
	err = set_syncpt_ro_map_gpu_va_locked(vm);
	nvgpu_mutex_release(&vm->syncpt_ro_map_lock);
	if (err != 0) {
		return err;
	}

	base_gpuva[0] = vm->syncpt_ro_map_gpu_va[0];
	if (g->num_nvhost > 1U) {
		base_gpuva[1] = vm->syncpt_ro_map_gpu_va[1];
	}

	*sync_size = g->syncpt_size;

	tmp = (g->syncpt_size != 0UL) ? (g->syncpt_unit_size / g->syncpt_size) : 0U;
	*num_syncpoints = (tmp <= U32_MAX) ? (u32)tmp : U32_MAX;

	return 0;
}
