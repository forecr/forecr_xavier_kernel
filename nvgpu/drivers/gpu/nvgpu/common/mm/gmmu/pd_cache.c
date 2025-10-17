// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bug.h>
#include <nvgpu/log.h>
#include <nvgpu/dma.h>
#include <nvgpu/gmmu.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/list.h>
#include <nvgpu/log2.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/enabled.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/string.h>

#include "pd_cache_priv.h"

/**
 * @brief Convert a list node to the containing pd_mem_entry structure.
 *
 * This function takes a pointer to a list node and computes the starting
 * address of the containing nvgpu_pd_mem_entry structure. It uses the
 * standard C macro 'offsetof' to find the offset of the 'list_entry' member
 * within the nvgpu_pd_mem_entry structure and subtracts that offset from
 * the list node pointer to get the base address of the nvgpu_pd_mem_entry.
 *
 * The steps performed by the function are as follows:
 * -# Cast the input list node pointer to an unsigned integer type that is
 *    capable of holding a pointer.
 * -# Use the 'offsetof' macro to determine the byte offset of the 'list_entry'
 *    member within the nvgpu_pd_mem_entry structure.
 * -# Subtract the offset from the casted list node pointer to find the
 *    starting address of the nvgpu_pd_mem_entry structure.
 * -# Cast the resulting pointer back to a pointer to nvgpu_pd_mem_entry.
 *
 * @param [in] node  The list node contained within a nvgpu_pd_mem_entry.
 *
 * @return Pointer to the nvgpu_pd_mem_entry structure containing the list node.
 */
static inline struct nvgpu_pd_mem_entry *
nvgpu_pd_mem_entry_from_list_entry(struct nvgpu_list_node *node)
{
	return (struct nvgpu_pd_mem_entry *)
		((uintptr_t)node -
		 offsetof(struct nvgpu_pd_mem_entry, list_entry));
};

/**
 * @brief Convert a tree node to the containing pd_mem_entry structure.
 *
 * This function takes a pointer to a red-black tree node and computes the
 * starting address of the containing nvgpu_pd_mem_entry structure. It uses
 * the standard C macro 'offsetof' to find the offset of the 'tree_entry'
 * member within the nvgpu_pd_mem_entry structure and subtracts that offset
 * from the tree node pointer to get the base address of the
 * nvgpu_pd_mem_entry.
 *
 * The steps performed by the function are as follows:
 * -# Cast the input tree node pointer to an unsigned integer type that is
 *    capable of holding a pointer.
 * -# Use the 'offsetof' macro to determine the byte offset of the 'tree_entry'
 *    member within the nvgpu_pd_mem_entry structure.
 * -# Subtract the offset from the casted tree node pointer to find the
 *    starting address of the nvgpu_pd_mem_entry structure.
 * -# Cast the resulting pointer back to a pointer to nvgpu_pd_mem_entry.
 *
 * @param [in] node  The tree node contained within a nvgpu_pd_mem_entry.
 *
 * @return Pointer to the nvgpu_pd_mem_entry structure containing the tree node.
 */
static inline struct nvgpu_pd_mem_entry *
nvgpu_pd_mem_entry_from_tree_entry(struct nvgpu_rbtree_node *node)
{
	return (struct nvgpu_pd_mem_entry *)
		((uintptr_t)node -
		 offsetof(struct nvgpu_pd_mem_entry, tree_entry));
};

/**
 * @brief Calculate the index for the PD cache array based on the size of the PD.
 *
 * This function computes the index in the PD cache array that corresponds to
 * the size of a page directory. It shifts the input size by the minimum shift
 * value minus one and then calculates the base-2 logarithm of the shifted
 * value to determine the index.
 *
 * The steps performed by the function are as follows:
 * -# Cast the input size in bytes to an unsigned long type.
 * -# Right shift the casted size by one less than the #NVGPU_PD_CACHE_MIN_SHIFT
 *    to normalize the size to the minimum cache size unit.
 * -# Assert that the shifted size is greater than zero.
 * -# Call the function 'nvgpu_ilog2()' to calculate the base-2 logarithm of the
 *    shifted size, which gives the index in the PD cache array.
 * -# Assert that the calculated index does not exceed the maximum value for a
 *    32-bit unsigned integer.
 * -# Cast the index back to a 32-bit unsigned integer and return it.
 *
 * @param [in] bytes  The size of the PD for which the index is to be calculated.
 *
 * @return The index in the PD cache array corresponding to the input size.
 */
static u32 nvgpu_pd_cache_nr(u32 bytes)
{
	unsigned long bytes_shift = (unsigned long)bytes >>
			((unsigned long)NVGPU_PD_CACHE_MIN_SHIFT - 1UL);
	unsigned long tmp;

	nvgpu_assert(bytes_shift > 0UL);
	tmp = nvgpu_ilog2(bytes_shift);

	nvgpu_assert(tmp <= U32_MAX);
	return (u32)tmp;
}

/**
 * @brief Calculate the number of entries in a page directory cache.
 *
 * This function calculates the number of entries that can fit into a page
 * directory cache based on the size of each entry. It divides the total size
 * of the cache by the size of a single page directory entry.
 *
 * The steps performed by the function are as follows:
 * -# Check if the size of the page directory entry is zero and trigger a bug
 *    if it is, using the 'BUG_ON' macro.
 * -# Cast the #NVGPU_PD_CACHE_SIZE to a 64-bit unsigned integer and then
 *    safely cast it down to a 32-bit unsigned integer using the function
 *    'nvgpu_safe_cast_u64_to_u32()'.
 * -# Divide the casted cache size by the size of the page directory entry to
 *    get the number of entries that can fit into the cache.
 * -# Return the calculated number of entries.
 *
 * @param [in] pentry  Pointer to the page directory memory entry structure.
 *
 * @return The number of entries that can fit into the page directory cache.
 */
static u32 nvgpu_pd_cache_get_nr_entries(struct nvgpu_pd_mem_entry *pentry)
{
	BUG_ON(pentry->pd_size == 0U);

	return (nvgpu_safe_cast_u64_to_u32(NVGPU_PD_CACHE_SIZE)) /
			pentry->pd_size;
}

/*
 * Return the _physical_ address of a page directory.
 */
u64 nvgpu_pd_gpu_addr(struct gk20a *g, struct nvgpu_gmmu_pd *pd)
{
	u64 page_addr;

	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_NVLINK)) {
		page_addr = nvgpu_mem_get_phys_addr(g, pd->mem);
	} else {
		page_addr = nvgpu_mem_get_addr(g, pd->mem);
	}

	return nvgpu_safe_add_u64(page_addr, U64(pd->mem_offs));
}

u32 nvgpu_pd_offset_from_index(const struct gk20a_mmu_level *l, u32 pd_idx)
{
	return nvgpu_safe_mult_u32(pd_idx, l->entry_size) / U32(sizeof(u32));
}

void nvgpu_pd_write(struct gk20a *g, struct nvgpu_gmmu_pd *pd,
		    size_t w, u32 data)
{
	u64 tmp_offset = nvgpu_safe_add_u64((pd->mem_offs / sizeof(u32)), w);

	nvgpu_mem_wr32(g, pd->mem,
		       nvgpu_safe_cast_u64_to_u32(tmp_offset),
		       data);
}

int nvgpu_pd_cache_init(struct gk20a *g)
{
	struct nvgpu_pd_cache *cache;
	u32 i;

	/*
	 * This gets called from finalize_poweron() so we need to make sure we
	 * don't reinit the pd_cache over and over.
	 */
	if (g->mm.pd_cache != NULL) {
		return 0;
	}

	cache = nvgpu_kzalloc(g, sizeof(*cache));
	if (cache == NULL) {
		nvgpu_err(g, "Failed to alloc pd_cache!");
		return -ENOMEM;
	}

	for (i = 0U; i < NVGPU_PD_CACHE_COUNT; i++) {
		nvgpu_init_list_node(&cache->full[i]);
		nvgpu_init_list_node(&cache->partial[i]);
	}
	nvgpu_init_list_node(&cache->direct);

	cache->mem_tree = NULL;

	nvgpu_mutex_init(&cache->lock);

	g->mm.pd_cache = cache;

	pd_dbg(g, "PD cache initialized!");

	return 0;
}

#ifdef CONFIG_NVGPU_CHECKER_FAULT_INJECTION
static void nvgpu_inject_pattern_fault(struct nvgpu_mem *mem, u32 fault_type)
{
	u32 i;
	u32 *ptr = mem->cpu_va;
	for (i = 0U; i < PAGE_SIZE / sizeof(*ptr); i += 2) {
		if (fault_type == NVGPU_PTE_INJECT_CBC_FAULT) {
		/* Inject fault in comptag index */
			*(ptr + i + 1) &= 0xffcafecf;
		} else if (fault_type == NVGPU_PTE_INJECT_KIND_FAULT) {
		/* Inject fault in kind type */
			*(ptr + i + 1) = (*(ptr + i + 1) & 0xffffff) | (~(*(ptr + i + 1)) & 0xff000000);
		} else if (fault_type == NVGPU_PTE_INJECT_RO_RW_FAULT) {
		/* Inject fault in ro/rw mapping type */
			*(ptr + i) = (*(ptr + i) & 0xffffffbf) | (~(*(ptr + i)) & 0x40);
		} else if (fault_type == NVGPU_PTE_INJECT_CACHE_FAULT) {
		/* Inject fault in caching type */
			*(ptr + i) = (*(ptr + i) & 0xfffffff7) | (~(*(ptr + i)) & 0x8);
		} else if (fault_type == NVGPU_PTE_INJECT_APERTURE_FAULT) {
		/* Inject fault in aperture type */
			*(ptr + i) = (*(ptr + i) & 0xfffffff9) | (~(*(ptr + i)) & 0x6);
		} else {
			*(ptr + i) = 0xcafecafe;
		}
	}
}

void nvgpu_pd_cache_inject_fault(struct gk20a *g, u32 fault_type)
{
	u32 i;
	struct nvgpu_pd_mem_entry *nvgpu_pdmem_entry;
	struct nvgpu_pd_cache *cache = g->mm.pd_cache;

	switch (fault_type) {
	case NVGPU_PTE_INJECT_LEVEL0_FAULT:
	{
		if (!nvgpu_list_empty(&cache->direct)) {
			nvgpu_pdmem_entry = nvgpu_list_first_entry(&cache->direct,
				nvgpu_pd_mem_entry, list_entry);
			nvgpu_inject_pattern_fault(&nvgpu_pdmem_entry->mem, fault_type);
		}
	}
	break;
	case NVGPU_PTE_INJECT_LEVEL1_4_FAULT:
	case NVGPU_PTE_INJECT_CBC_FAULT:
	case NVGPU_PTE_INJECT_KIND_FAULT:
	{
		for (i = 0U; i < NVGPU_PD_CACHE_COUNT; i++) {
			if (!nvgpu_list_empty(&cache->partial[i])) {
				nvgpu_pdmem_entry = nvgpu_list_first_entry(&cache->partial[i],
					nvgpu_pd_mem_entry, list_entry);
			} else if (!nvgpu_list_empty(&cache->full[i])) {
				nvgpu_pdmem_entry = nvgpu_list_first_entry(&cache->full[i],
					nvgpu_pd_mem_entry, list_entry);
			} else {
				continue;
			}
			if (nvgpu_pdmem_entry->pd_size == PAGE_SIZE) {
				nvgpu_inject_pattern_fault(&nvgpu_pdmem_entry->mem, fault_type);
				return;
			}
		}
	}
	break;
	case NVGPU_PTE_INJECT_LEVEL1_4_FAULT:
	case NVGPU_PTE_INJECT_CBC_FAULT:
	case NVGPU_PTE_INJECT_KIND_FAULT:
	case NVGPU_PTE_INJECT_RO_RW_FAULT:
	case NVGPU_PTE_INJECT_APERTURE_FAULT:
	case NVGPU_PTE_INJECT_CACHE_FAULT:
	{
		for (i = 0U; i < NVGPU_PD_CACHE_COUNT; i++) {
			if (!nvgpu_list_empty(&cache->partial[i])) {
				nvgpu_pdmem_entry = nvgpu_list_first_entry(&cache->partial[i],
					nvgpu_pd_mem_entry, list_entry);
			} else if (!nvgpu_list_empty(&cache->full[i])) {
				nvgpu_pdmem_entry = nvgpu_list_first_entry(&cache->full[i],
					nvgpu_pd_mem_entry, list_entry);
			} else {
				continue;
			}
			if (nvgpu_pdmem_entry->pd_size == PAGE_SIZE) {
				nvgpu_inject_pattern_fault(&nvgpu_pdmem_entry->mem, fault_type);
				return;
			}
		}
	}
	break;
	default:
		nvgpu_err(g, "invalid fault_type");
	}
	return;
}
#endif

struct nvgpu_mem **nvgpu_pd_cache_get_nvgpu_mems(struct gk20a *g, uint32_t *no_of_mems)
{
	struct nvgpu_mem **mem_arr;
	uint32_t count = 0U;
	u32 i, list_len;
	struct nvgpu_pd_mem_entry *nvgpu_pdmem_entry;
	struct nvgpu_pd_cache *cache = g->mm.pd_cache;

	for (i = 0U; i < NVGPU_PD_CACHE_COUNT; i++) {
		list_len = nvgpu_list_length(&cache->full[i]);
		if (list_len == UINT_MAX) {
			nvgpu_err(g, "overflow in nvgpu_pd_cache full list");
			return NULL;
		}
		count = nvgpu_safe_add_u32(count, nvgpu_safe_sub_u32(list_len, 1U));

		list_len = nvgpu_list_length(&cache->partial[i]);
		if (list_len == UINT_MAX) {
			nvgpu_err(g, "overflow in nvgpu_pd_cache partial list");
			return NULL;
		}
		count = nvgpu_safe_add_u32(count, nvgpu_safe_sub_u32(list_len, 1U));
	}
	list_len = nvgpu_list_length(&cache->direct);
	if (list_len == UINT_MAX) {
		nvgpu_err(g, "overflow in nvgpu_pd_cache direct list");
		return NULL;
	}
	count = nvgpu_safe_add_u32(count, nvgpu_safe_sub_u32(list_len, 1U));

	mem_arr = nvgpu_kzalloc(g, sizeof(*mem_arr) * count);
	if (mem_arr == NULL) {
		nvgpu_err(g, "Failed to alloc mem array");
		return NULL;
	}
	*no_of_mems = count;
	count = 0U;
	for (i = 0U; i < NVGPU_PD_CACHE_COUNT; i++) {
		nvgpu_list_for_each_entry(nvgpu_pdmem_entry, &cache->full[i],
				nvgpu_pd_mem_entry, list_entry) {
			mem_arr[count] = &nvgpu_pdmem_entry->mem;
			count = nvgpu_safe_add_u32(count, 1U);
		}
		nvgpu_list_for_each_entry(nvgpu_pdmem_entry, &cache->partial[i],
				nvgpu_pd_mem_entry, list_entry) {
			mem_arr[count] = &nvgpu_pdmem_entry->mem;
			count = nvgpu_safe_add_u32(count, 1U);
		}
	}
	nvgpu_list_for_each_entry(nvgpu_pdmem_entry, &cache->direct,
			nvgpu_pd_mem_entry, list_entry) {
		mem_arr[count] = &nvgpu_pdmem_entry->mem;
		count = nvgpu_safe_add_u32(count, 1U);
	}
	return mem_arr;
}

void nvgpu_pd_cache_fini(struct gk20a *g)
{
	u32 i;
	struct nvgpu_pd_cache *cache = g->mm.pd_cache;

	if (cache == NULL) {
		return;
	}

	for (i = 0U; i < NVGPU_PD_CACHE_COUNT; i++) {
		nvgpu_assert(nvgpu_list_empty(&cache->full[i]));
		nvgpu_assert(nvgpu_list_empty(&cache->partial[i]));
	}
	nvgpu_assert(nvgpu_list_empty(&cache->direct));

	nvgpu_kfree(g, g->mm.pd_cache);
	g->mm.pd_cache = NULL;
}

/**
 * @brief Allocate memory for a page directory directly without using the cache.
 *
 * This function allocates memory for a page directory directly, bypassing the
 * PD cache mechanism. It is used when the size of the page directory is larger
 * than the page size or when the GPU is not using an IOMMU, requiring physically
 * contiguous memory. The function also initializes the page directory memory
 * entry and inserts it into the PD cache's red-black tree for fast lookup.
 *
 * The steps performed by the function are as follows:
 * -# Allocate memory for a new page directory memory entry structure using
 *    'nvgpu_kzalloc()'.
 * -# If the allocation fails, log an out-of-memory error using 'nvgpu_err()'
 *    and return -ENOMEM.
 * -# Log the size of the page directory being allocated using 'pd_dbg()'.
 * -# Determine if the GPU is not using an IOMMU using 'nvgpu_iommuable()' and
 *    the requested size is larger than the CPU page size. If so, set the flag
 *    to request physically contiguous memory.
 * -# Allocate DMA memory with the specified flags and size for the page
 *    directory using 'nvgpu_dma_alloc_flags()'.
 * -# If the DMA allocation fails, log an out-of-memory error using 'nvgpu_err()',
 *    free the previously allocated page directory memory entry structure using
 *    'nvgpu_kfree()', and return -ENOMEM.
 * -# Add the new page directory memory entry to the list of directly allocated
 *    page directories using 'nvgpu_list_add()'.
 * -# Set the page directory pointer in the GMMU page directory structure to
 *    the allocated memory.
 * -# Set the offset within the page directory memory to zero.
 * -# Set the size of the page directory in the page directory memory entry
 *    structure to the requested size.
 * -# Set the allocation count in the page directory memory entry structure to 1.
 * -# Mark the GMMU page directory as cached.
 * -# Set the key_start in the page directory memory entry's tree node to the
 *    address of the allocated memory.
 * -# Insert the page directory memory entry's tree node into the PD cache's
 *    red-black tree using 'nvgpu_rbtree_insert()'.
 * -# Return 0 to indicate success.
 *
 * @param [in] g      Pointer to the GPU structure.
 * @param [in] pd     Pointer to the GMMU page directory structure.
 * @param [in] bytes  Size of the page directory to allocate.
 *
 * @return 0 on success, -ENOMEM if memory allocation fails.
 */
static int nvgpu_pd_cache_alloc_direct_locked(struct gk20a *g,
				       struct nvgpu_gmmu_pd *pd, u32 bytes)
{
	int err;
	unsigned long flags = 0;
	struct nvgpu_pd_mem_entry *pentry;
	struct nvgpu_pd_cache *cache = g->mm.pd_cache;

	pentry = nvgpu_kzalloc(g, sizeof(*pentry));
	if (pentry == NULL) {
		nvgpu_err(g, "OOM allocating pentry!");
		return -ENOMEM;
	}

	pd_dbg(g, "PD-Alloc [D] %u bytes", bytes);

	/*
	 * If bytes == NVGPU_CPU_PAGE_SIZE then it's impossible to get a discontiguous DMA
	 * allocation. Some DMA implementations may, despite this fact, still
	 * use the contiguous pool for page sized allocations. As such only
	 * request explicitly contiguous allocs if the page directory is larger
	 * than the page size. Also, of course, this is all only revelant for
	 * GPUs not using an IOMMU. If there is an IOMMU DMA allocs are always
	 * going to be virtually contiguous and we don't have to force the
	 * underlying allocations to be physically contiguous as well.
	 */
	if (!nvgpu_iommuable(g) && (bytes > NVGPU_CPU_PAGE_SIZE)) {
		flags = NVGPU_DMA_PHYSICALLY_ADDRESSED;
	}

	err = nvgpu_dma_alloc_flags(g, flags, bytes, &(pentry->mem));
	if (err != 0) {
		nvgpu_err(g, "OOM allocating page directory!");
		nvgpu_kfree(g, pentry);
		return -ENOMEM;
	}

	nvgpu_list_add(&pentry->list_entry,
		       &cache->direct);
	pd->mem = &pentry->mem;
	pd->mem_offs = 0;
	pentry->pd_size = bytes;
	pentry->allocs = 1;
	pd->cached = true;
	pentry->tree_entry.key_start = (u64)(uintptr_t)&pentry->mem;
	nvgpu_rbtree_insert(&pentry->tree_entry, &cache->mem_tree);

	return 0;
}

/*
 * This is the simple pass-through for greater than page or page sized PDs.
 *
 * Note: this does not need the cache lock since it does not modify any of the
 * PD cache data structures.
 */
int nvgpu_pd_cache_alloc_direct(struct gk20a *g,
				       struct nvgpu_gmmu_pd *pd, u32 bytes)
{
	int ret;

	nvgpu_mutex_acquire(&g->mm.pd_cache->lock);
	ret = nvgpu_pd_cache_alloc_direct_locked(g, pd, bytes);
	nvgpu_mutex_release(&g->mm.pd_cache->lock);
	return ret;
}

/**
 * @brief Allocates a new page directory memory entry.
 *
 * This function allocates a new page directory memory entry for the GPU's
 * memory management unit. It determines the appropriate size for the page
 * directory based on the requested bytes and updates the page directory
 * cache with the new entry. If the allocation fails due to insufficient
 * contiguous memory, it attempts a direct allocation.
 *
 * The steps performed by the function are as follows:
 * -# Allocate memory for a new page directory memory entry structure using
 *    nvgpu_kzalloc().
 * -# Check if the GPU is not IOMMU-able and the PD cache size is greater than
 *    the CPU page size. If so, set the flag to indicate physically addressed
 *    memory is required.
 * -# Allocate DMA memory for the page directory with the appropriate flags
 *    using nvgpu_dma_alloc_flags().
 * -# If DMA allocation fails, free the allocated memory entry structure using
 *    nvgpu_kfree() and attempt a direct allocation using
 *    nvgpu_pd_cache_alloc_direct_locked() if the error is due to insufficient
 *    memory.
 * -# Calculate the index for the page directory size using nvgpu_pd_cache_nr().
 * -# Assert that the calculated index is within the valid range of the PD cache
 *    count.
 * -# Set the size of the page directory in the memory entry structure.
 * -# Add the memory entry to the partial list in the page directory cache at
 *    the calculated index using nvgpu_list_add().
 * -# Allocate the first page directory table in the memory entry by setting the
 *    first bit in the allocation map.
 * -# Update the number of allocations in the memory entry.
 * -# Update the page directory structure to reflect the new allocation.
 * -# Insert the memory entry into the page directory cache's memory tree using
 *    nvgpu_rbtree_insert().
 *
 * @param [in]  g       Pointer to the GPU structure.
 * @param [in]  cache   Pointer to the page directory cache.
 * @param [out] pd      Pointer to the GMMU page directory structure.
 * @param [in]  bytes   Size of the page directory to allocate.
 *
 * @return 0 if the allocation is successful.
 * @return -ENOMEM if there is an error in memory allocation.
 */
static int nvgpu_pd_cache_alloc_new(struct gk20a *g,
				    struct nvgpu_pd_cache *cache,
				    struct nvgpu_gmmu_pd *pd,
				    u32 bytes)
{
	struct nvgpu_pd_mem_entry *pentry;
	u64 flags = 0UL;
	int32_t err;
	u32 index;

	pd_dbg(g, "PD-Alloc [C]   New: offs=0");

	pentry = nvgpu_kzalloc(g, sizeof(*pentry));
	if (pentry == NULL) {
		nvgpu_err(g, "OOM allocating pentry!");
		return -ENOMEM;
	}

	NVGPU_COV_WHITELIST(deviate, NVGPU_MISRA(Rule, 14_3), "SWE-NVGPU-052-SWSADR.docx")
	if (!nvgpu_iommuable(g) && (NVGPU_PD_CACHE_SIZE > NVGPU_CPU_PAGE_SIZE)) {
		flags = NVGPU_DMA_PHYSICALLY_ADDRESSED;
	}

	err = nvgpu_dma_alloc_flags(g, flags,
				  NVGPU_PD_CACHE_SIZE, &pentry->mem);
	if (err != 0) {
		nvgpu_kfree(g, pentry);

		/* Not enough contiguous space, but a direct
		 * allocation may work
		 */
		if (err == -ENOMEM) {
			return nvgpu_pd_cache_alloc_direct_locked(g, pd, bytes);
		}
		nvgpu_err(g, "Unable to DMA alloc!");
		return -ENOMEM;
	}

	index = nvgpu_pd_cache_nr(bytes);
	nvgpu_assert(index < NVGPU_PD_CACHE_COUNT);

	pentry->pd_size = bytes;
	nvgpu_list_add(&pentry->list_entry, &cache->partial[index]);

	/*
	 * This allocates the very first PD table in the set of tables in this
	 * nvgpu_pd_mem_entry.
	 */
	nvgpu_set_bit(0U, pentry->alloc_map);
	pentry->allocs = 1;

	/*
	 * Now update the nvgpu_gmmu_pd to reflect this allocation.
	 */
	pd->mem = &pentry->mem;
	pd->mem_offs = 0;
	pd->cached = true;

	pentry->tree_entry.key_start = (u64)(uintptr_t)&pentry->mem;
	nvgpu_rbtree_insert(&pentry->tree_entry, &cache->mem_tree);

	return 0;
}

/**
 * @brief Allocates page directory memory from a partially used entry.
 *
 * This function allocates memory for a page directory from a partially used
 * memory entry within the page directory cache. It updates the allocation map
 * and the number of allocations for the memory entry. If the memory entry
 * becomes fully allocated, it moves the entry from the partial list to the
 * full list in the page directory cache.
 *
 * The steps performed by the function are as follows:
 * -# Calculate the number of bits in the allocation map for the memory entry
 *    using nvgpu_pd_cache_get_nr_entries().
 * -# Find the first zero bit in the allocation map, which represents an open
 *    page directory, and calculate the memory offset for the page directory
 *    using nvgpu_safe_mult_u32().
 * -# Assert that the found bit offset is less than the number of bits, ensuring
 *    the allocation map is not full.
 * -# Set the found bit in the allocation map to mark the page directory as
 *    allocated using nvgpu_set_bit().
 * -# Increment the number of allocations in the memory entry.
 * -# Update the page directory structure with the memory information of the
 *    allocated page directory.
 * -# If the number of allocations equals the number of bits, indicating the
 *    memory entry is now fully allocated, move the memory entry from the
 *    partial list to the full list in the page directory cache. This involves
 *    calculating the index for the memory entry size, asserting the index is
 *    within the valid range, and then moving the list entry.
 *
 * @param [in]  g       Pointer to the GPU structure.
 * @param [in]  cache   Pointer to the page directory cache.
 * @param [in]  pentry  Pointer to the partially used memory entry.
 * @param [out] pd      Pointer to the GMMU page directory structure.
 *
 * @return 0 if the allocation is successful.
 */
static int nvgpu_pd_cache_alloc_from_partial(struct gk20a *g,
					     struct nvgpu_pd_cache *cache,
					     struct nvgpu_pd_mem_entry *pentry,
					     struct nvgpu_gmmu_pd *pd)
{
	u32 bit_offs;
	u32 mem_offs;
	u32 nr_bits = nvgpu_pd_cache_get_nr_entries(pentry);

	/*
	 * Find and allocate an open PD.
	 */
	bit_offs = nvgpu_safe_cast_u64_to_u32(
			find_first_zero_bit(pentry->alloc_map, nr_bits));
	mem_offs = nvgpu_safe_mult_u32(bit_offs, pentry->pd_size);

	pd_dbg(g, "PD-Alloc [C]   Partial: offs=%u nr_bits=%d src=0x%p",
	       bit_offs, nr_bits, pentry);

	/* Bit map full. Somethings wrong. */
	nvgpu_assert(bit_offs < nr_bits);

	nvgpu_set_bit(bit_offs, pentry->alloc_map);
	pentry->allocs = nvgpu_safe_add_u32(pentry->allocs, 1U);

	/*
	 * First update the pd.
	 */
	pd->mem = &pentry->mem;
	pd->mem_offs = mem_offs;
	pd->cached = true;

	/*
	 * Now make sure the pentry is in the correct list (full vs partial).
	 */
	if (pentry->allocs >= nr_bits) {
		u32 index;

		index = nvgpu_pd_cache_nr(pentry->pd_size);
		nvgpu_assert(index < NVGPU_PD_CACHE_COUNT);

		pd_dbg(g, "Adding pentry to full list!");
		nvgpu_list_del(&pentry->list_entry);
		nvgpu_list_add(&pentry->list_entry, &cache->full[index]);
	}

	return 0;
}

/**
 * @brief Retrieves a partially used memory entry from the page directory cache.
 *
 * This function looks up the partial list in the page directory cache based on
 * the requested size of the page directory. It returns the first memory entry
 * from the partial list that can be used for further allocations.
 *
 * The steps performed by the function are as follows:
 * -# Calculate the index in the partial list corresponding to the requested
 *    size of the page directory using nvgpu_pd_cache_nr().
 * -# Assert that the calculated index is within the valid range of the PD cache
 *    count.
 * -# Retrieve the list at the calculated index from the page directory cache's
 *    partial list array.
 * -# Check if the retrieved list is empty using nvgpu_list_empty(). If it is,
 *    return NULL indicating there are no partially used memory entries
 *    available.
 * -# If the list is not empty, return the first memory entry in the list using
 *    nvgpu_list_first_entry().
 *
 * @param [in]  cache   Pointer to the page directory cache.
 * @param [in]  bytes   Size of the page directory to allocate.
 *
 * @return Pointer to the first partially used memory entry if available.
 * @return NULL if there are no partially used memory entries available.
 */
static struct nvgpu_pd_mem_entry *nvgpu_pd_cache_get_partial(
	struct nvgpu_pd_cache *cache, u32 bytes)
{
	u32 index;
	struct nvgpu_list_node *list;

	index = nvgpu_pd_cache_nr(bytes);
	nvgpu_assert(index < NVGPU_PD_CACHE_COUNT);

	list = &cache->partial[index];

	if (nvgpu_list_empty(list)) {
		return NULL;
	}

	return nvgpu_list_first_entry(list,
				      nvgpu_pd_mem_entry,
				      list_entry);
}

/**
 * @brief Allocates memory for a page directory.
 *
 * This function attempts to allocate memory for a page directory from the
 * page directory cache. It first checks if the requested size is valid and
 * then tries to allocate from a partially used memory entry. If no partial
 * entries are available, it attempts to allocate a new memory entry.
 *
 * The steps performed by the function are as follows:
 * -# Log the attempt to allocate a page directory of the given size.
 * -# Validate that the requested size is at least the minimum cache size and
 *    is a power of two.
 * -# If the requested size is not valid, log the error and return -EINVAL.
 * -# Assert that the requested size is less than the maximum PD cache size.
 * -# Attempt to retrieve a partially used memory entry from the page directory
 *    cache that matches the requested size using nvgpu_pd_cache_get_partial().
 * -# If a partial entry is not available, attempt to allocate a new memory
 *    entry for the page directory using nvgpu_pd_cache_alloc_new().
 * -# If a partial entry is available, attempt to allocate from the partial
 *    entry using nvgpu_pd_cache_alloc_from_partial().
 * -# If the allocation fails, log the failure.
 * -# Return the result of the allocation attempt.
 *
 * @param [in]  g       Pointer to the GPU structure.
 * @param [in]  cache   Pointer to the page directory cache.
 * @param [out] pd      Pointer to the GMMU page directory structure.
 * @param [in]  bytes   Size of the page directory to allocate.
 *
 * @return 0 if the allocation is successful.
 * @return -EINVAL if the requested size is not valid.
 * @return Error code from the allocation attempt if it fails.
 */
static int nvgpu_pd_cache_alloc(struct gk20a *g, struct nvgpu_pd_cache *cache,
				struct nvgpu_gmmu_pd *pd, u32 bytes)
{
	struct nvgpu_pd_mem_entry *pentry;
	int err;
	bool bytes_valid;

	pd_dbg(g, "PD-Alloc [C] %u bytes", bytes);

	bytes_valid = bytes >= NVGPU_PD_CACHE_MIN;
	if (bytes_valid) {
		bytes_valid = (bytes & nvgpu_safe_sub_u32(bytes, 1U)) == 0U;
	}
	if (!bytes_valid) {
		pd_dbg(g, "PD-Alloc [C]   Invalid (bytes=%u)!", bytes);
		return -EINVAL;
	}

	nvgpu_assert(bytes < NVGPU_PD_CACHE_SIZE);

	pentry = nvgpu_pd_cache_get_partial(cache, bytes);
	if (pentry == NULL) {
		err = nvgpu_pd_cache_alloc_new(g, cache, pd, bytes);
	} else {
		err = nvgpu_pd_cache_alloc_from_partial(g, cache, pentry, pd);
	}

	if (err != 0) {
		nvgpu_err(g, "PD-Alloc [C] Failed!");
	}

	return err;
}

/*
 * Allocate the DMA memory for a page directory. This handles the necessary PD
 * cache logistics. Since on Parker and later GPUs some of the page  directories
 * are smaller than a page packing these PDs together saves a lot of memory.
 */
int nvgpu_pd_alloc(struct vm_gk20a *vm, struct nvgpu_gmmu_pd *pd, u32 bytes)
{
	struct gk20a *g = gk20a_from_vm(vm);
	int err;

	nvgpu_mutex_acquire(&g->mm.pd_cache->lock);
	/*
	 * Simple case: PD is bigger than a page so just do a regular DMA
	 * alloc.
	 */
	if (bytes >= NVGPU_PD_CACHE_SIZE) {
		err = nvgpu_pd_cache_alloc_direct_locked(g, pd, bytes);
		if (err != 0) {
			goto release_lock;
		}
		pd->pd_size = bytes;

		goto release_lock;
	}

	err = nvgpu_pd_cache_alloc(g, g->mm.pd_cache, pd, bytes);
	if (err == 0) {
		pd->pd_size = bytes;
	}
release_lock:
	nvgpu_mutex_release(&g->mm.pd_cache->lock);

	return err;
}

/**
 * @brief Frees a memory entry from the page directory cache.
 *
 * This function releases the resources associated with a memory entry in the
 * page directory cache. It frees the DMA memory, removes the entry from the
 * list, unlinks it from the red-black tree, and frees the memory entry structure.
 *
 * The steps performed by the function are as follows:
 * -# Free the DMA memory associated with the memory entry using nvgpu_dma_free().
 * -# Remove the memory entry from the list it is part of using nvgpu_list_del().
 * -# Unlink the memory entry from the page directory cache's memory tree using
 *    nvgpu_rbtree_unlink().
 * -# Free the memory entry structure itself using nvgpu_kfree().
 *
 * @param [in] g       Pointer to the GPU structure.
 * @param [in] cache   Pointer to the page directory cache.
 * @param [in] pentry  Pointer to the memory entry to be freed.
 */
static void nvgpu_pd_cache_free_mem_entry(struct gk20a *g,
					  struct nvgpu_pd_cache *cache,
					  struct nvgpu_pd_mem_entry *pentry)
{
	nvgpu_dma_free(g, &pentry->mem);
	nvgpu_list_del(&pentry->list_entry);
	nvgpu_rbtree_unlink(&pentry->tree_entry, &cache->mem_tree);
	nvgpu_kfree(g, pentry);
}

/**
 * @brief Frees a page directory from the page directory cache.
 *
 * This function handles the freeing of a page directory from the page directory
 * cache. It clears the bit in the allocation map corresponding to the page
 * directory, decrements the allocation count, and if necessary, moves the entry
 * to the partial list or frees it if it's completely unused.
 *
 * The steps performed by the function are as follows:
 * -# Return immediately if the page directory memory pointer is NULL.
 * -# Calculate the bit in the allocation map corresponding to the page
 *    directory offset.
 * -# Clear the bit in the allocation map to mark the page directory as free
 *    using nvgpu_clear_bit().
 * -# Decrement the allocation count for the memory entry.
 * -# If the memory entry is still partially used, move it to the appropriate
 *    partial list:
 *    -# If the CPU virtual address is not NULL, zero out the memory for the
 *       page directory to prepare it for reuse.
 *    -# Calculate the index for the partial list based on the page directory
 *       size using nvgpu_pd_cache_nr().
 *    -# Assert that the index is within the valid range.
 *    -# Remove the memory entry from its current list and add it to the partial
 *       list at the calculated index using nvgpu_list_del() and
 *       nvgpu_list_add().
 * -# If the memory entry is completely unused, free the memory entry using
 *    nvgpu_pd_cache_free_mem_entry().
 * -# Set the page directory memory pointer to NULL.
 *
 * @param [in] g       Pointer to the GPU structure.
 * @param [in] cache   Pointer to the page directory cache.
 * @param [in] pentry  Pointer to the memory entry being freed.
 * @param [in] pd      Pointer to the GMMU page directory structure.
 */
static void nvgpu_pd_cache_do_free(struct gk20a *g,
				   struct nvgpu_pd_cache *cache,
				   struct nvgpu_pd_mem_entry *pentry,
				   struct nvgpu_gmmu_pd *pd)
{
	u32 bit;

	if (pd->mem == NULL) {
		return;
	}

	bit = pd->mem_offs / pentry->pd_size;

	/* Mark entry as free. */
	nvgpu_clear_bit(bit, pentry->alloc_map);
	pentry->allocs = nvgpu_safe_sub_u32(pentry->allocs, 1U);

	if (pentry->allocs > 0U) {
		u32 index;

		/*
		 * Partially full still. If it was already on the partial list
		 * this just re-adds it.
		 *
		 * Zero the memory for reusing.
		 */
		if (pd->mem->cpu_va != NULL) {
			(void)memset(((u8 *)pd->mem->cpu_va + pd->mem_offs), 0,
					pd->pd_size);
		}

		index = nvgpu_pd_cache_nr(pentry->pd_size);
		nvgpu_assert(index < NVGPU_PD_CACHE_COUNT);

		nvgpu_list_del(&pentry->list_entry);
		nvgpu_list_add(&pentry->list_entry, &cache->partial[index]);
	} else {
		/* Empty now so free it. */
		nvgpu_pd_cache_free_mem_entry(g, cache, pentry);
	}

	pd->mem = NULL;
}

/**
 * @brief Looks up a memory entry in the page directory cache.
 *
 * This function searches for a memory entry in the page directory cache's
 * red-black tree using the memory pointer of the page directory. It returns
 * the corresponding memory entry if found.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a red-black tree node pointer to NULL.
 * -# Search the red-black tree for a node with a key corresponding to the
 *    memory pointer of the page directory using nvgpu_rbtree_search().
 * -# If the node is not found, return NULL indicating the memory entry does
 *    not exist in the cache.
 * -# If the node is found, convert the tree node to a memory entry using
 *    nvgpu_pd_mem_entry_from_tree_entry() and return it.
 *
 * @param [in] cache   Pointer to the page directory cache.
 * @param [in] pd      Pointer to the GMMU page directory structure.
 *
 * @return Pointer to the found memory entry.
 * @return NULL if the memory entry is not found in the cache.
 */
static struct nvgpu_pd_mem_entry *nvgpu_pd_cache_look_up(
	struct nvgpu_pd_cache *cache,
	struct nvgpu_gmmu_pd *pd)
{
	struct nvgpu_rbtree_node *node = NULL;

	nvgpu_rbtree_search((u64)(uintptr_t)pd->mem, &node,
			    cache->mem_tree);
	if (node == NULL) {
		return NULL;
	}

	return nvgpu_pd_mem_entry_from_tree_entry(node);
}

void nvgpu_pd_free(struct vm_gk20a *vm, struct nvgpu_gmmu_pd *pd)
{
	struct gk20a *g = gk20a_from_vm(vm);
	struct nvgpu_pd_mem_entry *pentry;

	nvgpu_mutex_acquire(&g->mm.pd_cache->lock);
	pentry = nvgpu_pd_cache_look_up(g->mm.pd_cache, pd);
	if (pentry == NULL) {
		nvgpu_mutex_release(&g->mm.pd_cache->lock);
		nvgpu_do_assert_print(g, "Attempting to free non-existent pd");
		return;
	}

	nvgpu_pd_cache_do_free(g, g->mm.pd_cache, pentry, pd);

	nvgpu_mutex_release(&g->mm.pd_cache->lock);
}
