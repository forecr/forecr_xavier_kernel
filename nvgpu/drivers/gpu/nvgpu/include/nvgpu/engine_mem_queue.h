/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ENGINE_MEM_QUEUE_H
#define NVGPU_ENGINE_MEM_QUEUE_H

/**
 * @file
 *
 * Abstract interface for interacting with the engine queue operations.
 */

#include <nvgpu/types.h>

struct nvgpu_falcon;
struct nvgpu_engine_mem_queue;

/**
 * This structure holds the necessary parameters required for
 * engine queue initialization for a given engine queue type.
 */
struct nvgpu_engine_mem_queue_params {
	/** Back pointer to G. */
	struct gk20a *g;
	/** Falcon processor identifier. */
	u32 flcn_id;

	/** Queue Type (engine_queue_type). */
	u8 engine_queue_type;
	/** current write position. */
	u32 position;
	/** physical dmem offset where this queue begins. */
	u32 offset;
	/** logical queue identifier. */
	u32 id;
	/** physical queue index. */
	u32 index;
	/** Size of queuer in bytes. */
	u32 size;
	/** accessibility / open flag. */
	u32 oflag;

	/**
	 * engine specific ops - GET or SET current head of the engine queue.
	 * @see ga10b_gsp_queue_head()
	 * @see gk20a_pmu_queue_head()
	 * @see tu104_sec2_queue_head()
	 */
	int (*queue_head)(struct gk20a *g, u32 queue_id, u32 queue_index,
		u32 *head, bool set);
	/**
	 * engine specific ops - GET or SET current tail of the engine queue.
	 * @see ga10b_gsp_queue_tail()
	 * @see gk20a_pmu_queue_tail()
	 * @see tu104_sec2_queue_tail()
	 */
	int (*queue_tail)(struct gk20a *g, u32 queue_id, u32 queue_index,
		u32 *tail, bool set);
};

/* queue public functions */
/**
 * @brief Initializes an engine memory queue.
 *
 * This function allocates memory for an engine memory queue, initializes it with
 * the provided parameters, and sets up the appropriate queue type initialization
 * based on the engine queue type specified in the parameters. It also initializes
 * a mutex for the queue.
 *
 * The steps performed by the function are as follows:
 * -# Allocate memory for the engine memory queue structure.
 * -# Initialize the allocated queue structure with the provided parameters.
 * -# Based on the engine queue type specified in the parameters, call the
 *    appropriate initialization function for #QUEUE_TYPE_DMEM or #QUEUE_TYPE_EMEM
 *    queue types.
 * -# Initialize a mutex for the queue.
 * -# Assign the address of the initialized queue structure to the provided
 *    queue pointer.
 *
 * @param [out] queue_p Pointer to the pointer of the engine memory queue to be initialized.
 * @param [in]  params  Parameters for initializing the engine memory queue.
 *
 * @return 0 on success.
 * @return -EINVAL if the input pointer is NULL or if the queue type is invalid.
 * @return -ENOMEM if memory allocation for the queue structure fails.
 */
int nvgpu_engine_mem_queue_init(struct nvgpu_engine_mem_queue **queue_p,
	struct nvgpu_engine_mem_queue_params params);

/**
 * @brief Checks if the memory queue is empty.
 *
 * This function determines whether a given engine memory queue is empty by comparing
 * the head and tail positions of the queue. It performs the check in a thread-safe
 * manner by acquiring and releasing a mutex associated with the queue.
 *
 * The steps performed by the function are as follows:
 * -# Acquire the mutex associated with the queue to ensure thread-safe access using
 *    nvgpu_mutex_acquire().
 * -# Retrieve the current head and tail positions of the queue using mem_queue_get_head_tail().
 * -# Release the mutex associated with the queue using nvgpu_mutex_release().
 * -# Compare the head and tail positions and return true if they are equal
 *    (indicating the queue is empty), otherwise return false.
 *
 * @param [in] queue The pointer to the engine memory queue structure.
 *
 * @return True if the queue is empty or if the queue pointer is NULL.
 * @return False if the queue contains elements.
 */
bool nvgpu_engine_mem_queue_is_empty(struct nvgpu_engine_mem_queue *queue);

/**
 * @brief Rewinds the memory queue for a given Falcon.
 *
 * This function attempts to rewind the memory queue associated with a given Falcon
 * processor. It ensures mutual exclusion during the rewind operation by acquiring
 * and releasing a mutex before and after the operation.
 *
 * The steps performed by the function are as follows:
 * -# Check if the provided Falcon processor or queue pointers are NULL. If any of
 *    them is NULL, return an error indicating invalid arguments.
 * -# Call the 'engine_mem_queue_rewind()' function to perform the actual rewind
 *    operation on the queue.
 * -# Return the result of the rewind operation.
 *
 * @param [in] flcn       Pointer to the Falcon processor structure.
 *                        Must not be NULL.
 * @param [in] queue      Pointer to the memory queue structure.
 *                        Must not be NULL.
 *
 * @return 0 on success, negative error code on failure.
 */
int nvgpu_engine_mem_queue_rewind(struct nvgpu_falcon *flcn,
	struct nvgpu_engine_mem_queue *queue);

/**
 * @brief Pops data from the specified memory queue.
 *
 * This function reads data from a memory queue associated with a given Falcon
 * processor. It ensures that the queue is open for reading, acquires a mutex
 * to prevent concurrent access, and then reads the specified amount of data
 * from the queue's current tail position. The function updates the queue's tail
 * position after reading and releases the mutex before returning.
 *
 * The steps performed by the function are as follows:
 * -# Acquire the mutex to prevent concurrent access to the queue.
 * -# Retrieve the current head and tail positions of the queue by calling
 *    'mem_queue_get_head_tail()'.
 * -# If the head and tail positions are equal, the queue is empty, so release
 *    the mutex and exit.
 * -# Calculate the amount of used space in the queue based on the head and tail
 *    positions.
 * -# If the requested size to read is greater than the used space, adjust the
 *    size to the maximum available data.
 * -# Call the queue's 'pop()' function to read data from the queue at the tail
 *    position.
 * -# Update the queue's tail position to reflect the data that has been read,
 *    aligning it to the queue's alignment requirement.
 * -# Set the new tail position in the queue by calling 'tail()' with the
 *    updated position.
 * -# Set the number of bytes read to the size of the data that was requested.
 * -# Release the mutex to allow access to the queue by other threads.
 * -# Return the error code if any operation failed, or zero on success.
 *
 * @param [in]  flcn        Pointer to the Falcon processor.
 * @param [in]  queue       Pointer to the memory queue to read from.
 * @param [out] data        Pointer to the buffer where read data will be stored.
 * @param [in]  size        The number of bytes to read from the queue.
 *                          Range: [0, queue->size]
 * @param [out] bytes_read  Pointer to a variable where the function will store
 *                          the number of bytes actually read.
 *
 * @return 0 on success, negative error code on failure.
 */
int nvgpu_engine_mem_queue_pop(struct nvgpu_falcon *flcn,
	struct nvgpu_engine_mem_queue *queue, void *data, u32 size,
	u32 *bytes_read);

/**
 * @brief Pushes data into the specified engine memory queue.
 *
 * This function pushes a block of data of a specified size into an engine memory queue
 * associated with a given Falcon microcontroller. It ensures that the queue is open for
 * writing and that there is enough space to accommodate the data. The function handles
 * synchronization through mutex locking and updates the queue's write position.
 *
 * The steps performed by the function are as follows:
 * -# Acquire the mutex associated with the queue to ensure exclusive access.
 * -# Prepare the queue for writing by ensuring there is enough space to push the data.
 *    If there is an error, release the mutex and return the error.
 * -# Push the data into the queue at the current write position and handle any errors
 *    by logging them, releasing the mutex, and returning the error.
 * -# Update the queue's write position by adding the size of the data, aligned to the
 *    queue's alignment requirement.
 * -# Update the queue's head pointer to the new write position and handle any errors
 *    by logging them.
 * -# Release the mutex previously acquired.
 *
 * @param [in]  flcn       Pointer to the Falcon microcontroller structure.
 * @param [in]  queue      Pointer to the engine memory queue structure.
 * @param [in]  data       Pointer to the data to be pushed into the queue.
 * @param [in]  size       Size of the data to be pushed into the queue.
 *                         Range: [0, queue->size - QUEUE_ALIGNMENT]
 *
 * @return 0 on success, negative error code on failure.
 */
int nvgpu_engine_mem_queue_push(struct nvgpu_falcon *flcn,
	struct nvgpu_engine_mem_queue *queue, void *data, u32 size);

/**
 * @brief Frees the memory allocated for an engine memory queue and destroys its mutex.
 *
 * This function checks if the provided pointer to the engine memory queue is not NULL,
 * logs the queue information, destroys the mutex associated with the queue, frees the
 * memory allocated for the queue, and sets the pointer to the queue to NULL.
 *
 * The steps performed by the function are as follows:
 * -# Check if the pointer to the pointer of the engine memory queue is not NULL and
 *    that the pointer it points to is also not NULL. If either is NULL, return
 *    immediately.
 * -# Retrieve the pointer to the engine memory queue from the provided pointer to
 *    pointer.
 * -# Destroy the mutex associated with the engine memory queue using nvgpu_mutex_destroy().
 * -# Free the memory allocated for the engine memory queue using nvgpu_kfree().
 * -# Set the provided pointer to the engine memory queue to NULL to indicate that the
 *    queue has been freed.
 *
 * @param [in,out] queue_p Pointer to the pointer of the engine memory queue to be freed.
 *                          Must not be NULL and must point to a valid memory queue.
 *
 * @return None.
 */
void nvgpu_engine_mem_queue_free(struct nvgpu_engine_mem_queue **queue_p);

/**
 * @brief Returns the size of the engine memory queue.
 *
 * This function retrieves the size of the specified engine memory queue, which
 * represents the total capacity of the queue in bytes.
 *
 * The steps performed by the function are as follows:
 * -# Return the size field from the engine memory queue structure.
 *
 * @param [in] queue The engine memory queue from which to retrieve the size.
 *                   The queue must be a valid pointer to an initialized
 *                   nvgpu_engine_mem_queue structure.
 *
 * @return The size of the engine memory queue in bytes.
 */
u32 nvgpu_engine_mem_queue_get_size(struct nvgpu_engine_mem_queue *queue);

#endif /* NVGPU_ENGINE_MEM_QUEUE_H */
