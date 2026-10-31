// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/trace.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>
#include <nvgpu/fifo.h>
#include <nvgpu/fifo/userd.h>
#include <nvgpu/vm_area.h>
#include <nvgpu/dma.h>

int nvgpu_userd_init_slabs(struct gk20a *g)
{
	struct nvgpu_fifo *f = &g->fifo;
	int err;

	nvgpu_mutex_init(&f->userd_mutex);

	f->num_channels_per_slab = NVGPU_CPU_PAGE_SIZE /  g->ops.userd.entry_size(g);
	f->num_userd_slabs =
		DIV_ROUND_UP(f->num_channels, f->num_channels_per_slab);

	f->userd_slabs = nvgpu_big_zalloc(g, f->num_userd_slabs *
					sizeof(struct nvgpu_mem));
	if (f->userd_slabs == NULL) {
		nvgpu_err(g, "could not allocate userd slabs");
		err = -ENOMEM;
		goto clean_up;
	}

	return 0;

clean_up:
	nvgpu_mutex_destroy(&f->userd_mutex);

	return err;
}

void nvgpu_userd_free_slabs(struct gk20a *g)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 slab;

	for (slab = 0; slab < f->num_userd_slabs; slab++) {
		nvgpu_dma_free(g, &f->userd_slabs[slab]);
	}
	nvgpu_big_free(g, f->userd_slabs);
	f->userd_slabs = NULL;

	nvgpu_mutex_destroy(&f->userd_mutex);
}

int nvgpu_userd_init_channel(struct gk20a *g, struct nvgpu_channel *c)
{
	struct nvgpu_fifo *f = &g->fifo;
	struct nvgpu_mem *mem;
	u32 slab = c->chid / f->num_channels_per_slab;
	int err = 0;

	if (slab > f->num_userd_slabs) {
		nvgpu_err(g, "chid %u, slab %u out of range (max=%u)",
			c->chid, slab,  f->num_userd_slabs);
		return -EINVAL;
	}

	mem = &g->fifo.userd_slabs[slab];

	nvgpu_mutex_acquire(&f->userd_mutex);
	if (!nvgpu_mem_is_valid(mem)) {
		err = nvgpu_dma_alloc_sys(g, NVGPU_CPU_PAGE_SIZE, mem);
		if (err != 0) {
			nvgpu_err(g, "userd allocation failed, err=%d", err);
			goto done;
		}

		if (g->ops.mm.is_bar1_supported(g)) {
			mem->gpu_va = g->ops.mm.bar1_map_userd(g, mem,
							 slab * NVGPU_CPU_PAGE_SIZE);
		}
	}
	c->userd_mem = mem;
	c->userd_offset = (c->chid % f->num_channels_per_slab) *
				g->ops.userd.entry_size(g);
	c->userd_iova = nvgpu_channel_userd_addr(c);

	nvgpu_log(g, gpu_dbg_info,
		"chid=%u slab=%u mem=%p offset=%u addr=%llx gpu_va=%llx",
		c->chid, slab, mem, c->userd_offset,
		nvgpu_channel_userd_addr(c),
		nvgpu_channel_userd_gpu_va(c));

done:
	nvgpu_mutex_release(&f->userd_mutex);
	return err;
}

int nvgpu_userd_setup_sw(struct gk20a *g)
{
	struct nvgpu_fifo *f = &g->fifo;
	int err;
	u32 size, num_pages;

	err = nvgpu_userd_init_slabs(g);
	if (err != 0) {
		nvgpu_err(g, "failed to init userd support");
		return err;
	}

	size = f->num_channels * g->ops.userd.entry_size(g);
	num_pages = DIV_ROUND_UP(size, NVGPU_CPU_PAGE_SIZE);
	err = nvgpu_vm_area_alloc(g->mm.bar1.vm,
			num_pages, NVGPU_CPU_PAGE_SIZE, &f->userd_gpu_va, 0);
	if (err != 0) {
		nvgpu_err(g, "userd gpu va allocation failed, err=%d", err);
		goto clean_up;
	}

	return 0;

clean_up:
	nvgpu_userd_free_slabs(g);

	return err;
}

void nvgpu_userd_cleanup_sw(struct gk20a *g)
{
	struct nvgpu_fifo *f = &g->fifo;

	if (f->userd_gpu_va != 0ULL) {
		(void) nvgpu_vm_area_free(g->mm.bar1.vm, f->userd_gpu_va);
		f->userd_gpu_va = 0ULL;
	}

	nvgpu_userd_free_slabs(g);
}
