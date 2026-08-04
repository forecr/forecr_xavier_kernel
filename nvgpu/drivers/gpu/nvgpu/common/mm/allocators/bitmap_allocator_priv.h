/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef BITMAP_ALLOCATOR_PRIV_H
#define BITMAP_ALLOCATOR_PRIV_H

/**
 * @file
 *
 * Implementation of a bitmap allocator.
 */

#include <nvgpu/rbtree.h>
#include <nvgpu/kmem.h>

struct nvgpu_allocator;

/**
 * Structure to hold the implementation details of the bitmap allocator.
 */
struct nvgpu_bitmap_allocator {
	/**
	 * Pointer to the common allocator structure.
	 */
	struct nvgpu_allocator *owner;

	/**
	 * Base address of the space.
	 */
	u64 base;

	/**
	 * Length of the space.
	 */
	u64 length;

	/**
	 * Size that corresponds to 1 bit.
	 */
	u64 blk_size;

	/**
	 * Bit shift to divide by blk_size.
	 */
	u64 blk_shift;

	/**
	 * Number of allocatable bits.
	 */
	u64 num_bits;

	/**
	 * Offset of bitmap.
	 */
	u64 bit_offs;

	/**
	 * Optimization for making repeated allocations faster. Keep track of
	 * the next bit after the most recent allocation. This is where the next
	 * search will start from. This should make allocation faster in cases
	 * where lots of allocations get made one after another. It shouldn't
	 * have a negative impact on the case where the allocator is fragmented.
	 */
	u64 next_blk;

	/**
	 * The actual bitmap used for allocations.
	 */
	unsigned long *bitmap;

	/**
	 * Tree of outstanding allocations.
	 */
	struct nvgpu_rbtree_node *allocs;

	/**
	 * Metadata cache of allocations (contains address and size of
	 * allocations).
	 */
	struct nvgpu_kmem_cache *meta_data_cache;

	/**
	 * Configuration flags of the allocator. See \a GPU_ALLOC_* flags.
	 */
	u64 flags;

	/**
	 * Boolean to indicate if the allocator has been fully initialized.
	 */
	bool inited;

	/**
	 * Statistics: track the number of non-fixed allocations.
	 */
	u64 nr_allocs;

	/**
	 * Statistics: track the number of fixed allocations.
	 */
	u64 nr_fixed_allocs;

	/**
	 * Statistics: total number of bytes allocated for both fixed and non-
	 * fixed allocations.
	 */
	u64 bytes_alloced;

	/**
	 * Statistics: total number of bytes freed for both fixed and non-fixed
	 * allocations.
	 */
	u64 bytes_freed;
};

/**
 * Structure to hold the allocation metadata.
 */
struct nvgpu_bitmap_alloc {
	/**
	 * Base address of the allocation.
	 */
	u64 base;

	/**
	 * Size of the allocation.
	 */
	u64 length;

	/**
	 * RB tree of allocations.
	 */
	struct nvgpu_rbtree_node alloc_entry;
};

/**
 * @brief Given a tree node, retrieve the metdata of the allocation.
 *
 * @param[in] node	Pointer to the tree node.
 *
 * @return pointer to the struct nvgpu_bitmap_alloc of the node.
 */
static inline struct nvgpu_bitmap_alloc *
nvgpu_bitmap_alloc_from_rbtree_node(struct nvgpu_rbtree_node *node)
{
	return (struct nvgpu_bitmap_alloc *)
	((uintptr_t)node - offsetof(struct nvgpu_bitmap_alloc, alloc_entry));
};

/**
 * @brief Given a generic allocator context, retrieve a pointer to the bitmap
 * allocator context structure.
 *
 * @param[in] a		Pointer to nvgpu allocator.
 *
 * @return pointer to the struct nvgpu_bitmap_allocator.
 */
static inline struct nvgpu_bitmap_allocator *bitmap_allocator(
	struct nvgpu_allocator *a)
{
	return (struct nvgpu_bitmap_allocator *)(a)->priv;
}


#endif
