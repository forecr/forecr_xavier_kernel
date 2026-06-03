// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/errno.h>
#include <nvgpu/kmem.h>
#include <nvgpu/lock.h>
#include <nvgpu/log.h>
#include <nvgpu/flcnif_cmn.h>
#include <nvgpu/pmu/pmuif/nvgpu_cmdif.h>
#include <nvgpu/engine_queue.h>
#include <nvgpu/engine_mem_queue.h>
#include <nvgpu/pmu/cmd.h>

#include "engine_mem_queue_priv.h"
#include "engine_dmem_queue.h"
#include "engine_emem_queue.h"

/**
 * @brief Retrieves the head and tail positions of a memory queue.
 *
 * This function queries the current head and tail positions of a given memory
 * queue by invoking the respective head()[nvgpu_engine_mem_queue_params.queue_head()] and
 * tail()[nvgpu_engine_mem_queue_params.queue_tail()] functions provided in the
 * queue structure. If either of these operations fails, the function logs an
 * error message and returns the error code.
 *
 * The steps performed by the function are as follows:
 * -# Call the head()[]nvgpu_engine_mem_queue_params.queue_head() function with the GPU structure,
 *    queue identifier, queue index, and a pointer to store the head position, with the intention
 *    to retrieve (GET) the current head position.
 * -# If the head()[nvgpu_engine_mem_queue_params.queue_head()] function returns a non-zero error
 *    code, log an error message indicating that the head GET operation failed.
 * -# Call the tail()[nvgpu_engine_mem_queue_params.queue_tail()] function with the GPU structure,
 *    queue identifier, queue index, and a pointer to store the tail position, with the intention
 *    to retrieve (GET) the current tail position.
 * -# If the tail()[nvgpu_engine_mem_queue_params.queue_tail()] function returns a non-zero error
 *    code, log an error message indicating that the tail GET operation failed.
 * -# If both head()[nvgpu_engine_mem_queue_params.queue_head()] and tail()
 *    [nvgpu_engine_mem_queue_params.queue_tail()]functions succeed, return 0 indicating success.
 * -# If an error occurred in either the head()[nvgpu_engine_mem_queue_params.queue_head()] or
 *    tail()[nvgpu_engine_mem_queue_params.queue_tail()] function, return the error code.
 *
 * @param [in]  queue    Pointer to the memory queue structure.
 * @param [out] q_head   Pointer to store the retrieved head position of the queue.
 *                       Range: Pointer to a valid memory location.
 * @param [out] q_tail   Pointer to store the retrieved tail position of the queue.
 *                       Range: Pointer to a valid memory location.
 *
 * @return 0 if the head and tail positions are successfully retrieved.
 * @return Non-zero error code if the retrieval of head or tail positions fails.
 */
static int mem_queue_get_head_tail(struct nvgpu_engine_mem_queue *queue,
				   u32 *q_head, u32 *q_tail)
{
	int err = 0;

	err = queue->head(queue->g, queue->id, queue->index,
			  q_head, QUEUE_GET);
	if (err != 0) {
		nvgpu_err(queue->g, "flcn-%d, queue-%d, head GET failed",
			  queue->flcn_id, queue->id);
		goto exit;
	}

	err = queue->tail(queue->g, queue->id, queue->index,
			  q_tail, QUEUE_GET);
	if (err != 0) {
		nvgpu_err(queue->g, "flcn-%d, queue-%d, tail GET failed",
			  queue->flcn_id, queue->id);
		goto exit;
	}

	return 0;
exit:
	return err;
}

/* common falcon queue ops */
/**
 * @brief Checks if there is enough room in the memory queue for a given size.
 *
 * This function determines if a specified size can fit into the memory queue
 * without exceeding its boundaries. It also indicates whether a rewind is
 * necessary if the queue is full. The function aligns the size to the queue's
 * alignment requirement, calculates the available space, and sets the rewind
 * flag if needed.
 *
 * The steps performed by the function are as follows:
 * -# Align the requested size to the queue's alignment requirement.
 * -# Retrieve the current head and tail positions of the queue by calling
 *    'mem_queue_get_head_tail()'.
 * -# If the head position is greater than or equal to the tail position,
 *    calculate the free space from the head to the end of the queue.
 * -# Subtract the size of the command header from the free space to ensure
 *    there is enough room for the command header.
 * -# If the aligned size is greater than the free space, set the rewind flag
 *    and reset the head position to the start of the queue.
 * -# If the head position is less than the tail position, calculate the free
 *    space as the difference between the tail and head positions minus one.
 * -# If the 'need_rewind' pointer is not NULL, update it with the value of the
 *    rewind flag.
 * -# Return true if the aligned size is less than or equal to the free space,
 *    otherwise return false.
 *
 * @param [in]  queue        Pointer to the memory queue structure.
 * @param [in]  size         The size to check for available space in the queue.
 *                           Range: [0, queue->size - PMU_CMD_HDR_SIZE]
 * @param [out] need_rewind  Pointer to a boolean that will be set to true if
 *                           a rewind is needed, otherwise it will be set to false.
 *
 * @return True if there is enough room for the specified size, false otherwise.
 */
static bool engine_mem_queue_has_room(struct nvgpu_engine_mem_queue *queue,
			u32 size, bool *need_rewind)
{
	u32 q_head = 0;
	u32 q_tail = 0;
	u32 q_free = 0;
	bool q_rewind = false;
	int err = 0;

	size = NVGPU_ALIGN(size, QUEUE_ALIGNMENT);

	err = mem_queue_get_head_tail(queue, &q_head, &q_tail);
	if (err != 0) {
		goto exit;
	}

	if (q_head >= q_tail) {
		q_free = nvgpu_wrapping_add_u32(queue->offset, queue->size) - q_head;
		q_free = nvgpu_wrapping_sub_u32(q_free, (u32)PMU_CMD_HDR_SIZE);

		if (size > q_free) {
			q_rewind = true;
			q_head = queue->offset;
		}
	}

	if (q_head < q_tail) {
		q_free = q_tail - q_head - 1U;
	}

	if (need_rewind != NULL) {
		*need_rewind = q_rewind;
	}

exit:
	return size <= q_free;
}

/**
 * @brief Rewinds the engine memory queue to the start position.
 *
 * This function either sends a rewind command to the queue if it is in write mode,
 * or sets the queue's tail to the start position if it is in read mode. This is
 * used to reset the queue's position to the beginning.
 *
 * The steps performed by the function are as follows:
 * -# Check if the queue is opened in write mode.
 * -# If in write mode, prepare a rewind command with the appropriate unit ID and
 *    size.
 * -# Push the rewind command to the queue using the push() function.
 * -# If the push is successful, update the queue's position to the next aligned
 *    position using nvgpu_wrapping_add_u32() and NVGPU_ALIGN().
 * -# If the queue is opened in read mode, set the queue's tail to the start
 *    offset using the tail() function with #QUEUE_SET operation.
 * -# Return the error code if any operation fails, otherwise return 0 indicating
 *    success.
 *
 * @param [in]  flcn       Pointer to the Falcon structure.
 * @param [in]  queue      Pointer to the engine memory queue structure.
 *
 * @return 0 on success, negative error code on failure.
 */
static int engine_mem_queue_rewind(struct nvgpu_falcon *flcn,
	struct nvgpu_engine_mem_queue *queue)
{
	struct gk20a *g = queue->g;
	struct pmu_cmd cmd;
	int err = 0;

	if (queue->oflag == OFLAG_WRITE) {
		cmd.hdr.unit_id = PMU_UNIT_REWIND;
		cmd.hdr.size = (u8)PMU_CMD_HDR_SIZE;
		err = queue->push(flcn, queue, queue->position,
				  &cmd, cmd.hdr.size);
		if (err != 0) {
			nvgpu_err(g, "flcn-%d queue-%d, rewind request failed",
				queue->flcn_id, queue->id);
			goto exit;
		} else {
			queue->position = nvgpu_wrapping_add_u32(queue->position,
								 nvgpu_safe_cast_u32_to_u8(NVGPU_ALIGN(U32(cmd.hdr.size),
												       QUEUE_ALIGNMENT)));
			nvgpu_log_info(g, "flcn-%d queue-%d, rewinded",
			queue->flcn_id, queue->id);
		}
	}

	/* update queue position */
	queue->position = queue->offset;

	if (queue->oflag == OFLAG_READ) {
		err = queue->tail(g, queue->id, queue->index, &queue->position,
			QUEUE_SET);
		if (err != 0) {
			nvgpu_err(g, "flcn-%d queue-%d, position SET failed",
				queue->flcn_id, queue->id);
			goto exit;
		}
	}

exit:
	return err;
}

/**
 * @brief Prepares the memory queue for writing data.
 *
 * This function checks if there is enough space in the memory queue to write the
 * specified size of data. If not enough space is available, it sets an error code
 * and exits. If there is enough space but a rewind is needed, it performs the rewind
 * operation. It also retrieves the current head position of the queue.
 *
 * The steps performed by the function are as follows:
 * -# Check if there is enough room in the queue to write the specified size of data
 *    by calling engine_mem_queue_has_room().
 * -# Retrieve the current head position of the queue by calling the head()
 *    [nvgpu_engine_mem_queue_params.queue_head()] function with #QUEUE_GET as the operation.
 * -# If a rewind is needed, as indicated by the q_rewind flag, call the
 *    engine_mem_queue_rewind() function to rewind the queue.
 *
 * @param [in]  flcn       Pointer to the Falcon structure.
 * @param [in]  queue      Pointer to the engine memory queue structure.
 * @param [in]  size       Size of the data to be written to the queue.
 *                         Range: [0, nvgpu_engine_mem_queue_get_size(queue)]
 *
 * @return 0 on success, negative error code on failure.
 */
static int engine_mem_queue_prepare_write(struct nvgpu_falcon *flcn,
	struct nvgpu_engine_mem_queue *queue, u32 size)
{
	bool q_rewind = false;
	int err = 0;

	/* make sure there's enough free space for the write */
	if (!engine_mem_queue_has_room(queue, size, &q_rewind)) {
		nvgpu_log_info(queue->g, "queue full: queue-id %d: index %d",
			queue->id, queue->index);
		err = -EAGAIN;
		goto exit;
	}

	err = queue->head(queue->g, queue->id, queue->index,
			  &queue->position, QUEUE_GET);
	if (err != 0) {
		nvgpu_err(queue->g, "flcn-%d queue-%d, position GET failed",
			queue->flcn_id, queue->id);
		goto exit;
	}

	if (q_rewind) {
		err = engine_mem_queue_rewind(flcn, queue);
		if (err != 0) {
			goto exit;
		}
	}

exit:
	return err;
}

/* queue public functions */

/* queue push operation with lock */
int nvgpu_engine_mem_queue_push(struct nvgpu_falcon *flcn,
	struct nvgpu_engine_mem_queue *queue, void *data, u32 size)
{
	struct gk20a *g;
	int err = 0;

	if ((flcn == NULL) || (queue == NULL)) {
		return -EINVAL;
	}

	g = queue->g;

	if (queue->oflag != OFLAG_WRITE) {
		nvgpu_err(g, "flcn-%d, queue-%d not opened for write",
			queue->flcn_id, queue->id);
		err = -EINVAL;
		goto exit;
	}

	/* acquire mutex */
	nvgpu_mutex_acquire(&queue->mutex);

	err = engine_mem_queue_prepare_write(flcn, queue, size);
	if (err != 0) {
		goto unlock_mutex;
	}

	err = queue->push(flcn, queue, queue->position, data, size);
	if (err != 0) {
		nvgpu_err(g, "flcn-%d queue-%d, fail to write",
			queue->flcn_id, queue->id);
		goto unlock_mutex;
	}

	queue->position = nvgpu_wrapping_add_u32(queue->position,
						 NVGPU_ALIGN(size, QUEUE_ALIGNMENT));

	err = queue->head(g, queue->id, queue->index,
			  &queue->position, QUEUE_SET);
	if (err != 0) {
		nvgpu_err(g, "flcn-%d queue-%d, position SET failed",
			queue->flcn_id, queue->id);
	}

unlock_mutex:
	/* release mutex */
	nvgpu_mutex_release(&queue->mutex);
exit:
	return err;
}

/* queue pop operation with lock */
int nvgpu_engine_mem_queue_pop(struct nvgpu_falcon *flcn,
	struct nvgpu_engine_mem_queue *queue, void *data, u32 size,
	u32 *bytes_read)
{
	struct gk20a *g;
	u32 q_tail = 0;
	u32 q_head = 0;
	u32 used = 0;
	int err = 0;

	*bytes_read = 0;

	if ((flcn == NULL) || (queue == NULL)) {
		return -EINVAL;
	}

	g = queue->g;

	if (queue->oflag != OFLAG_READ) {
		nvgpu_err(g, "flcn-%d, queue-%d, not opened for read",
			queue->flcn_id, queue->id);
		err = -EINVAL;
		goto exit;
	}

	/* acquire mutex */
	nvgpu_mutex_acquire(&queue->mutex);

	err = mem_queue_get_head_tail(queue, &q_head, &queue->position);
	if (err != 0) {
		goto unlock_mutex;
	}

	q_tail = queue->position;

	if (q_head == q_tail) {
		goto unlock_mutex;
	} else if (q_head > q_tail) {
		used = q_head - q_tail;
	} else {
		used = queue->offset + queue->size - q_tail;
	}

	if (size > used) {
		nvgpu_warn(g, "queue size smaller than request read");
		size = used;
	}

	err = queue->pop(flcn, queue, q_tail, data, size);
	if (err != 0) {
		nvgpu_err(g, "flcn-%d queue-%d, fail to read",
			queue->flcn_id, queue->id);
		goto unlock_mutex;
	}

	queue->position = nvgpu_wrapping_add_u32(queue->position,
						 NVGPU_ALIGN(size, QUEUE_ALIGNMENT));

	err = queue->tail(g, queue->id, queue->index,
			  &queue->position, QUEUE_SET);
	if (err != 0) {
		nvgpu_err(g, "flcn-%d queue-%d, position SET failed",
			queue->flcn_id, queue->id);
		goto unlock_mutex;
	}

	*bytes_read = size;

unlock_mutex:
	/* release mutex */
	nvgpu_mutex_release(&queue->mutex);
exit:
	return err;
}

int nvgpu_engine_mem_queue_rewind(struct nvgpu_falcon *flcn,
	struct nvgpu_engine_mem_queue *queue)
{
	int err = 0;

	if ((flcn == NULL) || (queue == NULL)) {
		return -EINVAL;
	}

	/* acquire mutex */
	nvgpu_mutex_acquire(&queue->mutex);

	err = engine_mem_queue_rewind(flcn, queue);
	if (err != 0) {
		goto exit;
	}

exit:
	/* release mutex */
	nvgpu_mutex_release(&queue->mutex);

	return err;
}

/* queue is_empty check with lock */
bool nvgpu_engine_mem_queue_is_empty(struct nvgpu_engine_mem_queue *queue)
{
	u32 q_head = 0;
	u32 q_tail = 0;
	int err = 0;

	if (queue == NULL) {
		return true;
	}

	/* acquire mutex */
	nvgpu_mutex_acquire(&queue->mutex);

	err = mem_queue_get_head_tail(queue, &q_head, &q_tail);
	if (err != 0) {
		goto exit;
	}

exit:
	/* release mutex */
	nvgpu_mutex_release(&queue->mutex);

	return q_head == q_tail;
}

void nvgpu_engine_mem_queue_free(struct nvgpu_engine_mem_queue **queue_p)
{
	struct nvgpu_engine_mem_queue *queue = NULL;
	struct gk20a *g;

	if ((queue_p == NULL) || (*queue_p == NULL)) {
		return;
	}

	queue = *queue_p;

	g = queue->g;

	nvgpu_log_info(g, "flcn id-%d q-id %d: index %d ",
		       queue->flcn_id, queue->id, queue->index);

	/* destroy mutex */
	nvgpu_mutex_destroy(&queue->mutex);

	nvgpu_kfree(g, queue);
	*queue_p = NULL;
}

u32 nvgpu_engine_mem_queue_get_size(struct nvgpu_engine_mem_queue *queue)
{
	return queue->size;
}

int nvgpu_engine_mem_queue_init(struct nvgpu_engine_mem_queue **queue_p,
	struct nvgpu_engine_mem_queue_params params)
{
	struct nvgpu_engine_mem_queue *queue = NULL;
	struct gk20a *g = params.g;
	int err = 0;

	if (queue_p == NULL) {
		return -EINVAL;
	}

	queue = (struct nvgpu_engine_mem_queue *)
		   nvgpu_kmalloc(g, sizeof(struct nvgpu_engine_mem_queue));

	if (queue == NULL) {
		return -ENOMEM;
	}

	queue->g = params.g;
	queue->flcn_id = params.flcn_id;
	queue->id = params.id;
	queue->index = params.index;
	queue->offset = params.offset;
	queue->position = params.position;
	queue->size = params.size;
	queue->oflag = params.oflag;
	queue->engine_queue_type = params.engine_queue_type;

	queue->head = params.queue_head;
	queue->tail = params.queue_tail;

	nvgpu_log_info(g,
		"flcn id-%d q-id %d: index %d, offset 0x%08x, size 0x%08x",
		queue->flcn_id, queue->id, queue->index,
		queue->offset, queue->size);

	switch (queue->engine_queue_type) {
	case QUEUE_TYPE_DMEM:
		engine_dmem_queue_init(queue);
		break;
	case QUEUE_TYPE_EMEM:
		engine_emem_queue_init(queue);
		break;
	default:
		err = -EINVAL;
		break;
	}

	if (err != 0) {
		nvgpu_err(g, "flcn-%d queue-%d, init failed",
			queue->flcn_id, queue->id);
		nvgpu_kfree(g, queue);
		goto exit;
	}

	/* init mutex */
	nvgpu_mutex_init(&queue->mutex);

	*queue_p = queue;
exit:
	return err;
}
