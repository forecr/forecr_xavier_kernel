// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/mm.h>
#include <nvgpu/vm.h>
#include <nvgpu/dma.h>
#include <nvgpu/vm_area.h>
#include <nvgpu/acr.h>
#include <nvgpu/gmmu.h>
#include <nvgpu/vidmem.h>
#include <nvgpu/semaphore.h>
#include <nvgpu/pramin.h>
#include <nvgpu/enabled.h>
#include <nvgpu/errata.h>
#include <nvgpu/ce_app.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/engines.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/power_features/pg.h>

int nvgpu_mm_suspend(struct gk20a *g)
{
	int err;

	nvgpu_log_info(g, "MM suspend running...");

#ifdef CONFIG_NVGPU_DGPU
	nvgpu_vidmem_thread_pause_sync(&g->mm);
#endif

#ifdef CONFIG_NVGPU_COMPRESSION
	g->ops.mm.cache.cbc_clean(g);
#endif
	err = g->ops.mm.cache.l2_flush(g, false);
	if (err != 0) {
		nvgpu_err(g, "l2_flush failed");
		return err;
	}

	if (g->ops.fb.intr.disable != NULL) {
		g->ops.fb.intr.disable(g);
	}

	if (g->ops.mm.mmu_fault.disable_hw != NULL) {
		g->ops.mm.mmu_fault.disable_hw(g);
	}

	nvgpu_log_info(g, "MM suspend done!");

	return err;
}

u64 nvgpu_inst_block_addr(struct gk20a *g, struct nvgpu_mem *inst_block)
{
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_NVLINK)) {
		return nvgpu_mem_get_phys_addr(g, inst_block);
	} else {
		return nvgpu_mem_get_addr(g, inst_block);
	}
}

u32 nvgpu_inst_block_ptr(struct gk20a *g, struct nvgpu_mem *inst_block)
{
	u64 addr = nvgpu_inst_block_addr(g, inst_block) >>
			g->ops.ramin.base_shift();

	nvgpu_assert(u64_hi32(addr) == 0U);
	return u64_lo32(addr);
}

void nvgpu_free_inst_block(struct gk20a *g, struct nvgpu_mem *inst_block)
{
	if (nvgpu_mem_is_valid(inst_block)) {
		nvgpu_dma_free(g, inst_block);
	}
}

int nvgpu_alloc_inst_block(struct gk20a *g, struct nvgpu_mem *inst_block)
{
	int err;

	nvgpu_log_fn(g, " ");

	err = nvgpu_dma_alloc(g, g->ops.ramin.alloc_size(), inst_block);
	if (err != 0) {
		nvgpu_err(g, "%s: memory allocation failed", __func__);
		return err;
	}

	if (g->ops.ramin.set_magic_value != NULL) {
		g->ops.ramin.set_magic_value(g, inst_block);
	}
	if (nvgpu_iommuable(g) &&
			nvgpu_is_enabled(g, NVGPU_MM_SUPPORT_NONPASID_ATS)) {
		err = g->ops.fb.tlb_flush(g);
	}

	nvgpu_log_fn(g, "done");
	return err;
}

/**
 * @brief Allocate system memory for sysmem flush.
 *
 * This function allocates a 4KB block of system memory that is used for flushing
 * operations. The allocated memory is stored in the 'sysmem_flush' member of the
 * memory management structure within the GPU context.
 *
 * The steps performed by the function are as follows:
 * -# Call the 'nvgpu_dma_alloc_sys()' function to allocate 4KB of system memory
 *    and store the allocation information in the 'sysmem_flush' member of the
 *    memory management structure.
 *
 * @param [in]  g  The GPU context.
 *
 * @return The status code of the operation; 0 on success, non-zero on failure.
 */
static int nvgpu_alloc_sysmem_flush(struct gk20a *g)
{
	return nvgpu_dma_alloc_sys(g, SZ_4K, &g->mm.sysmem_flush);
}

/**
 * @brief Free system memory allocated for sysmem flush.
 *
 * This function releases the 4KB block of system memory that was previously
 * allocated for flushing operations. The memory is freed from the 'sysmem_flush'
 * member of the memory management structure within the GPU context.
 *
 * The steps performed by the function are as follows:
 * -# Call the 'nvgpu_dma_free()' function to release the system memory that is
 *    referenced by the 'sysmem_flush' member of the memory management structure.
 *
 * @param [in]  g  The GPU context.
 */
static void nvgpu_free_sysmem_flush(struct gk20a *g)
{
	nvgpu_dma_free(g, &g->mm.sysmem_flush);
}

#if defined(CONFIG_NVGPU_DGPU) || defined(CONFIG_NVGPU_COMPRESSION_RAW)
static void nvgpu_remove_mm_ce_support(struct mm_gk20a *mm)
{
	struct gk20a *g = gk20a_from_mm(mm);

#ifdef CONFIG_NVGPU_DGPU
	if (mm->vidmem.ce_ctx_id != NVGPU_CE_INVAL_CTX_ID) {
		nvgpu_ce_app_delete_context(g, mm->vidmem.ce_ctx_id);
	}
	mm->vidmem.ce_ctx_id = NVGPU_CE_INVAL_CTX_ID;

	nvgpu_vm_put(mm->ce.vm);
#endif
#ifdef CONFIG_NVGPU_COMPRESSION_RAW
	if (mm->cbc.ce_ctx_id != NVGPU_CE_INVAL_CTX_ID) {
		nvgpu_ce_app_delete_context(g, mm->cbc.ce_ctx_id);
	}
	mm->cbc.ce_ctx_id = NVGPU_CE_INVAL_CTX_ID;

	nvgpu_vm_put(mm->ce.vm);
#endif

}
#endif

/**
 * @brief Clean up memory management resources.
 *
 * This function releases all memory management related resources including
 * instance blocks, virtual memory (VM) allocations, and system memory flush
 * resources. It also performs cleanup operations specific to SW semaphores,
 * video memory, and errata workarounds if applicable.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the GPU context from the memory management structure using 'gk20a_from_mm()'.
 * -# Free the write and read DMA memory allocations used for MMU operations using 'nvgpu_dma_free()'.
 * -# If the function to remove BAR2 VM is defined, call it to clean up BAR2 VM resources using 'gops_mm.remove_bar2_vm()'.
 * -# Free the instance block and VM for BAR1 using 'nvgpu_free_inst_block()' and 'nvgpu_vm_put()'.
 * -# Free the instance blocks for PMU and HWPM, and release the PMU VM using 'nvgpu_free_inst_block()' and 'nvgpu_vm_put()'.
 * -# If SEC2 VM support is enabled, free the instance block for SEC2 and release the SEC2 VM using 'nvgpu_free_inst_block()' and 'nvgpu_vm_put()'.
 * -# If GSP VM support is enabled, free the instance block for GSP and release the GSP VM using 'nvgpu_free_inst_block()' and 'nvgpu_vm_put()'.
 * -# If CDE is present and not fused off, release the CDE VM using 'nvgpu_vm_put()'.
 * -# Free the system memory allocated for sysmem flush operations using 'nvgpu_free_sysmem_flush()'.
 * -# If SW semaphores are supported, destroy the semaphore sea using 'nvgpu_semaphore_sea_destroy()'.
 * -# If running on a discrete GPU, destroy the video memory allocator using 'nvgpu_vidmem_destroy()' and
 *    deinitialize any errata workarounds related to PDB cache using 'deinit_pdb_cache_errata()'.
 * -# Finally, deinitialize the page directory cache using 'nvgpu_pd_cache_fini()'.
 *
 * @param [in]  mm  The memory management structure.
 */
static void nvgpu_remove_mm_support(struct mm_gk20a *mm)
{
	struct gk20a *g = gk20a_from_mm(mm);

	nvgpu_dma_free(g, &mm->mmu_wr_mem);
	nvgpu_dma_free(g, &mm->mmu_rd_mem);

	if (g->ops.mm.remove_bar2_vm != NULL) {
		g->ops.mm.remove_bar2_vm(g);
	}

	nvgpu_free_inst_block(g, &mm->bar1.inst_block);
	nvgpu_vm_put(mm->bar1.vm);

	nvgpu_free_inst_block(g, &mm->pmu.inst_block);
	nvgpu_free_inst_block(g, &mm->hwpm.inst_block);
	nvgpu_vm_put(mm->pmu.vm);

	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_SEC2_VM)) {
		nvgpu_free_inst_block(g, &mm->sec2.inst_block);
		nvgpu_vm_put(mm->sec2.vm);
	}

	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_GSP_VM)) {
		nvgpu_free_inst_block(g, &mm->gsp.inst_block);
		nvgpu_vm_put(mm->gsp.vm);
	}

#ifdef CONFIG_NVGPU_NON_FUSA
	if (g->has_cde) {
		nvgpu_vm_put(mm->cde.vm);
	}
#endif

	nvgpu_free_sysmem_flush(g);

#ifdef CONFIG_NVGPU_SW_SEMAPHORE
	nvgpu_semaphore_sea_destroy(g);
#endif
#ifdef CONFIG_NVGPU_DGPU
	nvgpu_vidmem_destroy(g);

	if (nvgpu_is_errata_present(g, NVGPU_ERRATA_INIT_PDB_CACHE)) {
		g->ops.ramin.deinit_pdb_cache_errata(g);
	}
#endif
	nvgpu_pd_cache_fini(g);
}

/**
 * @brief Initialize the system virtual memory (VM) for the PMU (Power Management Unit).
 *
 * This function sets up the virtual memory for the PMU, including allocating an
 * instance block and initializing it. It also considers errata that may affect
 * the large page size used for the VM.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the GPU context from the memory management structure using 'gk20a_from_mm()'.
 * -# Get the default big page size from the GPU operations using 'gops_mm_gmmu.get_default_big_page_size()'.
 * -# Check for the presence of an errata that forces the PMU VM to use a 128K page size using 'nvgpu_is_errata_present()'.
 *    If present, override the big page size with #SZ_128K.
 * -# Define the low hole size and the total aperture size for the PMU VM.
 * -# Initialize the PMU VM using 'nvgpu_vm_init()' with the calculated big page size, low hole, and aperture size.
 *    If the VM initialization fails, return -ENOMEM.
 * -# Allocate an instance block for the PMU using 'nvgpu_alloc_inst_block()'.
 *    If the allocation fails, go to the clean-up step.
 * -# Initialize the instance block for the PMU VM using 'gops_mm.init_inst_block_core()'.
 *    If the initialization fails, free the instance block using 'nvgpu_free_inst_block()' and go to the clean-up step.
 * -# If all steps are successful, return 0.
 * -# In the clean-up step, release the PMU VM using 'nvgpu_vm_put()' and return the error code.
 *
 * @param [in]  mm  The memory management structure.
 *
 * @return 0 on success, or an error code on failure.
 */
static int nvgpu_init_system_vm(struct mm_gk20a *mm)
{
	int err;
	struct gk20a *g = gk20a_from_mm(mm);
	struct nvgpu_mem *inst_block = &mm->pmu.inst_block;
	u32 big_page_size = g->ops.mm.gmmu.get_default_big_page_size();
	u64 low_hole, aperture_size;

	/*
	 * For some reason the maxwell PMU code is dependent on the large page
	 * size. No reason AFAICT for this. Probably a bug somewhere.
	 */
	if (nvgpu_is_errata_present(g, NVGPU_ERRATA_MM_FORCE_128K_PMU_VM)) {
		big_page_size = nvgpu_safe_cast_u64_to_u32(SZ_128K);
	}

	/*
	 * No user region - so we will pass that as zero sized.
	 */
	low_hole = SZ_4K * 16UL;
	aperture_size = GK20A_PMU_VA_SIZE;

	mm->pmu.aperture_size = GK20A_PMU_VA_SIZE;
	nvgpu_log_info(g, "pmu vm size = 0x%x", mm->pmu.aperture_size);

	mm->pmu.vm = nvgpu_vm_init(g, big_page_size,
				   low_hole,
				   0ULL,
				   nvgpu_safe_sub_u64(aperture_size, low_hole),
				   0ULL,
				   true,
				   false,
				   "system");
	if (mm->pmu.vm == NULL) {
		return -ENOMEM;
	}

	err = nvgpu_alloc_inst_block(g, inst_block);
	if (err != 0) {
		goto clean_up_vm;
	}

	err = g->ops.mm.init_inst_block_core(inst_block, mm->pmu.vm, big_page_size);
	if (err != 0) {
		nvgpu_free_inst_block(g, inst_block);
		goto clean_up_vm;
	}

	return 0;

clean_up_vm:
	nvgpu_vm_put(mm->pmu.vm);
	return err;
}

/**
 * @brief Initialize the hardware performance monitoring (HWPM) instance block.
 *
 * This function allocates and initializes an instance block for HWPM within the
 * memory management structure. It uses the PMU VM for the HWPM instance block.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the GPU context from the memory management structure using 'gk20a_from_mm()'.
 * -# Allocate an instance block for HWPM using 'nvgpu_alloc_inst_block()'.
 *    If the allocation fails, return the error code.
 * -# Initialize the HWPM instance block with the PMU VM using 'gops_mm.init_inst_block_core()'.
 *    If the initialization fails, free the HWPM instance block using 'nvgpu_free_inst_block()'
 *    and return the error code.
 * -# If all steps are successful, return 0 indicating success.
 *
 * @param [in]  mm  The memory management structure.
 *
 * @return 0 on success, or an error code on failure.
 */
static int nvgpu_init_hwpm(struct mm_gk20a *mm)
{
	int err;
	struct gk20a *g = gk20a_from_mm(mm);
	struct nvgpu_mem *inst_block = &mm->hwpm.inst_block;

	err = nvgpu_alloc_inst_block(g, inst_block);
	if (err != 0) {
		return err;
	}

	err = g->ops.mm.init_inst_block_core(inst_block, mm->pmu.vm, 0);
	if (err != 0) {
		nvgpu_free_inst_block(g, inst_block);
		return err;
	}

	return 0;
}

#ifdef CONFIG_NVGPU_NON_FUSA
static int nvgpu_init_cde_vm(struct mm_gk20a *mm)
{
	struct gk20a *g = gk20a_from_mm(mm);
	u64 user_size, kernel_size;
	u32 big_page_size = g->ops.mm.gmmu.get_default_big_page_size();

	g->ops.mm.get_default_va_sizes(NULL, &user_size, &kernel_size);

	mm->cde.vm = nvgpu_vm_init(g, big_page_size,
				U64(big_page_size) << U64(10),
				nvgpu_safe_sub_u64(user_size,
					U64(big_page_size) << U64(10)),
				kernel_size,
				0ULL,
				false, false, "cde");
	if (mm->cde.vm == NULL) {
		return -ENOMEM;
	}
	return 0;
}
#endif

int nvgpu_init_ce_vm(struct mm_gk20a *mm)
{
	struct gk20a *g = gk20a_from_mm(mm);
	u64 user_size, kernel_size;
	u32 big_page_size = g->ops.mm.gmmu.get_default_big_page_size();

	g->ops.mm.get_default_va_sizes(NULL, &user_size, &kernel_size);

	mm->ce.vm = nvgpu_vm_init(g, big_page_size,
				U64(big_page_size) << U64(10),
				nvgpu_safe_sub_u64(user_size,
					U64(big_page_size) << U64(10)),
				kernel_size,
				0ULL,
				false, false, "ce");
	if (mm->ce.vm == NULL) {
		return -ENOMEM;
	}
	return 0;
}

/**
 * @brief Initialize memory used for MMU (Memory Management Unit) debugging.
 *
 * This function allocates system memory for MMU write and read operations used
 * in debugging. It ensures that the memory is valid and allocates it if not
 * already allocated.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the GPU context from the memory management structure using 'gk20a_from_mm()'.
 * -# Check if the MMU write memory is valid using 'nvgpu_mem_is_valid()'.
 *    If it is not valid, allocate 4KB of system memory for MMU write operations
 *    using 'nvgpu_dma_alloc_sys()' with #SZ_4K. If the allocation fails, go to the error handling step.
 * -# Check if the MMU read memory is valid using 'nvgpu_mem_is_valid()'.
 *    If it is not valid, allocate 4KB of system memory for MMU read operations
 *    using 'nvgpu_dma_alloc_sys()' with #SZ_4K. If the allocation fails, free the previously
 *    allocated MMU write memory using 'nvgpu_dma_free()' and go to the error handling step.
 * -# If both allocations are successful, return 0 indicating success.
 * -# In the error handling step, return -ENOMEM indicating a memory allocation failure.
 *
 * @param [in]  mm  The memory management structure.
 *
 * @return 0 on success, or -ENOMEM if memory allocation fails.
 */
static int nvgpu_init_mmu_debug(struct mm_gk20a *mm)
{
	struct gk20a *g = gk20a_from_mm(mm);
	int err;

	if (!nvgpu_mem_is_valid(&mm->mmu_wr_mem)) {
		err = nvgpu_dma_alloc_sys(g, SZ_4K, &mm->mmu_wr_mem);
		if (err != 0) {
			goto err;
		}
	}

	if (!nvgpu_mem_is_valid(&mm->mmu_rd_mem)) {
		err = nvgpu_dma_alloc_sys(g, SZ_4K, &mm->mmu_rd_mem);
		if (err != 0) {
			goto err_free_wr_mem;
		}
	}
	return 0;

 err_free_wr_mem:
	nvgpu_dma_free(g, &mm->mmu_wr_mem);
 err:
	return -ENOMEM;
}

#if defined(CONFIG_NVGPU_DGPU) || defined(CONFIG_NVGPU_COMPRESSION_RAW)
int nvgpu_init_mm_ce_context_find_gfx_gpu_instance(struct gk20a *g, u32 *out_gpu_instance_id)
{
	u32 gpu_instance_id;
	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_MIG)) {
		*out_gpu_instance_id = 0;
		return 0;
	}
	for (gpu_instance_id = 0; gpu_instance_id < g->mig.num_gpu_instances; gpu_instance_id++) {
		if (g->mig.gpu_instance[gpu_instance_id].is_gfx_capable) {
			*out_gpu_instance_id = gpu_instance_id;
			return 0;
		}
	}
	return -ENOENT;
}

int nvgpu_init_mm_ce_context(struct gk20a *g, u32 gpu_instance_id)
{
	int err;
	u32 ce_runlist_id;

	err = nvgpu_engine_get_fast_ce_runlist_id(g, gpu_instance_id, &ce_runlist_id);
	if (err != 0) {
		nvgpu_err(g, "Failed to find fast CE runlist id");
		return err;
	}

#ifdef CONFIG_NVGPU_DGPU
	if (g->mm.vidmem.size > 0U &&
	   (g->mm.vidmem.ce_ctx_id == NVGPU_CE_INVAL_CTX_ID)) {
		g->mm.vidmem.ce_ctx_id =
			nvgpu_ce_app_create_context(g,
				ce_runlist_id,
				-1,
				-1);

		if (g->mm.vidmem.ce_ctx_id == NVGPU_CE_INVAL_CTX_ID) {
			nvgpu_err(g,
				"Failed to allocate CE context for vidmem page clearing support");
			return -ENOMEM;
		}
	}
#endif
#ifdef CONFIG_NVGPU_COMPRESSION_RAW
	if (g->mm.cbc.ce_ctx_id == NVGPU_CE_INVAL_CTX_ID) {
		g->mm.cbc.ce_ctx_id =
			nvgpu_ce_app_create_context(g,
						    ce_runlist_id,
						    -1,
						    -1);

		if (g->mm.cbc.ce_ctx_id == NVGPU_CE_INVAL_CTX_ID) {
			nvgpu_err(g,
				  "Failed to allocate CE context for cbc fast scrub support");
			return -ENOMEM;
		}
	}
#endif
	return 0;
}
#endif

/**
 * @brief Initialize the BAR1 virtual memory (VM) for the GPU.
 *
 * This function sets up the BAR1 VM, including calculating the aperture size,
 * initializing the VM with the appropriate big page size, and allocating and
 * initializing an instance block for BAR1.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the GPU context from the memory management structure using 'gk20a_from_mm()'.
 * -# Get the default big page size from the GPU operations using 'gops_mm_gmmu.get_default_big_page_size()'.
 * -# Calculate the aperture size for BAR1 using 'bar1_aperture_size_mb_gk20a()' and convert it to bytes.
 * -# Initialize the BAR1 VM using 'nvgpu_vm_init()' with the calculated big page size and aperture size.
 *    If the VM initialization fails, return -ENOMEM.
 * -# Allocate an instance block for BAR1 using 'nvgpu_alloc_inst_block()'.
 *    If the allocation fails, go to the clean-up step.
 * -# Initialize the instance block for the BAR1 VM using 'gops_mm.init_inst_block_core()'.
 *    If the initialization fails, free the BAR1 instance block using 'nvgpu_free_inst_block()' and go to the clean-up step.
 * -# If all steps are successful, return 0 indicating success.
 * -# In the clean-up step, release the BAR1 VM using 'nvgpu_vm_put()' and return the error code.
 *
 * @param [in]  mm  The memory management structure.
 *
 * @return 0 on success, or an error code on failure.
 */
static int nvgpu_init_bar1_vm(struct mm_gk20a *mm)
{
	int err;
	struct gk20a *g = gk20a_from_mm(mm);
	struct nvgpu_mem *inst_block = &mm->bar1.inst_block;
	u32 big_page_size = g->ops.mm.gmmu.get_default_big_page_size();

	mm->bar1.aperture_size = bar1_aperture_size_mb_gk20a() << 20;
	nvgpu_log_info(g, "bar1 vm size = 0x%x", mm->bar1.aperture_size);
	mm->bar1.vm = nvgpu_vm_init(g,
			big_page_size,
			SZ_64K,
			0ULL,
			nvgpu_safe_sub_u64(mm->bar1.aperture_size, SZ_64K),
			0ULL,
			true, false,
			"bar1");
	if (mm->bar1.vm == NULL) {
		return -ENOMEM;
	}

	err = nvgpu_alloc_inst_block(g, inst_block);
	if (err != 0) {
		goto clean_up_vm;
	}

	err = g->ops.mm.init_inst_block_core(inst_block, mm->bar1.vm, big_page_size);
	if (err != 0) {
		nvgpu_free_inst_block(g, inst_block);
		goto clean_up_vm;
	}

	return 0;

clean_up_vm:
	nvgpu_vm_put(mm->bar1.vm);
	return err;
}

/**
 * @brief Initialize the virtual memory (VM) for engine microcode (ucode).
 *
 * This function sets up the VM for engine ucode, including allocating an
 * instance block and initializing it with the appropriate big page size.
 * The VM is named according to the provided address space name.
 *
 * The steps performed by the function are as follows:
 * -# Get the default big page size from the GPU operations using 'gops_mm_gmmu.get_default_big_page_size()'.
 * -# Set the ucode aperture size to 32MB.
 * -# Log the VM size information using 'nvgpu_log_info()'.
 * -# Initialize the ucode VM using 'nvgpu_vm_init()' with the calculated big page size and aperture size.
 *    If the VM initialization fails, return -ENOMEM.
 * -# Allocate an instance block for the engine ucode using 'nvgpu_alloc_inst_block()'.
 *    If the allocation fails, go to the clean-up step.
 * -# Initialize the instance block for the ucode VM using 'gops_mm.init_inst_block_core()'.
 *    If the initialization fails, free the ucode instance block using 'nvgpu_free_inst_block()' and go to the clean-up step.
 * -# If all steps are successful, return 0 indicating success.
 * -# In the clean-up step, release the ucode VM using 'nvgpu_vm_put()' and return the error code.
 *
 * @param [in]  g                    The GPU context.
 * @param [in]  ucode                The engine ucode structure.
 * @param [in]  address_space_name   The name of the address space for logging purposes.
 *
 * @return 0 on success, or an error code on failure.
 */
static int nvgpu_init_engine_ucode_vm(struct gk20a *g,
	struct engine_ucode *ucode, const char *address_space_name)
{
	int err;
	struct nvgpu_mem *inst_block = &ucode->inst_block;
	u32 big_page_size = g->ops.mm.gmmu.get_default_big_page_size();

	/* ucode aperture size is 32MB */
	ucode->aperture_size = U32(32) << 20U;
	nvgpu_log_info(g, "%s vm size = 0x%x", address_space_name,
		ucode->aperture_size);

	ucode->vm = nvgpu_vm_init(g, big_page_size, SZ_4K,
		0ULL, nvgpu_safe_sub_u64(ucode->aperture_size, SZ_4K), 0ULL,
		false, false,
		address_space_name);
	if (ucode->vm == NULL) {
		return -ENOMEM;
	}

	/* allocate instance mem for engine ucode */
	err = nvgpu_alloc_inst_block(g, inst_block);
	if (err != 0) {
		goto clean_up_va;
	}

	err = g->ops.mm.init_inst_block_core(inst_block, ucode->vm, big_page_size);
	if (err != 0) {
		nvgpu_free_inst_block(g, inst_block);
		goto clean_up_va;
	}

	return 0;

clean_up_va:
	nvgpu_vm_put(ucode->vm);
	return err;
}

/**
 * @brief Set up the BAR1, BAR2, system VM, and HWPM for the GPU.
 *
 * This function initializes the BAR1 and BAR2 virtual memory (VM) areas, system VM,
 * and hardware performance monitoring (HWPM) instance block for the GPU.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the memory management structure from the GPU context.
 * -# Initialize the BAR1 VM using 'nvgpu_init_bar1_vm()'. If an error occurs,
 *    return the error code.
 * -# If the function to initialize BAR2 VM is defined, call it using 'gops_mm.init_bar2_vm()'.
 *    If an error occurs, return the error code.
 * -# Initialize the system VM using 'nvgpu_init_system_vm()'. If an error occurs,
 *    return the error code.
 * -# Initialize the HWPM instance block using 'nvgpu_init_hwpm()'. If an error occurs,
 *    return the error code.
 * -# If all initializations are successful, return 0 indicating success.
 *
 * @param [in]  g  The GPU context.
 *
 * @return 0 on success, or an error code on failure.
 */
static int nvgpu_init_mm_setup_bar(struct gk20a *g)
{
	struct mm_gk20a *mm = &g->mm;
	int err;

	err = nvgpu_init_bar1_vm(mm);
	if (err != 0) {
		return err;
	}

	if (g->ops.mm.init_bar2_vm != NULL) {
		err = g->ops.mm.init_bar2_vm(g);
		if (err != 0) {
			return err;
		}
	}
	err = nvgpu_init_system_vm(mm);
	if (err != 0) {
		return err;
	}

	err = nvgpu_init_hwpm(mm);
	if (err != 0) {
		return err;
	}

	return err;
}

/**
 * @brief Set up virtual memory (VM) for various engine microcodes and the Copy Engine (CE).
 *
 * This function initializes the VMs for SEC2, GSP, CDE (if applicable), and CE
 * by checking if the corresponding support is enabled and then calling the
 * initialization functions for each VM.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the memory management structure from the GPU context.
 * -# If SEC2 VM support is enabled, initialize the SEC2 engine ucode VM using 'nvgpu_init_engine_ucode_vm()'.
 *    If an error occurs, return the error code.
 * -# If GSP VM support is enabled, initialize the GSP engine ucode VM using 'nvgpu_init_engine_ucode_vm()'.
 *    If an error occurs, return the error code.
 * -# If the build is configured with CONFIG_NVGPU_NON_FUSA and the GPU has CDE,
 *    initialize the CDE VM using 'nvgpu_init_cde_vm()'. If an error occurs, return the error code.
 * -# Initialize the CE VM using 'nvgpu_init_ce_vm()'. If an error occurs, return the error code.
 * -# If all VM initializations are successful, return 0 indicating success.
 *
 * @param [in]  g  The GPU context.
 *
 * @return 0 on success, or an error code on failure.
 */
static int nvgpu_init_mm_setup_vm(struct gk20a *g)
{
	struct mm_gk20a *mm = &g->mm;
	int err;

	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_SEC2_VM)) {
		err = nvgpu_init_engine_ucode_vm(g, &mm->sec2, "sec2");
		if (err != 0) {
			return err;
		}
	}

	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_GSP_VM)) {
		err = nvgpu_init_engine_ucode_vm(g, &mm->gsp, "gsp");
		if (err != 0) {
			return err;
		}
	}

#ifdef CONFIG_NVGPU_NON_FUSA
	if (g->has_cde) {
		err = nvgpu_init_cde_vm(mm);
			if (err != 0) {
				return err;
			}
	}
#endif

	err = nvgpu_init_ce_vm(mm);
	if (err != 0) {
		return err;
	}

	return err;
}

/**
 * @brief Initialize various memory management components for the GPU.
 *
 * This function sets up the system memory flush, BAR1 and BAR2 VMs, VMs for
 * engine microcodes and CE, and MMU debugging components. It also initializes
 * software for replayable MMU faults if supported by the chip.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the memory management structure from the GPU context.
 * -# Allocate system memory for flushing operations using 'nvgpu_alloc_sysmem_flush()'.
 *    If an error occurs, return the error code.
 * -# Set up the BAR1 and BAR2 VMs using 'nvgpu_init_mm_setup_bar()'.
 *    If an error occurs, return the error code.
 * -# Set up the VMs for engine microcodes and CE using 'nvgpu_init_mm_setup_vm()'.
 *    If an error occurs, return the error code.
 * -# Initialize MMU debugging components using 'nvgpu_init_mmu_debug()'.
 *    If an error occurs, return the error code.
 * -# If the GPU supports replayable MMU faults and the corresponding software
 *    setup function is defined, initialize the software for replayable MMU faults
 *    using 'gops_mm_mmu_fault.setup_sw()'. If an error occurs, return the error code.
 * -# If all initializations are successful, return 0 indicating success.
 *
 * @param [in]  g  The GPU context.
 *
 * @return 0 on success, or an error code on failure.
 */
static int nvgpu_init_mm_components(struct gk20a *g)
{
	int err = 0;
	struct mm_gk20a *mm = &g->mm;

	err = nvgpu_alloc_sysmem_flush(g);
	if (err != 0) {
		return err;
	}

	err = nvgpu_init_mm_setup_bar(g);
	if (err != 0) {
		return err;
	}

	err = nvgpu_init_mm_setup_vm(g);
	if (err != 0) {
		return err;
	}

	err = nvgpu_init_mmu_debug(mm);
	if (err != 0) {
		return err;
	}

	/*
	 * Some chips support replayable MMU faults. For such chips make sure
	 * SW is initialized.
	 */
	if (g->ops.mm.mmu_fault.setup_sw != NULL) {
		err = g->ops.mm.mmu_fault.setup_sw(g);
		if (err != 0) {
			return err;
		}
	}

	return 0;
}

/**
 * @brief Set up the software components of the memory management subsystem.
 *
 * This function initializes the software state of the memory management subsystem,
 * including mutexes, channel VM sizes, and other components such as video memory
 * and ACR blob prerequisites. It also sets up cleanup functions and marks the
 * initialization as complete.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the memory management structure from the GPU context.
 * -# If the software is already initialized, indicated by 'sw_ready', skip further initialization.
 * -# Initialize the L2 operation lock mutex using 'nvgpu_mutex_init()'.
 * -# Retrieve the default user and kernel virtual address space sizes using 'gops_mm.get_default_va_sizes()'.
 * -# If running on a discrete GPU, perform the following additional initializations:
 *    - Initialize the PRAMIN context using 'nvgpu_init_pramin()'.
 *    - Initialize video memory using 'nvgpu_vidmem_init()'. If an error occurs, return the error code.
 *    - If unified memory is not enabled and private security is enabled, allocate the ACR blob prerequisites
 *      using 'nvgpu_acr_alloc_blob_prerequisite()'. If an error occurs, return the error code.
 * -# Initialize various memory management components using 'nvgpu_init_mm_components()'.
 *    If an error occurs, return the error code.
 * -# If ECC counter initialization is required and not already done, initialize ECC counters using
 *    'gops_fb_ecc.ecc_counter_init()'. If an error occurs, return the error code.
 * -# Assign the cleanup function 'nvgpu_remove_mm_support()' to the 'remove_support' member.
 * -# If running on a discrete GPU, assign the cleanup function 'nvgpu_remove_mm_ce_support()' to the 'remove_ce_support' member.
 * -# Mark the software initialization as complete by setting 'sw_ready' to true.
 * -# Return 0 indicating success.
 *
 * @param [in]  g  The GPU context.
 *
 * @return 0 on success, or an error code on failure.
 */
static int nvgpu_init_mm_setup_sw(struct gk20a *g)
{
	struct mm_gk20a *mm = &g->mm;
	int err = 0;

	if (mm->sw_ready) {
		nvgpu_log_info(g, "skip init");
		return 0;
	}

	mm->g = g;
	nvgpu_mutex_init(&mm->l2_op_lock);

	/*TBD: make channel vm size configurable */
	g->ops.mm.get_default_va_sizes(NULL, &mm->channel.user_size,
		&mm->channel.kernel_size);

	nvgpu_log_info(g, "channel vm size: user %uMB  kernel %uMB",
		nvgpu_safe_cast_u64_to_u32(mm->channel.user_size >> U64(20)),
		nvgpu_safe_cast_u64_to_u32(mm->channel.kernel_size >> U64(20)));

#ifdef CONFIG_NVGPU_DGPU
	mm->vidmem.ce_ctx_id = NVGPU_CE_INVAL_CTX_ID;

	nvgpu_init_pramin(mm);

	err = nvgpu_vidmem_init(mm);
	if (err != 0) {
		return err;
	}

	/*
	 * this requires fixed allocations in vidmem which must be
	 * allocated before all other buffers
	 */

	if (!nvgpu_is_enabled(g, NVGPU_MM_UNIFIED_MEMORY) &&
			nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
		err = nvgpu_acr_alloc_blob_prerequisite(g, g->acr, 0);
		if (err != 0) {
			return err;
		}
	}
#endif

#ifdef CONFIG_NVGPU_COMPRESSION_RAW
	mm->cbc.ce_ctx_id = NVGPU_CE_INVAL_CTX_ID;
#endif

	err = nvgpu_init_mm_components(g);
	if (err != 0) {
		return err;
	}

	if ((g->ops.fb.ecc.ecc_counter_init != NULL) && !g->ecc.initialized) {
		err = g->ops.fb.ecc.ecc_counter_init(g);
		if (err != 0) {
			return err;
		}
	}

	mm->remove_support = nvgpu_remove_mm_support;
#if defined(CONFIG_NVGPU_DGPU) || defined(CONFIG_NVGPU_COMPRESSION_RAW)
	mm->remove_ce_support = nvgpu_remove_mm_ce_support;
#endif

	mm->sw_ready = true;

	return 0;
}

#ifdef CONFIG_NVGPU_DGPU
static int nvgpu_init_mm_pdb_cache_errata(struct gk20a *g)
{
	int err;

	if (nvgpu_is_errata_present(g, NVGPU_ERRATA_INIT_PDB_CACHE)) {
		err = g->ops.ramin.init_pdb_cache_errata(g);
		if (err != 0) {
			return err;
		}
	}

	if (nvgpu_is_errata_present(g, NVGPU_ERRATA_FB_PDB_CACHE)) {
		err = g->ops.fb.apply_pdb_cache_errata(g);
		if (err != 0) {
			return err;
		}
	}

	return 0;
}
#endif

/*
 * Called through the HAL to handle vGPU: the vGPU doesn't have HW to initialize
 * here.
 */
int nvgpu_mm_setup_hw(struct gk20a *g)
{
	struct mm_gk20a *mm = &g->mm;
	int err;

	nvgpu_log_fn(g, " ");

	if (g->ops.fb.set_mmu_page_size != NULL) {
		g->ops.fb.set_mmu_page_size(g);
	}

#ifdef CONFIG_NVGPU_COMPRESSION
	if (g->ops.fb.set_use_full_comp_tag_line != NULL) {
		mm->use_full_comp_tag_line =
			g->ops.fb.set_use_full_comp_tag_line(g);
	}
#endif

	g->ops.fb.init_hw(g);

	if (g->ops.bus.bar1_bind != NULL) {
		err = g->ops.bus.bar1_bind(g, &mm->bar1.inst_block);
		if (err != 0) {
			return err;
		}
	}

	if (g->ops.bus.bar2_bind != NULL) {
		err = g->ops.bus.bar2_bind(g, &mm->bar2.inst_block);
		if (err != 0) {
			return err;
		}
	}

	if ((g->ops.mm.cache.fb_flush(g) != 0) ||
		(g->ops.mm.cache.fb_flush(g) != 0)) {
		return -EBUSY;
	}

	if (g->ops.mm.mmu_fault.setup_hw != NULL) {
		g->ops.mm.mmu_fault.setup_hw(g);
	}

	nvgpu_log_fn(g, "done");
	return 0;
}

int nvgpu_init_mm_support(struct gk20a *g)
{
	int err;

#ifdef CONFIG_NVGPU_DGPU
	err = nvgpu_init_mm_pdb_cache_errata(g);
	if (err != 0) {
		return err;
	}
#endif

	err = nvgpu_init_mm_setup_sw(g);
	if (err != 0) {
		return err;
	}

#if defined(CONFIG_NVGPU_NON_FUSA)
	if (nvgpu_fb_vab_init_hal(g) != 0) {
		nvgpu_err(g, "failed to init VAB");
	}
#endif

	if (g->ops.mm.setup_hw != NULL) {
		err = g->ops.mm.setup_hw(g);
	}

	return err;
}

u32 nvgpu_mm_get_default_huge_page_size(struct gk20a *g)
{
	u32 huge_page_size = 0;

	if (g->ops.mm.gmmu.get_default_huge_page_size) {
		huge_page_size = g->ops.mm.gmmu.get_default_huge_page_size();
	}

	return huge_page_size;
}

u32 nvgpu_mm_get_default_big_page_size(struct gk20a *g)
{
	u32 big_page_size;

	big_page_size = g->ops.mm.gmmu.get_default_big_page_size();

	if (g->mm.disable_bigpage) {
		big_page_size = 0;
	}

	return big_page_size;
}

u32 nvgpu_mm_get_available_big_page_sizes(struct gk20a *g)
{
	u32 available_big_page_sizes = 0;

	if (g->mm.disable_bigpage) {
		return available_big_page_sizes;
	}

	available_big_page_sizes = g->ops.mm.gmmu.get_default_big_page_size();
	if (g->ops.mm.gmmu.get_big_page_sizes != NULL) {
		available_big_page_sizes |= g->ops.mm.gmmu.get_big_page_sizes();
	}

	return available_big_page_sizes;
}
