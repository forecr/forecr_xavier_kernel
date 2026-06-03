/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_CIRC_BUF_H
#define NVGPU_POSIX_CIRC_BUF_H

#include <nvgpu/bug.h>

/**
 * @brief Return count in buffer.
 *
 * Calculates the number of elements present in the circular buffer and
 * returns the value. The circular buffer should have a power of 2 size.
 * Macro does not perform any validation of the parameters.
 *
 * @param head [in]        Head of the buffer.
 * @param tail [in]        Tail of the buffer.
 * @param size [in]        Max number of elements in buffer.
 *
 * @return Count of elements in the buffer.
 */
#define CIRC_CNT(head, tail, size) ((__typeof(head))(((head) - (tail))) & ((size)-1U))

/**
 * @brief Return space in buffer.
 *
 * Calculates the space available in the circular buffer and returns the value.
 * The circular buffer should have a power of 2 size.
 * Macro does not perform any validation of the parameters.
 *
 * @param head [in]        Head of the buffer.
 * @param tail [in]        Tail of the buffer.
 * @param size [in]        Max number of elements in buffer.
 *
 * @return The space available in the buffer.
 */
#define CIRC_SPACE(head, tail, size) CIRC_CNT((tail), ((head)+1U), (size))

#endif /* NVGPU_POSIX_CIRC_BUF_H */
