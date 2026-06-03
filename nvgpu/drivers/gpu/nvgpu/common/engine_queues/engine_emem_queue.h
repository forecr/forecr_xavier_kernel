/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ENGINE_EMEM_QUEUE_H
#define NVGPU_ENGINE_EMEM_QUEUE_H

/**
 * @brief Initializes the EMEM queue with specific operations.
 *
 * This function assigns the push and pop operations specific to the EMEM queue type
 * to the provided queue structure.
 *
 * The steps performed by the function are as follows:
 * -# Assign the address of the function 'engine_emem_queue_push()' to the push
 *    operation of the queue.
 * -# Assign the address of the function 'engine_emem_queue_pop()' to the pop
 *    operation of the queue.
 *
 * @param [in] queue The pointer to the queue structure to be initialized.
 *                   The queue must be a valid non-NULL pointer.
 *
 * @return None.
 */
void engine_emem_queue_init(struct nvgpu_engine_mem_queue *queue);

#endif /* NVGPU_ENGINE_EMEM_QUEUE_H */
