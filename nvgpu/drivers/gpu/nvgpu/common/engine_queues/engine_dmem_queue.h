/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ENGINE_DMEM_QUEUE_H
#define NVGPU_ENGINE_DMEM_QUEUE_H

/**
 * @brief Initializes the DMEM queue operations.
 *
 * This function sets the push and pop operation pointers for a DMEM queue to the
 * corresponding DMEM queue operations.
 *
 * The steps performed by the function are as follows:
 * -# Assign the address of the function 'engine_dmem_queue_push()' to the push
 *    operation pointer of the input queue.
 * -# Assign the address of the function 'engine_dmem_queue_pop()' to the pop
 *    operation pointer of the input queue.
 *
 * @param [in] queue The DMEM queue to initialize. The queue must be a valid
 *                   non-NULL pointer to a 'struct nvgpu_engine_mem_queue'.
 *
 * @return None.
 */
void engine_dmem_queue_init(struct nvgpu_engine_mem_queue *queue);

#endif /* NVGPU_ENGINE_DMEM_QUEUE_H */
