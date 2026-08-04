// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/cbc.h>
#include <nvgpu/ce_app.h>
#include <nvgpu/dma.h>
#include <nvgpu/log.h>
#include <nvgpu/string.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/nvgpu_sgt.h>
#include <nvgpu/comptags.h>
#include <nvgpu/fence.h>
#include <nvgpu/soc.h>

void nvgpu_cbc_remove_support(struct gk20a *g)
{
	struct nvgpu_cbc *cbc = g->cbc;

	nvgpu_log_fn(g, " ");

	if (cbc == NULL) {
		return;
	}
#ifdef CONFIG_NVGPU_IVM_BUILD
	nvgpu_cbc_contig_deinit(g);
#endif
	if (nvgpu_mem_is_valid(&cbc->compbit_store.mem)) {
		nvgpu_dma_free(g, &cbc->compbit_store.mem);
		(void) memset(&cbc->compbit_store, 0,
			sizeof(struct compbit_store_desc));
	}
	gk20a_comptag_allocator_destroy(g, &cbc->comp_tags);

	nvgpu_kfree(g, cbc);
	g->cbc = NULL;
}

/*
 * This function is triggered during finalize_poweron multiple times.
 * This function should not return if cbc is not NULL.
 * cbc.init(), which re-writes HW registers that are reset during suspend,
 * should be allowed to execute each time.
 */
int nvgpu_cbc_init_support(struct gk20a *g)
{
	int err = 0;
	struct nvgpu_cbc *cbc = g->cbc;
	bool is_resume = true;

	nvgpu_log_fn(g, " ");

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_COMPRESSION)) {
		return 0;
	}

	/*
	 * If cbc == NULL, the device is being powered-on for the first
	 * time and hence nvgpu_cbc_init_support is not called as part of
	 * suspend/resume cycle, so set is_resume to false.
	 */
	if (cbc == NULL) {
		cbc = nvgpu_kzalloc(g, sizeof(*cbc));
		if (cbc == NULL) {
			return -ENOMEM;
		}
		g->cbc = cbc;

		if (g->ops.cbc.alloc_comptags != NULL) {
			err = g->ops.cbc.alloc_comptags(g, g->cbc);
			if (err != 0) {
				nvgpu_err(g, "Failed to allocate comptags");
				nvgpu_kfree(g, cbc);
				g->cbc = NULL;
				return err;
			}
		}
		is_resume = false;
	}

	if (g->ops.cbc.init != NULL) {
		g->ops.cbc.init(g, g->cbc, is_resume);
	}

	return err;
}

#ifdef CONFIG_NVGPU_COMPRESSION_RAW
static int nvgpu_cbc_scrub_fence_wait(struct gk20a *g,
		struct nvgpu_fence_type *fence_out)
{
	struct nvgpu_timeout timeout;
	bool done;
	int err;

	nvgpu_timeout_init_cpu_timer(g, &timeout, nvgpu_get_poll_timeout(g));

	do {
		err = nvgpu_fence_wait(g, fence_out,
				       nvgpu_get_poll_timeout(g));
		if (err != -ERESTARTSYS) {
			done = true;
		} else if (nvgpu_timeout_expired(&timeout) != 0) {
			done = true;
		} else {
			done = false;
		}
	} while (!done);

	nvgpu_fence_put(fence_out);
	if (err != 0) {
		nvgpu_err(g,
			"fence wait failed for CE execute ops");
		return err;
	}

	return 0;
}

static int nvgpu_scrub_cbc_mem_no_iommu(struct gk20a *g, struct nvgpu_sgt *nvgpu_sgt, u32 ce_ctx_id)
{
	struct nvgpu_fence_type *last_fence = NULL;
	struct nvgpu_fence_type *fence_out = NULL;
	const u32 buffer_scrub_value = 0U;
	void *sgl = NULL;
	u64 size;
	u64 spa;
	int err;


	nvgpu_sgt_for_each_sgl(sgl, nvgpu_sgt) {
		if (last_fence != NULL) {
			nvgpu_fence_put(last_fence);
		}
		spa = nvgpu_sgt_get_phys(g, nvgpu_sgt, sgl);
		size = nvgpu_sgt_get_length(nvgpu_sgt, sgl);

		nvgpu_log(g, gpu_dbg_ce, "SCRUB memory start (spa) 0x%16llx size 0x%16llx",
			  spa, size);

		err = nvgpu_ce_execute_ops(g,
					   ce_ctx_id,
					   spa,
					   size,
					   buffer_scrub_value,
					   &fence_out);
		if (err != 0) {
			return err;
		}
		last_fence = fence_out;
	}

	if (last_fence != NULL)
		err = nvgpu_cbc_scrub_fence_wait(g, last_fence);

	return err;
}
#endif

/*
 * Submit a CE Fill command with scrub enabled to zero out both the physical
 * pages and corresponding cbc backing storage.
 */
int nvgpu_scrub_cbc_mem(struct gk20a *g, struct sg_table *sgt)
{
#ifdef CONFIG_NVGPU_COMPRESSION_RAW
	struct nvgpu_fence_type *fence_out = NULL;
	struct nvgpu_sgt *nvgpu_sgt = NULL;
	const u32 buffer_scrub_value = 0U;
	void *sgl = NULL;
	u32 ce_ctx_id;
	u64 size = 0;
	int err = 0;
	u64 iova;

	ce_ctx_id = g->mm.cbc.ce_ctx_id;
	if (ce_ctx_id == NVGPU_CE_INVAL_CTX_ID) {
		return -EINVAL;
	}

	nvgpu_sgt = nvgpu_linux_sgt_create(g, sgt);
	if (!nvgpu_sgt) {
		return -ENOMEM;
	}

	if (!nvgpu_iommuable(g)) {
		err = nvgpu_scrub_cbc_mem_no_iommu(g, nvgpu_sgt, ce_ctx_id);
		nvgpu_sgt_free(g, nvgpu_sgt);
		if (err != 0)
			nvgpu_err(g,
				  "Failed nvgpu_ce_execute_ops[%d]", err);
		return err;
	}

	iova = nvgpu_sgt_get_gpu_addr(g, nvgpu_sgt, nvgpu_sgt->sgl, NULL);
	/* Find the total buffer size */
	nvgpu_sgt_for_each_sgl(sgl, nvgpu_sgt) {
		size += nvgpu_sgt_get_length(nvgpu_sgt, sgl);
	}

	nvgpu_log(g, gpu_dbg_ce, "SCRUB memory start (iova) 0x%16llx size 0x%16llx",
		   iova, size);
	err = nvgpu_ce_execute_ops(g,
				   ce_ctx_id,
				   iova,
				   size,
				   buffer_scrub_value,
				   &fence_out);
	nvgpu_sgt_free(g, nvgpu_sgt);
	if (err != 0) {
		nvgpu_err(g,
			  "Failed nvgpu_ce_execute_ops[%d]", err);
		return err;
	}

	if (fence_out != NULL)
		err = nvgpu_cbc_scrub_fence_wait(g, fence_out);

	return err;
#else
	(void)sgt;
	(void)g;
	/* scrub is not supported */
	return -ENOSYS;
#endif
}

#ifdef CONFIG_NVGPU_IVM_BUILD
static int nvgpu_init_cbc_mem(struct gk20a *g, u64 pa, u64 size)
{
	u64 nr_pages;
	int err = 0;
	struct nvgpu_cbc *cbc = g->cbc;

	nr_pages = size / NVGPU_CPU_PAGE_SIZE;
	err = nvgpu_mem_create_from_phys(g, &cbc->compbit_store.mem,
		pa, nr_pages);
	return err;
}

static int nvgpu_get_mem_from_contigpool(struct gk20a *g, size_t size)
{
	struct nvgpu_contig_cbcmempool *contig_pool;
	u64 pa;
	int err = 0;

	contig_pool = g->cbc->cbc_contig_mempool;
	if (contig_pool->size < size) {
		return -ENOMEM;
	}

	pa = contig_pool->base_addr;
	err = nvgpu_init_cbc_mem(g, pa, size);
	if (err != 0) {
		return err;
	}

	return 0;

}
#endif

int nvgpu_cbc_alloc(struct gk20a *g, size_t compbit_backing_size,
			bool vidmem_alloc)
{
	struct nvgpu_cbc *cbc = g->cbc;
	int err = 0;
	(void)vidmem_alloc;

	if (nvgpu_mem_is_valid(&cbc->compbit_store.mem) != false) {
		return 0;
	}

#ifdef CONFIG_NVGPU_DGPU
	if (vidmem_alloc == true) {
		/*
		 * Backing store MUST be physically contiguous and allocated in
		 * one chunk
		 * Vidmem allocation API does not support FORCE_CONTIGUOUS like
		 * flag to allocate contiguous memory
		 * But this allocation will happen in vidmem bootstrap allocator
		 * which always allocates contiguous memory
		 */
		return nvgpu_dma_alloc_vid(g,
					 compbit_backing_size,
					 &cbc->compbit_store.mem);
	}
#endif
#ifdef CONFIG_NVGPU_IVM_BUILD
	if (nvgpu_is_hypervisor_mode(g) && !g->is_virtual &&
			(g->ops.cbc.use_contig_pool != NULL)) {
		if (cbc->cbc_contig_mempool == NULL) {
			err = nvgpu_cbc_contig_init(g);
			if (err != 0) {
				nvgpu_err(g, "Contig pool initialization failed");
				return -ENOMEM;
			}
		}

		err = nvgpu_get_mem_from_contigpool(g,
				compbit_backing_size);
		if (err != 0) {
			nvgpu_err(g, "Failed to create mem");
		}
	} else
#endif
	{
		err = nvgpu_dma_alloc_flags_sys(g,
				NVGPU_DMA_PHYSICALLY_ADDRESSED,
				compbit_backing_size,
				&cbc->compbit_store.mem);
		if (err != 0) {
			nvgpu_err(g, "alloc for sysmem desc failed");
		}
	}
	return err;
}
