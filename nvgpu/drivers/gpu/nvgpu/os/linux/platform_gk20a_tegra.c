// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2014-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

/*
 * GK20A Tegra Platform Interface
 */

#include <linux/dma-buf.h>
#include <linux/iommu.h>
#include <nvgpu/vpr.h>
#include <nvgpu/dma.h>

#if NVGPU_VPR_RESIZE_SUPPORTED
#include <linux/platform/tegra/common.h>
#endif

#if defined(CONFIG_NVGPU_NVMAP_NEXT)
#include <linux/nvmap_exports.h>
#endif

#include <nvgpu/kmem.h>
#include <nvgpu/bug.h>
#include <nvgpu/enabled.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/gr/global_ctx.h>
#include <nvgpu/nvhost.h>
#include <nvgpu/linux/dma.h>

#include "platform_gk20a.h"
#include "platform_gk20a_tegra.h"
#include "os_linux.h"

#ifdef CONFIG_NVGPU_VPR
static struct device *gk20a_tegra_choose_vpr_dev(struct gk20a *g)
{
	struct device *dev = dev_from_gk20a(g);
	const int numa_node = dev_to_node(dev);
	if (numa_node == 0 || numa_node == -1) {
		return nvmap_get_vpr_dev();
	} else {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
		return nvmap_get_vpr1_dev();
#else
		return NULL;
#endif
	}
}

static void gk20a_tegra_secure_page_destroy(struct gk20a *g,
				       struct secure_page_buffer *secure_buffer)
{
	struct device *vpr_dev = gk20a_tegra_choose_vpr_dev(g);
#if defined(CONFIG_NVGPU_NVMAP_NEXT)
	if ((nvgpu_iommuable(g)) && (secure_buffer->mapped_iova != 0U)) {
		dma_unmap_resource(vpr_dev, secure_buffer->mapped_iova,
				secure_buffer->size, DMA_BIDIRECTIONAL, 0);
	}
	nvmap_dma_free_attrs(vpr_dev, secure_buffer->size,
			(void *)(uintptr_t)secure_buffer->phys,
			secure_buffer->phys, DMA_ATTR_NO_KERNEL_MAPPING);
#else
	dma_free_attrs(vpr_dev, secure_buffer->size,
			(void *)(uintptr_t)secure_buffer->phys,
			secure_buffer->phys, DMA_ATTR_NO_KERNEL_MAPPING);
#endif

	secure_buffer->destroy = NULL;
}

static void gk20a_free_secure_buffer(struct gk20a *g,
				struct nvgpu_mem *mem)
{
	if (!nvgpu_mem_is_valid(mem))
		return;

	if (mem->priv.sgt != NULL) {
		sg_free_table(mem->priv.sgt);
	}

	nvgpu_kfree(g, mem->priv.sgt);
	mem->priv.sgt = NULL;

	mem->size = 0;
	mem->aligned_size = 0;
	mem->aperture = APERTURE_INVALID;

}

static int gk20a_tegra_secure_alloc(struct gk20a *g,
				struct nvgpu_mem *desc_mem, size_t size,
				global_ctx_mem_destroy_fn *destroy)
{
	struct device *dev = dev_from_gk20a(g);
	struct gk20a_platform *platform = dev_get_drvdata(dev);
	struct secure_page_buffer *secure_buffer = &platform->secure_buffer;
	dma_addr_t phys;
	struct sg_table *sgt;
	struct page *page;
	int err = 0;
	size_t aligned_size = PAGE_ALIGN(size);

	if (nvgpu_mem_is_valid(desc_mem))
		return 0;

	/* We ran out of preallocated memory */
	if (secure_buffer->used + aligned_size > secure_buffer->size) {
		nvgpu_err(platform->g, "failed to alloc %zu bytes of VPR, %zu/%zu used",
				size, secure_buffer->used, secure_buffer->size);
		return -ENOMEM;
	}

	phys = secure_buffer->phys + secure_buffer->used;

	sgt = nvgpu_kzalloc(platform->g, sizeof(*sgt));
	if (!sgt) {
		nvgpu_err(platform->g, "failed to allocate memory");
		return -ENOMEM;
	}
	err = sg_alloc_table(sgt, 1, GFP_KERNEL);
	if (err) {
		nvgpu_err(platform->g, "failed to allocate sg_table");
		goto fail_sgt;
	}
	page = phys_to_page(phys);
	sg_set_page(sgt->sgl, page, size, 0);
	/* This bypasses SMMU for VPR during gmmu_map. */
	if (!nvgpu_iommuable(platform->g)) {
		sg_dma_address(sgt->sgl) = 0;
	}

	*destroy = gk20a_free_secure_buffer;

	desc_mem->priv.sgt = sgt;
	desc_mem->size = size;
	desc_mem->aperture = APERTURE_SYSMEM;

	secure_buffer->used += aligned_size;

	return err;

fail_sgt:
	nvgpu_kfree(platform->g, sgt);
	return err;
}
#endif

void gk20a_tegra_debug_dump(struct device *dev)
{
#ifdef CONFIG_TEGRA_GK20A_NVHOST
	struct gk20a_platform *platform = gk20a_get_platform(dev);
	struct gk20a *g = platform->g;

	if (g->nvhost)
		nvgpu_nvhost_debug_dump_device(g->nvhost);
#endif
}

int gk20a_tegra_busy(struct device *dev)
{
#ifdef CONFIG_TEGRA_GK20A_NVHOST
	struct gk20a_platform *platform = gk20a_get_platform(dev);
	struct gk20a *g = platform->g;

	if (g->nvhost)
		return nvgpu_nvhost_module_busy_ext(g->nvhost);
#endif
	return 0;
}

void gk20a_tegra_idle(struct device *dev)
{
#ifdef CONFIG_TEGRA_GK20A_NVHOST
	struct gk20a_platform *platform = gk20a_get_platform(dev);
	struct gk20a *g = platform->g;

	if (g->nvhost)
		nvgpu_nvhost_module_idle_ext(g->nvhost);
#endif
}

int gk20a_tegra_init_secure_alloc(struct gk20a_platform *platform)
{
#ifdef CONFIG_NVGPU_VPR
	struct gk20a *g = platform->g;
	struct device *vpr_dev = gk20a_tegra_choose_vpr_dev(g);
	struct secure_page_buffer *secure_buffer = &platform->secure_buffer;
	dma_addr_t iova;
	dma_addr_t iova_mapped;

	if (!vpr_dev) {
		nvgpu_log_info(g, "VPR carveout not available");
#if NVGPU_VPR_RESIZE_SUPPORTED
		tegra_unregister_idle_unidle(gk20a_do_idle);
#endif
		return 0;
	}

#if NVGPU_CPU_PAGE_SIZE > 4096
	platform->secure_buffer_size += SZ_64K;
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0)
	vpr_dev->coherent_dma_mask = DMA_BIT_MASK(32);
#endif
#if defined(CONFIG_NVGPU_NVMAP_NEXT)
	(void)nvmap_dma_alloc_attrs(vpr_dev,
				    platform->secure_buffer_size, &iova,
				    GFP_KERNEL, DMA_ATTR_NO_KERNEL_MAPPING);
	if (nvgpu_iommuable(g)) {
		iova_mapped = dma_map_resource(dev_from_gk20a(g), iova,
			platform->secure_buffer_size, DMA_BIDIRECTIONAL, 0);
		if (dma_mapping_error(vpr_dev, iova_mapped)) {
#if NVGPU_VPR_RESIZE_SUPPORTED
			tegra_unregister_idle_unidle(gk20a_do_idle);
#endif
			return 0;
		}
		secure_buffer->mapped_iova = iova_mapped;
	}

#else
	(void)dma_alloc_attrs(vpr_dev, platform->secure_buffer_size, &iova,
				      GFP_KERNEL, DMA_ATTR_NO_KERNEL_MAPPING);
#endif
	/* Some platforms disable VPR. In that case VPR allocations always
	 * fail. Just disable VPR usage in nvgpu in that case. */
	if (dma_mapping_error(vpr_dev, iova)) {
#if NVGPU_VPR_RESIZE_SUPPORTED
		tegra_unregister_idle_unidle(gk20a_do_idle);
#endif
		return 0;
	}

	secure_buffer->size = platform->secure_buffer_size;
	if (!nvgpu_iommuable(g)) {
		secure_buffer->phys = iova;
	} else {
		secure_buffer->phys = iova_mapped;
	}
	secure_buffer->destroy = gk20a_tegra_secure_page_destroy;

	g->ops.secure_alloc = gk20a_tegra_secure_alloc;
	nvgpu_set_enabled(g, NVGPU_SUPPORT_VPR, true);
#endif
	return 0;
}
