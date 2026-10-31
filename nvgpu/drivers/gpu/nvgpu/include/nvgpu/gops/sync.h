/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_SYNC_H
#define NVGPU_GOPS_SYNC_H

#include <nvgpu/types.h>


/**
 * @file
 *
 * Sync HAL interface.
 */
struct gk20a;
struct nvgpu_channel;
struct nvgpu_mem;
struct vm_gk20a;
struct priv_cmd_entry;
struct nvgpu_semaphore;

struct gops_sync_syncpt {
	/**
	 * @brief Map syncpoint aperture as read-only.
	 *
	 * @param vm [in]		VM area for channel.
	 * @param base_gpu [out]	Base GPU VA for mapped
	 * 				syncpoint aperture.
	 * @param sync_size [out]	Size per syncpoint in bytes.
	 * @param num_syncpoints [out]	Number of syncpoints in the
	 * 				aperture.
	 *
	 * Map syncpoint aperture in GPU virtual memory as read-only:
	 * - Acquire syncpoint read-only map lock.
	 * - Map syncpoint aperture in sysmem to GPU virtual memory using
	 *   set_syncpt_ro_map_gpu_va_locked(), if not already mapped. Map as read-only.
	 * - Release syncpoint read-only map lock.
	 *
	 * The syncpoint shim mapping size is sync_size * num_syncpoints.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 * @retval -ENOMEM if syncpoint aperture could not be
	 *         mapped to GPU virtual memory.
	 */
	int (*get_sync_ro_map)(struct vm_gk20a *vm,
			u64 *base_gpuva, u32 *sync_size, u32 *num_syncpoints);

	/**
	 * @brief Allocates and maps a buffer for a given sync point ID.
	 *
	 * This function performs the following steps:
	 * -# Acquire the mutex lock for the sync point read-only map.
	 * -# Call set_syncpt_ro_map_gpu_va_locked() to add a read-only map for the complete
	 *    sync point shim range in the virtual memory. All channels sharing the same
	 *    virtual memory will share the same read-only mapping.
	 * -# Release the mutex lock for the sync point read-only map.
	 * -# If the previous step resulted in an error, return the error code.
	 * -# Calculate the number of pages required for the sync point buffer by dividing
	 *    the sync point size by the CPU page size and rounding up.
	 * -# Call nvgpu_mem_create_from_phys() to create a memory buffer from a physical
	 *    address, using the base address of the sync point unit and the byte offset
	 *    for the given sync point ID.
	 * -# If the memory buffer creation fails, log the error and return the error code.
	 * -# Call nvgpu_gmmu_map_partial() to map the memory buffer to a GPU virtual address
	 *    with system memory aperture.
	 * -# If the mapping fails (i.e., GPU virtual address is 0), log the error, free the
	 *    DMA memory for the sync point buffer, and set the error code to -ENOMEM.
	 * -# Return the error code, which is 0 if all operations were successful.
	 *
	 * @param [in]  c           Pointer to the channel structure.
	 * @param [in]  syncpt_id   Sync point ID for which the buffer is to be allocated.
	 * @param [out] syncpt_buf  Pointer to the memory structure where the buffer will be allocated.
	 *
	 * @return 0 if successful, negative error code otherwise.
	 */
	int (*alloc_buf)(struct nvgpu_channel *c,
			u32 syncpt_id,
			struct nvgpu_mem *syncpt_buf);
	/**
	 * @brief Free a previously allocated buffer for syncpoint values.
	 *
	 * -# Free the memory allocated for the syncpoint buffer using the
	 *    nvgpu_dma_unmap_free() function.
	 *
	 * @param [in]  c          Pointer to the channel structure.
	 * @param [in]  syncpt_buf Pointer to the buffer information to be freed.
	 */
	void (*free_buf)(struct nvgpu_channel *c,
			struct nvgpu_mem *syncpt_buf);

/** @cond DOXYGEN_SHOULD_SKIP_THIS */
#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
	void (*add_wait_cmd)(struct gk20a *g,
			struct priv_cmd_entry *cmd,
			u32 id, u32 thresh, u64 gpu_va_base);
	u32 (*get_wait_cmd_size)(void);
	void (*add_incr_cmd)(struct gk20a *g,
			struct priv_cmd_entry *cmd,
			u32 id, u64 gpu_va,
			bool wfi);
	u32 (*get_incr_cmd_size)(bool wfi_cmd);
	u32 (*get_incr_per_release)(void);
#endif
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */
};

#if defined(CONFIG_NVGPU_KERNEL_MODE_SUBMIT) && \
	defined(CONFIG_NVGPU_SW_SEMAPHORE)
/** @cond DOXYGEN_SHOULD_SKIP_THIS */
struct gops_sync_sema {
	u32 (*get_wait_cmd_size)(void);
	u32 (*get_incr_cmd_size)(void);
	void (*add_wait_cmd)(struct gk20a *g,
		struct priv_cmd_entry *cmd,
		struct nvgpu_semaphore *s, u64 sema_va);
	void (*add_incr_cmd)(struct gk20a *g,
		struct priv_cmd_entry *cmd,
		struct nvgpu_semaphore *s, u64 sema_va,
		bool wfi);
};
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */
#endif

/**
 * Sync HAL operations.
 *
 * @see gpu_ops
 */
struct gops_sync {
#ifdef CONFIG_TEGRA_GK20A_NVHOST
	struct gops_sync_syncpt syncpt;
#endif /* CONFIG_TEGRA_GK20A_NVHOST */
#if defined(CONFIG_NVGPU_KERNEL_MODE_SUBMIT) && \
	defined(CONFIG_NVGPU_SW_SEMAPHORE)
/** @cond DOXYGEN_SHOULD_SKIP_THIS */
	struct gops_sync_sema sema;
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */
#endif
};

#endif
