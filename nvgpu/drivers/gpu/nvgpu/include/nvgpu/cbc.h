/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CBC_H
#define NVGPU_CBC_H

#ifdef CONFIG_NVGPU_COMPRESSION

#include <nvgpu/types.h>
#include <nvgpu/comptags.h>
#include <nvgpu/nvgpu_mem.h>
/* forward declaration */
struct gk20a;

/* Structure that deals with comptag allocation and
 * operations on the comptags allocated.
 */
enum nvgpu_cbc_op {
	/*
	 * Clearing cache involves removing all data stored in
	 * the cache, essentially resetting it to an empty state.
	 * Cleaning cache can be automated based on predefined policies
	 * or triggered manually as needed
	 */
	nvgpu_cbc_op_clear,
	/*
	 * Cleaning cache typically involves removing expired or
	 * stale entries from the cache.
	 */
	nvgpu_cbc_op_clean,
	/*
	 * Invalidating cache involves marking specific cache
	 * entries or entire cache regions as invalid or outdated.
	 */
	nvgpu_cbc_op_invalidate,
};

struct compbit_store_desc {
	/*
	 * The #nvgpu_mem structure holds the physical address
	 * information of the comptag mem.
	 */
	struct nvgpu_mem mem;
	/*
	 * The value that is written to the hardware. This depends on
	 * on the number of ltcs and is not an address.
	 */
	u64 base_hw;
};

struct nvgpu_contig_cbcmempool {
	/*
	 * Pointer to the gk20a structure .
	 */
	struct gk20a *g;
	/*
	 * cookie to hold the information about the IVM.
	 */
	struct tegra_hv_ivm_cookie *cookie;
	/*
	 * base physical address of the contig pool.
	 */
	u64 base_addr;
	/* size of the contig_pool */
	u64 size;
	/* Cpu mapped address for the given pool. */
	void *cbc_cpuva;
	/* Mutex to protect the allocation requests. */
	struct nvgpu_mutex contigmem_mutex;
};

struct nvgpu_cbc {
	/*
	 * - Compbit backing store is a memory buffer to store compressed data
	 * corresponding to total compressible memory.
	 * - In GA10B, 1 ROP tile = 256B data is compressed to 1B compression
	 * bits. i.e. 1 GOB = 512B data is compressed to 2B compbits.
	 * - A comptagline is a collection of compbits corresponding to a
	 * compressible page size. In GA10B, compressible page size is 64KB.
	 *
	 * - GA10B has 2 LTCs with 4 slices each. A 64KB page is distributed
	 *   into 8 slices having 8KB (16 GOBs) data each.
	 * - Thus, each comptagline per slice contains compression status bits
	 * corresponding to 16 GOBs.
	 */
	/*
	 * Total size of the compbit memory.
	 */
	u32 compbit_backing_size;
	/*
	 * Comptags for each cacheline.
	 */
	u32 comptags_per_cacheline;
	u32 gobs_per_comptagline_per_slice;
	u32 max_comptag_lines;
	/*
	 * Bit map allocator for the clients requesting
	 * comptag.
	 */
	struct gk20a_comptag_allocator comp_tags;
	/*
	 * Physical memory information of the comptags backing
	 * store.
	 */
	struct compbit_store_desc compbit_store;
	/*
	 * pointer to the #nvgpu_contig_cbcmempool from where
	 * the contiguous pool for comptag created.
	 */
	struct nvgpu_contig_cbcmempool *cbc_contig_mempool;
};

/**
 * @brief This function is used to initialize compbit backing store support by
 * calling respective HALS.
 *
 * This function is triggered during finalize_poweron multiple times.
 * This function should not return if cbc is not NULL.
 * cbc.init(), which re-writes HW registers that are reset during suspend,
 * should be allowed to execute each time.
 *
 * Check compression support is enabled or not.
 * If Compression support is enabled then initialize the cbc and Call
 *  cbc.alloc_comptags HAL function to initialize the compatag pool.
 *
 * @param g     [in]    Pointer to the gk20a structure.
 *
 * @return 0 on success, < 0 in case of failure.
 */
int nvgpu_cbc_init_support(struct gk20a *g);

/**
 * @brief This function is used to remove compbit backing store support by
 * calling respective HALS.
 *
 * Check compression support is enabled or not.
 * If Compression support is enabled then uninitialize the contig pool
 *  by calling #nvgpu_cbc_contig_deinit.
 * Remove the physical backing store memory of CBC by calling
 *  #nvgpu_dma_free.
 * Remove the support of the bit map allocator by calling
 *  #gk20a_comptag_allocator_destroy.
 * Free the memory allocation for the cbc.
 *
 * @param g     [in]    Pointer to the gk20a structure.
 *
 * @return None.
 */
void nvgpu_cbc_remove_support(struct gk20a *g);
int nvgpu_scrub_cbc_mem(struct gk20a *g, struct sg_table *sgt);

/**
 * @brief This function is used to initialize compbit backing store support.
 *
 * This function initializes the comptag allocation support by calling
 *  different memory allocation APIs
 * Check the validity of the backing store memory by calling #nvgpu_mem_is_valid
 *  and if it is valid return success.
 * Check cbc_contig_mempool attached to g.cbc is valid , if it is not valid
 *  the call #nvgpu_cbc_contig_init to initialize the cbc_contig_mempool.
 * Initialize the CBC contig_pool from the memory backed by the contig_pool.
 *
 * @param g			[in]    Pointer to the gk20a structure.
 * @param compbit_backing_size	[in]	Size of the total compbit memory requested.
 * @param vidmem_alloc		[in]	The requested memory is vidmem or not. The
 *					value is always false.
 *
 * @return 0 on success, < 0 in case of failure.
 */
int nvgpu_cbc_alloc(struct gk20a *g, size_t compbit_backing_size,
			bool vidmem_alloc);

/**
 * @brief This function is used to initialize contiguous memory pool used
 *  for compbit backing store allocation.
 *
 * Allocate required memory for #nvgpu_contig_cbcmempool.
 * Call #nvgpu_dt_read_u32_index to get the dt entry with
 *  "phys_contiguous_mempool" and obtain the pool-id.
 * Call #nvgpu_ivm_mempool_reserve and #nvgpu_ivm_mempool_map with
 *  the pool-id to make the GPU driver to access the contig-pool.
 * Call #nvgpu_nvlink_prot_update to update the required protection
 *  for the contig-pool.
 * Fill the physical address information by calling #nvgpu_init_cbc_contig_pa.
 *
 * @param g                     [in]    Pointer to the gk20a structure.
 *
 * @return 0 on success, < 0 in case of failure.
 */
int  nvgpu_cbc_contig_init(struct gk20a *g);

/**
 * @brief This function is used to uninitialize contiguous memory pool used
 *  for compbit backing store allocation.
 *
 * Call #nvgpu_ivm_mempool_unreserve and #nvgpu_ivm_mempool_unmap with
 *  contig-pool attached in #nvgpu_cbc.
 * Free all the allocated memory by #nvgpu_cbc_contig_init.
 *
 * @param g                     [in]    Pointer to the gk20a structure.
 *
 * @return None.
 */
void nvgpu_cbc_contig_deinit(struct gk20a *g);
#endif
#endif /* NVGPU_CBC_H */
