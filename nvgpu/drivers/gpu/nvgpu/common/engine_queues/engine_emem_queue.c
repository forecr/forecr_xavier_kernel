// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/falcon.h>
#include <nvgpu/log.h>

#include "engine_mem_queue_priv.h"
#include "engine_emem_queue.h"

/* EMEM-Q specific ops */
/**
 * @brief Writes data to the EMEM (External Memory) of a Falcon microcontroller.
 *
 * This function pushes data into the external memory queue of a given Falcon
 * microcontroller. It uses the 'nvgpu_falcon_copy_to_emem()' function to perform
 * the actual memory copy operation. If the copy operation fails, it logs an error
 * message and exits.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the GPU context from the memory queue structure.
 * -# Call 'nvgpu_falcon_copy_to_emem()' to copy the data to the specified
 *    destination address in EMEM.
 * -# If the copy operation fails, log an error message indicating the failure
 *    along with the Falcon ID and queue ID.
 * -# Return the error code from the copy operation.
 *
 * @param [in] flcn       The Falcon microcontroller structure.
 * @param [in] queue      The memory queue structure associated with the Falcon.
 * @param [in] dst        The destination address in EMEM where data is to be written.
 * @param [in] data       The pointer to the data to be written to EMEM.
 * @param [in] size       The size of the data to be written.
 *
 * @return 0 on success, negative error code on failure.
 */
static int engine_emem_queue_push(struct nvgpu_falcon *flcn,
	struct nvgpu_engine_mem_queue *queue, u32 dst, void *data, u32 size)
{
	struct gk20a *g = queue->g;
	int err = 0;

	err = nvgpu_falcon_copy_to_emem(flcn, dst, data, size, 0);
	if (err != 0) {
		nvgpu_err(g, "flcn-%d, queue-%d", queue->flcn_id, queue->id);
		nvgpu_err(g, "emem queue write failed");
		goto exit;
	}

exit:
	return err;
}

/**
 * @brief Pop data from the engine's EMEM queue.
 *
 * This function reads data from the engine's external memory (EMEM) queue at a specified
 * source offset into a provided buffer. It uses the 'nvgpu_falcon_copy_from_emem()'
 * function to perform the read operation. If the read fails, it logs an error message.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the pointer to the GPU driver's state structure.
 * -# Call 'nvgpu_falcon_copy_from_emem()' to copy data from the EMEM queue into the
 *    provided buffer.
 * -# If 'nvgpu_falcon_copy_from_emem()' returns a non-zero error code, log an error
 *    message indicating that the EMEM queue read operation failed.
 * -# Return the error code from 'nvgpu_falcon_copy_from_emem()' or 0 if the operation
 *    was successful.
 *
 * @param [in]  flcn       Pointer to the Falcon microcontroller structure.
 * @param [in]  queue      Pointer to the engine memory queue structure.
 * @param [in]  src        Source offset within the EMEM queue from where data is to be read.
 *                         Range: [0, EMEM queue size - 1]
 * @param [out] data       Pointer to the buffer where the read data will be stored.
 * @param [in]  size       Number of bytes to read from the EMEM queue.
 *                         Range: [1, EMEM queue size - src]
 *
 * @return Non-zero error code on failure, 0 on success.
 */
static int engine_emem_queue_pop(struct nvgpu_falcon *flcn,
	struct nvgpu_engine_mem_queue *queue, u32 src, void *data, u32 size)
{
	struct gk20a *g = queue->g;
	int err = 0;

	err = nvgpu_falcon_copy_from_emem(flcn, src, data, size, 0);
	if (err != 0) {
		nvgpu_err(g, "flcn-%d, queue-%d", queue->flcn_id, queue->id);
		nvgpu_err(g, "emem queue read failed");
		goto exit;
	}

exit:
	return err;
}

/* assign EMEM queue type specific ops */
void engine_emem_queue_init(struct nvgpu_engine_mem_queue *queue)
{
	queue->push = engine_emem_queue_push;
	queue->pop = engine_emem_queue_pop;
}
