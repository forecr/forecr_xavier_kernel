// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/falcon.h>
#include <nvgpu/log.h>

#include "engine_mem_queue_priv.h"
#include "engine_dmem_queue.h"

/* DMEM-Q specific ops */
/**
 * @brief Pushes data to the DMEM queue of a Falcon engine.
 *
 * This function copies the specified data to the designated location in the DMEM
 * queue of the given Falcon engine. If the copy operation fails, it logs an error
 * message and exits.
 *
 * The steps performed by the function are as follows:
 * -# Call nvgpu_falcon_copy_to_dmem() to copy the data to the DMEM queue at the
 *    specified destination offset.
 * -# If the copy operation fails, log an error message indicating the failure
 *    along with the Falcon ID and queue ID.
 * -# Return the error code from the copy operation.
 *
 * @param [in]  flcn       The Falcon engine to which the data is to be pushed.
 * @param [in]  queue      The DMEM queue structure where the data is to be pushed.
 * @param [in]  dst        The destination offset within the DMEM queue.
 * @param [in]  data       The pointer to the data to be copied.
 * @param [in]  size       The size of the data to be copied, in bytes.
 *
 * @return 0 on success, negative error code on failure.
 */
static int engine_dmem_queue_push(struct nvgpu_falcon *flcn,
	struct nvgpu_engine_mem_queue *queue, u32 dst, void *data, u32 size)
{
	struct gk20a *g = queue->g;
	int err = 0;

	err = nvgpu_falcon_copy_to_dmem(flcn, dst, data, size, 0);
	if (err != 0) {
		nvgpu_err(g, "flcn-%d, queue-%d", queue->flcn_id, queue->id);
		nvgpu_err(g, "dmem queue write failed");
		goto exit;
	}

exit:
	return err;
}

/**
 * @brief Pop data from the DMEM queue.
 *
 * This function reads data from the DMEM queue associated with a given Falcon
 * processor. It performs conditional compilation based on the build configuration
 * flags CONFIG_NVGPU_FALCON_DEBUG or CONFIG_NVGPU_FALCON_NON_FUSA. If either
 * flag is defined, it attempts to read from DMEM; otherwise, it sets an error
 * code indicating invalid arguments (safety).
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the global GPU state pointer from the queue structure.
 * -# If the configuration flags for Falcon debug or non-functional safety are
 *    defined, proceed with the DMEM read operation:
 *    -# Call nvgpu_falcon_copy_from_dmem() to read data from the DMEM queue
 *       into the provided buffer.
 *    -# If nvgpu_falcon_copy_from_dmem() returns a non-zero error code, log
 *       an error message indicating the Falcon ID and queue ID, and that the
 *       DMEM queue read operation failed.
 * -# If the configuration flags are not defined (safety), set the error code to
 *    -EINVAL to indicate invalid arguments.
 * -# Return the error code.
 *
 * @param [in]  flcn       Pointer to the Falcon processor structure.
 * @param [in]  queue      Pointer to the engine memory queue structure.
 * @param [in]  src        Source offset within DMEM from where to start reading.
 * @param [out] data       Pointer to the buffer where the read data will be stored.
 * @param [in]  size       Number of bytes to read from the DMEM queue.
 *
 * @return 0 if the read operation is successful.
 * @return Non-zero error code if the read operation fails or if the function
 *         is compiled without the necessary debug flags (on safety).
 */
static int engine_dmem_queue_pop(struct nvgpu_falcon *flcn,
	struct nvgpu_engine_mem_queue *queue, u32 src, void *data, u32 size)
{
	struct gk20a *g = queue->g;
	int err = 0;

#if (defined(CONFIG_NVGPU_FALCON_DEBUG) || defined(CONFIG_NVGPU_FALCON_NON_FUSA))
	err = nvgpu_falcon_copy_from_dmem(flcn, src, data, size, 0);
	if (err != 0) {
		nvgpu_err(g, "flcn-%d, queue-%d", queue->flcn_id, queue->id);
		nvgpu_err(g, "dmem queue read failed");
	}
#else
	(void)flcn;
	(void)src;
	(void)data;
	(void)size;
	(void)g;
	err = -EINVAL;
#endif
	return err;
}

/* assign DMEM queue type specific ops */
void engine_dmem_queue_init(struct nvgpu_engine_mem_queue *queue)
{
	queue->push = engine_dmem_queue_push;
	queue->pop = engine_dmem_queue_pop;
}
