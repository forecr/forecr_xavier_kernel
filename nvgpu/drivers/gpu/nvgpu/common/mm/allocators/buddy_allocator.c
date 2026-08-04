// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/allocator.h>
#include <nvgpu/kmem.h>
#include <nvgpu/bug.h>
#include <nvgpu/log2.h>
#include <nvgpu/barrier.h>
#include <nvgpu/mm.h>
#include <nvgpu/vm.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/string.h>

#include "buddy_allocator_priv.h"

/* Some other buddy allocator functions. */
static struct nvgpu_buddy *balloc_free_buddy(struct nvgpu_buddy_allocator *a,
					     u64 addr);
static void balloc_coalesce(struct nvgpu_buddy_allocator *a,
			    struct nvgpu_buddy *b);
static void balloc_do_free_fixed(struct nvgpu_buddy_allocator *a,
				 struct nvgpu_fixed_alloc *falloc);

/*
 * This function is not present in older kernel's list.h code.
 */
#ifndef list_last_entry
#define list_last_entry(ptr, type, member)	\
	list_entry((ptr)->prev, type, member)
#endif

/*
 * GPU buddy allocator for various address spaces.
 *
 * Current limitations:
 *   o  A fixed allocation could potentially be made that borders PDEs with
 *      different PTE sizes. This would require that fixed buffer to have
 *      different sized PTEs for different parts of the allocation. Probably
 *      best to just require PDE alignment for fixed address allocs.
 *
 *   o  It is currently possible to make an allocator that has a buddy alignment
 *      out of sync with the PDE block size alignment. A simple example is a
 *      32GB address space starting at byte 1. Every buddy is shifted off by 1
 *      which means each buddy corresponf to more than one actual GPU page. The
 *      best way to fix this is probably just require PDE blocksize alignment
 *      for the start of the address space. At the moment all allocators are
 *      easily PDE aligned so this hasn't been a problem.
 */

/**
 * @brief Maps a page size to a corresponding PTE (Page Table Entry) size.
 *
 * This function checks the page size against known sizes and returns a
 * corresponding PTE size code. It first checks if the allocator is for GVA
 * space and then ensures that the page size is valid within that context,
 * returning the appropriate PTE size code or an invalid code if the page size
 * does not match expected values.
 *
 * The steps performed by the function are as follows:
 * -# Check if the allocator is not for GVA space by performing a bitwise AND
 *    operation between the allocator's flags and #GPU_ALLOC_GVA_SPACE and
 *    comparing the result to zero. If the allocator is not for GVA space,
 *    return a PTE size that can be used for any page size.
 * -# If the allocator is for GVA space, compare the given page size with the
 *    big page size of the VM. If they match, return the big PTE size.
 * -# If the given page size is the standard small page size (#SZ_4K), return
 *    the small PTE size.
 * -# If the given page size is set to a value that indicates any page size is
 *    acceptable (#BALLOC_PTE_SIZE_ANY), return the small PTE size, as only two
 *    types of PTE sizes are allowed when GVA space is enabled.
 * -# If none of the above conditions are met, return an invalid PTE size to
 *    indicate that the given page size does not correspond to a valid PTE size.
 *
 * @param [in] a           The buddy allocator structure containing allocator
 *                         flags and a pointer to the associated VM.
 * @param [in] page_size   The page size for which the PTE size is to be determined.
 *
 * @return The corresponding PTE size code, which can be any of the following:
 *         #BALLOC_PTE_SIZE_ANY for any page size when not in GVA space,
 *         #BALLOC_PTE_SIZE_BIG for a big page size,
 *         #BALLOC_PTE_SIZE_SMALL for a small page size, or
 *         #BALLOC_PTE_SIZE_INVALID if the page size is not recognized.
 */
static u32 nvgpu_balloc_page_size_to_pte_size(struct nvgpu_buddy_allocator *a,
					      u32 page_size)
{
	if ((a->flags & GPU_ALLOC_GVA_SPACE) == 0ULL) {
		return BALLOC_PTE_SIZE_ANY;
	}

	/*
	 * Make sure the page size is actually valid!
	 */
	if (page_size == a->vm->huge_page_size) {
		return BALLOC_PTE_SIZE_HUGE;
	} else if (page_size == a->vm->big_page_size) {
		return BALLOC_PTE_SIZE_BIG;
	} else if (page_size == SZ_4K) {
		return BALLOC_PTE_SIZE_SMALL;
	} else if (page_size == BALLOC_PTE_SIZE_ANY) {
		/* With gva_space enabled, only 3 types of PTE sizes allowed */
		return BALLOC_PTE_SIZE_SMALL;
	} else {
		return BALLOC_PTE_SIZE_INVALID;
	}
}

/*
 * Pick a suitable maximum order for this allocator.
 *
 * Hueristic: Just guessing that the best max order is the largest single
 * block that will fit in the address space.
 */

/**
 * @brief Computes the maximum order for a buddy allocator.
 *
 * This function calculates the maximum order of allocation that can be
 * managed by the buddy allocator based on the number of blocks it manages.
 * It ensures that the maximum order does not exceed the predefined limit.
 * If the current maximum order is unset or greater than the true maximum,
 * it is updated to the true maximum order.
 *
 * The steps performed by the function are as follows:
 * -# Calculate the true maximum order by finding the base-2 logarithm of the
 *    number of blocks managed by the allocator using nvgpu_ilog2().
 * -# Check if the true maximum order exceeds the predefined maximum order limit
 *    #GPU_BALLOC_MAX_ORDER. If it does, output a debug message and assert.
 * -# If the allocator's maximum order is unset (zero) or greater than the true
 *    maximum order, update the allocator's maximum order to the true maximum
 *    order.
 *
 * @param [in] a  The buddy allocator structure containing the number of blocks
 *                it manages and the current maximum order.
 *
 * @return None.
 */
static void balloc_compute_max_order(struct nvgpu_buddy_allocator *a)
{
	u64 true_max_order = nvgpu_ilog2(a->blks);

	if (true_max_order > GPU_BALLOC_MAX_ORDER) {
		alloc_dbg(balloc_owner(a),
			  "Oops: Can't manage more than 1 Exabyte memory");
		nvgpu_do_assert();
	}

	if ((a->max_order == 0ULL) || (a->max_order > true_max_order)) {
		a->max_order = true_max_order;
	}
}

/*
 * Since we can only allocate in chucks of a->blk_size we need to trim off
 * any excess data that is not aligned to a->blk_size.
 */

/**
 * @brief Aligns the start and end addresses of the buddy allocator.
 *
 * This function aligns the start and end addresses of the buddy allocator to
 * the block size boundaries and calculates the number of blocks that can be
 * managed by the allocator. It ensures that the block size is non-zero and
 * that the number of blocks is non-zero after alignment, otherwise it returns
 * an error.
 *
 * The steps performed by the function are as follows:
 * -# Align the base address of the allocator to the block size boundary and
 *    store it as the start address.
 * -# Use WARN_ON() to check if the start address does not match the base
 *    address, which may indicate a potential issue.
 * -# Assert that the block size is greater than zero using nvgpu_assert().
 * -# Calculate the end address by adding the length to the base address,
 *    then aligning it down to the block size boundary.
 * -# Calculate the count of blocks by subtracting the start address from the
 *    end address.
 * -# Right shift the count by the block shift value to get the number of
 *    blocks and store it in the allocator structure.
 * -# If the number of blocks is zero, set the error code to -EINVAL and go to
 *    the fail label.
 * -# At the fail label, return the error code.
 *
 * @param [in] a  The buddy allocator structure containing the base address,
 *                length, block size, and block shift value.
 *
 * @return 0 if alignment is successful, -EINVAL if the block size or number of
 *         blocks is zero after alignment.
 */
static int balloc_allocator_align(struct nvgpu_buddy_allocator *a)
{
	int err = 0;
	u64 blks;
	a->start = NVGPU_ALIGN(a->base, a->blk_size);
	WARN_ON(a->start != a->base);
	nvgpu_assert(a->blk_size > 0ULL);
	a->end   = nvgpu_safe_add_u64(a->base, a->length) &
						~(a->blk_size - 1U);
	a->count = nvgpu_safe_sub_u64(a->end, a->start);
	blks = a->count >> a->blk_shift;
	if (blks == 0ULL) {
		err = -EINVAL;
		goto fail;
	}
	a->blks  = blks;

fail:
	return err;
}

/*
 * Pass NULL for parent if you want a top level buddy.
 */

/**
 * @brief Creates and initializes a new buddy block structure.
 *
 * Allocates memory for a new buddy block from the buddy allocator's cache and
 * initializes it. The new buddy block's parent, start, order, and end fields
 * are set according to the provided arguments. The end field is calculated
 * by shifting 1 left by the order number of times, multiplying by the block
 * size, and then adding the start address. The PTE size is set to a default
 * value indicating that any PTE size is acceptable.
 *
 * The steps performed by the function are as follows:
 * -# Allocate memory for a new buddy block from the buddy allocator's cache
 *    using nvgpu_kmem_cache_alloc(). If the allocation fails, return NULL.
 * -# Initialize the allocated buddy block to zero using memset().
 * -# Set the parent of the new buddy block to the provided parent parameter.
 * -# Set the start address of the new buddy block to the provided start parameter.
 * -# Set the order of the new buddy block to the provided order parameter.
 * -# Calculate the end address of the new buddy block by shifting 1 left by
 *    the order number of times, multiplying by the allocator's block size,
 *    and adding the start address using nvgpu_safe_mult_u64() and
 *    nvgpu_safe_add_u64().
 * -# Set the PTE size of the new buddy block to #BALLOC_PTE_SIZE_ANY.
 * -# Return a pointer to the newly created buddy block.
 *
 * @param [in] a       The buddy allocator from which to allocate the new buddy.
 * @param [in] parent  The parent buddy block of the new buddy block.
 * @param [in] start   The start address for the new buddy block.
 * @param [in] order   The order of the new buddy block.
 *
 * @return A pointer to the newly allocated and initialized buddy block, or
 *         NULL if the memory allocation fails.
 */
static struct nvgpu_buddy *balloc_new_buddy(struct nvgpu_buddy_allocator *a,
					    struct nvgpu_buddy *parent,
					    u64 start, u64 order)
{
	struct nvgpu_buddy *new_buddy;

	new_buddy = nvgpu_kmem_cache_alloc(a->buddy_cache);
	if (new_buddy == NULL) {
		return NULL;
	}

	(void) memset(new_buddy, 0, sizeof(struct nvgpu_buddy));

	new_buddy->parent = parent;
	new_buddy->start = start;
	new_buddy->order = order;
	new_buddy->end = nvgpu_safe_mult_u64(U64(1) << order, a->blk_size);
	new_buddy->end = nvgpu_safe_add_u64(new_buddy->end, start);
	new_buddy->pte_size = BALLOC_PTE_SIZE_ANY;

	return new_buddy;
}

/**
 * @brief Adds a buddy block to a list within the buddy allocator.
 *
 * This function adds a buddy block to a specified list in the buddy allocator.
 * It first checks if the buddy block is already in a list and asserts if so.
 * Depending on whether the allocator is for GVA space and the PTE size of the
 * buddy block, it adds the buddy block to either the head or tail of the list.
 * It then marks the buddy block as being in a list.
 *
 * The steps performed by the function are as follows:
 * -# Check if the buddy block is already in a list using buddy_is_in_list() (See
 *    #nvgpu_buddy_allocator_flag_ops). If it is, output a debug message and assert using
 *    nvgpu_do_assert().
 * -# If the allocator is for GVA space and the buddy block's PTE size is big,
 *    add the buddy block to the tail of the list using nvgpu_list_add_tail().
 *    This is done to optimize the search for available blocks.
 * -# If the allocator is not for GVA space or the buddy block's PTE size is
 *    not big, add the buddy block to the head of the list using
 *    nvgpu_list_add().
 * -# Mark the buddy block as being in a list using buddy_set_in_list() (See
 *    #nvgpu_buddy_allocator_flag_ops).
 *
 * @param [in] a     The buddy allocator to which the buddy block belongs.
 * @param [in] b     The buddy block to be added to the list.
 * @param [in] list  The list to which the buddy block will be added.
 *
 * @return None.
 */
static void balloc_buddy_list_do_add(struct nvgpu_buddy_allocator *a,
				     struct nvgpu_buddy *b,
				     struct nvgpu_list_node *list)
{
	if (buddy_is_in_list(b)) {
		alloc_dbg(balloc_owner(a),
			  "Oops: adding added buddy (%llu:0x%llx)",
			  b->order, b->start);
		nvgpu_do_assert();
	}

	/*
	 * Add big PTE blocks to the tail, small to the head for GVA spaces.
	 * This lets the code that checks if there are available blocks check
	 * without cycling through the entire list.
	 */
	if (((a->flags & GPU_ALLOC_GVA_SPACE) != 0ULL) &&
		(b->pte_size == BALLOC_PTE_SIZE_BIG)) {
		nvgpu_list_add_tail(&b->buddy_entry, list);
	} else {
		nvgpu_list_add(&b->buddy_entry, list);
	}

	buddy_set_in_list(b);
}

/**
 * @brief Removes a buddy block from a list within the buddy allocator.
 *
 * This function removes a buddy block from whichever list it is currently in
 * within the buddy allocator. It first checks if the buddy block is actually
 * in a list and asserts if not. It then removes the buddy block from the list
 * and marks it as not being in a list.
 *
 * The steps performed by the function are as follows:
 * -# Check if the buddy block is already in a list using buddy_is_in_list() (See
 *    #nvgpu_buddy_allocator_flag_ops). If it is, output a debug message and assert using
 *    nvgpu_do_assert().
 * -# Remove the buddy block from the list using nvgpu_list_del().
 * -# Clear the in-list flag for the buddy block using buddy_clr_in_list() (See
 *    #nvgpu_buddy_allocator_flag_ops).
 *
 * @param [in] a  The buddy allocator from which the buddy block is being removed.
 * @param [in] b  The buddy block to be removed from the list.
 *
 * @return None.
 */
static void balloc_buddy_list_do_rem(struct nvgpu_buddy_allocator *a,
				     struct nvgpu_buddy *b)
{
	if (!buddy_is_in_list(b)) {
		alloc_dbg(balloc_owner(a),
			  "Oops: removing removed buddy (%llu:0x%llx)",
			  b->order, b->start);
		nvgpu_do_assert();
	}

	nvgpu_list_del(&b->buddy_entry);
	buddy_clr_in_list(b);
}

/*
 * Add a buddy to one of the buddy lists and deal with the necessary
 * book keeping. Adds the buddy to the list specified by the buddy's order.
 */

/**
 * @brief Adds a buddy block to the appropriate order list in the allocator.
 *
 * This function adds a buddy block to the list corresponding to its order in
 * the buddy allocator. It also increments the length of the list for that
 * particular order to keep track of the number of blocks in the list.
 *
 * The steps performed by the function are as follows:
 * -# Call balloc_buddy_list_do_add() to add the buddy block to the list
 *    corresponding to its order. The list is retrieved using balloc_get_order_list().
 * -# Increment the length of the buddy list for the given order by one to
 *    maintain an accurate count of the blocks in the list. This is done using
 *    nvgpu_safe_add_u64().
 *
 * @param [in] a  The buddy allocator to which the buddy block is being added.
 * @param [in] b  The buddy block to be added to the allocator's list.
 *
 * @return None.
 */
static void balloc_blist_add(struct nvgpu_buddy_allocator *a,
			     struct nvgpu_buddy *b)
{
	balloc_buddy_list_do_add(a, b, balloc_get_order_list(a, b->order));
	a->buddy_list_len[b->order] =
		nvgpu_safe_add_u64(a->buddy_list_len[b->order], 1ULL);
}

/**
 * @brief Removes a buddy block from the appropriate order list in the allocator.
 *
 * This function removes a buddy block from the list corresponding to its order
 * in the buddy allocator. It also decrements the length of the list for that
 * particular order to keep track of the number of blocks in the list.
 *
 * The steps performed by the function are as follows:
 * -# Call balloc_buddy_list_do_rem() to remove the buddy block from the list
 *    it is currently in.
 * -# Assert that the length of the buddy list for the given order is greater
 *    than zero using nvgpu_assert().
 * -# Decrement the length of the buddy list for the given order by one to
 *    maintain an accurate count of the blocks in the list.
 *
 * @param [in] a  The buddy allocator from which the buddy block is being removed.
 * @param [in] b  The buddy block to be removed from the allocator's list.
 *
 * @return None.
 */
static void balloc_blist_rem(struct nvgpu_buddy_allocator *a,
			     struct nvgpu_buddy *b)
{
	balloc_buddy_list_do_rem(a, b);
	nvgpu_assert(a->buddy_list_len[b->order] > 0ULL);
	a->buddy_list_len[b->order]--;
}

/**
 * @brief Calculates the order of a given length within the buddy allocator.
 *
 * This function computes the order of a block of memory of a specified length
 * in terms of the allocator's block size. The order is the power of two that
 * represents the number of minimum-sized blocks needed to satisfy the length.
 *
 * The steps performed by the function are as follows:
 * -# If the length is zero, return an order of zero as no blocks are needed.
 * -# Decrement the length by one to handle the case where the length is an
 *    exact power of two.
 * -# Right shift the length by the block shift value of the allocator to get
 *    the number of minimum-sized blocks needed.
 * -# Call nvgpu_fls() to find the position of the first (most-significant) bit
 *    set in the length, which gives the order.
 *
 * @param [in] a     The buddy allocator structure containing the block shift value.
 * @param [in] len   The length of memory for which to calculate the order.
 *
 * @return The order of the given length within the buddy allocator.
 */
static u64 balloc_get_order(struct nvgpu_buddy_allocator *a, u64 len)
{
	if (len == 0U) {
		return 0;
	}

	len--;
	len >>= a->blk_shift;

	return nvgpu_fls(len);
}

/**
 * @brief Determines the maximum order that can fit within a given range.
 *
 * This function calculates the largest order of blocks that can fit within the
 * specified start and end addresses in the buddy allocator. It ensures that the
 * calculated order does not exceed the allocator's maximum order.
 *
 * The steps performed by the function are as follows:
 * -# Calculate the size by subtracting the start address from the end address
 *    and then right shifting by the block shift value of the allocator using
 *    nvgpu_safe_sub_u64().
 * -# If the size is greater than zero, calculate
 *    the logarithm base 2 of the size using nvgpu_ilog2() to find the order,
 *    and then return the minimum of that order and the allocator's maximum
 *    order using min_t().
 * -# If the size is zero or less, return the predefined maximum order for the
 *    GPU buddy allocator #GPU_BALLOC_MAX_ORDER.
 *
 * @param [in] a      The buddy allocator structure containing the block shift
 *                    value and the maximum order.
 * @param [in] start  The start address of the range.
 * @param [in] end    The end address of the range.
 *
 * @return The maximum order that can fit within the given range without
 *         exceeding the allocator's maximum order, or the predefined maximum
 *         order if the size is zero or less.
 */
static u64 balloc_max_order_in(struct nvgpu_buddy_allocator *a,
				 u64 start, u64 end)
{
	u64 size = nvgpu_safe_sub_u64(end, start) >> a->blk_shift;

	if (size > 0U) {
		return min_t(u64, nvgpu_ilog2(size), a->max_order);
	} else {
		return GPU_BALLOC_MAX_ORDER;
	}
}

/*
 * Initialize the buddy lists.
 */

/**
 * @brief Initializes the order lists for the buddy allocator.
 *
 * This function sets up the linked lists for each order within the buddy
 * allocator. It initializes the lists, then creates and adds buddies to the
 * lists based on the maximum order that can fit between the start and end
 * addresses of the allocator. If buddy allocation fails, it cleans up any
 * allocated buddies.
 *
 * The steps performed by the function are as follows:
 * -# Initialize all order lists within the allocator using nvgpu_init_list_node().
 * -# Loop from the start address to the end address of the allocator:
 *    -# Calculate the maximum order that can fit in the current range using
 *       balloc_max_order_in().
 *    -# Allocate a new buddy with the calculated order using balloc_new_buddy().
 *       If allocation fails, go to cleanup.
 *    -# Add the new buddy to the appropriate order list using balloc_blist_add().
 *    -# Increment the start address by the length corresponding to the order
 *       using nvgpu_safe_add_u64() and balloc_order_to_len().
 * -# If all buddies are added successfully, return 0.
 * -# In the cleanup section, loop through all order lists:
 *    -# If the list is not empty, remove the first buddy from the list using
 *       balloc_blist_rem().
 *    -# Free the removed buddy using nvgpu_kmem_cache_free().
 * -# If cleanup is needed, return -ENOMEM to indicate memory allocation failure.
 *
 * @param [in] a  The buddy allocator structure containing the start and end
 *                addresses, and the buddy cache for allocating new buddies.
 *
 * @return 0 on success, -ENOMEM on memory allocation failure.
 */
static int balloc_init_lists(struct nvgpu_buddy_allocator *a)
{
	u32 i;
	u64 bstart, bend, order;
	struct nvgpu_buddy *buddy;

	bstart = a->start;
	bend = a->end;

	/* First make sure the LLs are valid. */
	for (i = 0U; i < GPU_BALLOC_ORDER_LIST_LEN; i++) {
		nvgpu_init_list_node(balloc_get_order_list(a, i));
	}

	while (bstart < bend) {
		order = balloc_max_order_in(a, bstart, bend);

		buddy = balloc_new_buddy(a, NULL, bstart, order);
		if (buddy == NULL) {
			goto cleanup;
		}

		balloc_blist_add(a, buddy);
		bstart = nvgpu_safe_add_u64(bstart,
					balloc_order_to_len(a, order));
	}

	return 0;

cleanup:
	for (i = 0U; i < GPU_BALLOC_ORDER_LIST_LEN; i++) {
		if (!nvgpu_list_empty(balloc_get_order_list(a, i))) {
			buddy = nvgpu_list_first_entry(
					balloc_get_order_list(a, i),
					nvgpu_buddy, buddy_entry);
			balloc_blist_rem(a, buddy);
			nvgpu_kmem_cache_free(a->buddy_cache, buddy);
		}
	}

	return -ENOMEM;
}

/*
 * Clean up and destroy the passed allocator.
 * Walk the allocator for any pending allocations.
 * Free up all pending allocations.
 * Free any memory allocated a allocator init time.
 * Destroy the lock and bzero the allocator completely.
 */

/**
 * @brief Destroys the buddy allocator and frees all associated memory.
 *
 * This function cleans up all the allocations and internal structures of the
 * buddy allocator. It frees fixed allocations, coalesces and frees buddy blocks,
 * and finally destroys the buddy cache and the allocator itself.
 *
 * The steps performed by the function are as follows:
 * -# Lock the allocator to prevent concurrent access using alloc_lock().
 * -# Finalize the allocator's debug information using nvgpu_fini_alloc_debug().
 * -# Free all fixed allocations by iterating through the fixed_allocs RB tree
 *    and unlinking and freeing each node using nvgpu_rbtree_unlink() and
 *    balloc_do_free_fixed().
 * -# Free all outstanding buddy allocations by iterating through the
 *    alloced_buddies RB tree, freeing each buddy using balloc_free_buddy(),
 *    adding them back to the free list using balloc_blist_add(), and then
 *    attempting to coalesce buddies using balloc_coalesce().
 * -# Clean up all unallocated buddies by iterating through each order list and
 *    freeing each buddy using balloc_blist_rem() and nvgpu_kmem_cache_free().
 * -# Check for any inconsistencies in the allocator's tracking counters and
 *    assert if any are found using BUG_ON() and nvgpu_do_assert().
 * -# Destroy the buddy cache using nvgpu_kmem_cache_destroy().
 * -# Free the allocator structure itself using nvgpu_kfree().
 * -# Unlock the allocator using alloc_unlock().
 *
 * @param [in] na  The generic allocator structure which contains the buddy
 *                 allocator to be destroyed.
 *
 * @return None.
 */
static void nvgpu_buddy_allocator_destroy(struct nvgpu_allocator *na)
{
	u32 i;
	struct nvgpu_rbtree_node *node = NULL;
	struct nvgpu_buddy *bud;
	struct nvgpu_fixed_alloc *falloc;
	struct nvgpu_buddy_allocator *a = buddy_allocator(na);

	alloc_lock(na);

	nvgpu_fini_alloc_debug(na);

	/*
	 * Free the fixed allocs first.
	 */
	nvgpu_rbtree_enum_start(0, &node, a->fixed_allocs);
	while (node != NULL) {
		falloc = nvgpu_fixed_alloc_from_rbtree_node(node);

		nvgpu_rbtree_unlink(node, &a->fixed_allocs);
		balloc_do_free_fixed(a, falloc);

		nvgpu_rbtree_enum_start(0, &node, a->fixed_allocs);
	}

	/*
	 * And now free all outstanding allocations.
	 */
	nvgpu_rbtree_enum_start(0, &node, a->alloced_buddies);
	while (node != NULL) {
		bud = nvgpu_buddy_from_rbtree_node(node);

		(void) balloc_free_buddy(a, bud->start);
		balloc_blist_add(a, bud);
		balloc_coalesce(a, bud);

		nvgpu_rbtree_enum_start(0, &node, a->alloced_buddies);
	}

	/*
	 * Now clean up the unallocated buddies.
	 */
	for (i = 0U; i < GPU_BALLOC_ORDER_LIST_LEN; i++) {
		BUG_ON(a->buddy_list_alloced[i] != 0U);

		while (!nvgpu_list_empty(balloc_get_order_list(a, i))) {
			bud = nvgpu_list_first_entry(
						balloc_get_order_list(a, i),
						nvgpu_buddy, buddy_entry);
			balloc_blist_rem(a, bud);
			nvgpu_kmem_cache_free(a->buddy_cache, bud);
		}

		if (a->buddy_list_len[i] != 0U) {
			nvgpu_info(na->g,
					"Excess buddies!!! (%d: %llu)",
				i, a->buddy_list_len[i]);
			nvgpu_do_assert();
		}
		if (a->buddy_list_split[i] != 0U) {
			nvgpu_info(na->g,
					"Excess split nodes!!! (%d: %llu)",
				i, a->buddy_list_split[i]);
			nvgpu_do_assert();
		}
		if (a->buddy_list_alloced[i] != 0U) {
			nvgpu_info(na->g,
					"Excess alloced nodes!!! (%d: %llu)",
				i, a->buddy_list_alloced[i]);
			nvgpu_do_assert();
		}
	}

	nvgpu_kmem_cache_destroy(a->buddy_cache);
	nvgpu_kfree(nvgpu_alloc_to_gpu(na), a);

	alloc_unlock(na);
}

/*
 * Combine the passed buddy if possible. The pointer in @b may not be valid
 * after this as the buddy may be freed.
 *
 * @a must be locked.
 */

/**
 * @brief Coalesces buddy blocks into larger blocks if possible.
 *
 * This function attempts to merge a buddy block with its buddy to form a larger
 * block. It continues to coalesce up the buddy hierarchy as long as the blocks
 * are not allocated or split. If coalescing is possible, it updates the parent
 * block and the allocator's tracking of split blocks.
 *
 * The steps performed by the function are as follows:
 * -# Loop while the current buddy block is neither allocated nor split:
 *    -# If the buddy block has no buddy, return as no coalescing is possible.
 *    -# If the buddy block's buddy is either allocated or split, return as
 *       coalescing is not possible.
 *    -# Retrieve the parent of the current buddy block.
 *    -# Remove the current buddy block and its buddy from their respective
 *       order lists using balloc_blist_rem().
 *    -# Clear the split flag on the parent buddy block using buddy_clr_split() (See
 *       #nvgpu_buddy_allocator_flag_ops).
 *    -# Assert that the split count for the parent's order is greater than zero
 *       using nvgpu_assert().
 *    -# Decrement the split count for the parent's order.
 *    -# Add the parent buddy block to the appropriate order list using
 *       balloc_blist_add().
 *    -# Free the memory of the current buddy block and its buddy using
 *       nvgpu_kmem_cache_free().
 *    -# Update the current buddy block to the parent for the next iteration.
 *
 * @param [in] a  The buddy allocator from which the buddy block is being coalesced.
 * @param [in] b  The buddy block to coalesce.
 *
 * @return None.
 */
static void balloc_coalesce(struct nvgpu_buddy_allocator *a,
			    struct nvgpu_buddy *b)
{
	struct nvgpu_buddy *parent;

	while (!buddy_is_alloced(b) && !buddy_is_split(b)) {
		/*
		 * If both our buddy and I are both not allocated and not split
		 *  then we can coalesce ourselves.
		 */
		if (b->buddy == NULL) {
			return;
		}
		if (buddy_is_alloced(b->buddy) || buddy_is_split(b->buddy)) {
			return;
		}

		parent = b->parent;

		balloc_blist_rem(a, b);
		balloc_blist_rem(a, b->buddy);

		buddy_clr_split(parent);
		nvgpu_assert(a->buddy_list_split[parent->order] > 0ULL);
		a->buddy_list_split[parent->order]--;
		balloc_blist_add(a, parent);

		/* Clean up the remains. */
		nvgpu_kmem_cache_free(a->buddy_cache, b->buddy);
		nvgpu_kmem_cache_free(a->buddy_cache, b);

		b = parent;
	}
}

/*
 * Split a buddy into two new buddies who are 1/2 the size of the parent buddy.
 *
 * @a must be locked.
 */

/**
 * @brief Splits a buddy block into two smaller buddy blocks.
 *
 * This function divides a buddy block into two smaller blocks of the next lower
 * order. It updates the parent block to indicate it has been split and assigns
 * PTE sizes to the new buddies if necessary, based on whether the allocator is
 * managing a GVA space and the size of the buddies relative to the PDE block size.
 *
 * The steps performed by the function are as follows:
 * -# Assert that the order of the buddy block to be split is greater than zero.
 * -# Allocate a new buddy block for the left half of the split using
 *    balloc_new_buddy(). If the allocation fails, return -ENOMEM.
 * -# Calculate the halfway point between the start and end of the buddy block
 *    to be split using nvgpu_safe_sub_u64() and division.
 * -# Calculate the start address for the new right buddy block using
 *    nvgpu_safe_add_u64() and the calculated halfway point.
 * -# Allocate a new buddy block for the right half of the split using
 *    balloc_new_buddy(). If the allocation fails, free the previously allocated
 *    left buddy block using nvgpu_kmem_cache_free() and return -ENOMEM.
 * -# Set the split flag on the parent buddy block using buddy_set_split() (See
 *    #nvgpu_buddy_allocator_flag_ops).
 * -# Increment the split count for the parent's order in the allocator's list
 *    of split buddies using nvgpu_safe_add_u64().
 * -# Set the left and right pointers of the parent buddy block to the newly
 *    created left and right buddy blocks, respectively.
 * -# Set the buddy pointers of the left and right buddy blocks to each other.
 * -# Set the parent pointers of the left and right buddy blocks to the original
 *    parent buddy block.
 * -# If the allocator is for GVA space and the order of the new buddy blocks is
 *    less than the PDE block order, assign the passed PTE size to both new
 *    buddy blocks.
 * -# Remove the original buddy block from its current list using
 *    balloc_blist_rem().
 * -# Add the new left and right buddy blocks to the appropriate lists using
 *    balloc_blist_add().
 * -# Return 0 to indicate success.
 *
 * @param [in] a         The buddy allocator from which the buddy block is being split.
 * @param [in] b         The buddy block to be split.
 * @param [in] pte_size  The PTE size to assign to the new buddy blocks if necessary.
 *
 * @return 0 on success, -ENOMEM on memory allocation failure.
 */
static int balloc_split_buddy(struct nvgpu_buddy_allocator *a,
			      struct nvgpu_buddy *b, u32 pte_size)
{
	struct nvgpu_buddy *left, *right;
	u64 half;
	u64 new_buddy_start;

	nvgpu_assert(b->order > 0ULL);
	left = balloc_new_buddy(a, b, b->start, b->order - 1U);
	if (left == NULL) {
		return -ENOMEM;
	}

	half = nvgpu_safe_sub_u64(b->end, b->start) / 2U;

	new_buddy_start = nvgpu_safe_add_u64(b->start, half);
	right = balloc_new_buddy(a, b, new_buddy_start, b->order - 1U);
	if (right == NULL) {
		nvgpu_kmem_cache_free(a->buddy_cache, left);
		return -ENOMEM;
	}

	buddy_set_split(b);
	a->buddy_list_split[b->order] =
		nvgpu_safe_add_u64(a->buddy_list_split[b->order], 1ULL);

	b->left = left;
	b->right = right;
	left->buddy = right;
	right->buddy = left;
	left->parent = b;
	right->parent = b;

	/*
	 * Potentially assign a PTE size to the new buddies. The obvious case is
	 * when we don't have a GPU VA space; just leave it alone. When we do
	 * have a GVA space we need to assign the passed PTE size to the buddy
	 * only if the buddy is less than the PDE block size. This is because if
	 * the buddy is less than the PDE block size then the buddy's  parent
	 * may already have a PTE size. Thus we can only allocate this buddy to
	 * mappings with that PTE size (due to the large/small PTE separation
	 * requirement).
	 *
	 * When the buddy size is greater than or equal to the block size then
	 * we can leave the buddies PTE field alone since the PDE block has yet
	 * to be assigned a PTE size.
	 */
	if (((a->flags & GPU_ALLOC_GVA_SPACE) != 0ULL) &&
		(left->order < a->pte_blk_order)) {
		left->pte_size = pte_size;
		right->pte_size = pte_size;
	}

	balloc_blist_rem(a, b);
	balloc_blist_add(a, left);
	balloc_blist_add(a, right);

	return 0;
}

/*
 * Place the passed buddy into the RB tree for allocated buddies. Never fails
 * unless the passed entry is a duplicate which is a bug.
 *
 * @a must be locked.
 */

/**
 * @brief Insert a buddy into the allocated buddies red-black tree and update allocation stats.
 *
 * The function 'balloc_alloc_buddy' is responsible for inserting a given buddy into the
 * allocated buddies red-black tree of the buddy allocator. It also sets the allocated flag
 * for the buddy and updates the count of allocated buddies for the corresponding order.
 * The steps performed by the function are as follows:
 * -# Set the start and end keys of the buddy's allocated entry with the buddy's start and end addresses.
 * -# Insert the buddy's allocated entry into the allocated buddies red-black tree using nvgpu_rbtree_insert().
 * -# Set the buddy's status to allocated using buddy_set_alloced() (See #nvgpu_buddy_allocator_flag_ops).
 * -# Increment the count of allocated buddies for the buddy's order in the buddy allocator's list of allocated buddies.
 *
 * @param [in]  a  Pointer to the buddy allocator structure.
 * @param [in]  b  Pointer to the buddy to be allocated.
 */
static void balloc_alloc_buddy(struct nvgpu_buddy_allocator *a,
			       struct nvgpu_buddy *b)
{
	b->alloced_entry.key_start = b->start;
	b->alloced_entry.key_end = b->end;

	nvgpu_rbtree_insert(&b->alloced_entry, &a->alloced_buddies);

	buddy_set_alloced(b);
	a->buddy_list_alloced[b->order] =
		nvgpu_safe_add_u64(a->buddy_list_alloced[b->order], 1ULL);
}

/*
 * Remove the passed buddy from the allocated buddy RB tree. Returns the
 * deallocated buddy for further processing.
 *
 * @a must be locked.
 */

/**
 * @brief Free a buddy based on the provided address and update the allocator's statistics.
 *
 * The function 'balloc_free_buddy' searches for a buddy in the allocated buddies red-black tree
 * using the provided address. If found, it removes the buddy from the tree, clears its allocated
 * status, and decrements the count of allocated buddies for the corresponding order. The buddy
 * is then returned for further processing.
 * The steps performed by the function are as follows:
 * -# Search for a node in the allocated buddies red-black tree that matches the provided address using nvgpu_rbtree_search().
 * -# If no matching node is found, return NULL indicating the buddy to free does not exist.
 * -# Retrieve the buddy from the found node using nvgpu_buddy_from_rbtree_node().
 * -# Unlink the node from the allocated buddies red-black tree using nvgpu_rbtree_unlink().
 * -# Clear the allocated status of the buddy using buddy_clr_alloced() (See
 *    #nvgpu_buddy_allocator_flag_ops).
 * -# Assert that the count of allocated buddies for the buddy's order is greater than zero.
 * -# Decrement the count of allocated buddies for the buddy's order.
 * -# Return the pointer to the buddy that was freed.
 *
 * @param [in]  a    Pointer to the buddy allocator structure.
 * @param [in]  addr Address of the buddy to be freed.
 *
 * @return Pointer to the freed buddy, or NULL if the buddy was not found.
 */
static struct nvgpu_buddy *balloc_free_buddy(struct nvgpu_buddy_allocator *a,
					     u64 addr)
{
	struct nvgpu_rbtree_node *node = NULL;
	struct nvgpu_buddy *bud;

	nvgpu_rbtree_search(addr, &node, a->alloced_buddies);
	if (node == NULL) {
		return NULL;
	}

	bud = nvgpu_buddy_from_rbtree_node(node);

	nvgpu_rbtree_unlink(node, &a->alloced_buddies);
	buddy_clr_alloced(bud);
	nvgpu_assert(a->buddy_list_alloced[bud->order] > 0ULL);
	a->buddy_list_alloced[bud->order]--;

	return bud;
}

/*
 * Find a suitable buddy for the given order and PTE type (big or little).
 */

/**
 * @brief Find a buddy of a given order and PTE size in the buddy allocator.
 *
 * The function 'balloc_find_buddy' searches for a buddy of a specified order and PTE size
 * within the buddy allocator. If the allocator is managing a GVA space and a big PTE size
 * is requested, it returns the last buddy in the list. Otherwise, it returns the first buddy.
 * If the PTE size does not match the requested size and is not set to ANY, the function
 * returns NULL.
 * The steps performed by the function are as follows:
 * -# Check if the requested order is greater than the maximum order supported by the allocator
 *    or if the list of buddies for the requested order is empty. If either is true, return NULL.
 * -# If the allocator is managing a GVA space and a big PTE size is requested, retrieve the
 *    last buddy in the list for the requested order using nvgpu_list_last_entry().
 * -# Otherwise, retrieve the first buddy in the list for the requested order using
 *    nvgpu_list_first_entry().
 * -# If the requested PTE size is not ANY and does not match the PTE size of the found buddy,
 *    and the buddy's PTE size is not ANY, return NULL.
 * -# Return the pointer to the found buddy.
 *
 * @param [in]  a        Pointer to the buddy allocator structure.
 * @param [in]  order    The order of the buddy to find.
 * @param [in]  pte_size The PTE size of the buddy to find.
 *
 * @return Pointer to the found buddy, or NULL if no matching buddy is found.
 */
static struct nvgpu_buddy *balloc_find_buddy(struct nvgpu_buddy_allocator *a,
					     u64 order, u32 pte_size)
{
	struct nvgpu_buddy *bud;

	if ((order > a->max_order) ||
	    nvgpu_list_empty(balloc_get_order_list(a, order))) {
		return NULL;
	}

	if (((a->flags & GPU_ALLOC_GVA_SPACE) != 0ULL) &&
		(pte_size == BALLOC_PTE_SIZE_BIG)) {
		bud = nvgpu_list_last_entry(balloc_get_order_list(a, order),
				      nvgpu_buddy, buddy_entry);
	} else {
		bud = nvgpu_list_first_entry(balloc_get_order_list(a, order),
				       nvgpu_buddy, buddy_entry);
	}

	if ((pte_size != BALLOC_PTE_SIZE_ANY) &&
		(pte_size != bud->pte_size) &&
		(bud->pte_size != BALLOC_PTE_SIZE_ANY)) {
		return NULL;
	}

	return bud;
}

/*
 * Allocate a suitably sized buddy. If no suitable buddy exists split higher
 * order buddies until we have a suitable buddy to allocate.
 *
 * For PDE grouping add an extra check to see if a buddy is suitable: that the
 * buddy exists in a PDE who's PTE size is reasonable
 *
 * @a must be locked.
 */

/**
 * @brief Allocate a block of memory of a given order and PTE size from the buddy allocator.
 *
 * The function 'balloc_do_alloc' attempts to allocate a block of memory by finding a suitable
 * buddy and potentially splitting it until a buddy of the desired order is obtained. It then
 * removes the buddy from the free list and marks it as allocated. If no suitable buddy is found
 * or if splitting fails, it returns 0 to indicate out of memory.
 * The steps performed by the function are as follows:
 * -# Initialize a loop to search for a buddy starting from the requested order up to the maximum
 *    order supported by the allocator.
 * -# Within the loop, call balloc_find_buddy() to find a suitable buddy of the current split order.
 * -# If a buddy is found, break out of the loop.
 * -# If no buddy is found after searching all orders, return 0 to indicate out of memory.
 * -# If the found buddy is of a higher order than requested, enter a loop to split the buddy down
 *    to the requested order.
 * -# Within the loop, call balloc_split_buddy() to split the buddy. If splitting fails, call
 *    balloc_coalesce() to merge the buddy back and return 0 to indicate out of memory.
 * -# After splitting, update the pointer to the buddy to its left child, which is the result of
 *    the split operation.
 * -# Once a buddy of the requested order is obtained, remove it from the free list using
 *    balloc_blist_rem().
 * -# Mark the buddy as allocated using balloc_alloc_buddy().
 * -# Return the start address of the allocated buddy.
 *
 * @param [in]  a        Pointer to the buddy allocator structure.
 * @param [in]  order    The order of the block to allocate.
 * @param [in]  pte_size The PTE size for the allocation.
 *
 * @return The start address of the allocated block, or 0 if out of memory.
 */
static u64 balloc_do_alloc(struct nvgpu_buddy_allocator *a,
			   u64 order, u32 pte_size)
{
	u64 split_order;
	struct nvgpu_buddy *bud = NULL;

	for (split_order = order; split_order <= a->max_order; split_order++) {
		bud = balloc_find_buddy(a, split_order, pte_size);
		if (bud != NULL) {
			break;
		}
	}

	/* Out of memory! */
	if (bud == NULL) {
		return 0;
	}

	while (bud->order != order) {
		if (balloc_split_buddy(a, bud, pte_size) != 0) {
			balloc_coalesce(a, bud);
			return 0; /* No mem... */
		}
		bud = bud->left;
	}

	balloc_blist_rem(a, bud);
	balloc_alloc_buddy(a, bud);

	return bud->start;
}

/*
 * See if the passed range is actually available for allocation. If so, then
 * return 1, otherwise return 0.
 *
 * TODO: Right now this uses the unoptimal approach of going through all
 * outstanding allocations and checking their base/ends. This could be better.
 */

/**
 * @brief Check if a given range of addresses is free in the buddy allocator.
 *
 * The function 'balloc_is_range_free' iterates through the allocated buddies red-black tree
 * to determine if the specified range of addresses is free (i.e., not overlapping with any
 * currently allocated buddies). It returns true if the range is free, otherwise false.
 * The steps performed by the function are as follows:
 * -# Start enumerating the allocated buddies red-black tree from the beginning using nvgpu_rbtree_enum_start().
 * -# If no allocated buddies are found (the tree is empty), return true as the entire range is free.
 * -# Retrieve the buddy from the current node of the tree using nvgpu_buddy_from_rbtree_node().
 * -# Enter a loop to check each buddy in the tree against the specified range.
 * -# If the current buddy's start or end address falls within the specified range, return false as the range is not free.
 * -# Move to the next buddy in the tree using nvgpu_rbtree_enum_next().
 * -# If there are no more buddies in the tree, break out of the loop.
 * -# If the loop completes without finding any overlapping buddies, return true indicating the range is free.
 *
 * @param [in]  a    Pointer to the buddy allocator structure.
 * @param [in]  base The start address of the range to check.
 * @param [in]  end  The end address of the range to check.
 *
 * @return True if the range is free, False if the range is not free.
 */
static bool balloc_is_range_free(struct nvgpu_buddy_allocator *a,
				u64 base, u64 end)
{
	struct nvgpu_rbtree_node *node = NULL;
	struct nvgpu_buddy *bud;

	nvgpu_rbtree_enum_start(0, &node, a->alloced_buddies);
	if (node == NULL) {
		return true; /* No allocs yet. */
	}

	bud = nvgpu_buddy_from_rbtree_node(node);

	while (bud->start < end) {
		if (((bud->start > base) && (bud->start < end)) ||
			((bud->end > base) && (bud->end < end))) {
			return false;
		}

		nvgpu_rbtree_enum_next(&node, node);
		if (node == NULL) {
			break;
		}
		bud = nvgpu_buddy_from_rbtree_node(node);
	}

	return true;
}

/**
 * @brief Insert a fixed allocation into the fixed allocations red-black tree.
 *
 * The function balloc_alloc_fixed() is responsible for inserting a fixed allocation
 * into the fixed allocations red-black tree of the buddy allocator. It sets the start
 * and end keys of the fixed allocation's entry and then inserts it into the tree.
 * The steps performed by the function are as follows:
 * -# Set the start and end keys of the fixed allocation's entry with the fixed allocation's
 *    start and end addresses.
 * -# Insert the fixed allocation's entry into the fixed allocations red-black tree using
 *    nvgpu_rbtree_insert().
 *
 * @param [in]  a  Pointer to the buddy allocator structure.
 * @param [in]  f  Pointer to the fixed allocation to be inserted.
 */
static void balloc_alloc_fixed(struct nvgpu_buddy_allocator *a,
			       struct nvgpu_fixed_alloc *f)
{
	f->alloced_entry.key_start = f->start;
	f->alloced_entry.key_end = f->end;

	nvgpu_rbtree_insert(&f->alloced_entry, &a->fixed_allocs);
}

/*
 * Remove the passed buddy from the allocated buddy RB tree. Returns the
 * deallocated buddy for further processing.
 *
 * @a must be locked.
 */

/**
 * @brief Remove a fixed allocation from the fixed allocations red-black tree.
 *
 * The function 'balloc_free_fixed' searches for a fixed allocation in the fixed
 * allocations red-black tree using the provided address. If found, it removes the
 * allocation from the tree and returns it. If not found, it returns NULL.
 * The steps performed by the function are as follows:
 * -# Search for a node in the fixed allocations red-black tree that matches the provided
 *    address using nvgpu_rbtree_search().
 * -# If no matching node is found, return NULL indicating the fixed allocation to free
 *    does not exist.
 * -# Retrieve the fixed allocation from the found node using nvgpu_fixed_alloc_from_rbtree_node().
 * -# Unlink the node from the fixed allocations red-black tree using nvgpu_rbtree_unlink().
 * -# Return the pointer to the fixed allocation that was removed.
 *
 * @param [in]  a    Pointer to the buddy allocator structure.
 * @param [in]  addr Address of the fixed allocation to be freed.
 *
 * @return Pointer to the freed fixed allocation, or NULL if the allocation was not found.
 */
static struct nvgpu_fixed_alloc *balloc_free_fixed(
	struct nvgpu_buddy_allocator *a, u64 addr)
{
	struct nvgpu_fixed_alloc *falloc;
	struct nvgpu_rbtree_node *node = NULL;

	nvgpu_rbtree_search(addr, &node, a->fixed_allocs);
	if (node == NULL) {
		return NULL;
	}

	falloc = nvgpu_fixed_alloc_from_rbtree_node(node);

	nvgpu_rbtree_unlink(node, &a->fixed_allocs);

	return falloc;
}

/*
 * Find the parent range - doesn't necessarily need the parent to actually exist
 * as a buddy. Finding an existing parent comes later...
 */

/**
 * @brief Calculate the base address and order of the parent buddy.
 *
 * The function 'balloc_get_parent_range' computes the base address and order of the parent
 * buddy for a given buddy specified by its base address and order. It applies a mask to the
 * shifted base address to align it to the parent buddy's boundaries and increments the order
 * to represent the parent buddy's order.
 * The steps performed by the function are as follows:
 * -# Shift the base address of the buddy left by the block size shift amount using balloc_base_shift().
 * -# Assert that the order of the buddy is less than 63.
 * -# Increment the order to represent the parent buddy's order.
 * -# Calculate the base mask for the parent buddy by inverting the bits after subtracting 1 from
 *    the block size shifted left by the incremented order using nvgpu_safe_sub_u64().
 * -# Apply the base mask to the shifted base address to align it to the parent buddy's boundaries.
 * -# Unshift the masked base address to get the actual base address of the parent buddy using
 *    balloc_base_unshift().
 * -# Store the calculated base address and order of the parent buddy in the provided pointers.
 *
 * @param [in]  a       Pointer to the buddy allocator structure.
 * @param [in]  base    The base address of the current buddy.
 * @param [in]  order   The order of the current buddy.
 * @param [out] pbase   Pointer to store the calculated base address of the parent buddy.
 * @param [out] porder  Pointer to store the calculated order of the parent buddy.
 */
static void balloc_get_parent_range(struct nvgpu_buddy_allocator *a,
				    u64 base, u64 order,
				    u64 *pbase, u64 *porder)
{
	u64 base_mask;
	u64 shifted_base = balloc_base_shift(a, base);

	nvgpu_assert(order < 63U);
	order++;
	base_mask = ~nvgpu_safe_sub_u64((a->blk_size << order), 1U);

	shifted_base &= base_mask;

	*pbase = balloc_base_unshift(a, shifted_base);
	*porder = order;
}

/**
 * @brief Obtain the target buddy that matches the specified base address and order.
 *
 * The function 'balloc_get_target_buddy' iteratively splits a given buddy until it matches
 * the specified base address and order. If the buddy cannot be split as required, the function
 * coalesces the buddy and returns NULL.
 * The steps performed by the function are as follows:
 * -# Enter a loop to split the given buddy until it matches the specified base address and order.
 * -# Within the loop, call balloc_split_buddy() to split the buddy. If splitting fails, output
 *    a debug message using alloc_dbg(), coalesce the buddy back to its original state using
 *    balloc_coalesce(), and return NULL.
 * -# Determine which child of the current buddy contains the target base address. If the target
 *    base address is less than the start address of the right child, update the current buddy to
 *    the left child. Otherwise, update it to the right child.
 * -# Continue the loop until the current buddy matches the specified base address and order.
 * -# Return the pointer to the target buddy that matches the specified criteria.
 *
 * @param [in]  a        Pointer to the buddy allocator structure.
 * @param [in]  bud      Pointer to the initial buddy to start splitting from.
 * @param [in]  base     The base address of the target buddy.
 * @param [in]  order    The order of the target buddy.
 * @param [in]  pte_size The PTE size for the allocation, used during splitting.
 *
 * @return Pointer to the target buddy, or NULL if the buddy cannot be split as required.
 */
static struct nvgpu_buddy *balloc_get_target_buddy(
				struct nvgpu_buddy_allocator *a,
				struct nvgpu_buddy *bud,
				u64 base, u64 order, u32 pte_size)
{
	/* Split this buddy as necessary until we get the target buddy. */
	while ((bud->start != base) || (bud->order != order)) {
		if (balloc_split_buddy(a, bud, pte_size) != 0) {
			alloc_dbg(balloc_owner(a),
				  "split buddy failed? {0x%llx, %llu}",
				  bud->start, bud->order);
			balloc_coalesce(a, bud);
			return NULL;
		}

		if (base < bud->right->start) {
			bud = bud->left;
		} else {
			bud = bud->right;
		}
	}

	return bud;
}

/*
 * Makes a buddy at the passed address. This will make all parent buddies
 * necessary for this buddy to exist as well.
 */

/**
 * @brief Create a fixed buddy in the buddy allocator for a given base address and order.
 *
 * The function 'balloc_make_fixed_buddy' searches for a buddy that encompasses the given base
 * address and order. If necessary, it traverses up the buddy hierarchy to find the encompassing
 * buddy and then splits buddies down to create the fixed buddy of the desired order and base address.
 * The steps performed by the function are as follows:
 * -# Initialize a loop to search for the real buddy that the fixed buddy exists in, starting from
 *    the given order and base address.
 * -# Retrieve the list of buddies for the current order using balloc_get_order_list().
 * -# Iterate over each buddy in the list to find one that matches the current base address.
 * -# If a matching buddy is found and its order is less than or equal to the PDE block order, check
 *    if the PTE size matches. If there is a mismatch, output a debug message using alloc_dbg() and
 *    return NULL.
 * -# If a matching buddy is found, break out of the loop.
 * -# If no matching buddy is found, calculate the parent range using balloc_get_parent_range() and
 *    update the current base address and order.
 * -# If the current order exceeds the maximum order of the allocator, output a debug message using
 *    alloc_dbg() and return NULL.
 * -# Once the encompassing buddy is found, call balloc_get_target_buddy() to get the target buddy
 *    at the specified base address and order.
 * -# Return the pointer to the target buddy.
 *
 * @param [in]  a        Pointer to the buddy allocator structure.
 * @param [in]  base     The base address of the fixed buddy to create.
 * @param [in]  order    The order of the fixed buddy to create.
 * @param [in]  pte_size The PTE size for the fixed buddy.
 *
 * @return Pointer to the created fixed buddy, or NULL if it cannot be created.
 */
static struct nvgpu_buddy *balloc_make_fixed_buddy(
	struct nvgpu_buddy_allocator *a, u64 base, u64 order, u32 pte_size)
{
	struct nvgpu_buddy *bud = NULL;
	struct nvgpu_list_node *order_list;
	u64 cur_order = order, cur_base = base;

	/*
	 * Algo:
	 *  1. Keep jumping up a buddy order until we find the real buddy that
	 *     this buddy exists in.
	 *  2. Then work our way down through the buddy tree until we hit a dead
	 *     end.
	 *  3. Start splitting buddies until we split to the one we need to
	 *     make.
	 */
	while (cur_order <= a->max_order) {
		bool found = false;

		order_list = balloc_get_order_list(a, cur_order);
		nvgpu_list_for_each_entry(bud, order_list,
					nvgpu_buddy, buddy_entry) {
			if (bud->start == cur_base) {
				/*
				 * Make sure page size matches if it's smaller
				 * than a PDE sized buddy.
				 */
				if ((bud->order <= a->pte_blk_order) &&
					(bud->pte_size !=
						BALLOC_PTE_SIZE_ANY) &&
					(bud->pte_size != pte_size)) {
					/* Welp, that's the end of that. */
					alloc_dbg(balloc_owner(a),
						  "Fixed buddy PTE "
						  "size mismatch!");
					return NULL;
				}

				found = true;
				break;
			}
		}

		if (found) {
			break;
		}

		balloc_get_parent_range(a, cur_base, cur_order,
					&cur_base, &cur_order);
	}

	if (cur_order > a->max_order) {
		alloc_dbg(balloc_owner(a), "No buddy for range ???");
		return NULL;
	}

	/* Get target buddy */
	bud = balloc_get_target_buddy(a, bud, base, order, pte_size);

	return bud;
}

/**
 * @brief Allocate a fixed range of memory with a specific PTE size in the buddy allocator.
 *
 * The function 'balloc_do_alloc_fixed' allocates a fixed range of memory by creating a list
 * of buddies that satisfy the allocation request. It ensures the PTE size is valid, calculates
 * the alignment order, and then iteratively creates and allocates buddies to cover the entire
 * range. If any part of the allocation fails, it cleans up by freeing any allocated buddies.
 * The steps performed by the function are as follows:
 * -# Check for an invalid PTE size and trigger a bug if encountered using BUG_ON().
 * -# Shift the base address left by the block size shift amount using balloc_base_shift().
 * -# Calculate the alignment order based on the shifted base and length of the allocation.
 * -# If the alignment order is greater than the maximum order supported by the allocator,
 *    output a debug message using alloc_dbg() and return 0.
 * -# Initialize a loop to generate a list of buddies that satisfy the allocation.
 * -# Within the loop, calculate the length of the current order and the remaining length of the allocation.
 * -# Create the fixed buddy for the current base and order using balloc_make_fixed_buddy().
 * -# If buddy creation fails, output a debug message using alloc_dbg() and go to the error cleanup.
 * -# Remove the buddy from the free list using balloc_blist_rem().
 * -# Mark the buddy as allocated using balloc_alloc_buddy().
 * -# Add the buddy to the fixed allocation's list of buddies using balloc_buddy_list_do_add().
 * -# Update the incremented base and alignment order for the next iteration.
 * -# If the remaining length is less than the length of the current order, adjust the alignment order.
 * -# Return the base address of the allocated range if successful.
 * -# In the error cleanup, free any allocated buddies and attempt to defragment the allocation.
 *    Return 0 to indicate failure.
 *
 * @param [in]  a        Pointer to the buddy allocator structure.
 * @param [in]  falloc   Pointer to the fixed allocation structure.
 * @param [in]  base     The base address of the range to allocate.
 * @param [in]  len      The length of the range to allocate.
 * @param [in]  pte_size The PTE size for the allocation.
 *
 * @return The base address of the allocated range, or 0 if the allocation fails.
 */
static u64 balloc_do_alloc_fixed(struct nvgpu_buddy_allocator *a,
				 struct nvgpu_fixed_alloc *falloc,
				 u64 base, u64 len, u32 pte_size)
{
	u64 shifted_base, inc_base;
	u64 align_order;

	/*
	 * Ensure that we have a valid PTE size here (ANY is a valid size). If
	 * this is INVALID then we are going to experience imminent corruption
	 * in the lists that hold buddies. This leads to some very strange
	 * crashes.
	 */
	BUG_ON(pte_size == BALLOC_PTE_SIZE_INVALID);

	shifted_base = balloc_base_shift(a, base);
	if (shifted_base == 0U) {
		align_order = nvgpu_safe_sub_u64(
				nvgpu_ffs(len >> a->blk_shift), 1UL);
	} else {
		u64 shifted_base_order =
			nvgpu_safe_sub_u64(
				nvgpu_ffs(shifted_base >> a->blk_shift), 1UL);
		u64 len_order =
			nvgpu_safe_sub_u64(
				nvgpu_ffs(len >> a->blk_shift), 1UL);
		align_order = min_t(u64, shifted_base_order, len_order);
	}

	if (align_order > a->max_order) {
		alloc_dbg(balloc_owner(a),
			  "Align order too big: %llu > %llu",
			  align_order, a->max_order);
		return 0;
	}

	/*
	 * Generate a list of buddies that satisfy this allocation.
	 */
	inc_base = shifted_base;
	while (inc_base < nvgpu_safe_add_u64(shifted_base, len)) {
		u64 order_len = balloc_order_to_len(a, align_order);
		u64 remaining;
		struct nvgpu_buddy *bud;

		bud = balloc_make_fixed_buddy(a,
					balloc_base_unshift(a, inc_base),
					align_order, pte_size);
		if (bud == NULL) {
			alloc_dbg(balloc_owner(a),
				  "Fixed buddy failed: {0x%llx, %llu}!",
				  balloc_base_unshift(a, inc_base),
				  align_order);
			goto err_and_cleanup;
		}

		balloc_blist_rem(a, bud);
		balloc_alloc_buddy(a, bud);
		balloc_buddy_list_do_add(a, bud, &falloc->buddies);

		/* Book keeping. */
		inc_base = nvgpu_safe_add_u64(inc_base, order_len);
		remaining = (shifted_base + len) - inc_base;
		align_order = nvgpu_safe_sub_u64(
				nvgpu_ffs(inc_base >> a->blk_shift), 1UL);

		/* If we don't have much left - trim down align_order. */
		if (balloc_order_to_len(a, align_order) > remaining) {
			align_order = balloc_max_order_in(a, inc_base,
						nvgpu_safe_add_u64(inc_base,
								remaining));
		}
	}

	return base;

err_and_cleanup:
	while (!nvgpu_list_empty(&falloc->buddies)) {
		struct nvgpu_buddy *bud = nvgpu_list_first_entry(
						&falloc->buddies,
						nvgpu_buddy, buddy_entry);

		balloc_buddy_list_do_rem(a, bud);
		(void) balloc_free_buddy(a, bud->start);
		balloc_blist_add(a, bud);
		/*
		 * Attemp to defrag the allocation.
		 */
		balloc_coalesce(a, bud);
	}

	return 0;
}

/**
 * @brief Free a fixed allocation and attempt to defragment the allocator.
 *
 * The function 'balloc_do_free_fixed' iterates over the list of buddies in a fixed allocation,
 * freeing each one and attempting to coalesce them back into larger free blocks. It updates the
 * allocator's statistics for the number of bytes freed and finally frees the memory used by the
 * fixed allocation structure.
 * The steps performed by the function are as follows:
 * -# Enter a loop that continues as long as the list of buddies in the fixed allocation is not empty.
 * -# Retrieve the first buddy in the fixed allocation's list of buddies using nvgpu_list_first_entry().
 * -# Remove the buddy from the fixed allocation's list of buddies using balloc_buddy_list_do_rem().
 * -# Free the buddy using balloc_free_buddy() and ignore the return value.
 * -# Add the buddy back to the allocator's list of free buddies using balloc_blist_add().
 * -# Update the allocator's bytes_freed statistic by adding the length of the buddy's order using
 *    balloc_order_to_len().
 * -# Attempt to defragment the allocator by coalescing the buddy back into larger blocks using
 *    balloc_coalesce().
 * -# After the loop, free the memory used by the fixed allocation structure using nvgpu_kfree().
 *
 * @param [in]  a       Pointer to the buddy allocator structure.
 * @param [in]  falloc  Pointer to the fixed allocation to be freed.
 */
static void balloc_do_free_fixed(struct nvgpu_buddy_allocator *a,
				 struct nvgpu_fixed_alloc *falloc)
{
	struct nvgpu_buddy *bud;

	while (!nvgpu_list_empty(&falloc->buddies)) {
		bud = nvgpu_list_first_entry(&falloc->buddies,
				       nvgpu_buddy,
				       buddy_entry);
		balloc_buddy_list_do_rem(a, bud);

		(void) balloc_free_buddy(a, bud->start);
		balloc_blist_add(a, bud);
		a->bytes_freed = nvgpu_safe_add_u64(a->bytes_freed,
					balloc_order_to_len(a, bud->order));

		/*
		 * Attemp to defrag the allocation.
		 */
		balloc_coalesce(a, bud);
	}

	nvgpu_kfree(nvgpu_alloc_to_gpu(a->owner), falloc);
}

/*
 * Allocate memory from the passed allocator.
 * Acquire the allocator lock.
 * Compute the order by calling balloc_get_order().
 * Compute the pte size supported for this allocation by calling
 *  nvgpu_balloc_page_size_to_pte_size().
 * If we could not satisfy the required size buddy then call
 *  balloc_split_buddy() to get the requiredsize by dividing the large size buddy.
 * Free the remaining buddy to the respective list.
 * Release the alloc_lock.
 */

/**
 * @brief Allocate a page table entry (PTE) of a specified size from the buddy allocator.
 *
 * The function 'nvgpu_buddy_balloc_pte' allocates memory for a PTE with the given length and
 * page size. It calculates the order of the allocation, converts the page size to a PTE size,
 * and then attempts to allocate the memory. If successful, it updates the allocator's statistics.
 * The steps performed by the function are as follows:
 * -# Check if the requested length is zero and output a debug message using alloc_dbg() if it is,
 *    then return 0.
 * -# Acquire the allocator lock using alloc_lock().
 * -# Calculate the order of the allocation based on the length using balloc_get_order().
 * -# If the calculated order is greater than the maximum order supported by the allocator, release
 *    the lock using alloc_unlock(), output a debug message using alloc_dbg(), and return 0.
 * -# Convert the page size to a PTE size using nvgpu_balloc_page_size_to_pte_size().
 * -# If the PTE size is invalid, release the lock using alloc_unlock() and return 0.
 * -# Attempt to allocate memory of the calculated order and PTE size using balloc_do_alloc().
 * -# If the allocation is successful, update the allocator's statistics for allocated bytes and
 *    output a debug message using alloc_dbg().
 * -# If the allocation fails, output a debug message using alloc_dbg().
 * -# Release the allocator lock using alloc_unlock().
 * -# Return the address of the allocated memory, or 0 if the allocation failed.
 *
 * @param [in]  na         Pointer to the generic allocator structure.
 * @param [in]  len        The length of the PTE to allocate.
 * @param [in]  page_size  The page size for the PTE allocation.
 *
 * @return The address of the allocated PTE, or 0 if the allocation fails.
 */
static u64 nvgpu_buddy_balloc_pte(struct nvgpu_allocator *na, u64 len,
				  u32 page_size, bool align_required, u64 align_size)
{
	u64 order, addr;
	u32 pte_size;
	struct nvgpu_buddy_allocator *a = buddy_allocator(na);
	u64 temp;

	if (len == 0ULL) {
		alloc_dbg(balloc_owner(a), "Alloc fail");
		return 0;
	}

	/**
	 * The alignment requirement can be satisfied by having
	 * the length be up-aligned to the VA alignment because
	 * the buddy allocator guarantees that the allocation base
	 * address is always aligned at least to the length size.
	 */
	if (align_required) {
		if (align_size == 0ULL) {
			alloc_dbg(balloc_owner(a), "Alloc fail: Invalid align size");
			return 0;
		}

		if (!nvgpu_safe_add_u64_return(len, align_size - 1UL, &temp)) {
			alloc_dbg(balloc_owner(a), "Alloc fail: Overflow");
			return 0;
		}

		len = NVGPU_ALIGN(len, align_size);
	}

	alloc_lock(na);

	order = balloc_get_order(a, len);

	if (order > a->max_order) {
		alloc_unlock(na);
		alloc_dbg(balloc_owner(a), "Alloc fail");
		return 0;
	}

	pte_size = nvgpu_balloc_page_size_to_pte_size(a, page_size);
	if (pte_size == BALLOC_PTE_SIZE_INVALID) {
		alloc_unlock(na);
		return 0;
	}

	addr = balloc_do_alloc(a, order, pte_size);

	if (addr != 0ULL) {
		a->bytes_alloced += len;
		a->bytes_alloced_real += balloc_order_to_len(a, order);
		alloc_dbg(balloc_owner(a),
			"Alloc 0x%-10llx %3lld:0x%-10llx pte_size=%s",
			addr, order, len,
			(pte_size == BALLOC_PTE_SIZE_HUGE) ? "huge" :
			(pte_size == BALLOC_PTE_SIZE_BIG) ? "big" :
			(pte_size == BALLOC_PTE_SIZE_SMALL) ? "small" :
			"NA/any");
		a->alloc_made = true;
	} else {
		alloc_dbg(balloc_owner(a), "Alloc failed: no mem!");
	}

	alloc_unlock(na);

	return addr;
}

/**
 * @brief Allocate memory from the buddy allocator without specifying a PTE size.
 *
 * The function 'nvgpu_buddy_balloc' is a wrapper that calls 'nvgpu_buddy_balloc_pte' with
 * a PTE size of 'BALLOC_PTE_SIZE_ANY', indicating that any PTE size can be used for the allocation.
 * The steps performed by the function are as follows:
 * -# Call nvgpu_buddy_balloc_pte() to allocate memory with the specified length and a PTE size
 *    of 'BALLOC_PTE_SIZE_ANY'.
 * -# Return the address of the allocated memory as provided by nvgpu_buddy_balloc_pte().
 *
 * @param [in]  na   Pointer to the generic allocator structure.
 * @param [in]  len  The length of the memory to allocate.
 *
 * @return The address of the allocated memory, or 0 if the allocation fails.
 */
static u64 nvgpu_buddy_balloc(struct nvgpu_allocator *na, u64 len)
{
	return nvgpu_buddy_balloc_pte(na, len, BALLOC_PTE_SIZE_ANY,
			false, len);
}

/*
 * Check the input parameter validity.
 * Acquire the alloc_lock.
 * Compute the order with respective to the input size.
 * Compute the pte_size for the given page size and return error for
 *  invalid pte size.
 * Call balloc_is_range_free() to get the free range with the address given.
 * Call balloc_make_fixed_buddy() to generate the list of buddies.
 * Make the book keeping of allocated objects to the respective lists.
 * Release the alloc_lock.
 */

/**
 * @brief Allocate a fixed range of memory with a specific page size in the buddy allocator.
 *
 * The function 'nvgpu_balloc_fixed_buddy_locked' allocates a fixed range of memory with the
 * given base address, length, and page size. It ensures the base is aligned, the range is free,
 * and the page size is valid before proceeding with the allocation. If any checks fail or the
 * allocation is unsuccessful, it returns 0.
 * The steps performed by the function are as follows:
 * -# Assert that the block size of the allocator is greater than zero.
 * -# Check if the base address is aligned to an order 0 block size. If not, go to fail.
 * -# Check if the length is zero or if the base address is less than the start of the allocator.
 *    If either is true, go to fail.
 * -# Check if the end of the allocator is less than the sum of the base address and length.
 *    If true, go to fail.
 * -# Convert the page size to a PTE size using nvgpu_balloc_page_size_to_pte_size().
 * -# If the PTE size is invalid, go to fail.
 * -# Allocate memory for the fixed allocation structure using nvgpu_kmalloc().
 * -# If the allocation for the fixed allocation structure fails, go to fail.
 * -# Initialize the list of buddies in the fixed allocation and set its start and end addresses.
 * -# Check if the range is free using balloc_is_range_free(). If not, output a debug message
 *    using alloc_dbg() and go to fail.
 * -# Attempt to allocate the fixed range using balloc_do_alloc_fixed(). If the allocation fails,
 *    output a debug message using alloc_dbg() and go to fail.
 * -# Insert the fixed allocation into the fixed allocations tree using balloc_alloc_fixed().
 * -# Iterate over each buddy in the fixed allocation's list of buddies to calculate the real
 *    bytes allocated.
 * -# Update the allocator's statistics for allocated bytes.
 * -# Output a debug message using alloc_dbg() indicating the fixed allocation was successful.
 * -# Return the base address of the allocated range.
 * -# If any checks fail or the allocation is unsuccessful, free the fixed allocation structure
 *    using nvgpu_kfree() and return 0.
 *
 * @param [in]  na         Pointer to the generic allocator structure.
 * @param [in]  base       The base address of the range to allocate.
 * @param [in]  len        The length of the range to allocate.
 * @param [in]  page_size  The page size for the allocation.
 *
 * @return The base address of the allocated range, or 0 if the allocation fails.
 */
static u64 nvgpu_balloc_fixed_buddy_locked(struct nvgpu_allocator *na,
					   u64 base, u64 len, u32 page_size)
{
	u32 pte_size;
	u64 ret, real_bytes = 0;
	struct nvgpu_buddy *bud;
	struct nvgpu_fixed_alloc *falloc = NULL;
	struct nvgpu_buddy_allocator *a = buddy_allocator(na);

	/* If base isn't aligned to an order 0 block, fail. */
	nvgpu_assert(a->blk_size > 0ULL);
	if ((base & (a->blk_size - 1ULL)) != 0ULL) {
		goto fail;
	}

	if (len == 0ULL) {
		goto fail;
	}

	if (base < a->start) {
		goto fail;
	}

	if (a->end < nvgpu_safe_add_u64(base, len)) {
		goto fail;
	}

	pte_size = nvgpu_balloc_page_size_to_pte_size(a, page_size);
	if (pte_size == BALLOC_PTE_SIZE_INVALID) {
		goto fail;
	}

	falloc = nvgpu_kmalloc(nvgpu_alloc_to_gpu(na), sizeof(*falloc));
	if (falloc == NULL) {
		goto fail;
	}

	nvgpu_init_list_node(&falloc->buddies);
	falloc->start = base;
	falloc->end = base + len;

	if (!balloc_is_range_free(a, base, base + len)) {
		alloc_dbg(balloc_owner(a),
			  "Range not free: 0x%llx -> 0x%llx",
			  base, base + len);
		goto fail;
	}

	ret = balloc_do_alloc_fixed(a, falloc, base, len, pte_size);
	if (ret == 0ULL) {
		alloc_dbg(balloc_owner(a),
			  "Alloc-fixed failed ?? 0x%llx -> 0x%llx",
			  base, base + len);
		goto fail;
	}

	balloc_alloc_fixed(a, falloc);

	nvgpu_list_for_each_entry(bud, &falloc->buddies,
				nvgpu_buddy, buddy_entry) {
		real_bytes = nvgpu_safe_add_u64(real_bytes,
				nvgpu_safe_sub_u64(bud->end, bud->start));
	}

	a->bytes_alloced = nvgpu_safe_add_u64(a->bytes_alloced, len);
	a->bytes_alloced_real = nvgpu_safe_add_u64(a->bytes_alloced_real,
						   real_bytes);

	alloc_dbg(balloc_owner(a), "Alloc (fixed) 0x%llx", base);

	return base;

fail:
	nvgpu_kfree(nvgpu_alloc_to_gpu(na), falloc);
	return 0;
}

/*
 * Allocate a fixed address allocation. The address of the allocation is @base
 * and the length is @len. This is not a typical buddy allocator operation and
 * as such has a high posibility of failure if the address space is heavily in
 * use.
 *
 * Please do not use this function unless _absolutely_ necessary.
 */

/**
 * @brief Allocate a fixed range of memory with a specific page size in the buddy allocator.
 *
 * The function 'nvgpu_balloc_fixed_buddy' is a wrapper that locks the allocator, calls
 * 'nvgpu_balloc_fixed_buddy_locked' to perform the actual allocation, and then unlocks the
 * allocator. It updates the allocator's 'alloc_made' flag if the allocation is successful.
 * The steps performed by the function are as follows:
 * -# Acquire the allocator lock using alloc_lock().
 * -# Call nvgpu_balloc_fixed_buddy_locked() to attempt the fixed allocation with the specified
 *    base address, length, and page size.
 * -# If the allocation is successful (non-zero address returned), set the allocator's 'alloc_made'
 *    flag to true.
 * -# Release the allocator lock using alloc_unlock().
 * -# Return the address of the allocated memory, or 0 if the allocation failed.
 *
 * @param [in]  na         Pointer to the generic allocator structure.
 * @param [in]  base       The base address of the range to allocate.
 * @param [in]  len        The length of the range to allocate.
 * @param [in]  page_size  The page size for the allocation.
 *
 * @return The base address of the allocated range, or 0 if the allocation fails.
 */
static u64 nvgpu_balloc_fixed_buddy(struct nvgpu_allocator *na,
				    u64 base, u64 len, u32 page_size)
{
	u64 alloc;
	struct nvgpu_buddy_allocator *a = buddy_allocator(na);

	alloc_lock(na);
	alloc = nvgpu_balloc_fixed_buddy_locked(na, base, len, page_size);

	if (alloc != 0ULL) {
		a->alloc_made = true;
	}

	alloc_unlock(na);

	return alloc;
}

/*
 * @a must be locked.
 */

/**
 * @brief Free a previously allocated block of memory in the buddy allocator.
 *
 * The function 'nvgpu_buddy_bfree_locked' frees a block of memory identified by its address.
 * It first checks if the address corresponds to a fixed allocation and frees it accordingly.
 * If not, it treats the address as a regular buddy allocation. It updates the allocator's
 * statistics and attempts to defragment the allocator by coalescing buddies.
 * The steps performed by the function are as follows:
 * -# Retrieve the buddy allocator structure from the generic allocator using buddy_allocator().
 * -# Attempt to free a fixed allocation using balloc_free_fixed(). If successful, proceed to
 *    free the fixed allocation using balloc_do_free_fixed() and go to 'done'.
 * -# If the address is not a fixed allocation, attempt to free a regular buddy using
 *    balloc_free_buddy(). If no buddy is found, go to 'done'.
 * -# Add the freed buddy back to the allocator's list of free buddies using balloc_blist_add().
 * -# Update the allocator's bytes_freed statistic by adding the length of the freed buddy's order
 *    using balloc_order_to_len().
 * -# Attempt to defragment the allocator by coalescing the buddy back into larger blocks using
 *    balloc_coalesce().
 * -# Output a debug message using alloc_dbg() indicating the address has been freed.
 * -# Label 'done' marks the end of the function's execution path.
 *
 * @param [in]  na   Pointer to the generic allocator structure.
 * @param [in]  addr The address of the memory block to free.
 */
static void nvgpu_buddy_bfree_locked(struct nvgpu_allocator *na, u64 addr)
{
	struct nvgpu_buddy *bud;
	struct nvgpu_fixed_alloc *falloc;
	struct nvgpu_buddy_allocator *a = buddy_allocator(na);

	/*
	 * First see if this is a fixed alloc. If not fall back to a regular
	 * buddy.
	 */
	falloc = balloc_free_fixed(a, addr);
	if (falloc != NULL) {
		balloc_do_free_fixed(a, falloc);
		goto done;
	}

	bud = balloc_free_buddy(a, addr);
	if (bud == NULL) {
		goto done;
	}

	balloc_blist_add(a, bud);
	a->bytes_freed = nvgpu_safe_add_u64(a->bytes_freed,
				balloc_order_to_len(a, bud->order));

	/*
	 * Attemp to defrag the allocation.
	 */
	balloc_coalesce(a, bud);

done:
	alloc_dbg(balloc_owner(a), "Free 0x%llx", addr);
}

/*
 * Free the passed allocation.
 */

/**
 * @brief Free a previously allocated block of memory in the buddy allocator.
 *
 * The function 'nvgpu_buddy_bfree' is a wrapper that locks the allocator, calls
 * 'nvgpu_buddy_bfree_locked' to perform the actual free operation, and then unlocks the
 * allocator. It checks if the address is non-zero before attempting to free the memory.
 * The steps performed by the function are as follows:
 * -# Check if the address is zero. If it is, return immediately as there is nothing to free.
 * -# Acquire the allocator lock using alloc_lock().
 * -# Call nvgpu_buddy_bfree_locked() to free the memory block at the specified address.
 * -# Release the allocator lock using alloc_unlock().
 *
 * @param [in]  na   Pointer to the generic allocator structure.
 * @param [in]  addr The address of the memory block to free.
 */
static void nvgpu_buddy_bfree(struct nvgpu_allocator *na, u64 addr)
{
	if (addr == 0ULL) {
		return;
	}

	alloc_lock(na);
	nvgpu_buddy_bfree_locked(na, addr);
	alloc_unlock(na);
}

static bool nvgpu_buddy_reserve_is_possible(struct nvgpu_buddy_allocator *a,
					    struct nvgpu_alloc_carveout *co)
{
	struct nvgpu_alloc_carveout *tmp;
	u64 co_base, co_end;

	co_base = co->base;
	co_end  = nvgpu_safe_add_u64(co->base, co->length);

	/*
	 * Not the fastest approach but we should not have that many carveouts
	 * for any reasonable allocator.
	 */
	nvgpu_list_for_each_entry(tmp, &a->co_list,
				nvgpu_alloc_carveout, co_entry) {
		u64 tmp_end = nvgpu_safe_add_u64(tmp->base, tmp->length);

		if (((co_base >= tmp->base) && (co_base < tmp_end)) ||
			((co_end >= tmp->base) && (co_end < tmp_end))) {
			return false;
		}
	}

	return true;
}

/*
 * Carveouts can only be reserved before any regular allocations have been
 * made.
 * - Check the validity of input paramemters.
 * - Acquire the allocator lock.
 * - Call nvgpu_balloc_fixed_buddy_locked() to reserve the object
 *   with \a co.base and \a co.length.
 * - Add the allocated object to the book keeping list.
 * - Release the allocator lock.
 */
static int nvgpu_buddy_reserve_co(struct nvgpu_allocator *na,
				  struct nvgpu_alloc_carveout *co)
{
	struct nvgpu_buddy_allocator *a = buddy_allocator(na);
	u64 addr;
	int err = 0;

	if (co->base < a->start) {
		return -EINVAL;
	}
	if (nvgpu_safe_add_u64(co->base, co->length) > a->end) {
		return -EINVAL;
	}
	if (a->alloc_made) {
		return -EINVAL;
	}

	alloc_lock(na);

	if (!nvgpu_buddy_reserve_is_possible(a, co)) {
		err = -EBUSY;
		goto done;
	}

	/* Should not be possible to fail... */
	addr = nvgpu_balloc_fixed_buddy_locked(na, co->base, co->length,
					       BALLOC_PTE_SIZE_ANY);
	if (addr == 0ULL) {
		err = -ENOMEM;
		nvgpu_warn(na->g,
				"%s: Failed to reserve a valid carveout!",
				__func__);
		goto done;
	}

	nvgpu_list_add(&co->co_entry, &a->co_list);

done:
	alloc_unlock(na);
	return err;
}

/*
 * Carveouts can be release at any time.
 * - Acquire the allocator lock.
 * - Remove the carve out from the allocator list.
 * - Call nvgpu_buddy_bfree_locked() to free the carve out
 *   - nvgpu_buddy_bfree_locked() will first check the address is fixed
 *     or not by calling balloc_free_fixed().
 *   - If the address is fixed then free it by calling balloc_do_free_fixed().
 *   - Else free it through balloc_free_buddy().
 *   - Recompute the size of the allocator and coalesce the objects.
 * - Release the lock.
 */
static void nvgpu_buddy_release_co(struct nvgpu_allocator *na,
				   struct nvgpu_alloc_carveout *co)
{
	alloc_lock(na);

	nvgpu_list_del(&co->co_entry);
	nvgpu_buddy_bfree_locked(na, co->base);

	alloc_unlock(na);
}

/**
 * @brief Retrieve the length of the memory managed by the buddy allocator.
 *
 * This function returns the total size of the memory region that the buddy
 * allocator is managing. It retrieves the length from the buddy allocator's
 * internal structure.
 *
 * The steps performed by the function are as follows:
 * -# Convert the generic allocator pointer to a buddy allocator specific
 *    pointer using the buddy_allocator() function.
 * -# Return the length field from the buddy allocator's internal structure.
 *
 * @param [in] a  The generic pointer to the allocator structure.
 *
 * @return The length of the memory region managed by the buddy allocator.
 */
static u64 nvgpu_buddy_alloc_length(struct nvgpu_allocator *a)
{
	struct nvgpu_buddy_allocator *ba = buddy_allocator(a);

	return ba->length;
}

/**
 * @brief Retrieves the base address of the memory managed by the buddy allocator.
 *
 * This function returns the starting address of the memory range managed by the
 * buddy allocator associated with the given allocator context.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the buddy allocator context from the generic allocator context
 *    using the buddy_allocator() function.
 * -# Return the 'start' field from the buddy allocator context, which represents
 *    the base address of the managed memory.
 *
 * @param [in] a  The generic allocator context.
 *
 * @return The base address of the memory range managed by the buddy allocator.
 */
static u64 nvgpu_buddy_alloc_base(struct nvgpu_allocator *a)
{
	struct nvgpu_buddy_allocator *ba = buddy_allocator(a);

	return ba->start;
}

/**
 * @brief Checks if the buddy allocator is initialized.
 *
 * This function determines whether the buddy allocator associated with the given
 * allocator context has been initialized. It ensures any memory operations
 * performed before the check are completed before accessing the 'initialized'
 * status.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the buddy allocator context from the generic allocator context
 *    using the buddy_allocator() function.
 * -# Store the 'initialized' field from the buddy allocator context, which
 *    indicates whether the allocator has been initialized.
 * -# Call nvgpu_smp_rmb() to ensure that all memory operations that precede the
 *    read memory barrier in program order are globally visible before the
 *    'initialized' status is read.
 * -# Return the stored 'initialized' status.
 *
 * @param [in] a  The generic allocator context.
 *
 * @return True if the buddy allocator is initialized, false otherwise.
 */
static bool nvgpu_buddy_alloc_inited(struct nvgpu_allocator *a)
{
	struct nvgpu_buddy_allocator *ba = buddy_allocator(a);
	bool inited = ba->initialized;

	nvgpu_smp_rmb();
	return inited;
}

/**
 * @brief Retrieves the end address of the memory managed by the buddy allocator.
 *
 * This function returns the ending address of the memory range managed by the
 * buddy allocator associated with the given allocator context.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the buddy allocator context from the generic allocator context
 *    using the buddy_allocator() function.
 * -# Return the 'end' field from the buddy allocator context, which represents
 *    the end address of the managed memory.
 *
 * @param [in] a  The generic allocator context.
 *
 * @return The end address of the memory range managed by the buddy allocator.
 */
static u64 nvgpu_buddy_alloc_end(struct nvgpu_allocator *a)
{
	struct nvgpu_buddy_allocator *ba = buddy_allocator(a);

	return ba->end;
}
/*
 * - Acquire the allocator lock.
 * - Check the availability of space between start and end of
 *   the allocator.
 * - Release the allocator lock.
 */

/**
 * @brief Calculates the available space in the buddy allocator.
 *
 * This function computes the amount of free space available in the buddy allocator
 * by subtracting the allocated bytes from the total size of the managed memory
 * range. It ensures thread safety by locking the allocator during the calculation.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the buddy allocator context from the generic allocator context
 *    using the buddy_allocator() function.
 * -# Lock the allocator to prevent concurrent access using alloc_lock().
 * -# Calculate the total size of the managed memory range by subtracting the
 *    'start' address from the 'end' address using nvgpu_safe_sub_u64().
 * -# Calculate the total allocated bytes by subtracting the 'bytes_freed' from
 *    the 'bytes_alloced_real' using nvgpu_safe_sub_u64().
 * -# Subtract the total allocated bytes from the total size of the managed memory
 *    range to get the available space using nvgpu_safe_sub_u64().
 * -# Unlock the allocator to allow access by other threads using alloc_unlock().
 * -# Return the calculated available space.
 *
 * @param [in] a  The generic allocator context.
 *
 * @return The amount of free space available in the buddy allocator.
 */
static u64 nvgpu_buddy_alloc_space(struct nvgpu_allocator *a)
{
	struct nvgpu_buddy_allocator *ba = buddy_allocator(a);
	u64 space;

	alloc_lock(a);
	space = nvgpu_safe_sub_u64(ba->end, ba->start);
	space = nvgpu_safe_sub_u64(space,
			nvgpu_safe_sub_u64(ba->bytes_alloced_real,
					   ba->bytes_freed));
	alloc_unlock(a);

	return space;
}

#ifdef __KERNEL__
/*
 * Print the buddy allocator top level stats. If you pass @s as NULL then the
 * stats are printed to the kernel log. This lets this code be used for
 * debugging purposes internal to the allocator.
 */
static void nvgpu_buddy_print_stats(struct nvgpu_allocator *na,
				    struct seq_file *s, int lock)
{
	int i = 0;
	struct nvgpu_rbtree_node *node = NULL;
	struct nvgpu_fixed_alloc *falloc;
	struct nvgpu_alloc_carveout *tmp;
	struct nvgpu_buddy_allocator *a = buddy_allocator(na);

	alloc_pstat(s, na, "base = %llu, limit = %llu, blk_size = %llu",
		      a->base, a->length, a->blk_size);
	alloc_pstat(s, na, "Internal params:");
	alloc_pstat(s, na, "  start = 0x%llx", a->start);
	alloc_pstat(s, na, "  end   = 0x%llx", a->end);
	alloc_pstat(s, na, "  count = 0x%llx", a->count);
	alloc_pstat(s, na, "  blks  = 0x%llx", a->blks);
	alloc_pstat(s, na, "  max_order = %llu", a->max_order);

	if (lock)
		alloc_lock(na);

	if (!nvgpu_list_empty(&a->co_list)) {
		alloc_pstat(s, na, "");
		alloc_pstat(s, na, "Carveouts:");
		nvgpu_list_for_each_entry(tmp, &a->co_list,
					nvgpu_alloc_carveout, co_entry)
			alloc_pstat(s, na,
				      "  CO %2d: %-20s 0x%010llx + 0x%llx",
				      i++, tmp->name, tmp->base, tmp->length);
	}

	alloc_pstat(s, na, "");
	alloc_pstat(s, na, "Buddy blocks:");
	alloc_pstat(s, na, "  Order   Free    Alloced   Split");
	alloc_pstat(s, na, "  -----   ----    -------   -----");

	for (i = a->max_order; i >= 0; i--) {
		if (a->buddy_list_len[i] == 0 &&
		    a->buddy_list_alloced[i] == 0 &&
		    a->buddy_list_split[i] == 0)
			continue;

		alloc_pstat(s, na, "  %3d     %-7llu %-9llu %llu", i,
			      a->buddy_list_len[i],
			      a->buddy_list_alloced[i],
			      a->buddy_list_split[i]);
	}

	alloc_pstat(s, na, "");

	nvgpu_rbtree_enum_start(0, &node, a->fixed_allocs);
	i = 1;
	while (node) {
		falloc = nvgpu_fixed_alloc_from_rbtree_node(node);

		alloc_pstat(s, na, "Fixed alloc (%d): [0x%llx -> 0x%llx]",
			      i, falloc->start, falloc->end);

		nvgpu_rbtree_enum_next(&node, a->fixed_allocs);
	}

	alloc_pstat(s, na, "");
	alloc_pstat(s, na, "Bytes allocated:        %llu",
		      a->bytes_alloced);
	alloc_pstat(s, na, "Bytes allocated (real): %llu",
		      a->bytes_alloced_real);
	alloc_pstat(s, na, "Bytes freed:            %llu",
		      a->bytes_freed);

	if (lock)
		alloc_unlock(na);
}
#endif

static const struct nvgpu_allocator_ops buddy_ops = {
	.alloc		= nvgpu_buddy_balloc,
	.alloc_pte	= nvgpu_buddy_balloc_pte,
	.free_alloc	= nvgpu_buddy_bfree,

	.alloc_fixed	= nvgpu_balloc_fixed_buddy,
	/* .free_fixed not needed. */

	.reserve_carveout	= nvgpu_buddy_reserve_co,
	.release_carveout	= nvgpu_buddy_release_co,

	.base		= nvgpu_buddy_alloc_base,
	.length		= nvgpu_buddy_alloc_length,
	.end		= nvgpu_buddy_alloc_end,
	.inited		= nvgpu_buddy_alloc_inited,
	.space		= nvgpu_buddy_alloc_space,

	.fini		= nvgpu_buddy_allocator_destroy,

#ifdef __KERNEL__
	.print_stats	= nvgpu_buddy_print_stats,
#endif
};

/**
 * @brief Validates the input arguments for buddy allocator initialization.
 *
 * This function checks the input arguments for initializing a buddy allocator
 * to ensure they meet the necessary conditions such as non-zero sizes, power of
 * two block sizes, and valid maximum order. It also checks for the presence of
 * a valid VM context when managing a GVA space.
 *
 * The steps performed by the function are as follows:
 * -# Determine if the allocator is for GVA space by checking the 'flags' for
 *    #GPU_ALLOC_GVA_SPACE.
 * -# Check if 'blk_size' is zero, which is invalid, and return -EINVAL if true.
 * -# Check if 'blk_size' is a power of two by performing a bitwise AND between
 *    'blk_size' and 'blk_size' minus one and checking if the result is zero.
 *    Return -EINVAL if 'blk_size' is not a power of two.
 * -# Check if 'max_order' exceeds the maximum allowed order defined by
 *    #GPU_BALLOC_MAX_ORDER and return -EINVAL if true.
 * -# Check if 'size' is zero, which is invalid, and return -EINVAL if true.
 * -# If managing a GVA space, check if 'vm' is NULL and return -EINVAL if true.
 * -# Return 0 to indicate the input arguments are valid.
 *
 * @param [in] vm        The VM context, required if managing GVA space.
 * @param [in] size      The size of the memory to manage.
 * @param [in] blk_size  The size of the blocks managed by the allocator.
 * @param [in] max_order The maximum order of blocks managed by the allocator.
 * @param [in] flags     Flags indicating the allocator's properties.
 *
 * @return 0 if the arguments are valid, -EINVAL if any argument is invalid.
 */
static int nvgpu_buddy_check_argument_limits(struct vm_gk20a *vm, u64 size,
				u64 blk_size, u64 max_order, u64 flags)
{
	bool is_blk_size_pwr_2;
	bool is_gva_space = (flags & GPU_ALLOC_GVA_SPACE) != 0ULL;

	/* blk_size must be greater than 0 and a power of 2. */
	if (blk_size == 0U) {
		return -EINVAL;
	}

	is_blk_size_pwr_2 = (blk_size & (blk_size - 1ULL)) == 0ULL;
	if (!is_blk_size_pwr_2) {
		return -EINVAL;
	}

	if (max_order > GPU_BALLOC_MAX_ORDER) {
		return -EINVAL;
	}

	if (size == 0U) {
		return -EINVAL;
	}

	/* If this is to manage a GVA space we need a VM. */
	if (is_gva_space && (vm == NULL)) {
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief Sets the attributes for a buddy allocator.
 *
 * This function initializes the buddy allocator with the provided attributes such
 * as base address, size, block size, and maximum order. It also aligns the base
 * and size if required for GVA spaces with big pages enabled.
 *
 * The steps performed by the function are as follows:
 * -# Determine if the allocator is for GVA space by checking the 'flags' for
 *    #GPU_ALLOC_GVA_SPACE.
 * -# Set the base address of the buddy allocator.
 * -# Set the total length of memory to manage in the buddy allocator.
 * -# Set the block size for the buddy allocator.
 * -# Calculate the block shift value by subtracting one from the result of
 *    nvgpu_ffs() called with 'blk_size'.
 * -# Set the owner of the buddy allocator to the provided generic allocator.
 * -# If the base address is zero, adjust the base to be the size of one block
 *    and reduce the length accordingly to allow for error reporting with an
 *    address of zero.
 * -# Set the VM context for the buddy allocator.
 * -# If managing a GVA space, calculate the PDE coverage size, block order for
 *    PTEs, and ensure that the base and size are aligned to PDE boundaries if
 *    big pages are enabled. Return -EINVAL if alignment is not correct.
 * -# Set the flags for the buddy allocator.
 * -# Set the maximum order for the buddy allocator.
 * -# Return 0 to indicate successful setting of attributes.
 *
 * @param [in] a         The buddy allocator context.
 * @param [in] na        The generic allocator context.
 * @param [in] vm        The VM context, required if managing GVA space.
 * @param [in] base      The base address for the buddy allocator.
 * @param [in] size      The size of the memory to manage.
 * @param [in] blk_size  The size of the blocks managed by the allocator.
 * @param [in] max_order The maximum order of blocks managed by the allocator.
 * @param [in] flags     Flags indicating the allocator's properties.
 *
 * @return 0 if the attributes are set successfully, -EINVAL if any attribute is invalid.
 */
static int nvgpu_buddy_set_attributes(struct nvgpu_buddy_allocator *a,
				struct nvgpu_allocator *na, struct vm_gk20a *vm,
				u64 base, u64 size, u64 blk_size, u64 max_order,
				u64 flags)
{
	bool is_gva_space = (flags & GPU_ALLOC_GVA_SPACE) != 0ULL;

	a->base = base;
	a->length = size;
	a->blk_size = blk_size;
	a->blk_shift = nvgpu_safe_sub_u64(nvgpu_ffs(blk_size), 1UL);
	a->owner = na;

	/*
	 * If base is 0 then modfy base to be the size of one block so that we
	 * can return errors by returning addr == 0.
	 */
	if (a->base == 0U) {
		a->base = a->blk_size;
		a->length = nvgpu_safe_sub_u64(a->length, a->blk_size);
	}

	a->vm = vm;
	if (is_gva_space) {
		u64 pde_size_mask, base_pde_align, size_pde_align;
		u64 pde_size = BIT64(nvgpu_vm_pde_coverage_bit_count(
				gk20a_from_vm(vm), vm->big_page_size));
		a->pte_blk_order = balloc_get_order(a, pde_size);

		/*
		 * When we have a GVA space with big_pages enabled the size and
		 * base must be PDE aligned. If big_pages are not enabled then
		 * this requirement is not necessary.
		 */

		pde_size_mask = nvgpu_safe_sub_u64(pde_size, U64(1));
		base_pde_align = a->base & pde_size_mask;
		size_pde_align = a->length & pde_size_mask;
		if (vm->big_pages &&
			((base_pde_align != 0ULL) ||
				(size_pde_align != 0ULL))) {
			return -EINVAL;
		}
	}

	a->flags = flags;
	a->max_order = max_order;
	return 0;
}

/*
 * Initialize a buddy allocator. Returns 0 on success. This allocator does
 * not necessarily manage bytes. It manages distinct ranges of resources. This
 * allows the allocator to work for things like comp_tags, semaphores, etc.
 *
 * @allocator: Ptr to an allocator struct to init.
 * @vm: GPU VM to associate this allocator with. Can be NULL. Will be used to
 *      get PTE size for GVA spaces.
 * @name: Name of the allocator. Doesn't have to be static storage.
 * @base: The base address of the resource pool being managed.
 * @size: Number of resources in the pool.
 * @blk_size: Minimum number of resources to allocate at once. For things like
 *            semaphores this is 1. For GVA this might be as much as 64k. This
 *            corresponds to order 0. Must be power of 2.
 * @max_order: Pick a maximum order. If you leave this as 0, the buddy allocator
 *             will try and pick a reasonable max order.
 * @flags: Extra flags necessary. See GPU_BALLOC_*.
 */
int nvgpu_buddy_allocator_init(struct gk20a *g, struct nvgpu_allocator *na,
			       struct vm_gk20a *vm, const char *name,
			       u64 base, u64 size, u64 blk_size,
			       u64 max_order, u64 flags)
{
	int err;
	bool is_gva_space = (flags & GPU_ALLOC_GVA_SPACE) != 0ULL;
	struct nvgpu_buddy_allocator *a;

	err = nvgpu_buddy_check_argument_limits(vm, size, blk_size, max_order,
							flags);
	if (err != 0) {
		return err;
	}

	a = nvgpu_kzalloc(g, sizeof(struct nvgpu_buddy_allocator));
	if (a == NULL) {
		return -ENOMEM;
	}

	err = nvgpu_alloc_common_init(na, g, name, a, false, &buddy_ops);
	if (err != 0) {
		goto fail;
	}

	err = nvgpu_buddy_set_attributes(a, na, vm, base, size, blk_size,
					max_order, flags);
	if (err != 0) {
		goto fail;
	}

	err = balloc_allocator_align(a);
	if (err != 0) {
		goto fail;
	}
	balloc_compute_max_order(a);

	a->buddy_cache = nvgpu_kmem_cache_create(g, sizeof(struct nvgpu_buddy));
	if (a->buddy_cache == NULL) {
		err = -ENOMEM;
		goto fail;
	}

	a->alloced_buddies = NULL;
	a->fixed_allocs = NULL;
	nvgpu_init_list_node(&a->co_list);
	err = balloc_init_lists(a);
	if (err != 0) {
		goto fail;
	}

	nvgpu_smp_wmb();
	a->initialized = true;

	nvgpu_init_alloc_debug(g, na);
	alloc_dbg(na, "New allocator: type      buddy");
	alloc_dbg(na, "               base      0x%llx", a->base);
	alloc_dbg(na, "               size      0x%llx", a->length);
	alloc_dbg(na, "               blk_size  0x%llx", a->blk_size);
	if (is_gva_space) {
		alloc_dbg(balloc_owner(a),
		       "               pde_size  0x%llx",
			  balloc_order_to_len(a, a->pte_blk_order));
	}
	alloc_dbg(na, "               max_order %llu", a->max_order);
	alloc_dbg(na, "               flags     0x%llx", a->flags);

	return 0;

fail:
	if (a->buddy_cache != NULL) {
		nvgpu_kmem_cache_destroy(a->buddy_cache);
	}
	nvgpu_kfree(g, a);
	return err;
}
