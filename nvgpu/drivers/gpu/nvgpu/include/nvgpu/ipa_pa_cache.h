/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_IPAPACACHE_H
#define NVGPU_IPAPACACHE_H

#include <nvgpu/rwsem.h>

/**
 * Struct to hold IPA PA information as per hypervisor format.
 */
struct nvgpu_hyp_ipa_pa_info {
	/**
	 * Start physical address of IPA-PA block.
	 */
	u64 base;
	/**
	 * Byte offset in IPA-PA block for the queried IPA.
	 */
	u64 offset;
	/**
	 * Size of the IPA-PA block.
	 */
	u64 size;
};

struct gk20a;

/**
 * Size of IPA-PA cache maintained by the nvgpu driver.
 */
#define MAX_IPA_PA_CACHE 256U

/**
 * Struct for holding base IPA and PA addresses of IPA-PA block. This is used as a single IPA-PA
 * cache entry.
 */
struct nvgpu_ipa_desc {
	/**
	 * Start intermediate physical address of IPA-PA block.
	 */
	u64 ipa_base;
	/**
	 * Size of the IPA-PA block.
	 */
	u64 ipa_size;
	/**
	 * Start physical address of IPA-PA block.
	 */
	u64 pa_base;
};

/**
 * Struct having all the IPA-PA data including its lock and cache count.
 */
struct nvgpu_ipa_pa_cache {
	/**
	 * RW Lock to protect access to cache.
	 */
	struct nvgpu_rwsem ipa_pa_rw_lock;
	/**
	 * Array of nvgpu_ipa_desc so that cache miss can be minimized.
	 */
	struct nvgpu_ipa_desc ipa[MAX_IPA_PA_CACHE];
	/**
	 * Number of cache descriptors in use.
	 */
	u32 num_ipa_desc;
};

/**
 * @brief Performs a thread-safe lookup of the physical address (PA) corresponding to a given
 *        intermediate physical address (IPA) in the IPA to PA cache.
 *
 * This function acquires a read lock on the IPA to PA cache, calls the
 * nvgpu_ipa_to_pa_cache_lookup() function to perform the actual lookup, and then releases
 * the read lock. It ensures that the lookup operation is performed without interference from
 * other threads that might be writing to the cache.
 *
 * The steps performed by the function are as follows:
 * -# Obtain a pointer to the IPA to PA cache structure from the GPU structure.
 * -# Acquire a read lock on the IPA to PA cache to ensure thread-safe read access by calling
 *    nvgpu_rwsem_down_read().
 * -# Perform the lookup for the given IPA in the cache and obtain the corresponding PA by calling
 *    nvgpu_ipa_to_pa_cache_lookup().
 * -# Release the read lock on the IPA to PA cache by calling nvgpu_rwsem_up_read().
 * -# Return the PA obtained from the lookup.
 *
 * @param [in]  g        Pointer to the GPU structure.
 * @param [in]  ipa      Intermediate physical address to look up.
 * @param [out] pa_len   Pointer to store the length of the memory region starting from the given
 *                       IPA within the matched descriptor, if found.
 *
 * @return The physical address corresponding to the given IPA if found, otherwise 0.
 */
u64 nvgpu_ipa_to_pa_cache_lookup_locked(struct gk20a *g, u64 ipa,
		u64 *pa_len);

/**
 * @brief Adds a new IPA to PA translation entry to the IPA to PA cache.
 *
 * This function attempts to add a new translation entry to the IPA to PA cache
 * if it does not already exist. It acquires a write lock before modifying the
 * cache and releases it afterwards. If the cache is full, it overwrites an
 * existing entry based on the current time. The function ensures that the
 * translation for the given IPA is not already present before adding a new one.
 *
 * The steps performed by the function are as follows:
 * -# Obtain a pointer to the IPA to PA cache structure from the GPU structure.
 * -# Acquire a write lock on the IPA to PA cache to ensure exclusive access by calling
 *    nvgpu_rwsem_down_write().
 * -# Look up the given IPA in the cache to check if it is already cached by calling
 *    nvgpu_ipa_to_pa_cache_lookup().
 * -# If the IPA is already cached, verify that the cached PA matches the given PA using
 *    nvgpu_assert(). If it matches, release the write lock by calling nvgpu_rwsem_up_write()
 *    and return without adding a new entry.
 * -# If the IPA is not cached and the number of descriptors in the cache has reached
 *    the maximum limit (#MAX_IPA_PA_CACHE), overwrite an existing entry based on the
 *    current time modulo the maximum number of cache entries by calling nvgpu_current_time_ns()
 *    and nvgpu_safe_cast_s64_to_u64().
 * -# If the cache has not reached the maximum limit, use the next available descriptor.
 * -# Set the base IPA, IPA size, and base PA for the new cache entry using the given
 *    IPA and the information from the provided 'info' structure by performing arithmetic
 *    operations and assignment.
 * -# Release the write lock on the IPA to PA cache by calling nvgpu_rwsem_up_write().
 *
 * @param [in] g     Pointer to the GPU structure.
 * @param [in] ipa   Intermediate physical address to be added to the cache.
 * @param [in] pa    Physical address corresponding to the given IPA.
 * @param [in] info  Pointer to the structure containing the base PA and size information.
 */
void nvgpu_ipa_to_pa_add_to_cache(struct gk20a *g, u64 ipa,
		u64 pa, struct nvgpu_hyp_ipa_pa_info *info);
#endif /* NVGPU_IPAPACACHE_H */
