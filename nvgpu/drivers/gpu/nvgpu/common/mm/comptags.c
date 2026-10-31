// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bug.h>
#include <nvgpu/bitops.h>
#include <nvgpu/comptags.h>
#include <nvgpu/gk20a.h>

int gk20a_comptaglines_alloc(struct gk20a_comptag_allocator *allocator,
			     u32 *offset, u32 len)
{
	unsigned long addr;
	int err = 0;

	if (allocator->size == 0UL) {
		return -EINVAL;
	}

	nvgpu_mutex_acquire(&allocator->lock);
	addr = bitmap_find_next_zero_area(allocator->bitmap, allocator->size,
			0, len, 0);
	if (addr < allocator->size) {
		/* number zero is reserved; bitmap base is 1 */
		nvgpu_assert(addr < U64(U32_MAX));
		*offset = 1U + U32(addr);
		nvgpu_bitmap_set(allocator->bitmap, U32(addr), len);
	} else {
		err = -ENOMEM;
	}
	nvgpu_mutex_release(&allocator->lock);

	return err;
}

void gk20a_comptaglines_free(struct gk20a_comptag_allocator *allocator,
			     u32 offset, u32 len)
{
	/* number zero is reserved; bitmap base is 1 */
	u32 addr = nvgpu_safe_sub_u32(offset, 1U);

	if (allocator->size == 0UL) {
		return;
	}

	WARN_ON(offset == 0U);
	WARN_ON(addr > allocator->size);
	WARN_ON((unsigned long)addr + (unsigned long)len > allocator->size);

	nvgpu_mutex_acquire(&allocator->lock);
	nvgpu_bitmap_clear(allocator->bitmap, addr, len);
	nvgpu_mutex_release(&allocator->lock);
}

int gk20a_comptag_allocator_init(struct gk20a *g,
				 struct gk20a_comptag_allocator *allocator,
				 unsigned long size)
{
	nvgpu_mutex_init(&allocator->lock);

	/*
	 * 0th comptag is special and is never used. The base for this bitmap
	 * is 1, and its size is one less than the size of comptag store.
	 */
	size = nvgpu_safe_sub_u64(size, 1UL);
	allocator->bitmap = nvgpu_vzalloc(g,
					  BITS_TO_LONGS(size) * sizeof(long));
	if (allocator->bitmap == NULL) {
		return -ENOMEM;
	}

	allocator->size = size;

	return 0;
}

void gk20a_comptag_allocator_destroy(struct gk20a *g,
				     struct gk20a_comptag_allocator *allocator)
{
	/*
	 * called only when exiting the driver (gk20a_remove, or unwinding the
	 * init stage); no users should be active, so taking the mutex is
	 * unnecessary here.
	 */
	allocator->size = 0;
	nvgpu_vfree(g, allocator->bitmap);
}
