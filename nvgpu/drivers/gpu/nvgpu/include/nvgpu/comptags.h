/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_COMPTAGS_H
#define NVGPU_COMPTAGS_H

#ifdef CONFIG_NVGPU_COMPRESSION

#include <nvgpu/lock.h>
#include <nvgpu/types.h>
/* Forward declarations.*/
struct gk20a;
struct nvgpu_os_buffer;

struct gk20a_comptags {
	/*
	 * Offset to which this comptag begins with.
	 */
	u32 offset;
	/*
	 * Number of compbit cache lines.
	 */
	u32 lines;
	/*
	 * This signals whether allocation has been attempted. Observe 'lines'
	 * to see whether the comptags were actually allocated. We try alloc
	 * only once per buffer in order not to break multiple compressible-kind
	 * mappings.
	 */
	bool allocated;

	/*
	 * "enabled" indicates if the comptags are in use for mapping the buffer
	 * as compressible. Buffer comptags usage may be changed at runtime by
	 * buffer metadata re-registration. However, comptags once allocated
	 * are freed only on freeing the buffer.
	 *
	 * "enabled" implies that comptags have been successfully allocated
	 * (offset > 0 and lines > 0)
	 */
	bool enabled;

	/*
	 * Do comptags need to be cleared before mapping?
	 */
	bool needs_clear;
};

struct gk20a_comptag_allocator {
	/*
	 * Pointer to the GK20A struture.
	 */
	struct gk20a *g;
	/*
	 * Lock to synchronise this structure.
	 */
	struct nvgpu_mutex lock;

	/* This bitmap starts at ctag 1. 0th cannot be taken. */
	unsigned long *bitmap;

	/* Size of bitmap, not max ctags, so one less. */
	unsigned long size;
};

/* real size here, but first (ctag 0) isn't used */

/**
 * @brief This function is used to initialize the bit map allocator
 *  for comptag allocation.
 *
 *
 * Allocate memory for bit map allocator and initialize with
 *  the #size. This function is used by initialization of the
 *  #nvgpu_cbc in #gk20a.
 *
 * @param g         [in]	Pointer to the gk20a structure.
 * @param allocator [in/out]	Pointer to the gk20a_comptag_allocator structure.
 * @param size      [in]	Allocator size to be initialized.
 *
 * @return 0 for success, < 0 for failure.
 */
int gk20a_comptag_allocator_init(struct gk20a *g,
				 struct gk20a_comptag_allocator *allocator,
				 unsigned long size);

/**
 * @brief This function is used to destroy the bit map allocator
 *  initialized by #gk20a_comptag_allocator_init.
 *
 * Free the  memory for bit map allocator in #allocator.
 *
 * @param g         [in]        Pointer to the gk20a structure.
 * @param allocator [in/out]    Pointer to the gk20a_comptag_allocator structure.
 *
 * @return None.
 */
void gk20a_comptag_allocator_destroy(struct gk20a *g,
				     struct gk20a_comptag_allocator *allocator);

/**
 * @brief This function is used to allocate the comptag lines from the
 *  bit map allocator
 *
 *  Acquire the require lock.
 *  Get the free lines by calling #bitmap_find_next_zero_area.
 *  Call #nvgpu_bitmap_set to set the bits for the allocated bits.
 *  Release the lock.
 *
 * @param allocator [in]    Pointer to the gk20a_comptag_allocator structure.
 * @param offset    [out]   Offset in the bitmap where this allocation starts.
 * @param len       [in]    Number of lines required.
 *
 * @return 0 for success, < 0 for failure.
 */
int gk20a_comptaglines_alloc(struct gk20a_comptag_allocator *allocator,
			     u32 *offset, u32 len);

/**
 * @brief This function is used to free the comptag lines from the
 *  bit map allocator allocated by #gk20a_comptaglines_alloc.
 *
 *  Acquire the require lock.
 *  Free lines by calling #nvgpu_bitmap_clear.
 *  Release the lock.
 *
 * @param allocator [in]    Pointer to the gk20a_comptag_allocator structure.
 * @param offset    [in]    Offset in the bitmap where this allocation starts.
 * @param len       [in]    Number of lines required.
 *
 * @return None.
 */
void gk20a_comptaglines_free(struct gk20a_comptag_allocator *allocator,
			     u32 offset, u32 len);

/*
 * Defined by OS specific code since comptags are stored in a highly OS specific
 * way.
 */

/**
 * @brief Allocates compression tags for a buffer.
 *
 * This function allocates compression tags for a buffer associated with a given GPU and buffer
 * description. It updates the private data of the buffer with the allocated compression tags.
 * If the allocation fails, it logs the error and sets the appropriate flags in the private data.
 *
 * The steps performed by the function are as follows:
 * -# Calculate the granularity of the compression tags using gops_fb.compression_page_size().
 * -# Calculate the number of lines required for the buffer size using #DIV_ROUND_UP_ULL.
 * -# Check if the buffer size is zero. If it is, log an error and return -EINVAL.
 * -# Allocate the compression tags using gk20a_comptaglines_alloc().
 * -# If the allocation fails, log the information and set the lines to zero.
 * -# Retrieve the private data associated with the buffer using gk20a_dma_buf_get_drvdata().
 * -# Assert that the private data is not NULL.
 * -# Store the allocator used for compression tags in the private data.
 * -# Update the private data with the allocated compression tags information.
 * -# Set the flags indicating whether the compression tags need to be cleared and if they were
 *    allocated.
 *
 * @param [in]  g          GPU for which to allocate compression tags.
 * @param [in]  buf        Buffer for which to allocate compression tags.
 * @param [in]  allocator  Allocator to use for compression tags.
 *
 * @return 0 if the allocation was successful.
 * @return -#EINVAL if there is an error in parameters or the allocation failed.
 */
int gk20a_alloc_comptags(struct gk20a *g, struct nvgpu_os_buffer *buf,
			 struct gk20a_comptag_allocator *allocator);

/**
 * @brief Retrieves compression tags for a buffer.
 *
 * This function retrieves the compression tags associated with a buffer and stores them in the
 * provided comptags structure. If the buffer has no associated compression tags, the comptags
 * structure is zeroed out.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the private data associated with the buffer using gk20a_dma_buf_get_drvdata().
 * -# If the comptags parameter is NULL, exit the function.
 * -# If the private data is NULL, zero out the comptags structure using memset() and exit the
 *    function.
 * -# Acquire the mutex lock associated with the private data to ensure thread safety.
 * -# Copy the compression tags from the private data to the comptags structure.
 * -# Release the mutex lock.
 *
 * @param [in]  buf        Buffer for which to retrieve compression tags.
 * @param [out] comptags   Structure to store the retrieved compression tags.
 */
void gk20a_get_comptags(struct nvgpu_os_buffer *buf,
			struct gk20a_comptags *comptags);

/**
 * @brief Allocates or retrieves existing compression tags for a buffer.
 *
 * This function either allocates new compression tags for a buffer or retrieves existing ones
 * if they have already been allocated. It updates the private data of the buffer with the
 * compression tags information. If the allocation fails and is required, it logs the error.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the private data associated with the buffer using gk20a_dmabuf_alloc_or_get_drvdata()
 *    which will allocate a new private data structure if not already allocated.
 * -# Acquire the mutex lock associated with the private data to ensure thread safety.
 * -# If the buffer's metadata is not locked or is mutable, attempt to allocate compression tags.
 * -# If the allocation fails, log the error and return the error code.
 * -# Update the private data with the allocated compression tags information.
 * -# Release the mutex lock.
 *
 * @param [in]  g          GPU for which to allocate or retrieve compression tags.
 * @param [in]  buf        Buffer for which to allocate or retrieve compression tags.
 * @param [in]  allocator  Allocator to use for compression tags.
 *
 * @return 0 if the operation was successful.
 * @return -#EINVAL if there is an error in parameters or the allocation failed and is required.
 */
void gk20a_alloc_or_get_comptags(struct gk20a *g,
				 struct nvgpu_os_buffer *buf,
				 struct gk20a_comptag_allocator *allocator,
				 struct gk20a_comptags *comptags);
/**
 * @brief This function is used for synchronize the comptags clear operation.
 *
 *   if (gk20a_comptags_start_clear(os_buf)) {
 *           COMMENT: we now hold the buffer lock for clearing
 *           bool successful = hw_clear_comptags();
 *
 *           COMMENT: mark the buf cleared (or not) and release the buffer lock
 *           gk20a_comptags_finish_clear(os_buf, successful);
 *   }
 *
 *  If gk20a_start_comptags_clear() returns false, another caller has
 *  already cleared the comptags.
 *
 *  Call #gk20a_dma_buf_get_drvdata to get the private data from the #buf.
 *  Acquire the priv->lock.
 *  Return the value of priv->comptags.needs_clear.
 *
 * @param buf       [in]    Pointer to the nvgpu_os_buffer structure.
 *
 * @return true or false depends on clear bit in private data.
 */

/**
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the private data associated with the buffer using gk20a_dma_buf_get_drvdata().
 * -# If the private data is NULL, return false indicating that the clear operation did not start.
 * -# Acquire the mutex lock associated with the private data to ensure thread safety.
 * -# Check if the compression tags need to be cleared.
 * -# If the compression tags need to be cleared, return true indicating that the clear operation
 *    started.
 * -# If the compression tags do not need to be cleared, release the mutex lock and return false.
 *
 * @param buf       [in]    Pointer to the nvgpu_os_buffer structure.
 *
 * @return true or false depends on clear bit in private data.
 */
bool gk20a_comptags_start_clear(struct nvgpu_os_buffer *buf);

/**
 * @brief This function is used for synchronize the comptags clear operation.
 *
 * The steps performed by the function are as follows:
 * Call #gk20a_dma_buf_get_drvdata to get the private data from the #buf.
 * If the #clear_successful is set, set priv->comptags.needs_clear to false.
 * Release the lock acquired by #gk20a_comptags_start_clear.
 *
 * @param buf              [in]    Pointer to the nvgpu_os_buffer structure.
 * @param clear_successful [in]    Flag to indicate clear operation status.
 *
 * @return None.
 */
void gk20a_comptags_finish_clear(struct nvgpu_os_buffer *buf,
				 bool clear_successful);
#endif
#endif /* NVGPU_COMPTAGS_H */
