// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bug.h>
#include <nvgpu/log.h>
#include <nvgpu/log2.h>
#include <nvgpu/dma.h>
#include <nvgpu/vm.h>
#include <nvgpu/vm_area.h>
#include <nvgpu/gmmu.h>
#include <nvgpu/lock.h>
#include <nvgpu/list.h>
#include <nvgpu/rbtree.h>
#include <nvgpu/semaphore.h>
#include <nvgpu/enabled.h>
#include <nvgpu/sizes.h>
#include <nvgpu/timers.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_sgt.h>
#include <nvgpu/vgpu/vm_vgpu.h>
#include <nvgpu/cbc.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/power_features/pg.h>
#include <nvgpu/nvhost.h>
#include <nvgpu/string.h>

struct nvgpu_ctag_buffer_info {
	u64			size;
	u32			pgsz_idx;
	u32			flags;

#ifdef CONFIG_NVGPU_COMPRESSION
	u32			ctag_offset;
	s16			compr_kind;
	bool			compr_kind_selected;
#endif
	s16			incompr_kind;

};

#ifdef CONFIG_NVGPU_COMPRESSION
static int nvgpu_vm_compute_compression(struct vm_gk20a *vm,
					struct nvgpu_ctag_buffer_info *binfo);
#endif

static int nvgpu_vm_do_unmap(struct nvgpu_mapped_buf *mapped_buffer,
			      struct vm_gk20a_mapping_batch *batch);

/*
 * Attempt to find a reserved memory area to determine PTE size for the passed
 * mapping. If no reserved area can be found use small pages.
 */
static u32 nvgpu_vm_get_pte_size_fixed_map(struct vm_gk20a *vm, u64 base)
{
	struct nvgpu_vm_area *vm_area;

	vm_area = nvgpu_vm_area_find(vm, base);
	if (vm_area == NULL) {
		return GMMU_PAGE_SIZE_SMALL;
	}

	return vm_area->pgsz_idx;
}

/*
 * This is for when the address space does not support unified address spaces.
 */
static u32 nvgpu_vm_get_pte_size_split_addr(struct vm_gk20a *vm,
					    u64 base, u64 size)
{
	if (base == 0ULL) {
		if (size >= vm->gmmu_page_sizes[GMMU_PAGE_SIZE_BIG]) {
			return GMMU_PAGE_SIZE_BIG;
		}
		return GMMU_PAGE_SIZE_SMALL;
	} else {
		if (base < nvgpu_gmmu_va_small_page_limit()) {
			return GMMU_PAGE_SIZE_SMALL;
		} else {
			return GMMU_PAGE_SIZE_BIG;
		}
	}
}

/*
 * This determines the PTE size for a given alloc. Used by both the GVA space
 * allocator and the mm core code so that agreement can be reached on how to
 * map allocations.
 *
 * The page size of a buffer is this:
 *
 *   o  If the VM doesn't support large pages then obviously small pages
 *      must be used.
 *   o  If the base address is non-zero (fixed address map):
 *      - Attempt to find a reserved memory area and use the page size
 *        based on that.
 *      - If no reserved page size is available, default to small pages.
 *   o  If the base is zero and we have an SMMU:
 *      - If the size is larger than or equal to the big page size, use big
 *        pages.
 *      - Otherwise use small pages.
 *   o If there's no SMMU:
 *      - Regardless of buffer size use small pages since we have no
 *      - guarantee of contiguity.
 */
static u32 nvgpu_vm_get_pte_size(struct vm_gk20a *vm, u64 base, u64 size)
{
	struct gk20a *g = gk20a_from_vm(vm);

	if (!vm->big_pages) {
		return GMMU_PAGE_SIZE_SMALL;
	}

	if (!vm->unified_va) {
		return nvgpu_vm_get_pte_size_split_addr(vm, base, size);
	}

	if (base != 0ULL) {
		return nvgpu_vm_get_pte_size_fixed_map(vm, base);
	}

	if ((size >= vm->gmmu_page_sizes[GMMU_PAGE_SIZE_BIG]) &&
	    nvgpu_iommuable(g)) {
		return GMMU_PAGE_SIZE_BIG;
	}
	return GMMU_PAGE_SIZE_SMALL;
}

int vm_aspace_id(struct vm_gk20a *vm)
{
	return (vm->as_share != NULL) ? vm->as_share->id : -1;
}

int nvgpu_vm_bind_channel(struct vm_gk20a *vm, struct nvgpu_channel *ch)
{
	if (ch == NULL) {
		return -EINVAL;
	}

	nvgpu_log_fn(ch->g, " ");

	nvgpu_vm_get(vm);
	ch->vm = vm;
	nvgpu_channel_commit_va(ch);

	nvgpu_log(gk20a_from_vm(vm), gpu_dbg_map, "Binding ch=%d -> VM:%s",
		  ch->chid, vm->name);

	return 0;
}

/*
 * Determine how many bits of the address space each last level PDE covers. For
 * example, for gp10b, with a last level address bit PDE range of 28 to 21 the
 * amount of memory each last level PDE addresses is 21 bits - i.e 2MB.
 */
u32 nvgpu_vm_pde_coverage_bit_count(struct gk20a *g, u64 big_page_size)
{
	int final_pde_level = 0;
	const struct gk20a_mmu_level *mmu_levels =
		g->ops.mm.gmmu.get_mmu_levels(g, big_page_size);

	/*
	 * Find the second to last level of the page table programming
	 * heirarchy: the last level is PTEs so we really want the level
	 * before that which is the last level of PDEs.
	 */
	while (mmu_levels[final_pde_level + 2].update_entry != NULL) {
		final_pde_level++;
	}

	return mmu_levels[final_pde_level].lo_bit[0];
}

NVGPU_COV_WHITELIST_BLOCK_BEGIN(deviate, 1, NVGPU_MISRA(Rule, 17_2), "TID-278")
/**
 * @brief Free page directory entries recursively.
 *
 * This function recursively frees the memory associated with the page directory
 * entries of a given GPU virtual memory (VM) area. It starts from the provided
 * level of page directory (PD) and continues to free all child PDs until the
 * last level is reached. It ensures that the memory for the PD and its entries
 * is properly released.
 *
 * The steps performed by the function are as follows:
 * -# Obtain the GPU device structure from the VM structure.
 * -# Assert that the provided level is less than the maximum page table levels
 *    supported by the GPU.
 * -# If the PD has an associated memory, free the PD memory using nvgpu_pd_free().
 * -# Set the PD memory pointer to NULL.
 * -# If the PD has entries, iterate over each entry:
 *    -# Assert that the current level is less than the maximum unsigned 32-bit value.
 *    -# Recursively call nvgpu_vm_do_free_entries() for each entry, incrementing
 *       the level by 1.
 * -# Free the memory allocated for the PD entries using nvgpu_vfree().
 * -# Set the PD entries pointer to NULL.
 *
 * @param [in]  vm    Pointer to the GPU VM structure.
 * @param [in]  pd    Pointer to the page directory structure.
 * @param [in]  level Current level of the page directory being freed.
 *
 * @return None.
 */
static void nvgpu_vm_do_free_entries(struct vm_gk20a *vm,
				     struct nvgpu_gmmu_pd *pd,
				     u32 level)
{
	struct gk20a *g = gk20a_from_vm(vm);
	u32 i;

	/* This limits recursion */
	nvgpu_assert(level < g->ops.mm.gmmu.get_max_page_table_levels(g));

	if (pd->mem != NULL) {
		nvgpu_pd_free(vm, pd);
		pd->mem = NULL;
	}

	if (pd->entries != NULL) {
		for (i = 0; i < pd->num_entries; i++) {
			nvgpu_assert(level < U32_MAX);
			nvgpu_vm_do_free_entries(vm, &pd->entries[i],
						 level + 1U);
		}
		nvgpu_vfree(vm->mm->g, pd->entries);
		pd->entries = NULL;
	}
}
NVGPU_COV_WHITELIST_BLOCK_END(NVGPU_MISRA(Rule, 17_2))

/**
 * @brief Free the page directory and its entries.
 *
 * This function frees the memory associated with the page directory and its entries
 * for a given GPU virtual memory (VM) area. It ensures that the memory for the page
 * directory base (pdb) and its entries is properly released.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the GPU device structure from the VM structure.
 * -# Free the page directory base memory using nvgpu_pd_free().
 * -# If the page directory base has no entries, return immediately.
 * -# Iterate over each entry in the page directory base:
 *    -# Call nvgpu_vm_do_free_entries() for each entry, starting from level 1.
 * -# Free the memory allocated for the page directory base entries using nvgpu_vfree().
 * -# Set the page directory base entries pointer to NULL.
 *
 * @param [in] vm   Pointer to the GPU VM structure.
 * @param [in] pdb  Pointer to the page directory base structure.
 *
 * @return None.
 */
static void nvgpu_vm_free_entries(struct vm_gk20a *vm,
				  struct nvgpu_gmmu_pd *pdb)
{
	struct gk20a *g = vm->mm->g;
	u32 i;

	nvgpu_pd_free(vm, pdb);

	if (pdb->entries == NULL) {
		return;
	}

	for (i = 0; i < pdb->num_entries; i++) {
		nvgpu_vm_do_free_entries(vm, &pdb->entries[i], 1U);
	}

	nvgpu_vfree(g, pdb->entries);
	pdb->entries = NULL;
}

u64 nvgpu_vm_alloc_va(struct vm_gk20a *vm, u64 size,
		u32 pgsz_idx, bool align_required, u64 align_size)
{
	struct gk20a *g = vm->mm->g;
	struct nvgpu_allocator *vma = NULL;
	u64 addr;
	u32 page_size = vm->gmmu_page_sizes[pgsz_idx];

	vma = vm->vma[pgsz_idx];

	if (pgsz_idx >= GMMU_NR_PAGE_SIZES) {
		nvgpu_err(g, "(%s) invalid page size requested", vma->name);
		return 0;
	}

	if ((pgsz_idx == GMMU_PAGE_SIZE_BIG) && !vm->big_pages) {
		nvgpu_err(g, "(%s) unsupportd page size requested", vma->name);
		return 0;
	}

	if ((pgsz_idx == GMMU_PAGE_SIZE_HUGE) && !vm->huge_pages) {
		nvgpu_err(g, "(%s) unsupportd page size requested", vma->name);
		return 0;
	}

	/* Be certain we round up to page_size if needed */
	size = NVGPU_ALIGN(size, page_size);

	addr = nvgpu_alloc_pte(vma, size, page_size, align_required, align_size);
	if (addr == 0ULL) {
		nvgpu_err(g, "(%s) oom: sz=0x%llx", vma->name, size);
		return 0;
	}

	return addr;
}

void nvgpu_vm_free_va(struct vm_gk20a *vm, u64 addr, u32 pgsz_idx)
{
	struct nvgpu_allocator *vma = vm->vma[pgsz_idx];

	nvgpu_free(vma, addr);
}

void nvgpu_vm_mapping_batch_start(struct vm_gk20a_mapping_batch *mapping_batch)
{
	(void) memset(mapping_batch, 0, sizeof(*mapping_batch));
	mapping_batch->gpu_l2_flushed = false;
	mapping_batch->need_tlb_invalidate = false;
}

void nvgpu_vm_mapping_batch_finish_locked(
	struct vm_gk20a *vm, struct vm_gk20a_mapping_batch *mapping_batch)
{
	int err;

	/* hanging kref_put batch pointer? */
	WARN_ON(vm->kref_put_batch == mapping_batch);

	if (mapping_batch->need_tlb_invalidate) {
		struct gk20a *g = gk20a_from_vm(vm);
		err = g->ops.fb.tlb_invalidate(g, vm->pdb.mem);
		if (err != 0) {
			nvgpu_err(g, "fb.tlb_invalidate() failed err=%d", err);
		}
	}
}

void nvgpu_vm_mapping_batch_finish(struct vm_gk20a *vm,
				   struct vm_gk20a_mapping_batch *mapping_batch)
{
	nvgpu_mutex_acquire(&vm->update_gmmu_lock);
	nvgpu_vm_mapping_batch_finish_locked(vm, mapping_batch);
	nvgpu_mutex_release(&vm->update_gmmu_lock);
}

/*
 * Determine if the passed address space can support huge pages or not.
 */
bool nvgpu_huge_pages_possible(struct vm_gk20a *vm, u64 base, u64 size)
{
	u64 pde_size = BIT64(nvgpu_vm_pde_coverage_bit_count(gk20a_from_vm(vm),
							     vm->huge_page_size));
	u64 mask = nvgpu_safe_sub_u64(pde_size, 1ULL);
	u64 base_huge_page = base & mask;
	u64 size_huge_page = size & mask;

	if ((base_huge_page != 0ULL) || (size_huge_page != 0ULL)) {
		return false;
	}
	return true;
}

/*
 * Determine if the passed address space can support big pages or not.
 */
bool nvgpu_big_pages_possible(struct vm_gk20a *vm, u64 base, u64 size)
{
	u64 pde_size = BIT64(nvgpu_vm_pde_coverage_bit_count(
				gk20a_from_vm(vm), vm->big_page_size));
	u64 mask = nvgpu_safe_sub_u64(pde_size, 1ULL);
	u64 base_big_page = base & mask;
	u64 size_big_page = size & mask;

	if ((base_big_page != 0ULL) || (size_big_page != 0ULL)) {
		return false;
	}
	return true;
}

#ifdef CONFIG_NVGPU_SW_SEMAPHORE
/*
 * Initialize a semaphore pool. Just return successfully if we do not need
 * semaphores (i.e when sync-pts are active).
 */
static int nvgpu_init_sema_pool(struct vm_gk20a *vm)
{
	struct nvgpu_semaphore_sea *sema_sea;
	struct mm_gk20a *mm = vm->mm;
	struct gk20a *g = mm->g;
	int err;

	/*
	 * Don't waste the memory on semaphores if we don't need them.
	 */
	if (nvgpu_has_syncpoints(g) &&
		!nvgpu_is_enabled(g, NVGPU_SUPPORT_SEMA_BASED_GPFIFO_GET)) {
		return 0;
	}

	if (vm->sema_pool != NULL) {
		return 0;
	}

	sema_sea = nvgpu_semaphore_sea_create(g);
	if (sema_sea == NULL) {
		return -ENOMEM;
	}

	err = nvgpu_semaphore_pool_alloc(sema_sea, &vm->sema_pool);
	if (err != 0) {
		return err;
	}

	/*
	 * Allocate a chunk of GPU VA space for mapping the semaphores. We will
	 * do a fixed alloc in the kernel VM so that all channels have the same
	 * RO address range for the semaphores.
	 *
	 * !!! TODO: cleanup.
	 */
	nvgpu_semaphore_sea_allocate_gpu_va(sema_sea, &vm->kernel,
					nvgpu_safe_sub_u64(vm->va_limit,
						mm->channel.kernel_size),
					512U * NVGPU_CPU_PAGE_SIZE,
					nvgpu_safe_cast_u64_to_u32(SZ_4K));
	if (nvgpu_semaphore_sea_get_gpu_va(sema_sea) == 0ULL) {
		nvgpu_free(&vm->kernel,
			nvgpu_semaphore_sea_get_gpu_va(sema_sea));
		nvgpu_vm_put(vm);
		return -ENOMEM;
	}

	err = nvgpu_semaphore_pool_map(vm->sema_pool, vm);
	if (err != 0) {
		nvgpu_semaphore_pool_unmap(vm->sema_pool, vm);
		nvgpu_free(vm->vma[GMMU_PAGE_SIZE_SMALL],
			   nvgpu_semaphore_pool_gpu_va(vm->sema_pool, false));
		return err;
	}

	return 0;
}
#endif

/**
 * @brief Initialize the user virtual memory allocator (VMA) for a VM.
 *
 * This function sets up the user VMA for a given GPU virtual memory (VM) area.
 * It creates an allocator name based on the VM name and initializes the user
 * VMA if the start address is less than the limit address. Otherwise, it points
 * the small and big page VMA pointers to the kernel allocator.
 *
 * The steps performed by the function are as follows:
 * -# Calculate the length of the allocator name by adding the length of the
 *    prefix "gk20a_" to the length of the provided VM name.
 * -# If the calculated name length exceeds the maximum allowed allocator name
 *    length, log an error and return with an invalid argument error code.
 * -# If the user VMA start address is less than the user VMA limit address:
 *    -# Construct the allocator name by concatenating the prefix "gk20a_" with
 *       the provided VM name.
 *    -# Initialize the user VMA allocator by calling nvgpu_allocator_init()
 *       with the constructed name, start and limit addresses, and other
 *       parameters for the allocator configuration.
 *    -# If the allocator initialization fails, return the error code.
 * -# If the user VMA start address is not less than the user VMA limit address:
 *    -# Point the small and big page VMA pointers to the kernel allocator.
 * -# Return 0 to indicate success.
 *
 * @param [in] g               Pointer to the GPU device structure.
 * @param [in] vm              Pointer to the GPU VM structure.
 * @param [in] user_vma_start  Start address of the user VMA.
 * @param [in] user_vma_limit  Limit address of the user VMA.
 * @param [in] name            Name of the VM.
 *
 * @return 0 if successful, or a negative error code if an error occurred.
 */
static int nvgpu_vm_init_user_vma(struct gk20a *g, struct vm_gk20a *vm,
			u64 user_vma_start, u64 user_vma_limit,
			const char *name)
{
	int err = 0;
	char alloc_name[NVGPU_ALLOC_NAME_LEN];
	size_t name_len;

	name_len  = strlen("gk20a_") + strlen(name);
	if (name_len >= NVGPU_ALLOC_NAME_LEN) {
		nvgpu_err(g, "Invalid MAX_NAME_SIZE %lu %u", name_len,
			NVGPU_ALLOC_NAME_LEN);
		return -EINVAL;
	}

	/*
	 * User VMA.
	 */
	if (user_vma_start < user_vma_limit) {
		(void) strcpy(alloc_name, "gk20a_");
		(void) strcat(alloc_name, name);
		err = nvgpu_allocator_init(g, &vm->user,
						 vm, alloc_name,
						 user_vma_start,
						 user_vma_limit -
						 user_vma_start,
						 SZ_4K,
						 GPU_BALLOC_MAX_ORDER,
						 GPU_ALLOC_GVA_SPACE,
						 BUDDY_ALLOCATOR);
		if (err != 0) {
			return err;
		}
	} else {
		/*
		 * Make these allocator pointers point to the kernel allocator
		 * since we still use the legacy notion of page size to choose
		 * the allocator.
		 */
		vm->vma[0] = &vm->kernel;
		vm->vma[1] = &vm->kernel;
	}
	return 0;
}

/**
 * @brief Initialize the user virtual memory allocator (VMA) for large pages.
 *
 * This function sets up the user VMA for large pages for a given GPU virtual memory
 * (VM) area when a split address range is used. It creates an allocator name based
 * on the VM name with an "_lp" suffix for large pages and initializes the user
 * large page VMA if the start address is less than the limit address.
 *
 * The steps performed by the function are as follows:
 * -# Calculate the length of the allocator name by adding the length of the
 *    prefix "gk20a_", the provided VM name, and the suffix "_lp".
 * -# If the calculated name length exceeds the maximum allowed allocator name
 *    length, log an error and return with an invalid argument error code.
 * -# If the user large page VMA start address is less than the user large page
 *    VMA limit address:
 *    -# Construct the allocator name by concatenating the prefix "gk20a_" with
 *       the provided VM name and the suffix "_lp".
 *    -# Initialize the user large page VMA allocator by calling nvgpu_allocator_init()
 *       with the constructed name, start and limit addresses, the VM's big page size,
 *       and other parameters for the allocator configuration.
 *    -# If the allocator initialization fails, return the error code.
 * -# Return 0 to indicate success.
 *
 * @param [in] g                  Pointer to the GPU device structure.
 * @param [in] vm                 Pointer to the GPU VM structure.
 * @param [in] user_lp_vma_start  Start address of the user large page VMA.
 * @param [in] user_lp_vma_limit  Limit address of the user large page VMA.
 * @param [in] name               Name of the VM.
 *
 * @return 0 if successful, or a negative error code if an error occurred.
 */
static int nvgpu_vm_init_user_lp_vma(struct gk20a *g, struct vm_gk20a *vm,
			u64 user_lp_vma_start, u64 user_lp_vma_limit,
			const char *name)
{
	int err = 0;
	char alloc_name[NVGPU_VM_NAME_LEN];
	size_t name_len;
	const size_t prefix_len = strlen("gk20a_");

	name_len  = nvgpu_safe_add_u64(nvgpu_safe_add_u64(prefix_len,
						strlen(name)), strlen("_lp"));
	if (name_len >= NVGPU_VM_NAME_LEN) {
		nvgpu_err(g, "Invalid MAX_NAME_SIZE %lu %u", name_len,
				  NVGPU_VM_NAME_LEN);
		return -EINVAL;
	}

	/*
	 * User VMA for large pages when a split address range is used.
	 */
	if (user_lp_vma_start < user_lp_vma_limit) {
		(void) strcpy(alloc_name, "gk20a_");
		(void) strncat(alloc_name, name, nvgpu_safe_sub_u64(
					NVGPU_VM_NAME_LEN, prefix_len));
		(void) strcat(alloc_name, "_lp");
		err = nvgpu_allocator_init(g, &vm->user_lp,
						 vm, alloc_name,
						 user_lp_vma_start,
						 user_lp_vma_limit -
						 user_lp_vma_start,
						 vm->big_page_size,
						 GPU_BALLOC_MAX_ORDER,
						 GPU_ALLOC_GVA_SPACE,
						 BUDDY_ALLOCATOR);
		if (err != 0) {
			return err;
		}
	}
	return 0;
}

/**
 * @brief Initialize the kernel virtual memory allocator (VMA) for a VM.
 *
 * This function sets up the kernel VMA for a given GPU virtual memory (VM) area.
 * It creates an allocator name based on the VM name with a "-sys" suffix for the
 * system (kernel) and initializes the kernel VMA if the start address is less than
 * the limit address.
 *
 * The steps performed by the function are as follows:
 * -# Calculate the length of the allocator name by adding the length of the
 *    prefix "gk20a_", the provided VM name, and the suffix "-sys".
 * -# If the calculated name length exceeds the maximum allowed allocator name
 *    length, log an error and return with an invalid argument error code.
 * -# If the kernel VMA start address is less than the kernel VMA limit address:
 *    -# Construct the allocator name by concatenating the prefix "gk20a_" with
 *       the provided VM name and the suffix "-sys".
 *    -# Initialize the kernel VMA allocator by calling nvgpu_allocator_init()
 *       with the constructed name, start and limit addresses, and other
 *       parameters for the allocator configuration, including the kernel VMA flags.
 *    -# If the allocator initialization fails, return the error code.
 * -# Return 0 to indicate success.
 *
 * @param [in] g                  Pointer to the GPU device structure.
 * @param [in] vm                 Pointer to the GPU VM structure.
 * @param [in] kernel_vma_start   Start address of the kernel VMA.
 * @param [in] kernel_vma_limit   Limit address of the kernel VMA.
 * @param [in] kernel_vma_flags   Flags for the kernel VMA allocator.
 * @param [in] name               Name of the VM.
 *
 * @return 0 if successful, or a negative error code if an error occurred.
 */
static int nvgpu_vm_init_kernel_vma(struct gk20a *g, struct vm_gk20a *vm,
			u64 kernel_vma_start, u64 kernel_vma_limit,
			u64 kernel_vma_flags, const char *name)
{
	int err = 0;
	char alloc_name[NVGPU_VM_NAME_LEN];
	size_t name_len;
	const size_t prefix_len = strlen("gk20a_");

	name_len  = nvgpu_safe_add_u64(nvgpu_safe_add_u64(prefix_len,
						strlen(name)),strlen("-sys"));
	if (name_len >= NVGPU_VM_NAME_LEN) {
		nvgpu_err(g, "Invalid MAX_NAME_SIZE %lu %u", name_len,
				  NVGPU_VM_NAME_LEN);
		return -EINVAL;
	}

	/*
	 * Kernel VMA.
	 */
	if (kernel_vma_start < kernel_vma_limit) {
		(void) strcpy(alloc_name, "gk20a_");
		(void) strncat(alloc_name, name, nvgpu_safe_sub_u64(
						NVGPU_VM_NAME_LEN, prefix_len));
		(void) strcat(alloc_name, "-sys");
		err = nvgpu_allocator_init(g, &vm->kernel,
						 vm, alloc_name,
						 kernel_vma_start,
						 kernel_vma_limit -
						 kernel_vma_start,
						 SZ_4K,
						 GPU_BALLOC_MAX_ORDER,
						 kernel_vma_flags,
						 BUDDY_ALLOCATOR);
		if (err != 0) {
			return err;
		}
	}
	return 0;
}

/**
 * @brief Initialize all virtual memory allocators (VMAs) for a VM.
 *
 * This function initializes the user, user large page, and kernel virtual memory
 * allocators for a given GPU virtual memory (VM) area. It sets up each allocator
 * and handles any errors by cleaning up previously initialized allocators before
 * returning.
 *
 * The steps performed by the function are as follows:
 * -# Initialize the user VMA by calling nvgpu_vm_init_user_vma(). If an error
 *    occurs, return the error code.
 * -# Initialize the user large page VMA by calling nvgpu_vm_init_user_lp_vma().
 *    If an error occurs, go to the clean-up label to destroy any previously
 *    initialized allocators and return the error code.
 * -# Initialize the kernel VMA by calling nvgpu_vm_init_kernel_vma(). If an error
 *    occurs, go to the clean-up label to destroy any previously initialized
 *    allocators and return the error code.
 * -# If all initializations are successful, return 0.
 * -# In the clean-up label, check if the kernel allocator is initialized and if so,
 *    destroy it using nvgpu_alloc_destroy().
 * -# Check if the user allocator is initialized and if so, destroy it using
 *    nvgpu_alloc_destroy().
 * -# Check if the user large page allocator is initialized and if so, destroy it
 *    using nvgpu_alloc_destroy().
 * -# Return the error code that caused the clean-up.
 *
 * @param [in] g                  Pointer to the GPU device structure.
 * @param [in] vm                 Pointer to the GPU VM structure.
 * @param [in] user_vma_start     Start address of the user VMA.
 * @param [in] user_vma_limit     Limit address of the user VMA.
 * @param [in] user_lp_vma_start  Start address of the user large page VMA.
 * @param [in] user_lp_vma_limit  Limit address of the user large page VMA.
 * @param [in] kernel_vma_start   Start address of the kernel VMA.
 * @param [in] kernel_vma_limit   Limit address of the kernel VMA.
 * @param [in] kernel_vma_flags   Flags for the kernel VMA allocator.
 * @param [in] name               Name of the VM.
 *
 * @return 0 if successful, or a negative error code if an error occurred.
 */
static int nvgpu_vm_init_vma_allocators(struct gk20a *g, struct vm_gk20a *vm,
			u64 user_vma_start, u64 user_vma_limit,
			u64 user_lp_vma_start, u64 user_lp_vma_limit,
			u64 kernel_vma_start, u64 kernel_vma_limit,
			u64 kernel_vma_flags, const char *name)
{
	int err = 0;

	err = nvgpu_vm_init_user_vma(g, vm,
			user_vma_start, user_vma_limit, name);
	if (err != 0) {
		return err;
	}

	err = nvgpu_vm_init_user_lp_vma(g, vm,
			user_lp_vma_start, user_lp_vma_limit, name);
	if (err != 0) {
		goto clean_up_allocators;
	}

	err = nvgpu_vm_init_kernel_vma(g, vm, kernel_vma_start,
			kernel_vma_limit, kernel_vma_flags, name);
	if (err != 0) {
		goto clean_up_allocators;
	}

	return 0;

clean_up_allocators:
	if (nvgpu_alloc_initialized(&vm->kernel)) {
		nvgpu_alloc_destroy(&vm->kernel);
	}
	if (nvgpu_alloc_initialized(&vm->user)) {
		nvgpu_alloc_destroy(&vm->user);
	}
	if (nvgpu_alloc_initialized(&vm->user_lp)) {
		nvgpu_alloc_destroy(&vm->user_lp);
	}
	return err;
}

static void nvgpu_vm_init_check_huge_pages(struct vm_gk20a *vm,
					   u64 user_vma_start, u64 user_vma_limit,
					   bool unified_va)
{
	/*
	 * Determine if huge pages are possible in this VM. If a split address
	 * space is used then check the user_lp vma instead of the user vma.
	 */
	if (!unified_va)
		vm->huge_pages = false;
	vm->huge_pages = nvgpu_huge_pages_possible(vm,
						   user_vma_start,
						   nvgpu_safe_sub_u64(user_vma_limit,
								      user_vma_start));
}

/**
 * @brief Check and set the big page capability for the VM.
 *
 * This function determines if big pages can be used within the given GPU virtual
 * memory (VM) area. It sets the VM's big page flag based on the provided
 * parameters and the result of the big page possibility check.
 *
 * The steps performed by the function are as follows:
 * -# If big pages are not enabled, set the VM's big page flag to false.
 * -# If big pages are enabled and a unified address space is used:
 *    -# Calculate the size of the user VMA by subtracting the user VMA start
 *       address from the user VMA limit address using nvgpu_safe_sub_u64().
 *    -# Call nvgpu_big_pages_possible() with the user VMA start address and
 *       the calculated size to determine if big pages can be used.
 *    -# Set the VM's big page flag to the result of the big page possibility check.
 * -# If big pages are enabled and a split address space is used:
 *    -# Calculate the size of the user large page VMA by subtracting the user
 *       large page VMA start address from the user large page VMA limit address
 *       using nvgpu_safe_sub_u64().
 *    -# Call nvgpu_big_pages_possible() with the user large page VMA start address
 *       and the calculated size to determine if big pages can be used.
 *    -# Set the VM's big page flag to the result of the big page possibility check.
 *
 * @param [in] vm                 Pointer to the GPU VM structure.
 * @param [in] user_vma_start     Start address of the user VMA.
 * @param [in] user_vma_limit     Limit address of the user VMA.
 * @param [in] user_lp_vma_start  Start address of the user large page VMA.
 * @param [in] user_lp_vma_limit  Limit address of the user large page VMA.
 * @param [in] big_pages          Flag indicating if big pages are enabled.
 * @param [in] unified_va         Flag indicating if a unified address space is used.
 *
 * @return None.
 */
static void nvgpu_vm_init_check_big_pages(struct vm_gk20a *vm,
				u64 user_vma_start, u64 user_vma_limit,
				u64 user_lp_vma_start, u64 user_lp_vma_limit,
				bool big_pages, bool unified_va)
{
	/*
	 * Determine if big pages are possible in this VM. If a split address
	 * space is used then check the user_lp vma instead of the user vma.
	 */
	if (!big_pages) {
		vm->big_pages = false;
	} else {
		if (unified_va) {
			vm->big_pages = nvgpu_big_pages_possible(vm,
					user_vma_start,
					nvgpu_safe_sub_u64(user_vma_limit,
							user_vma_start));
		} else {
			vm->big_pages = nvgpu_big_pages_possible(vm,
					user_lp_vma_start,
					nvgpu_safe_sub_u64(user_lp_vma_limit,
							user_lp_vma_start));
		}
	}
}

/**
 * @brief Validate the virtual memory area (VMA) limits for a VM.
 *
 * This function checks the start and limit addresses for user, user large page,
 * and kernel VMAs to ensure they are configured correctly. It validates that the
 * start address is not greater than the limit address for each VMA and that the
 * kernel VMA has a non-zero length when there is no user area.
 *
 * The steps performed by the function are as follows:
 * -# Check if the start address is greater than the limit address for the user VMA,
 *    user large page VMA, or if the kernel VMA start address is greater than or equal
 *    to the kernel VMA limit address. If any of these conditions are true, log an
 *    error, assert, and return an invalid argument error code.
 * -# Check if the user VMA has a zero-sized space (start address is equal to the limit
 *    address) and if the kernel VMA also has a zero-sized space (start address is
 *    greater than or equal to the limit address). If this condition is true, return
 *    an invalid argument error code.
 * -# If all checks pass, return 0 to indicate the VMA limits are valid.
 *
 * @param [in] g                  Pointer to the GPU device structure.
 * @param [in] vm                 Pointer to the GPU VM structure.
 * @param [in] user_vma_start     Start address of the user VMA.
 * @param [in] user_vma_limit     Limit address of the user VMA.
 * @param [in] user_lp_vma_start  Start address of the user large page VMA.
 * @param [in] user_lp_vma_limit  Limit address of the user large page VMA.
 * @param [in] kernel_vma_start   Start address of the kernel VMA.
 * @param [in] kernel_vma_limit   Limit address of the kernel VMA.
 *
 * @return 0 if the VMA limits are valid, or a negative error code if an error occurred.
 */
static int nvgpu_vm_init_check_vma_limits(struct gk20a *g, struct vm_gk20a *vm,
				u64 user_vma_start, u64 user_vma_limit,
				u64 user_lp_vma_start, u64 user_lp_vma_limit,
				u64 kernel_vma_start, u64 kernel_vma_limit)
{
	(void)vm;
	if ((user_vma_start > user_vma_limit) ||
		(user_lp_vma_start > user_lp_vma_limit) ||
		(kernel_vma_start >= kernel_vma_limit)) {
		nvgpu_err(g, "Invalid vm configuration");
		nvgpu_do_assert();
		return -EINVAL;
	}

	/*
	 * A "user" area only makes sense for the GVA spaces. For VMs where
	 * there is no "user" area user_vma_start will be equal to
	 * user_vma_limit (i.e a 0 sized space). In such a situation the kernel
	 * area must be non-zero in length.
	 */
	if ((user_vma_start >= user_vma_limit) &&
		(kernel_vma_start >= kernel_vma_limit)) {
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief Initialize the virtual memory areas (VMAs) for a VM.
 *
 * This function configures and initializes the user, user large page, and kernel
 * VMAs for a given GPU virtual memory (VM) area. It sets up the VMA limits based
 * on the reserved space for user and kernel, and the split between small and big
 * pages. It also checks the validity of the VMA limits and initializes the VMA
 * allocators.
 *
 * The steps performed by the function are as follows:
 * -# Set up VMA limits based on the reserved space for user and kernel VMAs.
 *    If user_reserved is greater than 0:
 *    -# Set kernel VMA flags to indicate #GPU_ALLOC_GVA_SPACE.
 *    -# If big pages are disabled or a unified address space is used, set the
 *       user VMA start and limit, and user large page VMA start and limit accordingly.
 *    -# If big pages are enabled and a split address space is used, validate the
 *       small_big_split and set the user and user large page VMA limits accordingly.
 *    -# If user_reserved is 0, set all user and user large page VMA limits to 0.
 * -# Set the kernel VMA start and limit based on the kernel_reserved space.
 * -# Log the VMA configurations.
 * -# Call nvgpu_vm_init_check_vma_limits() to validate the VMA limits. If an error
 *    occurs, go to the clean-up label.
 * -# Call nvgpu_vm_init_check_big_pages() to check and set the big page capability.
 * -# Call nvgpu_vm_init_vma_allocators() to initialize the VMA allocators. If an error
 *    occurs, go to the clean-up label.
 * -# Return 0 to indicate success.
 * -# In the clean-up label, free the page directory base by calling nvgpu_pd_free()
 *    and return the error code.
 *
 * @param [in] g                Pointer to the GPU device structure.
 * @param [in] vm               Pointer to the GPU VM structure.
 * @param [in] user_reserved    Reserved space for the user VMA.
 * @param [in] kernel_reserved  Reserved space for the kernel VMA.
 * @param [in] small_big_split  Address that splits small and big pages.
 * @param [in] big_pages        Flag indicating if big pages are enabled.
 * @param [in] unified_va       Flag indicating if a unified address space is used.
 * @param [in] name             Name of the VM.
 *
 * @return 0 if successful, or a negative error code if an error occurred.
 */
static int nvgpu_vm_init_vma(struct gk20a *g, struct vm_gk20a *vm,
		     u64 user_reserved,
		     u64 kernel_reserved,
		     u64 small_big_split,
		     bool big_pages,
		     bool unified_va,
		     const char *name)
{
	int err = 0;
	u64 kernel_vma_flags = 0ULL;
	u64 user_vma_start, user_vma_limit;
	u64 user_lp_vma_start, user_lp_vma_limit;
	u64 kernel_vma_start, kernel_vma_limit;

	/* Setup vma limits. */
	if (user_reserved > 0ULL) {
		kernel_vma_flags = GPU_ALLOC_GVA_SPACE;
		/*
		 * If big_pages are disabled for this VM then it only makes
		 * sense to make one VM, same as if the unified address flag
		 * is set.
		 */
		if (!big_pages || unified_va) {
			user_vma_start = vm->virtaddr_start;
			user_vma_limit = nvgpu_safe_sub_u64(vm->va_limit,
							kernel_reserved);
			user_lp_vma_start = user_vma_limit;
			user_lp_vma_limit = user_vma_limit;
		} else {
			/*
			 * Ensure small_big_split falls between user vma
			 * start and end.
			 */
			if ((small_big_split <= vm->virtaddr_start) ||
				(small_big_split >=
					nvgpu_safe_sub_u64(vm->va_limit,
							kernel_reserved))) {
				return -EINVAL;
			}

			user_vma_start = vm->virtaddr_start;
			user_vma_limit = small_big_split;
			user_lp_vma_start = small_big_split;
			user_lp_vma_limit = nvgpu_safe_sub_u64(vm->va_limit,
							kernel_reserved);
		}
	} else {
		user_vma_start = 0;
		user_vma_limit = 0;
		user_lp_vma_start = 0;
		user_lp_vma_limit = 0;
	}
	kernel_vma_start = nvgpu_safe_sub_u64(vm->va_limit, kernel_reserved);
	kernel_vma_limit = vm->va_limit;

	nvgpu_log_info(g, "user_vma     [0x%llx,0x%llx)",
		       user_vma_start, user_vma_limit);
	if (!unified_va) {
		nvgpu_log_info(g, "user_lp_vma  [0x%llx,0x%llx)",
			       user_lp_vma_start, user_lp_vma_limit);
	}
	nvgpu_log_info(g, "kernel_vma   [0x%llx,0x%llx)",
		       kernel_vma_start, kernel_vma_limit);

	err = nvgpu_vm_init_check_vma_limits(g, vm,
					user_vma_start, user_vma_limit,
					user_lp_vma_start, user_lp_vma_limit,
					kernel_vma_start, kernel_vma_limit);
	if (err != 0) {
		goto clean_up_page_tables;
	}

	nvgpu_vm_init_check_big_pages(vm, user_vma_start, user_vma_limit,
				user_lp_vma_start, user_lp_vma_limit,
				big_pages, unified_va);
	if (nvgpu_mm_get_default_huge_page_size(g) != 0) {
		nvgpu_vm_init_check_huge_pages(vm, user_vma_start, user_vma_limit,
					       unified_va);
	}

	err = nvgpu_vm_init_vma_allocators(g, vm,
			user_vma_start, user_vma_limit,
			user_lp_vma_start, user_lp_vma_limit,
			kernel_vma_start, kernel_vma_limit,
			kernel_vma_flags, name);
	if (err != 0) {
		goto clean_up_page_tables;
	}

	return 0;

clean_up_page_tables:
	/* Cleans up nvgpu_gmmu_init_page_table() */
	nvgpu_pd_free(vm, &vm->pdb);
	return err;
}

/**
 * @brief Initialize the attributes of a GPU virtual memory (VM) area.
 *
 * This function sets up the initial attributes for a given VM, including the
 * page sizes, the start of the virtual address space, and the limit of the
 * addressable space. It also checks for potential overlap between user and
 * kernel spaces based on the reserved sizes and the low hole.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the GPU device structure from the MM structure using gk20a_from_mm().
 * -# Get the default aperture size from the GPU operations gops_mm.get_default_va_sizes().
 * -# Calculate the total aperture size by adding the kernel reserved size,
 *    user reserved size, and the low hole size.
 * -# If the calculated aperture size exceeds the default aperture size, log an
 *    error and return an out-of-memory error code.
 * -# Log the initialization information for the VM.
 * -# Set the MM structure pointer in the VM structure to the provided MM structure.
 * -# Initialize the GMMU page sizes for small, big, and kernel pages in the VM
 *    structure.
 * -# Set up the VMA pointers for small, big, and kernel pages in the VM structure.
 *    If a unified address space is not used, set the big page VMA pointer to the
 *    user large page VMA.
 * -# Set the start of the virtual address space and the limit of the addressable
 *    space in the VM structure based on the low hole and calculated aperture size.
 * -# Set the big page size in the VM structure to the provided big page size.
 * -# Set the unified address space flag in the VM structure to the provided value.
 * -# Retrieve the number of MMU levels for the big page size from the GPU operations
 *    gops_mm_gmmu.get_mmu_levels() and set it in the VM structure.
 * -# Return 0 to indicate success.
 *
 * @param [in] mm               Pointer to the MM structure.
 * @param [in] vm               Pointer to the VM structure.
 * @param [in] big_page_size    Size of the big pages.
 * @param [in] low_hole         Size of the low hole in the virtual address space.
 * @param [in] user_reserved    Reserved size for the user VMA.
 * @param [in] kernel_reserved  Reserved size for the kernel VMA.
 * @param [in] big_pages        Flag indicating if big pages are enabled.
 * @param [in] unified_va       Flag indicating if a unified address space is used.
 * @param [in] name             Name of the VM.
 *
 * @return 0 if successful, or a negative error code if an error occurred.
 */
static int nvgpu_vm_init_attributes(struct mm_gk20a *mm,
		     struct vm_gk20a *vm,
		     u32 big_page_size,
		     u64 low_hole,
		     u64 user_reserved,
		     u64 kernel_reserved,
		     bool big_pages,
		     bool unified_va,
		     const char *name)
{
	struct gk20a *g = gk20a_from_mm(mm);
	u64 aperture_size;
	u64 default_aperture_size;

	(void)big_pages;

	g->ops.mm.get_default_va_sizes(&default_aperture_size, NULL, NULL);

	aperture_size = nvgpu_safe_add_u64(kernel_reserved,
		nvgpu_safe_add_u64(user_reserved, low_hole));

	if (aperture_size > default_aperture_size) {
		nvgpu_err(g, "Overlap between user and kernel spaces");
		return -ENOMEM;
	}

	nvgpu_log_info(g, "Init space for %s: valimit=0x%llx, "
		       "LP size=0x%x lowhole=0x%llx",
		       name, aperture_size,
		       (unsigned int)big_page_size, low_hole);

	vm->mm = mm;

	vm->gmmu_page_sizes[GMMU_PAGE_SIZE_SMALL]  =
					nvgpu_safe_cast_u64_to_u32(SZ_4K);
	vm->gmmu_page_sizes[GMMU_PAGE_SIZE_BIG]    = big_page_size;
	vm->gmmu_page_sizes[GMMU_PAGE_SIZE_HUGE]   =
					nvgpu_safe_cast_u64_to_u32(SZ_2M);
	vm->gmmu_page_sizes[GMMU_PAGE_SIZE_KERNEL] =
			nvgpu_safe_cast_u64_to_u32(NVGPU_CPU_PAGE_SIZE);

	/* Set up vma pointers. */
	vm->vma[GMMU_PAGE_SIZE_SMALL]  = &vm->user;
	vm->vma[GMMU_PAGE_SIZE_BIG]    = &vm->user;
	vm->vma[GMMU_PAGE_SIZE_HUGE]   = &vm->user;
	vm->vma[GMMU_PAGE_SIZE_KERNEL] = &vm->kernel;
	if (!unified_va) {
		vm->vma[GMMU_PAGE_SIZE_BIG] = &vm->user_lp;
	}

	vm->virtaddr_start = low_hole;
	vm->va_limit = aperture_size;

	vm->big_page_size     = vm->gmmu_page_sizes[GMMU_PAGE_SIZE_BIG];
	vm->huge_page_size    = vm->gmmu_page_sizes[GMMU_PAGE_SIZE_HUGE];
	vm->unified_va        = unified_va;
	vm->mmu_levels        =
		g->ops.mm.gmmu.get_mmu_levels(g, vm->big_page_size);

	return 0;
}

/*
 * Initialize a preallocated vm.
 */
int nvgpu_vm_do_init(struct mm_gk20a *mm,
		     struct vm_gk20a *vm,
		     u32 big_page_size,
		     u64 low_hole,
		     u64 user_reserved,
		     u64 kernel_reserved,
		     u64 small_big_split,
		     bool big_pages,
		     bool unified_va,
		     const char *name)
{
	struct gk20a *g = gk20a_from_mm(mm);
	int err = 0;

	err = nvgpu_vm_init_attributes(mm, vm, big_page_size, low_hole,
		user_reserved, kernel_reserved, big_pages, unified_va, name);
	if (err != 0) {
		return err;
	}

	if (g->ops.mm.vm_as_alloc_share != NULL) {
		err = g->ops.mm.vm_as_alloc_share(g, vm);
		if (err != 0) {
			nvgpu_err(g, "Failed to init gpu vm!");
			return err;
		}
	}

	/* Initialize the page table data structures. */
	(void) strncpy(vm->name, name, (size_t)(sizeof(vm->name)-1ULL));
	if (!nvgpu_is_legacy_vgpu(g)) {
		err = nvgpu_gmmu_init_page_table(vm);
		if (err != 0) {
			goto clean_up_gpu_vm;
		}
	}
	err = nvgpu_vm_init_vma(g, vm, user_reserved, kernel_reserved,
				small_big_split, big_pages, unified_va, name);
	if (err != 0) {
		goto clean_up_gpu_vm;
	}

	vm->mapped_buffers = NULL;

	nvgpu_mutex_init(&vm->syncpt_ro_map_lock);
	nvgpu_mutex_init(&vm->gpu_mmio_va_map_lock);
	nvgpu_mutex_init(&vm->update_gmmu_lock);

	nvgpu_ref_init(&vm->ref);
	nvgpu_init_list_node(&vm->vm_area_list);

#ifdef CONFIG_NVGPU_SW_SEMAPHORE
	/*
	 * This is only necessary for channel address spaces. The best way to
	 * distinguish channel address spaces from other address spaces is by
	 * size - if the address space is 4GB or less, it's not a channel.
	 */
	if (vm->va_limit > 4ULL * SZ_1G) {
		err = nvgpu_init_sema_pool(vm);
		if (err != 0) {
			goto clean_up_gmmu_lock;
		}
	}
#endif

	return 0;

#ifdef CONFIG_NVGPU_SW_SEMAPHORE
clean_up_gmmu_lock:
	nvgpu_mutex_destroy(&vm->update_gmmu_lock);
	nvgpu_mutex_destroy(&vm->syncpt_ro_map_lock);
	nvgpu_mutex_destroy(&vm->gpu_mmio_va_map_lock);
#endif
clean_up_gpu_vm:
	if (g->ops.mm.vm_as_free_share != NULL) {
		g->ops.mm.vm_as_free_share(vm);
	}
	return err;
}

struct vm_gk20a *nvgpu_vm_init(struct gk20a *g,
			       u32 big_page_size,
			       u64 low_hole,
			       u64 user_reserved,
			       u64 kernel_reserved,
			       u64 small_big_split,
			       bool big_pages,
			       bool unified_va,
			       const char *name)
{
	struct vm_gk20a *vm = nvgpu_kzalloc(g, sizeof(*vm));
	int err;

	if (vm == NULL) {
		return NULL;
	}

	err = nvgpu_vm_do_init(&g->mm, vm, big_page_size, low_hole,
			     user_reserved, kernel_reserved, small_big_split,
			     big_pages, unified_va, name);
	if (err != 0) {
		nvgpu_kfree(g, vm);
		return NULL;
	}

	return vm;
}

/**
 * @brief Remove a virtual memory context and free associated resources.
 *
 * The steps performed by the function are as follows:
 * -# If the GPU does not support syncpoints, unmap and release the semaphore
 *    pool associated with the virtual memory context using nvgpu_semaphore_pool_unmap()
 *    and nvgpu_semaphore_pool_put().
 * -# If the syncpt_mem is valid and the syncpt_ro_map_gpu_va is non-zero, unmap the
 *    sync point read-only mapping from the GPU using nvgpu_gmmu_unmap_addr().
 * -# Acquire the update_gmmu_lock to protect the virtual memory context during
 *    modifications.
 * -# Enumerate and unmap all buffers mapped in the virtual memory context using
 *    nvgpu_rbtree_enum_start() and nvgpu_vm_do_unmap().
 * -# Destroy all remaining reserved memory areas by iterating over the vm_area_list
 *    and freeing each nvgpu_vm_area structure.
 * -# Destroy the kernel, user, and user_lp allocators if they have been initialized
 *    using nvgpu_alloc_destroy().
 * -# If the GPU is not a legacy vGPU, free the page directory entries associated
 *    with the virtual memory context using nvgpu_vm_free_entries().
 * -# If the GPU has a gops_mm.vm_as_free_share() operation, call it to free the address space
 *    share associated with the virtual memory context.
 * -# Release the update_gmmu_lock and destroy it.
 * -# Destroy the syncpt_ro_map_lock.
 * -# Free the virtual memory context structure.
 *
 * @param [in] vm  Pointer to the virtual memory context to be removed.
 *
 * @return None.
 */
static void nvgpu_vm_remove(struct vm_gk20a *vm)
{
	struct nvgpu_mapped_buf *mapped_buffer;
	struct nvgpu_vm_area *vm_area;
	struct nvgpu_rbtree_node *node = NULL;
	struct gk20a *g = vm->mm->g;
	bool done;
	u32 i = 0U;
	int err = 0;

#ifdef CONFIG_NVGPU_SW_SEMAPHORE
	/*
	 * Do this outside of the update_gmmu_lock since unmapping the semaphore
	 * pool involves unmapping a GMMU mapping which means aquiring the
	 * update_gmmu_lock.
	 */
	if (!nvgpu_has_syncpoints(g) ||
		nvgpu_is_enabled(g, NVGPU_SUPPORT_SEMA_BASED_GPFIFO_GET)) {
		if (vm->sema_pool != NULL) {
			nvgpu_semaphore_pool_unmap(vm->sema_pool, vm);
			nvgpu_semaphore_pool_put(vm->sema_pool);
		}
	}
#endif
	for (i = 0U; i < g->num_nvhost; i++) {
		if (nvgpu_mem_is_valid(&g->syncpt_mem[i]) &&
				(vm->syncpt_ro_map_gpu_va[i] != 0ULL)) {
			nvgpu_gmmu_unmap_addr(vm, &g->syncpt_mem[i],
				vm->syncpt_ro_map_gpu_va[i]);
		}
	}
	nvgpu_mutex_acquire(&vm->gpu_mmio_va_map_lock);
	if (vm->gpummio_va != 0U) {
		nvgpu_gmmu_unmap_va(vm, vm->gpummio_va,
				vm->gpummio_va_mapsize);
		nvgpu_dma_free(g, &vm->gpummio_mem);
		vm->gpummio_va = 0U;
		vm->gpummio_va_mapsize = 0U;
	}
	nvgpu_mutex_release(&vm->gpu_mmio_va_map_lock);

	nvgpu_mutex_acquire(&vm->update_gmmu_lock);

	nvgpu_rbtree_enum_start(0, &node, vm->mapped_buffers);
	while (node != NULL) {
		mapped_buffer = mapped_buffer_from_rbtree_node(node);
		err = nvgpu_vm_do_unmap(mapped_buffer, NULL);
		if (err != 0) {
			nvgpu_err(g, "Unmap a buffer from the GPU virtual \
					memory space is failed [%d]", err);
		}
		nvgpu_rbtree_enum_start(0, &node, vm->mapped_buffers);
	}

	/* destroy remaining reserved memory areas */
	done = false;
	do {
		if (nvgpu_list_empty(&vm->vm_area_list)) {
			done = true;
		} else {
			vm_area = nvgpu_list_first_entry(&vm->vm_area_list,
							 nvgpu_vm_area,
							 vm_area_list);
			nvgpu_list_del(&vm_area->vm_area_list);
			nvgpu_kfree(vm->mm->g, vm_area);
		}
	} while (!done);

	if (nvgpu_alloc_initialized(&vm->kernel)) {
		nvgpu_alloc_destroy(&vm->kernel);
	}
	if (nvgpu_alloc_initialized(&vm->user)) {
		nvgpu_alloc_destroy(&vm->user);
	}
	if (nvgpu_alloc_initialized(&vm->user_lp)) {
		nvgpu_alloc_destroy(&vm->user_lp);
	}

	if (!nvgpu_is_legacy_vgpu(g)) {
		nvgpu_vm_free_entries(vm, &vm->pdb);
	}

	if (g->ops.mm.vm_as_free_share != NULL) {
		g->ops.mm.vm_as_free_share(vm);
	}

	nvgpu_mutex_release(&vm->update_gmmu_lock);
	nvgpu_mutex_destroy(&vm->update_gmmu_lock);

	nvgpu_mutex_destroy(&vm->syncpt_ro_map_lock);
	nvgpu_mutex_destroy(&vm->gpu_mmio_va_map_lock);
	nvgpu_kfree(g, vm);
}

/**
 * @brief Retrieve the vm_gk20a structure from a reference to it.
 *
 * This function computes the starting address of the vm_gk20a structure given a
 * reference to its 'ref' member. It does this by subtracting the offset of the
 * 'ref' member within the vm_gk20a structure from the address of the 'ref' member.
 *
 * The steps performed by the function are as follows:
 * -# Calculate the offset of the 'ref' member within the vm_gk20a structure using
 *    the #offsetof macro.
 * -# Subtract the offset from the address of the 'ref' member to obtain the
 *    starting address of the vm_gk20a structure.
 * -# Cast the computed address to a pointer to vm_gk20a and return it.
 *
 * @param [in] ref  Pointer to the reference member within the vm_gk20a structure.
 *
 * @return Pointer to the vm_gk20a structure containing 'ref'.
 */
static struct vm_gk20a *vm_gk20a_from_ref(struct nvgpu_ref *ref)
{
	return (struct vm_gk20a *)
		((uintptr_t)ref - offsetof(struct vm_gk20a, ref));
}

/**
 * @brief Remove a reference to a virtual memory context and free associated resources.
 *
 * This function is a wrapper that takes a reference to a virtual memory context's
 * 'ref' member, retrieves the containing vm_gk20a structure, and then removes the
 * virtual memory context and frees its associated resources.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the vm_gk20a structure from the provided reference using
 *    vm_gk20a_from_ref().
 * -# Remove the virtual memory context and free its associated resources using
 *    nvgpu_vm_remove().
 *
 * @param [in] ref  Pointer to the reference member within the vm_gk20a structure.
 *
 * @return None.
 */
static void nvgpu_vm_remove_ref(struct nvgpu_ref *ref)
{
	struct vm_gk20a *vm = vm_gk20a_from_ref(ref);

	nvgpu_vm_remove(vm);
}

void nvgpu_vm_get(struct vm_gk20a *vm)
{
	nvgpu_ref_get(&vm->ref);
}

void nvgpu_vm_put(struct vm_gk20a *vm)
{
	nvgpu_ref_put(&vm->ref, nvgpu_vm_remove_ref);
}

void nvgpu_insert_mapped_buf(struct vm_gk20a *vm,
			    struct nvgpu_mapped_buf *mapped_buffer)
{
	mapped_buffer->node.key_start = mapped_buffer->addr;
	mapped_buffer->node.key_end = nvgpu_safe_add_u64(mapped_buffer->addr,
						mapped_buffer->size);

	nvgpu_rbtree_insert(&mapped_buffer->node, &vm->mapped_buffers);
	nvgpu_assert(vm->num_user_mapped_buffers < U32_MAX);
	vm->num_user_mapped_buffers++;
}

/**
 * @brief Unlink a mapped buffer from the virtual memory context's RB tree.
 *
 * This function removes a mapped buffer from the red-black tree that tracks all
 * buffers mapped into a virtual memory context. It also decrements the count of
 * user-mapped buffers in the virtual memory context.
 *
 * The steps performed by the function are as follows:
 * -# Unlink the mapped buffer from the virtual memory context's red-black tree
 *    using nvgpu_rbtree_unlink().
 * -# Assert that the number of user-mapped buffers is greater than zero before
 *    decrementing to ensure the count does not underflow.
 * -# Decrement the count of user-mapped buffers in the virtual memory context.
 *
 * @param [in] vm             Pointer to the virtual memory context.
 * @param [in] mapped_buffer  Pointer to the mapped buffer to be removed.
 *
 * @return None.
 */
static void nvgpu_remove_mapped_buf(struct vm_gk20a *vm,
				    struct nvgpu_mapped_buf *mapped_buffer)
{
	nvgpu_rbtree_unlink(&mapped_buffer->node, &vm->mapped_buffers);
	nvgpu_assert(vm->num_user_mapped_buffers > 0U);
	vm->num_user_mapped_buffers--;
}

struct nvgpu_mapped_buf *nvgpu_vm_find_mapped_buf(
	struct vm_gk20a *vm, u64 addr)
{
	struct nvgpu_rbtree_node *node = NULL;
	struct nvgpu_rbtree_node *root = vm->mapped_buffers;

	nvgpu_rbtree_search(addr, &node, root);
	if (node == NULL) {
		return NULL;
	}

	return mapped_buffer_from_rbtree_node(node);
}

struct nvgpu_mapped_buf *nvgpu_vm_find_mapped_buf_range(
	struct vm_gk20a *vm, u64 addr)
{
	struct nvgpu_rbtree_node *node = NULL;
	struct nvgpu_rbtree_node *root = vm->mapped_buffers;

	nvgpu_rbtree_range_search(addr, &node, root);
	if (node == NULL) {
		return NULL;
	}

	return mapped_buffer_from_rbtree_node(node);
}

struct nvgpu_mapped_buf *nvgpu_vm_find_mapped_buf_less_than(
	struct vm_gk20a *vm, u64 addr)
{
	struct nvgpu_rbtree_node *node = NULL;
	struct nvgpu_rbtree_node *root = vm->mapped_buffers;

	nvgpu_rbtree_less_than_search(addr, &node, root);
	if (node == NULL) {
		return NULL;
	}

	return mapped_buffer_from_rbtree_node(node);
}

int nvgpu_vm_get_buffers(struct vm_gk20a *vm,
			 struct nvgpu_mapped_buf ***mapped_buffers,
			 u32 *num_buffers)
{
	struct nvgpu_mapped_buf *mapped_buffer;
	struct nvgpu_mapped_buf **buffer_list;
	struct nvgpu_rbtree_node *node = NULL;
	u32 i = 0;

	nvgpu_mutex_acquire(&vm->update_gmmu_lock);

	if (vm->num_user_mapped_buffers == 0U) {
		nvgpu_mutex_release(&vm->update_gmmu_lock);
		return 0;
	}

	buffer_list = nvgpu_big_zalloc(vm->mm->g,
				nvgpu_safe_mult_u64(sizeof(*buffer_list),
						vm->num_user_mapped_buffers));
	if (buffer_list == NULL) {
		nvgpu_mutex_release(&vm->update_gmmu_lock);
		return -ENOMEM;
	}

	nvgpu_rbtree_enum_start(0, &node, vm->mapped_buffers);
	while (node != NULL) {
		mapped_buffer = mapped_buffer_from_rbtree_node(node);
		buffer_list[i] = mapped_buffer;
		nvgpu_ref_get(&mapped_buffer->ref);
		nvgpu_assert(i < U32_MAX);
		i++;
		nvgpu_rbtree_enum_next(&node, node);
	}

	if (i != vm->num_user_mapped_buffers) {
		BUG();
	}

	*num_buffers = vm->num_user_mapped_buffers;
	*mapped_buffers = buffer_list;

	nvgpu_mutex_release(&vm->update_gmmu_lock);

	return 0;
}

void nvgpu_vm_put_buffers(struct vm_gk20a *vm,
				 struct nvgpu_mapped_buf **mapped_buffers,
				 u32 num_buffers)
{
	u32 i;
	int err = 0;
	struct vm_gk20a_mapping_batch batch;

	if (num_buffers == 0U) {
		return;
	}

	nvgpu_mutex_acquire(&vm->update_gmmu_lock);
	nvgpu_vm_mapping_batch_start(&batch);
	vm->kref_put_batch = &batch;

	for (i = 0U; i < num_buffers; ++i) {
		err = nvgpu_ref_put_int_ptr(&mapped_buffers[i]->ref,
			      nvgpu_vm_unmap_ref_internal);
		if (err != 0) {
			nvgpu_err(vm->mm->g, "Unmap a buffer from the GPU \
				virtual memory space is failed [%d]", err);
		}
	}

	vm->kref_put_batch = NULL;
	nvgpu_vm_mapping_batch_finish_locked(vm, &batch);
	nvgpu_mutex_release(&vm->update_gmmu_lock);

	nvgpu_big_free(vm->mm->g, mapped_buffers);
}

/**
 * @brief Map a buffer into the GPU virtual memory space.
 *
 * This function handles the mapping of a buffer into the GPU virtual memory space,
 * taking into account compression settings and alignment requirements. It computes
 * the correct parameters for the GMMU PTEs and performs the actual mapping.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the gk20a structure from the virtual memory context using gk20a_from_vm().
 * -# Initialize local variables for error tracking, compression tag (ctag) state,
 *    and the kind of memory to be used in the GMMU PTEs.
 * -# If compression is enabled, compute the compression settings using
 *    nvgpu_vm_compute_compression(). If an error occurs, log it and exit.
 * -# If a fixed-address compressible mapping is requested, ensure that the virtual
 *    address and buffer offset are aligned according to the GPU's requirements.
 *    If not aligned, log the error and exit.
 * -# If compression is applicable, retrieve and possibly allocate compression tags
 *    for the buffer. If the tags need to be cleared, mark them for clearing.
 * -# Compute the correct kind and ctag offset for the GMMU page tables based on
 *    whether the buffer is compressible or not.
 * -# If compression tags need to be cleared, start the clearing process.
 * -# Perform the actual mapping using the GPU operations gops_mm_gmmu.map() with the
 *    computed parameters.
 * -# If compression tags were cleared, finish the clearing process.
 * -# If the mapping fails, log the error and exit.
 * -# Update the provided map address pointer with the result of the mapping.
 *
 * @param [in]  vm             Pointer to the virtual memory context.
 * @param [in]  os_buf         Pointer to the buffer to be mapped.
 * @param [in]  sgt            Scatter-gather table describing the physical memory.
 * @param [out] map_addr_ptr   Pointer to the address where the mapping should be placed.
 * @param [in]  map_size       Size of the mapping.
 * @param [in]  phys_offset    Offset into the physical memory to start the mapping.
 * @param [in]  rw             Read/write flag for the memory region.
 * @param [in]  flags          Flags for the mapping.
 * @param [in]  batch          Pointer to a batch structure for batched mapping operations.
 * @param [in]  aperture       Aperture where the buffer should be mapped.
 * @param [in]  binfo_ptr      Pointer to buffer info containing compression info.
 *
 * @return 0 on success, or a negative error code on failure.
 */
static int nvgpu_vm_do_map(struct vm_gk20a *vm,
		 struct nvgpu_os_buffer *os_buf,
		 struct nvgpu_sgt *sgt,
		 u64 *map_addr_ptr,
		 u64 map_size,
		 u64 phys_offset,
		 enum gk20a_mem_rw_flag rw,
		 u32 flags,
		 struct vm_gk20a_mapping_batch *batch,
		 enum nvgpu_aperture aperture,
		 struct nvgpu_ctag_buffer_info *binfo_ptr)
{
	struct gk20a *g = gk20a_from_vm(vm);
	int err = 0;
	bool clear_ctags = false;
	u32 ctag_offset = 0;
	u64 map_addr = *map_addr_ptr;
	struct gk20a_comptags comptags = { };
	/*
	 * The actual GMMU PTE kind
	 */
	bool pte_kind_selected = false;
	u8 pte_kind;

	(void)os_buf;
	(void)flags;
#ifdef CONFIG_NVGPU_COMPRESSION
	err = nvgpu_vm_compute_compression(vm, binfo_ptr);
	if (err != 0) {
		nvgpu_err(g, "failure setting up compression");
		goto ret_err;
	}

	if ((binfo_ptr->compr_kind != NVGPU_KIND_INVALID) &&
	    ((flags & NVGPU_VM_MAP_FIXED_OFFSET) != 0U)) {
		/*
		 * Fixed-address compressible mapping is
		 * requested. Make sure we're respecting the alignment
		 * requirement for virtual addresses and buffer
		 * offsets.
		 *
		 * This check must be done before we may fall back to
		 * the incompressible kind.
		 */

		const u64 offset_mask = g->ops.fb.compression_align_mask(g);

		if ((map_addr & offset_mask) != (phys_offset & offset_mask)) {
			nvgpu_log(g, gpu_dbg_map,
				  "Misaligned compressible-kind fixed-address "
				  "mapping");
			err = -EINVAL;
			goto ret_err;
		}
	}

	if (binfo_ptr->compr_kind != NVGPU_KIND_INVALID) {
		/*
		 * Get the comptags state
		 */
		gk20a_get_comptags(os_buf, &comptags);

		if (!comptags.allocated) {
			nvgpu_log_info(g, "compr kind %d map requested without comptags allocated, allocating...",
				       binfo_ptr->compr_kind);

			/*
			 * best effort only, we don't really care if
			 * this fails
			 */
			gk20a_alloc_or_get_comptags(
				g, os_buf, &g->cbc->comp_tags, &comptags);
		}

		/*
		 * Newly allocated comptags needs to be cleared
		 */
		if (comptags.needs_clear) {
			if (g->ops.cbc.ctrl != NULL) {
				if (gk20a_comptags_start_clear(os_buf)) {
					err = g->ops.cbc.ctrl(
						g, nvgpu_cbc_op_clear,
						comptags.offset,
						(comptags.offset +
						 comptags.lines - 1U));
					gk20a_comptags_finish_clear(
						os_buf, err == 0);
					if (err != 0) {
						goto ret_err;
					}
				}
			} else {
				/*
				 * Cleared as part of gmmu map
				 */
				clear_ctags = true;
			}
		}

		/*
		 * Store the ctag offset for later use if we have the comptags
		 */
		if (comptags.enabled) {
			ctag_offset = comptags.offset;

			if (!g->cbc_use_raw_mode)
			{
				u64 compression_page_size = g->ops.fb.compression_page_size(g);
				nvgpu_assert(compression_page_size > 0ULL);
				/*
				 * Adjust the ctag_offset as per the buffer map offset
				 */
				ctag_offset = nvgpu_safe_add_u32(ctag_offset,
						 nvgpu_safe_cast_u64_to_u32(phys_offset >>
								     nvgpu_ilog2(compression_page_size)));
				nvgpu_assert((binfo_ptr->compr_kind >= 0) &&
						 (binfo_ptr->compr_kind <= (s16)U8_MAX));
				binfo_ptr->ctag_offset = ctag_offset;
			} else {
				binfo_ptr->ctag_offset = 0;
			}
			pte_kind = (u8)binfo_ptr->compr_kind;
			pte_kind_selected = true;
			binfo_ptr->compr_kind_selected = true;
		}
	}
#endif

	if (!pte_kind_selected) {
		if ((binfo_ptr->incompr_kind >= 0) &&
				(binfo_ptr->incompr_kind <= (s16)U8_MAX)) {
			/*
			 * Incompressible kind, ctag offset will not be programmed
			 */
			ctag_offset = 0;
			pte_kind = (u8)binfo_ptr->incompr_kind;
			pte_kind_selected = true;
		} else {
			/*
			 * Caller required compression, but we cannot provide it
			 */
			nvgpu_err(g, "No comptags and no incompressible fallback kind");
			err = -ENOMEM;
			goto ret_err;
		}
	}

#ifdef CONFIG_NVGPU_COMPRESSION
	if (clear_ctags) {
		clear_ctags = gk20a_comptags_start_clear(os_buf);
	}
#endif

	map_addr = g->ops.mm.gmmu.map(vm,
				      map_addr,
				      sgt,
				      phys_offset,
				      map_size,
				      binfo_ptr->pgsz_idx,
				      pte_kind,
				      ctag_offset,
				      binfo_ptr->flags,
				      rw,
				      clear_ctags,
				      false,
				      false,
				      batch,
				      aperture);

#ifdef CONFIG_NVGPU_COMPRESSION
	if (clear_ctags) {
		gk20a_comptags_finish_clear(os_buf, map_addr != 0U);
	}
#endif

	if (map_addr == 0ULL) {
		err = -ENOMEM;
		goto ret_err;
	}

	*map_addr_ptr = map_addr;

ret_err:
	return err;
}

/**
 * @brief Create a new mapping or retrieve an existing one for a buffer in a VM context.
 *
 * This function checks if a buffer is already mapped in the given virtual memory
 * context. If it is, it increases the reference count and returns the existing
 * mapping. If not, it allocates a new nvgpu_mapped_buf structure and initializes
 * it for a new mapping.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the gk20a structure from the virtual memory context using gk20a_from_vm().
 * -# Acquire the update_gmmu_lock to protect the virtual memory context during
 *    the mapping check.
 * -# Check if the buffer is already mapped in the virtual memory context using
 *    nvgpu_vm_find_mapping(). If it is, increment the reference count, release
 *    the lock, set the output mapped_buffer_arg to the found mapping, and return 1.
 * -# Release the update_gmmu_lock if no existing mapping is found.
 * -# Allocate memory for a new nvgpu_mapped_buf structure. If allocation fails,
 *    log a warning and return -ENOMEM.
 * -# Set the output mapped_buffer_arg to the newly allocated mapped_buffer.
 * -# Set the default page size index to small.
 * -# If not in a safety build, determine the alignment of the scatter-gather table
 *    and set the page size index accordingly.
 * -# Align the map size to the nearest 4K boundary and check if it exceeds the
 *    buffer size or if the physical offset is out of bounds. If so, return -EINVAL.
 * -# Update the map_size_ptr with the aligned map size.
 * -# Return 0 to indicate successful preparation for a new mapping.
 *
 * @param [in]  vm                  Pointer to the virtual memory context.
 * @param [in]  os_buf              Pointer to the buffer to be mapped.
 * @param [out] mapped_buffer       Pointer to the mapped buffer structure.
 * @param [in]  sgt                 Scatter-gather table describing the physical memory.
 * @param [in]  binfo_ptr           Pointer to buffer info containing compression info.
 * @param [in]  map_addr            GPU virtual address where the mapping should be placed.
 * @param [out] map_size_ptr        Pointer to the size of the mapping.
 * @param [in]  phys_offset         Offset into the physical memory to start the mapping.
 * @param [in]  map_key_kind        Kind parameter for the mapping.
 * @param [out] mapped_buffer_arg   Pointer to store the resulting mapped buffer.
 *
 * @return 1 if an existing mapping is found and reused, 0 if a new mapping is prepared,
 *         or a negative error code on failure.
 */
static int nvgpu_vm_new_mapping(struct vm_gk20a *vm,
			struct nvgpu_os_buffer *os_buf,
			struct nvgpu_mapped_buf *mapped_buffer,
			struct nvgpu_sgt *sgt,
			struct nvgpu_ctag_buffer_info *binfo_ptr,
			u64 map_addr, u64 *map_size_ptr,
			u64 phys_offset, s16 map_key_kind,
			struct nvgpu_mapped_buf **mapped_buffer_arg)
{
	struct gk20a *g = gk20a_from_vm(vm);
	u64 align;
	u64 map_size = *map_size_ptr;

	/*
	 * Check if this buffer is already mapped.
	 */
	nvgpu_mutex_acquire(&vm->update_gmmu_lock);
	mapped_buffer = nvgpu_vm_find_mapping(vm,
					      os_buf,
					      map_addr,
					      binfo_ptr->flags,
					      map_key_kind);

	if (mapped_buffer != NULL) {
		nvgpu_ref_get(&mapped_buffer->ref);
		nvgpu_mutex_release(&vm->update_gmmu_lock);
		*mapped_buffer_arg = mapped_buffer;
		return 1;
	}
	nvgpu_mutex_release(&vm->update_gmmu_lock);

	/*
	 * Generate a new mapping!
	 */
	mapped_buffer = nvgpu_kzalloc(g, sizeof(*mapped_buffer));
	if (mapped_buffer == NULL) {
		nvgpu_warn(g, "oom allocating tracking buffer");
		return -ENOMEM;
	}
	*mapped_buffer_arg = mapped_buffer;

	align = nvgpu_sgt_alignment(g, sgt);
	if (g->mm.disable_bigpage) {
		binfo_ptr->pgsz_idx = GMMU_PAGE_SIZE_SMALL;
	} else {
		binfo_ptr->pgsz_idx = nvgpu_vm_get_pte_size(vm, map_addr,
					min_t(u64, binfo_ptr->size, align));
	}
	map_size = (map_size != 0ULL) ? map_size : binfo_ptr->size;
	map_size = NVGPU_ALIGN(map_size, SZ_4K);

	if ((map_size > binfo_ptr->size) ||
	    (phys_offset > (binfo_ptr->size - map_size))) {
		return -EINVAL;
	}

	*map_size_ptr = map_size;
	return 0;
}

/**
 * @brief Check and set buffer attributes for mapping into a VM context.
 *
 * This function sets up the buffer information structure with the size and
 * compression attributes of the buffer to be mapped. It also determines the
 * mapping key kind based on whether compression is enabled and valid.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the gk20a structure from the virtual memory context using gk20a_from_vm().
 * -# Set the flags in the buffer info structure to the provided flags.
 * -# Retrieve the size of the buffer using nvgpu_os_buf_get_size() and store it in
 *    the buffer info structure. If the size is zero, log an error and return -EINVAL.
 * -# Set the incompressible kind in the buffer info structure to the provided
 *    incompressible kind.
 * -# If compression is configured and enabled in the VM context, and the provided
 *    compressible kind is valid, set the compressible kind in the buffer info
 *    structure to the provided compressible kind. Otherwise, set it to invalid.
 * -# Determine the mapping key kind based on whether the compressible kind is valid.
 *    If it is, use the compressible kind; otherwise, use the incompressible kind.
 * -# Set the mapping key kind pointer to the determined mapping key kind.
 * -# Return 0 to indicate success.
 *
 * @param [in]  vm               Pointer to the virtual memory context.
 * @param [in]  os_buf           Pointer to the buffer to be mapped.
 * @param [out] binfo_ptr        Pointer to the buffer info structure to be populated.
 * @param [in]  flags            Flags for the mapping.
 * @param [in]  compr_kind       Compressible kind for the buffer.
 * @param [in]  incompr_kind     Incompressible kind for the buffer.
 * @param [out] map_key_kind_ptr Pointer to store the determined mapping key kind.
 *
 * @return 0 on success, or a negative error code on failure.
 */
static int nvgpu_vm_map_check_attributes(struct vm_gk20a *vm,
			struct nvgpu_os_buffer *os_buf,
			struct nvgpu_ctag_buffer_info *binfo_ptr,
			u32 flags,
			s16 compr_kind,
			s16 incompr_kind,
			s16 *map_key_kind_ptr)
{
	struct gk20a *g = gk20a_from_vm(vm);

	(void)compr_kind;

	binfo_ptr->flags = flags;
	binfo_ptr->size = nvgpu_os_buf_get_size(os_buf);
	if (binfo_ptr->size == 0UL) {
		nvgpu_err(g, "Invalid buffer size");
		return -EINVAL;
	}
	binfo_ptr->incompr_kind = incompr_kind;

#ifdef CONFIG_NVGPU_COMPRESSION
	if (vm->enable_ctag && compr_kind != NVGPU_KIND_INVALID) {
		binfo_ptr->compr_kind = compr_kind;
	} else {
		binfo_ptr->compr_kind = NVGPU_KIND_INVALID;
	}

	if (compr_kind != NVGPU_KIND_INVALID) {
		*map_key_kind_ptr = compr_kind;
	} else {
		*map_key_kind_ptr = incompr_kind;
	}
#else
	*map_key_kind_ptr = incompr_kind;
#endif
	return 0;
}

int nvgpu_vm_map(struct vm_gk20a *vm,
		 struct nvgpu_os_buffer *os_buf,
		 struct nvgpu_sgt *sgt,
		 u64 map_addr,
		 u64 map_size,
		 u64 phys_offset,
		 enum gk20a_mem_rw_flag buffer_rw_mode,
		 u32 map_access_requested,
		 u32 flags,
		 s16 compr_kind,
		 s16 incompr_kind,
		 struct vm_gk20a_mapping_batch *batch,
		 enum nvgpu_aperture aperture,
		 struct nvgpu_mapped_buf **mapped_buffer_arg)
{
	struct gk20a *g = gk20a_from_vm(vm);
	struct nvgpu_mapped_buf *mapped_buffer = NULL;
	struct nvgpu_ctag_buffer_info binfo = { };
	enum gk20a_mem_rw_flag rw = buffer_rw_mode;
	struct nvgpu_vm_area *vm_area = NULL;
	int err = 0;
	bool va_allocated = true;

	/*
	 * The kind used as part of the key for map caching. HW may
	 * actually be programmed with the fallback kind in case the
	 * key kind is compressible but we're out of comptags.
	 */
	s16 map_key_kind;

	if ((map_access_requested == NVGPU_VM_MAP_ACCESS_READ_WRITE) &&
	    (buffer_rw_mode == gk20a_mem_flag_read_only)) {
		nvgpu_err(g, "RW mapping requested for RO buffer");
		return -EINVAL;
	}

	if (map_access_requested == NVGPU_VM_MAP_ACCESS_READ_ONLY) {
		rw = gk20a_mem_flag_read_only;
	}

	*mapped_buffer_arg = NULL;

	err = nvgpu_vm_map_check_attributes(vm, os_buf, &binfo, flags,
			compr_kind, incompr_kind, &map_key_kind);
	if (err != 0) {
		return err;
	}

	err = nvgpu_vm_new_mapping(vm, os_buf, mapped_buffer, sgt, &binfo,
			map_addr, &map_size, phys_offset, map_key_kind,
			mapped_buffer_arg);

	mapped_buffer = *mapped_buffer_arg;
	if (err < 0) {
		goto clean_up_nolock;
	}
	if (err == 1) {
		return 0;
	}

	nvgpu_mutex_acquire(&vm->update_gmmu_lock);

	/*
	 * Check if we should use a fixed offset for mapping this buffer.
	 */
	if ((flags & NVGPU_VM_MAP_FIXED_OFFSET) != 0U)  {
		err = nvgpu_vm_area_validate_buffer(vm,
						    map_addr,
						    map_size,
						    binfo.pgsz_idx,
						    &vm_area);
		if (err != 0) {
			goto clean_up;
		}

		va_allocated = false;
	}

	err = nvgpu_vm_do_map(vm, os_buf, sgt, &map_addr,
				map_size, phys_offset, rw, flags, batch,
				aperture, &binfo);
	if (err != 0) {
		goto clean_up;
	}

	nvgpu_init_list_node(&mapped_buffer->buffer_list);
	nvgpu_ref_init(&mapped_buffer->ref);
	mapped_buffer->addr         = map_addr;
	mapped_buffer->size         = map_size;
	mapped_buffer->pgsz_idx     = binfo.pgsz_idx;
	mapped_buffer->vm           = vm;
	mapped_buffer->flags        = binfo.flags;
	mapped_buffer->kind         = map_key_kind;
	mapped_buffer->va_allocated = va_allocated;
	mapped_buffer->vm_area      = vm_area;
#ifdef CONFIG_NVGPU_COMPRESSION
	mapped_buffer->ctag_offset  = binfo.ctag_offset;
	mapped_buffer->compr_kind_selected  = binfo.compr_kind_selected;
#endif
	mapped_buffer->rw_flag      = rw;
	mapped_buffer->aperture     = aperture;

	nvgpu_insert_mapped_buf(vm, mapped_buffer);

	if (vm_area != NULL) {
		nvgpu_list_add_tail(&mapped_buffer->buffer_list,
			      &vm_area->buffer_list_head);
		mapped_buffer->vm_area = vm_area;
	}

	nvgpu_mutex_release(&vm->update_gmmu_lock);

	return 0;

clean_up:
	nvgpu_mutex_release(&vm->update_gmmu_lock);
clean_up_nolock:
	nvgpu_kfree(g, mapped_buffer);

	return err;
}

/*
 * Really unmap. This does the real GMMU unmap and removes the mapping from the
 * VM map tracking tree (and vm_area list if necessary).
 */
/**
 * @brief Unmap a buffer from the GPU virtual memory space and free associated resources.
 *
 * This function performs the unmap operation for a given mapped buffer. It calls
 * the GPU operations to unmap the buffer, removes the buffer from the virtual memory
 * context's tracking structures, and performs any necessary OS-specific cleanup.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the virtual memory context and the gk20a structure from the mapped buffer.
 * -# Call the GPU operations to unmap the buffer from the virtual memory context using
 *    gops_mm_gmmu.unmap(). The parameters for this call include the virtual memory
 *    context, the address and size of the mapping, the page size index, whether the
 *    virtual address was allocated, the memory flags, the sparsity of the mapping,
 *    and the mapping batch.
 * -# Remove the mapped buffer from the virtual memory context's red-black tree using
 *    nvgpu_remove_mapped_buf().
 * -# Delete the buffer from the linked list of mapped buffers.
 * -# Perform any OS-specific cleanup required for unmapping the buffer using
 *    nvgpu_vm_unmap_system().
 * -# Free the memory allocated for the mapped buffer structure using nvgpu_kfree().
 *
 * @param [in] mapped_buffer  Pointer to the mapped buffer to be unmapped.
 * @param [in] batch          Pointer to a batch structure for batched unmap operations.
 *
 * @return 0 on success, or a negative error code on failure.
 */
static int nvgpu_vm_do_unmap(struct nvgpu_mapped_buf *mapped_buffer,
			      struct vm_gk20a_mapping_batch *batch)
{
	struct vm_gk20a *vm = mapped_buffer->vm;
	struct gk20a *g = vm->mm->g;
	int err = 0;

	err = g->ops.mm.gmmu.unmap(vm,
			     mapped_buffer->addr,
			     mapped_buffer->size,
			     mapped_buffer->pgsz_idx,
			     mapped_buffer->va_allocated,
			     gk20a_mem_flag_none,
			     (mapped_buffer->vm_area != NULL) ?
			     mapped_buffer->vm_area->sparse : false,
			     batch);

	/*
	 * Remove from mapped buffer tree. Then delete the buffer from the
	 * linked list of mapped buffers; though note: not all mapped buffers
	 * are part of a vm_area.
	 */
	nvgpu_remove_mapped_buf(vm, mapped_buffer);
	nvgpu_list_del(&mapped_buffer->buffer_list);

	/*
	 * OS specific freeing. This is after the generic freeing incase the
	 * generic freeing relies on some component of the OS specific
	 * nvgpu_mapped_buf in some abstraction or the like.
	 */
	nvgpu_vm_unmap_system(mapped_buffer);

	nvgpu_kfree(g, mapped_buffer);

	return err;
}

/**
 * @brief Retrieve the nvgpu_mapped_buf structure from a reference to it.
 *
 * This function computes the starting address of the nvgpu_mapped_buf structure
 * given a reference to its 'ref' member. It does this by subtracting the offset
 * of the 'ref' member within the nvgpu_mapped_buf structure from the address of
 * the 'ref' member.
 *
 * The steps performed by the function are as follows:
 * -# Calculate the offset of the 'ref' member within the nvgpu_mapped_buf structure
 *    using the #offsetof macro.
 * -# Subtract the offset from the address of the 'ref' member to obtain the
 *    starting address of the nvgpu_mapped_buf structure.
 * -# Cast the computed address to a pointer to nvgpu_mapped_buf and return it.
 *
 * @param [in] ref  Pointer to the reference member within the nvgpu_mapped_buf structure.
 *
 * @return Pointer to the nvgpu_mapped_buf structure containing 'ref'.
 */
static struct nvgpu_mapped_buf *nvgpu_mapped_buf_from_ref(struct nvgpu_ref *ref)
{
	return (struct nvgpu_mapped_buf *)
		((uintptr_t)ref - offsetof(struct nvgpu_mapped_buf, ref));
}

/*
 * Note: the update_gmmu_lock of the VM that owns this buffer must be locked
 * before calling nvgpu_ref_put() with this function as the unref function
 * argument since this can modify the tree of maps.
 */
int nvgpu_vm_unmap_ref_internal(struct nvgpu_ref *ref)
{
	struct nvgpu_mapped_buf *mapped_buffer = nvgpu_mapped_buf_from_ref(ref);
	int err = 0;

	err = nvgpu_vm_do_unmap(mapped_buffer, mapped_buffer->vm->kref_put_batch);

	if (err != 0) {
		nvgpu_err(NULL, "Unmap a buffer from the GPU virtual memory \
						space is failed [%d]", err);
	}

	return err;
}

/*
 * For fixed-offset buffers we must sync the buffer. That means we wait for the
 * buffer to hit a ref-count of 1 before proceeding.
 *
 * Note: this requires the update_gmmu_lock to be held since we release it and
 * re-aquire it in this function.
 */
/**
 * @brief Synchronize the unmapping of a buffer by waiting for its reference count to drop.
 *
 * This function waits for the reference count of a mapped buffer to drop to 1,
 * indicating that it is safe to proceed with unmapping. It uses a timeout mechanism
 * to prevent indefinite waiting.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a timeout structure with a 100ms timer using nvgpu_timeout_init_cpu_timer().
 * -# Release the update_gmmu_lock to allow other threads to decrease the reference count.
 * -# Enter a loop that continues until the reference count drops to 1 or the timeout expires.
 *    Within the loop:
 *    -# Check the current reference count using nvgpu_atomic_read(). If it is 1 or less,
 *       set done to true to exit the loop.
 *    -# If the timeout has expired, log a message with nvgpu_timeout_expired_msg(),
 *       set done to true, and prepare to return a timeout error.
 *    -# If neither condition is met, sleep for 10ms using nvgpu_msleep() to yield
 *       the processor.
 * -# After exiting the loop, check if the reference count is still greater than 1.
 *    If so, set the return value to -ETIMEDOUT to indicate a timeout occurred.
 * -# Re-acquire the update_gmmu_lock before returning.
 *
 * @param [in] vm             Pointer to the virtual memory context.
 * @param [in] mapped_buffer  Pointer to the mapped buffer to synchronize.
 *
 * @return 0 if the reference count dropped to 1 within the timeout period, or
 *         -ETIMEDOUT if the timeout period expired with the reference count still above 1.
 */
static int nvgpu_vm_unmap_sync_buffer(struct vm_gk20a *vm,
				      struct nvgpu_mapped_buf *mapped_buffer)
{
	struct nvgpu_timeout timeout;
	int ret = 0;
	bool done = false;

	/*
	 * 100ms timer.
	 */
	nvgpu_timeout_init_cpu_timer(vm->mm->g, &timeout, 100);

	nvgpu_mutex_release(&vm->update_gmmu_lock);

	do {
		if (nvgpu_atomic_read(&mapped_buffer->ref.refcount) <= 1) {
			done = true;
		} else if (nvgpu_timeout_expired_msg(&timeout,
			   "sync-unmap failed on 0x%llx",
			   mapped_buffer->addr) != 0) {
			done = true;
		} else {
			nvgpu_msleep(10);
		}
	} while (!done);

	if (nvgpu_atomic_read(&mapped_buffer->ref.refcount) > 1) {
		ret = -ETIMEDOUT;
	}

	nvgpu_mutex_acquire(&vm->update_gmmu_lock);

	return ret;
}

int nvgpu_vm_unmap(struct vm_gk20a *vm, u64 offset,
		    struct vm_gk20a_mapping_batch *batch)
{
	struct nvgpu_mapped_buf *mapped_buffer;
	int err = 0;

	nvgpu_mutex_acquire(&vm->update_gmmu_lock);

	mapped_buffer = nvgpu_vm_find_mapped_buf(vm, offset);
	if (mapped_buffer == NULL) {
		err = -EINVAL;
		goto done;
	}

	if ((mapped_buffer->flags & NVGPU_VM_MAP_FIXED_OFFSET) != 0U) {
		if (nvgpu_vm_unmap_sync_buffer(vm, mapped_buffer) != 0) {
			nvgpu_warn(vm->mm->g, "%d references remaining on 0x%llx",
				nvgpu_atomic_read(&mapped_buffer->ref.refcount),
				mapped_buffer->addr);
		}
	}

	/*
	 * Make sure we have access to the batch if we end up calling through to
	 * the unmap_ref function.
	 */
	vm->kref_put_batch = batch;
	err = nvgpu_ref_put_int_ptr(&mapped_buffer->ref, nvgpu_vm_unmap_ref_internal);

	if (err != 0) {
		nvgpu_err(vm->mm->g, "Unmap a buffer from the GPU virtual \
					memory space is failed [%d]", err);
	}
	vm->kref_put_batch = NULL;

done:
	nvgpu_mutex_release(&vm->update_gmmu_lock);
	return err;
}

#ifdef CONFIG_NVGPU_COMPRESSION
/**
 * @brief Compute and validate compression settings for a buffer mapping.
 *
 * This function checks if the compressible kind of a buffer is valid and if the
 * page size used for the mapping is supported for compression. If the page size
 * is not supported, it checks for a fallback incompressible kind and updates the
 * buffer info accordingly.
 *
 * The steps performed by the function are as follows:
 * -# Determine if the buffer's kind is compressible by checking if the compressible
 *    kind is not set to #NVGPU_KIND_INVALID.
 * -# Retrieve the gk20a structure from the virtual memory context using gk20a_from_vm().
 * -# If the buffer's kind is compressible and the page size for the mapping is smaller
 *    than the minimum compressible page size supported by the GPU, proceed to check
 *    for a fallback kind.
 * -# If no fallback incompressible kind is provided (indicated by #NVGPU_KIND_INVALID),
 *    log an error and return -EINVAL.
 * -# If a fallback incompressible kind is available, log a message indicating the
 *    demotion to incompressible and set the compressible kind to #NVGPU_KIND_INVALID
 *    in the buffer info.
 * -# Return 0 to indicate successful computation and validation of compression settings.
 *
 * @param [in] vm    Pointer to the virtual memory context.
 * @param [in] binfo Pointer to the buffer info structure containing compression details.
 *
 * @return 0 on success, or -EINVAL if the page size is not supported for compression
 *         and no fallback kind is provided.
 */
static int nvgpu_vm_compute_compression(struct vm_gk20a *vm,
					struct nvgpu_ctag_buffer_info *binfo)
{
	bool kind_compressible = (binfo->compr_kind != NVGPU_KIND_INVALID);
	struct gk20a *g = gk20a_from_vm(vm);

	if (kind_compressible &&
	    vm->gmmu_page_sizes[binfo->pgsz_idx] <
	    g->ops.fb.compressible_page_size(g)) {
		/*
		 * Let's double check that there is a fallback kind
		 */
		if (binfo->incompr_kind == NVGPU_KIND_INVALID) {
			nvgpu_err(g,
				  "Unsupported page size for compressible "
				  "kind, but no fallback kind");
			return -EINVAL;
		} else {
			nvgpu_log(g, gpu_dbg_map,
				  "Unsupported page size for compressible "
				  "kind, demoting to incompressible");
			binfo->compr_kind = NVGPU_KIND_INVALID;
		}
	}

	return 0;
}
#endif
