// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/trace.h>
#include <nvgpu/kmem.h>
#include <nvgpu/vm.h>
#include <nvgpu/log2.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>
#include <nvgpu/nvgpu_init.h>

#define VM_NAME_PREFIX	"as_"

static nvgpu_atomic_t unique_share_id = NVGPU_ATOMIC_INIT(0);

/**
 * @brief Generates a unique identifier for sharing resources.
 *
 * This function increments a global atomic counter to generate a unique identifier
 * that can be used for sharing resources among different contexts. It checks for
 * overflow to ensure that the identifier remains unique.
 *
 * The steps performed by the function are as follows:
 * -# Atomically increment the global counter and obtain the new value using
 *    nvgpu_atomic_inc_return().
 * -# Check if the returned value is zero, which indicates an overflow occurred.
 *    If an overflow is detected, log an error message.
 *
 * @return The unique identifier generated, which is a positive integer value.
 * @retval 0 Indicates that an overflow occurred and the identifier is not unique.
 */
static int generate_unique_share_id(void)
{
	int ret = 0;

	ret = nvgpu_atomic_inc_return(&unique_share_id);
	if (ret == 0) {
		nvgpu_err(NULL, "incrementing share_id overflow");
	}

	return ret;
}

/**
 * @brief Allocate and initialize a GPU address space share.
 *
 * This function sets up a GPU address space share, which includes calculating
 * the size of the user address space, determining if big pages can be used,
 * and initializing the VM (virtual memory) area with the given parameters.
 * It also checks for alignment and validity of the provided address space
 * ranges.
 *
 * The steps performed by the function are as follows:
 * -# Check if the provided big page size is zero, indicating the default size
 *    should be used. If not, validate that it is a power of two and is among
 *    the available big page sizes for the GPU using is_power_of_2() and
 *    nvgpu_mm_get_available_big_page_sizes().
 * -# Calculate the size of a page directory entry (PDE) based on the big page
 *    size using nvgpu_vm_pde_coverage_bit_count() and create a mask for
 *    validating address alignment using nvgpu_safe_sub_u64().
 * -# Validate that the start and end of the user-managed address range are
 *    aligned to the PDE size and that the start is less than the end.
 * -# If the address space is not unified or big pages are not used, further
 *    validate the address range split.
 * -# Append the share ID to the VM name prefix to create a unique VM name using
 *    strlen(), nvgpu_safe_cast_s32_to_u32(), and nvgpu_strnadd_u32().
 * -# Initialize the VM with the calculated parameters using nvgpu_vm_init().
 *    If VM initialization is successful, update the address space share
 *    structure with the new VM and enable ctags.
 *
 * @param [in]  as_share        The address space share structure.
 * @param [in]  big_page_size   The big page size to use for the VM.
 * @param [in]  flags           Flags for address space allocation.
 * @param [in]  va_range_start  The start of the user-managed virtual address range.
 * @param [in]  va_range_end    The end of the user-managed virtual address range.
 * @param [in]  va_range_split  The split between small and big page regions.
 *
 * @return 0 if the VM was successfully initialized and associated with the address space share.
 * @return -EINVAL if any parameter checks fail.
 * @return -ENOMEM if memory allocation for the VM fails.
 */
static int gk20a_vm_alloc_share(struct gk20a_as_share *as_share,
				u32 big_page_size, u32 flags,
				u64 va_range_start, u64 va_range_end,
				u64 va_range_split)
{
	struct gk20a *g = as_share->g;
	struct mm_gk20a *mm = &g->mm;
	struct vm_gk20a *vm;
	char name[NVGPU_VM_NAME_LEN] = VM_NAME_PREFIX;
	char *p;
	u64 user_size;
	u64 kernel_size = mm->channel.kernel_size;
	u64 pde_size, pde_size_mask;
	bool big_pages;
	const bool unified_va =
		nvgpu_is_enabled(g, NVGPU_MM_UNIFY_ADDRESS_SPACES) ||
		((flags & NVGPU_AS_ALLOC_UNIFIED_VA) != 0U);

	nvgpu_log_fn(g, " ");

	if (big_page_size == 0U) {
		big_pages = false;
		big_page_size = g->ops.mm.gmmu.get_default_big_page_size();
	} else {
		if (is_power_of_2(big_page_size) == false) {
			return -EINVAL;
		}

		if ((big_page_size &
		     nvgpu_mm_get_available_big_page_sizes(g)) == 0U) {
			return -EINVAL;
		}
		big_pages = true;
	}

	pde_size = BIT64(nvgpu_vm_pde_coverage_bit_count(g, big_page_size));
	pde_size_mask = nvgpu_safe_sub_u64(pde_size, U64(1));

	if ((va_range_start == 0ULL) ||
		((va_range_start & pde_size_mask) != 0ULL)) {
		return -EINVAL;
	}

	if ((va_range_end == 0ULL) ||
		((va_range_end & pde_size_mask) != 0ULL)) {
		return -EINVAL;
	}

	if (va_range_start >= va_range_end) {
		return -EINVAL;
	}

	user_size = nvgpu_safe_sub_u64(va_range_end, va_range_start);

	if (unified_va || !big_pages) {
		if (va_range_split != 0ULL) {
			return -EINVAL;
		}
	} else {
		/* non-unified VA: split required */
		if ((va_range_split == 0ULL) ||
			((va_range_split & pde_size_mask) != 0ULL)) {
			return -EINVAL;
		}

		/* non-unified VA: split range checks */
		if ((va_range_split <= va_range_start) ||
		    (va_range_split >= va_range_end)) {
			return -EINVAL;
		}
	}

	nvgpu_log_info(g,
		"vm: low_hole=0x%llx, user_size=0x%llx, kernel_size=0x%llx",
		va_range_start, user_size, kernel_size);

	p = name + strlen(name);
	(void) nvgpu_strnadd_u32(p, nvgpu_safe_cast_s32_to_u32(as_share->id),
				 sizeof(name) - sizeof(VM_NAME_PREFIX), 10U);

	vm = nvgpu_vm_init(g, big_page_size,
			   va_range_start,
			   user_size,
			   kernel_size,
			   va_range_split,
			   big_pages, unified_va, name);
	if (vm == NULL) {
		return -ENOMEM;
	}

	as_share->vm = vm;
	vm->as_share = as_share;
	vm->enable_ctag = true;

	return 0;
}

int gk20a_as_alloc_share(struct gk20a *g,
			u32 big_page_size, u32 flags, u64 va_range_start,
			u64 va_range_end, u64 va_range_split,
			struct gk20a_as_share **out)
{
	struct gk20a_as_share *as_share;
	int err = 0;

	nvgpu_log_fn(g, " ");
	g = nvgpu_get(g);
	if (g == NULL) {
		return -ENODEV;
	}

	*out = NULL;
	as_share = nvgpu_kzalloc(g, sizeof(*as_share));
	if (as_share == NULL) {
		return -ENOMEM;
	}

	as_share->g = g;
	as_share->id = generate_unique_share_id();

	/* this will set as_share->vm. */
	err = gk20a_busy(g);
	if (err != 0) {
		goto failed;
	}
	err = gk20a_vm_alloc_share(as_share, big_page_size, flags,
		va_range_start, va_range_end, va_range_split);
	gk20a_idle(g);

	if (err != 0) {
		goto failed;
	}

	*out = as_share;
	return 0;

failed:
	nvgpu_kfree(g, as_share);
	return err;
}

int gk20a_vm_release_share(struct gk20a_as_share *as_share)
{
	struct vm_gk20a *vm = as_share->vm;
	struct gk20a *g = gk20a_from_vm(vm);

	nvgpu_log_fn(g, " ");

	vm->as_share = NULL;
	as_share->vm = NULL;

	nvgpu_vm_put(vm);

	return 0;
}

/*
 * channels and the device nodes call this to release.
 * once the ref_cnt hits zero the share is deleted.
 */
int gk20a_as_release_share(struct gk20a_as_share *as_share)
{
	struct gk20a *g = as_share->vm->mm->g;
	int err;

	nvgpu_log_fn(g, " ");

	err = gk20a_busy(g);

	if (err != 0) {
		goto release_fail;
	}

	err = gk20a_vm_release_share(as_share);

	gk20a_idle(g);

release_fail:
	nvgpu_put(g);
	nvgpu_kfree(g, as_share);

	return err;
}
