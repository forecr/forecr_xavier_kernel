/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PD_CACHE_H
#define NVGPU_PD_CACHE_H

/**
 * @file
 *
 * Page directory cache interface.
 */


#include <nvgpu/types.h>

struct gk20a;
struct vm_gk20a;
struct nvgpu_mem;
struct gk20a_mmu_level;

/**
 * GMMU page directory. This is the kernel's tracking of a list of PDEs or PTEs
 * in the GMMU. PD size here must be at least 4096 bytes, but lower tier PDs
 * can be sub-4K aligned. Although lower PDE tables can be aligned at 256B
 * boundaries the PDB must be 4K aligned.
 */
struct nvgpu_gmmu_pd {
	/**
	 * DMA memory describing the PTEs or PDEs.
	 */
	struct nvgpu_mem	*mem;
	/**
	 * Describes the offset of the PDE table in @mem.
	 */
	u32			 mem_offs;
	/**
	 * This PD is using pd_cache memory if this flag is set to true.
	 */
	bool			 cached;
	/**
	 * PD size here must be at least 4096 bytes, but lower tier PDs can be
	 * sub-4K aligned.
	 */
	u32			 pd_size;

	/**
	 * List of pointers to the next level of page tables. Does not
	 * need to be populated when this PD is pointing to PTEs.
	 */
	struct nvgpu_gmmu_pd	*entries;
	/**
	 * Number of entries in a PD is easy to compute from the number of bits
	 * used to index the page directory. That is simply 2 raised to the
	 * number of bits.
	 */
	u32			 num_entries;
};

/**
 * @brief Allocates the DMA memory for a page directory.
 *
 * @param vm	[in]	Pointer to virtual memory structure.
 * @param pd	[in]	Pointer to pd_cache memory structure.
 * @param bytes	[in]	PD size.
 *
 * Allocates a page directory:
 * Allocates the DMA memory for a page directory.
 * This handles the necessary PD cache logistics. Since Parker and
 *  later GPUs, some of the page  directories are smaller than a page.
 *  Hence, packing these PDs together saves a lot of memory.
 * If PD is bigger than a page just do a regular DMA alloc.
 * #nvgpu_pd_cache_alloc_direct() does the pd cache allocation.
 *
 *
 * @return	0 in case of success.
 * @retval	-ENOMEM in case of failure. Reasons can be any one
 * 		of the following
 * 		--kzalloc failure.
 * 		--failures internal to dma alloc* functions.
 *
 */
int  nvgpu_pd_alloc(struct vm_gk20a *vm, struct nvgpu_gmmu_pd *pd, u32 bytes);

/**
 * @brief Free the DMA memory allocated using nvgpu_pd_alloc().
 *
 * @param vm	[in]	Pointer to virtual memory structure.
 * @param pd	[in]	Pointer to pd_cache memory structure.
 *
 * Free the Page Directory DMA memory:
 * Free the DMA memory allocated using nvgpu_pd_alloc by
 *  calling nvgpu_pd_cache_free_direct().
 * Call nvgpu_pd_cache_free() if the pd is cached.
 *
 * @return	None
 */
void nvgpu_pd_free(struct vm_gk20a *vm, struct nvgpu_gmmu_pd *pd);

/**
 * @brief Initializes the pd_cache tracking stuff.
 *
 * @param g	[in]	The GPU.
 *
 * Initialize the pd_cache:
 * Allocates the zero initialized memory area for #nvgpu_pd_cache.
 * Initializes the mutexes and list nodes for pd_cache tracking stuff.
 * Make sure not to reinitialize the pd_cache again by initilalizing
 *  mm.pd_cache.
 *
 * @return	0 in case of success.
 * @retval	-ENOMEM in case of kzalloc failure.
 */
int  nvgpu_pd_cache_init(struct gk20a *g);

/**
 * @brief Returns list of all page table memory buffers.
 *
 * @param g [in]	       The GPU.
 * @param no_of_mems [out]     No. of page table memory buffers.
 *
 * @return	Pointer to array of struct nvgpu_mem pointers in case of success.
 * @retval	NULL in case of failure.
 */
struct nvgpu_mem **nvgpu_pd_cache_get_nvgpu_mems(struct gk20a *g, uint32_t *no_of_mems);

#ifdef CONFIG_NVGPU_CHECKER_FAULT_INJECTION

/**
 * For injecting fault in level0 pde.
 */
#define NVGPU_PTE_INJECT_LEVEL0_FAULT			1
/**
 * For injecting fault in level 1-4 pde/pte.
 */
#define NVGPU_PTE_INJECT_LEVEL1_4_FAULT			2
/**
 * For injecting fault in cbc line number.
 */
#define NVGPU_PTE_INJECT_CBC_FAULT			3
/**
 * For injecting fault in compressed/uncompressed kind.
 */
#define NVGPU_PTE_INJECT_KIND_FAULT			4
/**
 * For injecting fault in ro/rw mapping type.
 */
#define NVGPU_PTE_INJECT_RO_RW_FAULT			5
/**
 * For injecting fault in mapping's cacheability.
 */
#define NVGPU_PTE_INJECT_CACHE_FAULT			6
/**
 * For injecting fault in mapping's aperture type.
 */
#define NVGPU_PTE_INJECT_APERTURE_FAULT			7

/**
 * @brief Injects fault in page table entries.
 *
 * @param g [in]	   The GPU.
 * @param fault_type [in]  One of the NVGPU_PTE_INJECT_* fault_types.
 *
 */
void nvgpu_pd_cache_inject_fault(struct gk20a *g, u32 fault_type);
#endif

/**
 * @brief Free the pd_cache tracking stuff allocated by nvgpu_pd_cache_init().
 *
 * @param g	[in]	The GPU.
 *
 * Free the pd_cache:
 * Reset the list nodes used for pd_cache tracking stuff.
 * Free the #nvgpu_pd_cache internal structure allocated
 *  by nvgpu_pd_cache_init().
 * Reset the mm.pd_cache to NULL.
 *
 * @return	None
 */
void nvgpu_pd_cache_fini(struct gk20a *g);

/**
 * @brief Compute the pd offset for GMMU programming.
 *
 * @param l		[in]	Structure describes the GMMU level
 *				entry format which is used for GMMU mapping
 *				understandable by GMMU H/W.
 * @param pd_idx	[in]	Index into the page size table.
 *				- Min: GMMU_PAGE_SIZE_SMALL
 *				- Max: GMMU_PAGE_SIZE_KERNEL
 *
 * Compute the pd offset:
 * ((@pd_idx * GMMU level entry size / 4).
 *
 * @return	pd offset at \a pd_idx.
 *
 */
u32  nvgpu_pd_offset_from_index(const struct gk20a_mmu_level *l, u32 pd_idx);

/**
 * @brief Write data content into pd mem.
 *
 * @param g	[in]	The GPU.
 * @param pd	[in]	Pointer to GMMU page directory structure.
 * @param w	[in]	Word offset from the start of the pd mem.
 * @param data	[in]	Data to write into pd mem.
 *
 * Write data content into pd mem:
 * Offset = ((start address of the pd / 4 + @w).
 * Write data content into offset address by calling #nvgpu_mem_wr32().
 *
 * @return	None
 */
void nvgpu_pd_write(struct gk20a *g, struct nvgpu_gmmu_pd *pd,
		    size_t w, u32 data);

/**
 * @brief Return the _physical_ address of a page directory.
 *
 * @param g	[in]	The GPU.
 * @param pd	[in]	Pointer to GMMU page directory structure.
 *
 * Write data content into pd mem:
 * Return the _physical_ address of a page directory for GMMU programming.
 * PD base in context inst block.
 * #nvgpu_mem_get_addr returns the _physical_ address of pd mem.
 *
 * @return	pd physical address in case of valid pd mem.
 * @retval	Zero in case of invalid/random pd mem.
 */
u64  nvgpu_pd_gpu_addr(struct gk20a *g, struct nvgpu_gmmu_pd *pd);

/**
 * @brief Allocate memory for a page directory.
 *
 * @param g	[in]	The GPU.
 * @param pd	[in]	Pointer to GMMU page directory structure.
 *
 * - Take pd_cache mutex lock.
 * - Call nvgpu_pd_cache_alloc_direct_locked() to allocate dmaable memory for pd.
 * - Unlock pd_cache mutex.
 *
 * @return	Zero For succcess.
 * @retval	-ENOMEM For any allocation failure.
 */
int nvgpu_pd_cache_alloc_direct(struct gk20a *g,
				struct nvgpu_gmmu_pd *pd, u32 bytes);

#endif
