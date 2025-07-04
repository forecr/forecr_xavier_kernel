// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2014-2024, NVIDIA CORPORATION. All rights reserved.
 */

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/nvmap.h>
#include <linux/version.h>
#include <linux/kmemleak.h>
#include <linux/io.h>

#if defined(NVMAP_LOADABLE_MODULE)
#include <linux/nvmap_t19x.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
#include <linux/sched/clock.h>
#endif

#include <linux/cma.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#include <linux/dma-map-ops.h>
#else
#include <linux/dma-contiguous.h>
#include <asm/dma-contiguous.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#include "include/linux/nvmap_exports.h"
#endif

#include "nvmap_priv.h"

#ifdef CONFIG_TEGRA_VIRTUALIZATION
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0)
#include <linux/tegra-ivc.h>
#else
#include <soc/tegra/virt/hv-ivc.h>
#endif
#include <soc/tegra/virt/syscalls.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#ifdef CONFIG_ARM_DMA_IOMMU_ALIGNMENT
#define DMA_BUF_ALIGNMENT CONFIG_ARM_DMA_IOMMU_ALIGNMENT
#else
#define DMA_BUF_ALIGNMENT 8
#endif
#endif /* LINUX_VERSION_CODE */

#ifndef NVMAP_UPSTREAM_KERNEL
#ifndef NVMAP_CONFIG_VPR_RESIZE
extern phys_addr_t tegra_vpr_start;
extern phys_addr_t tegra_vpr_size;
extern bool tegra_vpr_resize;
#endif /* NVMAP_CONFIG_VPR_RESIZE */
#endif /* !NVMAP_UPSTREAM_KERNEL */

struct device __weak tegra_generic_dev;

struct device __weak tegra_vpr_dev;
EXPORT_SYMBOL(tegra_vpr_dev);

struct device __weak tegra_generic_cma_dev;
struct device __weak tegra_vpr_cma_dev;

#ifdef NVMAP_LOADABLE_MODULE
static struct platform_device *pdev;
#endif /* NVMAP_LOADABLE_MODULE */

#ifdef NVMAP_CONFIG_VPR_RESIZE
struct dma_resize_notifier_ops __weak vpr_dev_ops;

static struct dma_declare_info generic_dma_info = {
        .name = "generic",
        .size = 0,
        .notifier.ops = NULL,
};

static struct dma_declare_info vpr_dma_info = {
	.name = "vpr",
	.size = SZ_32M,
	.notifier.ops = &vpr_dev_ops,
};
#endif

const struct of_device_id nvmap_of_ids[] = {
	{ .compatible = "nvidia,carveouts" },
	{ .compatible = "nvidia,carveouts-t18x" },
	{ .compatible = "nvidia,carveouts-t19x" },
        { }
};
MODULE_DEVICE_TABLE(of, nvmap_of_ids);

static struct nvmap_platform_carveout nvmap_carveouts[] = {
	[0] = {
		.name		= "generic-0",
		.usage_mask	= NVMAP_HEAP_CARVEOUT_GENERIC,
		.base		= 0,
		.size		= 0,
		.dma_dev	= &tegra_generic_dev,
		.cma_dev	= &tegra_generic_cma_dev,
#ifdef NVMAP_CONFIG_VPR_RESIZE
		.dma_info	= &generic_dma_info,
#endif
	},
	[1] = {
		.name		= "vpr",
		.usage_mask	= NVMAP_HEAP_CARVEOUT_VPR,
		.base		= 0,
		.size		= 0,
		.dma_dev	= &tegra_vpr_dev,
		.cma_dev	= &tegra_vpr_cma_dev,
#ifdef NVMAP_CONFIG_VPR_RESIZE
		.dma_info	= &vpr_dma_info,
#endif
		.enable_static_dma_map = true,
	},
	[2] = {
		.name		= "vidmem",
		.usage_mask	= NVMAP_HEAP_CARVEOUT_VIDMEM,
		.base		= 0,
		.size		= 0,
		.disable_dynamic_dma_map = true,
		.no_cpu_access = true,
	},
	[3] = {
		.name		= "fsi",
		.usage_mask	= NVMAP_HEAP_CARVEOUT_FSI,
		.base		= 0,
		.size		= 0,
	},
	[4] = {
		.name		= "gpu",
		.usage_mask	= NVMAP_HEAP_CARVEOUT_GPU,
		.base		= 0,
		.size		= 0,
	},
	/* Need uninitialized entries for IVM carveouts */
	[5] = {
		.name		= NULL,
		.usage_mask	= NVMAP_HEAP_CARVEOUT_IVM,
	},
	[6] = {
		.name		= NULL,
		.usage_mask	= NVMAP_HEAP_CARVEOUT_IVM,
	},
	[7] = {
		.name		= NULL,
		.usage_mask	= NVMAP_HEAP_CARVEOUT_IVM,
	},
	[8] = {
		.name		= NULL,
		.usage_mask	= NVMAP_HEAP_CARVEOUT_IVM,
	},
};

static struct nvmap_platform_data nvmap_data = {
	.carveouts	= nvmap_carveouts,
	.nr_carveouts	= 5,
};

static struct nvmap_platform_carveout *nvmap_get_carveout_pdata(const char *name)
{
	struct nvmap_platform_carveout *co;
	for (co = nvmap_carveouts;
	     co < nvmap_carveouts + ARRAY_SIZE(nvmap_carveouts); co++) {
		int i = min_t(int, strcspn(name, "_"), strcspn(name, "-"));
		/* handle IVM carveouts */
		if ((co->usage_mask == NVMAP_HEAP_CARVEOUT_IVM) &&  !co->name)
			goto found;

		if (strncmp(co->name, name, i))
			continue;
found:
		co->dma_dev = co->dma_dev ? co->dma_dev : &co->dev;
		return co;
	}
	pr_err("not enough space for all nvmap carveouts\n");
	return NULL;
}

int nvmap_register_vidmem_carveout(struct device *dma_dev,
				phys_addr_t base, size_t size)
{
	struct nvmap_platform_carveout *vidmem_co;

	if (!base || !size || (base != PAGE_ALIGN(base)) ||
	    (size != PAGE_ALIGN(size)))
		return -EINVAL;

	vidmem_co = nvmap_get_carveout_pdata("vidmem");
	if (!vidmem_co)
		return -ENODEV;

	if (vidmem_co->base || vidmem_co->size)
		return -EEXIST;

	vidmem_co->base = base;
	vidmem_co->size = size;
	if (dma_dev)
		vidmem_co->dma_dev = dma_dev;
	return nvmap_create_carveout(vidmem_co);
}
EXPORT_SYMBOL(nvmap_register_vidmem_carveout);

#ifdef CONFIG_TEGRA_VIRTUALIZATION
static int __init nvmap_populate_ivm_carveout(struct device *dev)
{
	char *name;
	const __be32 *prop;
	int ret = 0;
	struct nvmap_platform_carveout *co;
	struct of_phandle_iterator it;
	struct tegra_hv_ivm_cookie *ivm;
	unsigned long long id;
	unsigned int guestid;

	if (!of_phandle_iterator_init(&it, dev->of_node, "memory-region", NULL, 0)) {
		while (!of_phandle_iterator_next(&it) && it.node) {
			if (of_device_is_available(it.node) &&
			    of_device_is_compatible(it.node, "nvidia,ivm_carveout") > 0) {
				co = nvmap_get_carveout_pdata("nvidia,ivm_carveout");
				if (!co) {
					ret = -ENOMEM;
					goto err;
				}

				if (hyp_read_gid(&guestid)) {
					pr_err("failed to read gid\n");
					ret = -EINVAL;
					goto err;
				}

				prop = of_get_property(it.node, "ivm", NULL);
				if (!prop) {
					pr_err("failed to read ivm property\n");
					ret = -EINVAL;
					goto err;
				}

				id = of_read_number(prop + 1, 1);
				if (id > UINT_MAX) {
					ret = -EINVAL;
					goto err;
				}
				ivm = tegra_hv_mempool_reserve(id);
				if (IS_ERR_OR_NULL(ivm)) {
					pr_err("failed to reserve IVM memory pool %llu\n", id);
					ret = -ENOMEM;
					goto err;
				}
				/* XXX: Are these the available fields from IVM cookie? */
				co->base     = (phys_addr_t)ivm->ipa;
				co->peer     = ivm->peer_vmid;
				co->size     = ivm->size;
				co->vmid     = guestid;

				if (!co->base || !co->size) {
					ret = -EINVAL;
					goto fail;
				}

				/* See if this VM can allocate (or just create handle from ID)
				 * generated by peer partition
				 */
				prop = of_get_property(it.node, "alloc", NULL);
				if (!prop) {
					pr_err("failed to read alloc property\n");
					ret = -EINVAL;
					goto fail;
				}

				name = kzalloc(32, GFP_KERNEL);
				if (!name) {
					ret = -ENOMEM;
					goto fail;
				}

				co->can_alloc = of_read_number(prop, 1);
				co->is_ivm    = true;
				sprintf(name, "ivm%02u%02u%02d", co->vmid, co->peer, co->can_alloc);
				pr_info("IVM carveout IPA:%p, size=%zu, peer vmid=%u, name=%s\n",
					(void *)(uintptr_t)co->base, co->size, co->peer, name);
				co->name      = name;
				nvmap_data.nr_carveouts++;

			}
		}
	}
	return 0;
fail:
	co->base     = 0;
	co->peer     = 0;
	co->size     = 0;
	co->vmid     = 0;
err:
	return ret;

}
#endif /* CONFIG_TEGRA_VIRTUALIZATION */

/*
 * This requires proper kernel arguments to have been passed.
 */
#ifndef NVMAP_UPSTREAM_KERNEL
static int __nvmap_init_legacy(struct device *dev)
{
#ifndef NVMAP_CONFIG_VPR_RESIZE
	/* VPR */
	if (!nvmap_carveouts[1].base) {
		nvmap_carveouts[1].base = tegra_vpr_start;
		nvmap_carveouts[1].size = tegra_vpr_size;
		nvmap_carveouts[1].cma_dev = NULL;
	}
#endif /* NVMAP_CONFIG_VPR_RESIZE */

	return 0;
}
#endif /* !NVMAP_UPSTREAM_KERNEL */

static int __nvmap_init_dt(struct platform_device *pdev)
{
	if (!of_match_device(nvmap_of_ids, &pdev->dev)) {
		pr_err("Missing DT entry!\n");
		return -EINVAL;
	}

#ifndef NVMAP_UPSTREAM_KERNEL
	/* For VM_2 we need carveout. So, enabling it here */
	__nvmap_init_legacy(&pdev->dev);
#endif /* !NVMAP_UPSTREAM_KERNEL */

	pdev->dev.platform_data = &nvmap_data;

	return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
static inline struct page **nvmap_kvzalloc_pages(u32 count)
{
	if (count * sizeof(struct page *) <= PAGE_SIZE)
		return kzalloc(count * sizeof(struct page *), GFP_KERNEL);
	else
		return vzalloc(count * sizeof(struct page *));
}

static void *__nvmap_dma_alloc_from_coherent(struct device *dev,
					     struct dma_coherent_mem_replica *mem,
					     size_t size,
					     dma_addr_t *dma_handle,
					     unsigned long attrs,
					     unsigned long start)
{
	int order = get_order(size);
	unsigned long flags;
	unsigned int count = 0, i = 0, j = 0, k = 0;
	unsigned int alloc_size;
	unsigned long align, pageno, page_count, first_pageno;
	void *addr = NULL;
	struct page **pages = NULL;
	int do_memset = 0;
	int *bitmap_nos = NULL;
	const char *device_name;
	bool is_gpu = false;
	u32 granule_size = 0;

	device_name = dev_name(dev);
	if (!device_name) {
		pr_err("Could not get device_name\n");
		return NULL;
	}

	if (!strncmp(device_name, "gpu", 3)) {
		struct nvmap_platform_carveout *co;

		is_gpu = true;
		co = nvmap_get_carveout_pdata("gpu");
		granule_size = co->granule_size;
	}

	if (is_gpu) {
		/* Calculation for Gpu carveout should consider granule size */
		count = size >> PAGE_SHIFT_GRANULE(granule_size);
	} else {
		if (dma_get_attr(DMA_ATTR_ALLOC_EXACT_SIZE, attrs)) {
			page_count = PAGE_ALIGN(size) >> PAGE_SHIFT;
			if (page_count > UINT_MAX) {
				dev_err(dev, "Page count more than max value\n");
				return NULL;
			}
			count = (unsigned int)page_count;
		} else
			count = 1 << order;
	}

	if (!count)
		return NULL;

	bitmap_nos = vzalloc(count * sizeof(int));
	if (!bitmap_nos) {
		dev_err(dev, "failed to allocate memory\n");
		return NULL;
	}
	if ((mem->flags & DMA_MEMORY_NOMAP) &&
	    dma_get_attr(DMA_ATTR_ALLOC_SINGLE_PAGES, attrs)) {
		alloc_size = 1;
		/* pages contain the array of pages of kernel PAGE_SIZE */
		if (!is_gpu)
			pages = nvmap_kvzalloc_pages(count);
		else
			pages = nvmap_kvzalloc_pages(count * PAGES_PER_GRANULE(granule_size));

		if (!pages) {
			kvfree(bitmap_nos);
			return NULL;
		}
	} else {
		alloc_size = count;
	}

	spin_lock_irqsave(&mem->spinlock, flags);

	if (!is_gpu && unlikely(size > ((u64)mem->size << PAGE_SHIFT)))
		goto err;
	else if (is_gpu &&
		 unlikely(size > ((u64)mem->size << PAGE_SHIFT_GRANULE(granule_size))))
		goto err;

	if (((mem->flags & DMA_MEMORY_NOMAP) &&
	    dma_get_attr(DMA_ATTR_ALLOC_SINGLE_PAGES, attrs)) ||
	    is_gpu) {
		align = 0;
	} else  {
		if (order > DMA_BUF_ALIGNMENT)
			align = (1 << DMA_BUF_ALIGNMENT) - 1;
		else
			align = (1 << order) - 1;
	}

	while (count) {
		pageno = bitmap_find_next_zero_area(mem->bitmap, mem->size,
						    start, alloc_size, align);

		if (pageno >= mem->size)
			goto err;

		if (!i)
			first_pageno = pageno;

		count -= alloc_size;
		if (pages) {
			if (!is_gpu)
				pages[i++] = pfn_to_page(mem->pfn_base + pageno);
			else {
				/* Handle granules */
				for (k = 0; k < (alloc_size * PAGES_PER_GRANULE(granule_size)); k++)
					pages[i++] = pfn_to_page(mem->pfn_base + pageno *
								 PAGES_PER_GRANULE(granule_size) + k);
			}
		}

		bitmap_set(mem->bitmap, pageno, alloc_size);
		bitmap_nos[j++] = pageno;
	}

	/*
	 * Memory was found in the coherent area.
	 */
	if (!is_gpu)
		*dma_handle = mem->device_base + (first_pageno << PAGE_SHIFT);
	else
		*dma_handle = mem->device_base + (first_pageno << PAGE_SHIFT_GRANULE(granule_size));

	if (!(mem->flags & DMA_MEMORY_NOMAP)) {
		addr = mem->virt_base + (first_pageno << PAGE_SHIFT);
		do_memset = 1;
	} else if (dma_get_attr(DMA_ATTR_ALLOC_SINGLE_PAGES, attrs)) {
		addr = pages;
	}

	spin_unlock_irqrestore(&mem->spinlock, flags);

	if (do_memset)
		memset(addr, 0, size);

	kvfree(bitmap_nos);
	return addr;
err:
	while (j--)
		bitmap_clear(mem->bitmap, bitmap_nos[j], alloc_size);

	spin_unlock_irqrestore(&mem->spinlock, flags);
	kvfree(pages);
	kvfree(bitmap_nos);
	return ERR_PTR(-ENOMEM);
}

void *nvmap_dma_alloc_attrs(struct device *dev, size_t size,
			    dma_addr_t *dma_handle,
			    gfp_t flag, unsigned long attrs)
{
	struct dma_coherent_mem_replica *mem;

	if (!dev || !dev->dma_mem)
		return NULL;

	WARN_ON_ONCE(!dev->coherent_dma_mask);

	mem = (struct dma_coherent_mem_replica *)(dev->dma_mem);

	return __nvmap_dma_alloc_from_coherent(dev, mem, size, dma_handle,
						   attrs, 0);
}
EXPORT_SYMBOL(nvmap_dma_alloc_attrs);

void nvmap_dma_free_attrs(struct device *dev, size_t size, void *cpu_addr,
			  dma_addr_t dma_handle, unsigned long attrs)
{
	void *mem_addr;
	unsigned long flags;
	unsigned int pageno, page_shift_val;
	struct dma_coherent_mem_replica *mem;
	bool is_gpu = false;
	const char *device_name;
	u32 granule_size = 0;

	if (!dev || !dev->dma_mem)
		return;

	device_name = dev_name(dev);
	if (!device_name) {
		pr_err("Could not get device_name\n");
		return;
	}

	if (!strncmp(device_name, "gpu", 3)) {
		struct nvmap_platform_carveout *co;

		is_gpu = true;
		co = nvmap_get_carveout_pdata("gpu");
		granule_size = co->granule_size;
	}

	mem = (struct dma_coherent_mem_replica *)(dev->dma_mem);
	if ((mem->flags & DMA_MEMORY_NOMAP) &&
	    dma_get_attr(DMA_ATTR_ALLOC_SINGLE_PAGES, attrs)) {
		struct page **pages = cpu_addr;
		int i;

		spin_lock_irqsave(&mem->spinlock, flags);
		if (!is_gpu) {
			for (i = 0; i < (size >> PAGE_SHIFT); i++) {
				pageno = page_to_pfn(pages[i]) - mem->pfn_base;
				if (WARN_ONCE(pageno > mem->size,
				      "invalid pageno:%d\n", pageno))
					continue;
				bitmap_clear(mem->bitmap, pageno, 1);
			}
		} else {
			for (i = 0; i < (size >> PAGE_SHIFT); i += PAGES_PER_GRANULE(granule_size)) {
				pageno = (page_to_pfn(pages[i]) - mem->pfn_base) /
						PAGES_PER_GRANULE(granule_size);
				if (WARN_ONCE(pageno > mem->size,
				      "invalid pageno:%d\n", pageno))
					continue;
				bitmap_clear(mem->bitmap, pageno, 1);
			}
		}
		spin_unlock_irqrestore(&mem->spinlock, flags);
		kvfree(pages);
		return;
	}

	if (mem->flags & DMA_MEMORY_NOMAP)
		mem_addr =  (void *)(uintptr_t)mem->device_base;
	else
		mem_addr =  mem->virt_base;

	page_shift_val = is_gpu ? PAGE_SHIFT_GRANULE(granule_size) : PAGE_SHIFT;
	if (mem && cpu_addr >= mem_addr &&
	    cpu_addr - mem_addr < (u64)mem->size << page_shift_val) {
		unsigned int page = (cpu_addr - mem_addr) >> page_shift_val;
		unsigned long flags;
		unsigned int count;

		if (DMA_ATTR_ALLOC_EXACT_SIZE & attrs) {
			if (is_gpu)
				count = ALIGN_GRANULE_SIZE(size, granule_size) >> page_shift_val;
			else
				count = PAGE_ALIGN(size) >> page_shift_val;
		}
		else
			count = 1 << get_order(size);

		spin_lock_irqsave(&mem->spinlock, flags);
		bitmap_clear(mem->bitmap, page, count);
		spin_unlock_irqrestore(&mem->spinlock, flags);
	}
}
EXPORT_SYMBOL(nvmap_dma_free_attrs);

void *nvmap_dma_mark_declared_memory_occupied(struct device *dev,
					dma_addr_t device_addr, size_t size)
{
	struct dma_coherent_mem_replica *mem;
	unsigned long flags, pageno;
	unsigned int alloc_size;
	int pos;

	if (!dev || !dev->dma_mem)
		return ERR_PTR(-EINVAL);

	mem = (struct dma_coherent_mem_replica *)(dev->dma_mem);

	size += device_addr & ~PAGE_MASK;
	alloc_size = PAGE_ALIGN(size) >> PAGE_SHIFT;

	spin_lock_irqsave(&mem->spinlock, flags);
	pos = PFN_DOWN(device_addr - mem->device_base);
	pageno = bitmap_find_next_zero_area(mem->bitmap, mem->size, pos, alloc_size, 0);
	if (pageno != pos)
		goto error;
	bitmap_set(mem->bitmap, pageno, alloc_size);
	spin_unlock_irqrestore(&mem->spinlock, flags);
	return mem->virt_base + (pos << PAGE_SHIFT);

error:
	spin_unlock_irqrestore(&mem->spinlock, flags);
	return ERR_PTR(-ENOMEM);
}

void nvmap_dma_mark_declared_memory_unoccupied(struct device *dev,
					 dma_addr_t device_addr, size_t size)
{
	struct dma_coherent_mem_replica *mem;
	unsigned long flags;
	unsigned int alloc_size;
	int pos;

	if (!dev || !dev->dma_mem)
		return;

	mem = (struct dma_coherent_mem_replica *)(dev->dma_mem);

	size += device_addr & ~PAGE_MASK;
	alloc_size = PAGE_ALIGN(size) >> PAGE_SHIFT;

	spin_lock_irqsave(&mem->spinlock, flags);
	pos = PFN_DOWN(device_addr - mem->device_base);
	bitmap_clear(mem->bitmap, pos, alloc_size);
	spin_unlock_irqrestore(&mem->spinlock, flags);
}

void nvmap_dma_release_coherent_memory(struct dma_coherent_mem_replica *mem)
{
	if (!mem)
		return;
	if (!(mem->flags & DMA_MEMORY_NOMAP))
		memunmap(mem->virt_base);
	kfree(mem->bitmap);
	kfree(mem);
}

static int nvmap_dma_assign_coherent_memory(struct device *dev,
				      struct dma_coherent_mem_replica *mem)
{
	if (!dev)
		return -ENODEV;

	if (dev->dma_mem)
		return -EBUSY;

	dev->dma_mem = (struct dma_coherent_mem *)mem;
	return 0;
}

static int nvmap_dma_init_coherent_memory(
	phys_addr_t phys_addr, dma_addr_t device_addr, size_t size, int flags,
	struct dma_coherent_mem_replica **mem, bool is_gpu, u32 granule_size)
{
	struct dma_coherent_mem_replica *dma_mem = NULL;
	void *mem_base = NULL;
	int pages;
	int bitmap_size;
	int ret;

	if (!size)
		return -EINVAL;

	if (is_gpu)
		pages = size >> PAGE_SHIFT_GRANULE(granule_size);
	else
		pages = size >> PAGE_SHIFT;

	bitmap_size = BITS_TO_LONGS(pages) * sizeof(long);

	if (!(flags & DMA_MEMORY_NOMAP)) {
		mem_base = memremap(phys_addr, size, MEMREMAP_WC);
		if (!mem_base)
			return -EINVAL;
	}

	dma_mem = kzalloc(sizeof(struct dma_coherent_mem_replica), GFP_KERNEL);
	if (!dma_mem) {
		ret = -ENOMEM;
		goto err_memunmap;
	}

	dma_mem->bitmap = kzalloc(bitmap_size, GFP_KERNEL);
	if (!dma_mem->bitmap) {
		ret = -ENOMEM;
		goto err_free_dma_mem;
	}

	dma_mem->virt_base = mem_base;
	dma_mem->device_base = device_addr;
	dma_mem->pfn_base = PFN_DOWN(device_addr);
	dma_mem->size = pages;
	dma_mem->flags = flags;
	spin_lock_init(&dma_mem->spinlock);

	*mem = dma_mem;
	return 0;

err_free_dma_mem:
	kfree(dma_mem);

err_memunmap:
	memunmap(mem_base);
	return ret;
}

int nvmap_dma_declare_coherent_memory(struct device *dev, phys_addr_t phys_addr,
			dma_addr_t device_addr, size_t size, int flags, bool is_gpu,
			u32 granule_size)
{
	struct dma_coherent_mem_replica *mem;
	int ret;

	ret = nvmap_dma_init_coherent_memory(phys_addr, device_addr, size, flags, &mem,
					     is_gpu, granule_size);
	if (ret)
		return ret;

	ret = nvmap_dma_assign_coherent_memory(dev, mem);
	if (ret)
		nvmap_dma_release_coherent_memory(mem);
	return ret;
}
#endif /* LINUX_VERSION_CODE */

static int __init nvmap_co_device_init(struct reserved_mem *rmem,
					struct device *dev)
{
	struct nvmap_platform_carveout *co = rmem->priv;
	int err = 0;

	if (!co)
		return -ENODEV;

	/* if co size is 0, => co is not present. So, skip init. */
	if (!co->size)
		return 0;

	if (!co->cma_dev) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)
		err = dma_declare_coherent_memory(co->dma_dev, 0,
				co->base, co->size,
				DMA_MEMORY_NOMAP);
#else
		err = nvmap_dma_declare_coherent_memory(co->dma_dev, 0,
				co->base, co->size,
				DMA_MEMORY_NOMAP, co->is_gpu_co,
				co->granule_size);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
		if (!err) {
#else
		if (err & DMA_MEMORY_NOMAP) {
#endif
			pr_info("%s :dma coherent mem declare %pa,%zu\n",
				 co->name, &co->base, co->size);
			co->init_done = true;
			err = 0;
		} else
			pr_err("%s :dma coherent mem declare fail %pa,%zu,err:%d\n",
				co->name, &co->base, co->size, err);
	} else {
#ifdef NVMAP_CONFIG_VPR_RESIZE
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
		/*
		 * When vpr memory is reserved, kmemleak tries to scan vpr
		 * memory for pointers. vpr memory should not be accessed
		 * from cpu so avoid scanning it. When vpr memory is removed,
		 * the memblock_remove() API ensures that kmemleak won't scan
		 * a removed block.
		 */
		if (!strncmp(co->name, "vpr", 3))
			kmemleak_no_scan(__va(co->base));
#endif

		co->dma_info->cma_dev = co->cma_dev;
		err = dma_declare_coherent_resizable_cma_memory(
				co->dma_dev, co->dma_info);
		if (err)
			pr_err("%s coherent memory declaration failed\n",
				     co->name);
		else
#endif
			co->init_done = true;
	}
	return err;
}

static void nvmap_co_device_release(struct reserved_mem *rmem, struct device *dev)
{
	return;
}

static const struct reserved_mem_ops nvmap_co_ops = {
	.device_init	= nvmap_co_device_init,
	.device_release	= nvmap_co_device_release,
};

#ifndef NVMAP_LOADABLE_MODULE
int __init nvmap_co_setup(struct reserved_mem *rmem)
{
	struct nvmap_platform_carveout *co;
	int ret = 0;
#ifdef NVMAP_CONFIG_VPR_RESIZE
	struct cma *cma;
#endif
	ulong start = sched_clock();

	co = nvmap_get_carveout_pdata(rmem->name);
	if (!co)
		return ret;

	rmem->ops = &nvmap_co_ops;
	rmem->priv = co;

	co->base = rmem->base;
	co->size = rmem->size;

#ifdef NVMAP_CONFIG_VPR_RESIZE
	if (!of_get_flat_dt_prop(rmem->fdt_node, "reusable", NULL) ||
	    of_get_flat_dt_prop(rmem->fdt_node, "no-map", NULL))
		goto skip_cma;

	WARN_ON(!rmem->base);
	if (dev_get_cma_area(co->cma_dev)) {
		pr_info("cma area initialed in legacy way already\n");
		goto finish;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	ret = cma_init_reserved_mem(rmem->base, rmem->size, 0,
					rmem->name, &cma);
#else
	ret = cma_init_reserved_mem(rmem->base, rmem->size, 0, &cma);
#endif
	if (ret) {
		pr_info("cma_init_reserved_mem fails for %s\n", rmem->name);
		goto finish;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	dma_contiguous_early_fixup_vpr(rmem->base, rmem->size);
	if (co->cma_dev)
		co->cma_dev->cma_area = cma;
#else
	dma_contiguous_early_fixup(rmem->base, rmem->size);
	dev_set_cma_area(co->cma_dev, cma);
#endif
	pr_debug("tegra-carveouts carveout=%s %pa@%pa\n",
		 rmem->name, &rmem->size, &rmem->base);
	goto finish;

skip_cma:
#endif
	co->cma_dev = NULL;
#ifdef NVMAP_CONFIG_VPR_RESIZE
finish:
#endif
	nvmap_init_time += sched_clock() - start;
	return ret;
}
#else
int __init nvmap_co_setup(struct reserved_mem *rmem, u32 granule_size)
{
	struct nvmap_platform_carveout *co;
	ulong start = sched_clock();
	int ret = 0;

	co = nvmap_get_carveout_pdata(rmem->name);
	if (!co)
		return ret;

	rmem->ops = &nvmap_co_ops;
	rmem->priv = co;

	co->base = rmem->base;
	co->size = rmem->size;
	co->cma_dev = NULL;
	if (!strncmp(co->name, "gpu", 3)) {
		co->is_gpu_co = true;
		co->granule_size = granule_size;
	}

	nvmap_init_time += sched_clock() - start;
	return ret;
}
#endif /* !NVMAP_LOADABLE_MODULE */

#ifndef NVMAP_LOADABLE_MODULE
RESERVEDMEM_OF_DECLARE(nvmap_co, "nvidia,generic_carveout", nvmap_co_setup);
RESERVEDMEM_OF_DECLARE(nvmap_vpr_co, "nvidia,vpr-carveout", nvmap_co_setup);
RESERVEDMEM_OF_DECLARE(nvmap_fsi_co, "nvidia,fsi-carveout", nvmap_co_setup);
#endif /* !NVMAP_LOADABLE_MODULE */

/*
 * Fills in the platform data either from the device tree or with the
 * legacy path.
 */
int __init nvmap_init(struct platform_device *pdev)
{
	int err;
	struct reserved_mem rmem;

#ifdef NVMAP_LOADABLE_MODULE
	u32 granule_size = 0;
	struct reserved_mem *rmem2;
	struct device_node *np = pdev->dev.of_node;
	struct of_phandle_iterator it;
	const char *compp;

	if (!of_phandle_iterator_init(&it, np, "memory-region", NULL, 0)) {
		while (!of_phandle_iterator_next(&it) && it.node) {
			if (of_device_is_available(it.node) &&
			    !of_device_is_compatible(it.node, "nvidia,ivm_carveout")) {
				/* Read granule size in case of gpu carveout */
				if (of_device_is_compatible(it.node, "nvidia,gpu_carveout")
				    && of_property_read_u32(it.node, "granule-size", &granule_size)) {
					pr_err("granule-size property is missing\n");
					return -EINVAL;
				}
				rmem2 = of_reserved_mem_lookup(it.node);
				if (!rmem2) {
					if (!of_property_read_string(it.node, "compatible", &compp))
						pr_err("unable to acquire memory-region: %s\n",
							compp);
					return -EINVAL;
				}
				nvmap_co_setup(rmem2, granule_size);
			}
		}
	}
#endif /* NVMAP_LOADABLE_MODULE */

	if (pdev->dev.of_node) {
		err = __nvmap_init_dt(pdev);
		if (err)
			return err;
	}

	err = of_reserved_mem_device_init(&pdev->dev);
	if (err)
		pr_debug("reserved_mem_device_init fails, try legacy init\n");

	/* try legacy init */
	if (!nvmap_carveouts[0].init_done) {
		rmem.priv = &nvmap_carveouts[0];
		err = nvmap_co_device_init(&rmem, &pdev->dev);
		if (err)
			goto end;
	}

	if (!nvmap_carveouts[1].init_done) {
		rmem.priv = &nvmap_carveouts[1];
		err = nvmap_co_device_init(&rmem, &pdev->dev);
		if (err)
			goto end;
	}

#ifdef CONFIG_TEGRA_VIRTUALIZATION
	err = nvmap_populate_ivm_carveout(&pdev->dev);
#endif /* CONFIG_TEGRA_VIRTUALIZATION */

end:
	return err;
}

#ifdef NVMAP_LOADABLE_MODULE
static bool nvmap_is_carveout_node_present(void)
{
	struct device_node *np;

	np = of_find_node_by_name(NULL, "tegra-carveouts");
	if (of_device_is_available(np)) {
		of_node_put(np);
		return true;
	}
	of_node_put(np);
	return false;
}
#endif /* NVMAP_LOADABLE_MODULE */

static struct platform_driver __refdata nvmap_driver = {
	.probe		= nvmap_probe,
	.remove		= nvmap_remove,

	.driver = {
		.name	= "tegra-carveouts",
		.owner	= THIS_MODULE,
		.of_match_table = nvmap_of_ids,
		.suppress_bind_attrs = true,
	},
};

static int __init nvmap_init_driver(void)
{
	int e = 0;

	e = nvmap_heap_init();
	if (e)
		goto fail;

	e = platform_driver_register(&nvmap_driver);
	if (e) {
		nvmap_heap_deinit();
		goto fail;
	}

fail:
	return e;
}

#ifdef NVMAP_LOADABLE_MODULE
module_init(nvmap_init_driver);
#else
fs_initcall(nvmap_init_driver);
#endif /* NVMAP_LOADABLE_MODULE */

static void __exit nvmap_exit_driver(void)
{
#ifdef NVMAP_LOADABLE_MODULE
	if (!nvmap_is_carveout_node_present())
		platform_device_unregister(pdev);
#endif /* NVMAP_LOADABLE_MODULE */
	platform_driver_unregister(&nvmap_driver);
	nvmap_heap_deinit();
	nvmap_dev = NULL;
}
module_exit(nvmap_exit_driver);
MODULE_IMPORT_NS(DMA_BUF);
MODULE_DESCRIPTION("NvMap: Nvidia Tegra Memory Management Driver");
MODULE_AUTHOR("Puneet Saxena <puneets@nvidia.com>");
MODULE_LICENSE("GPL v2");
