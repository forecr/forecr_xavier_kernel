// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/gk20a.h>
#include <nvgpu/timers.h>
#include <nvgpu/ipa_pa_cache.h>

/**
 * @brief Looks up the physical address (PA) corresponding to a given intermediate
 *        physical address (IPA) in the IPA to PA cache.
 *
 * This function searches the IPA to PA cache for an entry matching the given IPA.
 * If a match is found, it calculates the corresponding PA and optionally returns
 * the size of the memory region starting from the given IPA within the matched
 * cache entry. If no match is found, it returns 0.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a pointer to the IPA to PA cache structure from the GPU structure.
 * -# Initialize a pointer to the first descriptor in the IPA to PA cache.
 * -# Iterate over each descriptor in the cache:
 *    -# Check if the given IPA is within the range specified by the current
 *       descriptor's base IPA and size.
 *    -# If a match is found, calculate the corresponding PA by subtracting the
 *       base IPA from the given IPA and adding the base PA of the descriptor.
 *    -# If the output parameter for the PA length is not NULL, calculate and
 *       set the length of the memory region starting from the given IPA within
 *       the matched descriptor.
 *    -# Return the calculated PA.
 * -# If no match is found after iterating through all descriptors, return 0.
 *
 * @param [in]  g        Pointer to the GPU structure.
 * @param [in]  ipa      Intermediate physical address to look up.
 * @param [out] pa_len   Pointer to store the length of the memory region starting
 *                       from the given IPA within the matched descriptor, if found.
 *
 * @return The physical address corresponding to the given IPA if found, otherwise 0.
 */
static u64 nvgpu_ipa_to_pa_cache_lookup(struct gk20a *g, u64 ipa,
				u64 *pa_len)
{
	struct nvgpu_ipa_desc *desc;
	struct nvgpu_ipa_pa_cache *ipa_cache;
	u32 i = 0U;
	u64 pa = 0UL;

	ipa_cache = &(g->ipa_pa_cache);
	desc = &(ipa_cache->ipa[0]);

	for (i = 0; i < ipa_cache->num_ipa_desc; ++i) {
		if (ipa >= desc->ipa_base &&
				(ipa < (desc->ipa_base + desc->ipa_size))) {
			pa = ipa - desc->ipa_base + desc->pa_base;
			if (pa_len != NULL) {
				*pa_len = desc->ipa_size -(ipa - desc->ipa_base);
			}

			return pa;
		}
		++desc;
	}
	return 0U;
}

u64 nvgpu_ipa_to_pa_cache_lookup_locked(struct gk20a *g, u64 ipa,
		u64 *pa_len)
{
	struct nvgpu_ipa_pa_cache *ipa_cache;
	u64 pa = 0UL;

	ipa_cache = &(g->ipa_pa_cache);
	nvgpu_rwsem_down_read(&(ipa_cache->ipa_pa_rw_lock));
	pa = nvgpu_ipa_to_pa_cache_lookup(g, ipa, pa_len);
	nvgpu_rwsem_up_read(&(ipa_cache->ipa_pa_rw_lock));
	return pa;
}

void nvgpu_ipa_to_pa_add_to_cache(struct gk20a *g, u64 ipa, u64 pa,
				struct nvgpu_hyp_ipa_pa_info *info)
{
	struct nvgpu_ipa_pa_cache *ipa_cache;
	struct nvgpu_ipa_desc *desc = NULL;
	u64 pa_cached = 0U;

	ipa_cache = &(g->ipa_pa_cache);
	nvgpu_rwsem_down_write(&(ipa_cache->ipa_pa_rw_lock));
	pa_cached = nvgpu_ipa_to_pa_cache_lookup(g, ipa, NULL);
	if (pa_cached != 0UL) {
		/* Check any other context insert the translation
		 * already and return.
		 */
		nvgpu_assert(pa_cached == pa);
		nvgpu_rwsem_up_write(&(ipa_cache->ipa_pa_rw_lock));
		return;
	}

        if (ipa_cache->num_ipa_desc >= MAX_IPA_PA_CACHE) {
                desc = &ipa_cache->ipa[nvgpu_safe_cast_s64_to_u64(nvgpu_current_time_ns()) %
			MAX_IPA_PA_CACHE];
        } else {
                desc = &ipa_cache->ipa[ipa_cache->num_ipa_desc++];
        }

	desc->ipa_base = nvgpu_safe_sub_u64(ipa, info->offset);
        desc->ipa_size = info->size;
        desc->pa_base = info->base;
	nvgpu_rwsem_up_write(&(ipa_cache->ipa_pa_rw_lock));

}
