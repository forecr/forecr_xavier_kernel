/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GSP_QUEUE_H
#define NVGPU_GSP_QUEUE_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_falcon;
struct nv_flcn_cmd_gsp;
struct nvgpu_engine_mem_queue;
struct gsp_init_msg_gsp_init;

/**
 * @brief Initializes all GSP queues using the provided initialization parameters.
 *
 * The steps performed by the function are as follows:
 * -# Iterate over each queue index up to the total number of GSP queues defined by #GSP_QUEUE_NUM.
 *    - For each queue index:
 *      -# Initialize the queue by calling gsp_queue_init() with the current index and
 *         initialization parameters.
 *      -# If the queue initialization fails, iterate over the previously initialized queues
 *         and free each one by calling gsp_queue_free().
 *      -# Log an error indicating the queue initialization failure and return the error code.
 * -# Return 0 to indicate successful initialization of all GSP queues.
 *
 * @param [in] g       The GPU structure.
 * @param [out] queues The array of engine memory queues to be initialized.
 * @param [in] init    The initialization parameters for the GSP queues.
 *
 * @return 0 if all GSP queues are successfully initialized.
 * @return Non-zero error code if any queue initialization fails.
 */
int nvgpu_gsp_queues_init(struct gk20a *g,
			   struct nvgpu_engine_mem_queue **queues,
			   struct gsp_init_msg_gsp_init *init);

/**
 * @brief Frees all GSP queues associated with the GPU.
 *
 * The steps performed by the function are as follows:
 * -# Iterate over each queue index up to the total number of GSP queues defined by #GSP_QUEUE_NUM.
 *    - For each queue index:
 *      -# Free the queue by calling gsp_queue_free() with the current index.
 *
 * @param [in] g       The GPU structure.
 * @param [in] queues  The array of engine memory queues to be freed.
 */
void nvgpu_gsp_queues_free(struct gk20a *g,
			    struct nvgpu_engine_mem_queue **queues);

/**
 * @brief Retrieves the size of the specified GSP queue.
 *
 * The steps performed by the function are as follows:
 * -# Call nvgpu_engine_mem_queue_get_size() with the specified queue ID to get the size
 *    of the GSP queue.
 * -# Return the size of the GSP queue.
 *
 * @param [in] queues   The array of engine memory queues.
 * @param [in] queue_id The ID of the queue whose size is to be retrieved.
 *
 * @return The size of the specified GSP queue.
 */
u32 nvgpu_gsp_queue_get_size(struct nvgpu_engine_mem_queue **queues,
			      u32 queue_id);

/**
 * @brief Pushes a command to the specified GSP queue.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the engine memory queue based on the provided queue ID.
 * -# Call nvgpu_engine_mem_queue_push() to push the command onto the queue.
 * -# If the push operation fails, log an error message indicating the failure to
 *    push the command to the queue.
 * -# Return the error code from the push operation.
 *
 * @param [in] queues   The array of engine memory queues.
 * @param [in] queue_id The ID of the queue to which the command is to be pushed.
 * @param [in] flcn     The falcon structure associated with the GSP.
 * @param [in] cmd      The command to be pushed to the queue.
 * @param [in] size     The size of the command to be pushed.
 *
 * @return 0 if the command is successfully pushed to the queue.
 * @return Non-zero error code if the push operation fails.
 */
int nvgpu_gsp_queue_push(struct nvgpu_engine_mem_queue **queues,
			  u32 queue_id, struct nvgpu_falcon *flcn,
			  struct nv_flcn_cmd_gsp *cmd, u32 size);

/**
 * @brief Checks if the specified GSP queue is empty.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the engine memory queue based on the provided queue ID.
 * -# Call nvgpu_engine_mem_queue_is_empty() to check if the queue is empty.
 * -# Return the result of the check, indicating whether the queue is empty or not.
 *
 * @param [in] queues   The array of engine memory queues.
 * @param [in] queue_id The ID of the queue to be checked.
 *
 * @return true if the queue is empty.
 * @return false if the queue is not empty.
 */
bool nvgpu_gsp_queue_is_empty(struct nvgpu_engine_mem_queue **queues,
			       u32 queue_id);

/**
 * @brief Reads data from the specified GSP queue.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the engine memory queue based on the provided queue ID.
 * -# Attempt to pop data from the queue using nvgpu_engine_mem_queue_pop().
 * -# If the pop operation fails, log an error with the failure code, update the status
 *    with the error code, and return false.
 * -# If the number of bytes read does not match the number of bytes requested, log an
 *    error indicating the mismatch, update the status with -EINVAL, and return false.
 * -# If the read operation is successful and the number of bytes read matches the number
 *    of bytes requested, return true.
 *
 * @param [in] g             The GPU structure.
 * @param [in] queues        The array of engine memory queues.
 * @param [in] queue_id      The ID of the queue from which data is to be read.
 * @param [in] flcn          The falcon structure associated with the GSP.
 * @param [out] data         The buffer where the read data will be stored.
 * @param [in] bytes_to_read The number of bytes to read from the queue.
 * @param [out] status       The status of the read operation.
 *
 * @return true if the data is successfully read from the queue.
 * @return false if the read operation fails or the number of bytes read does not match the request.
 */
bool nvgpu_gsp_queue_read(struct gk20a *g,
			   struct nvgpu_engine_mem_queue **queues,
			   u32 queue_id, struct nvgpu_falcon *flcn, void *data,
			   u32 bytes_to_read, int *status);

/**
 * @brief Rewinds the specified GSP queue to its starting position.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the engine memory queue based on the provided queue ID.
 * -# Call nvgpu_engine_mem_queue_rewind() to rewind the queue.
 * -# If the rewind operation fails, log an error message indicating the failure to
 *    rewind the queue and return the error code.
 * -# Return the error code from the rewind operation, which will be 0 if the queue
 *    is successfully rewound.
 *
 * @param [in] flcn      The falcon structure associated with the GSP.
 * @param [in] queues    The array of engine memory queues.
 * @param [in] queue_id  The ID of the queue to be rewound.
 *
 * @return 0 if the queue is successfully rewound.
 * @return Non-zero error code if the rewind operation fails.
 */
int nvgpu_gsp_queue_rewind(struct nvgpu_falcon *flcn,
			    struct nvgpu_engine_mem_queue **queues,
			    u32 queue_id);

#endif /* NVGPU_GSP_QUEUE_H */
