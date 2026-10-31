/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ENGINE_QUEUE_H
#define NVGPU_ENGINE_QUEUE_H

/* Queue Type */
/** DMEM engine queue. */
#define QUEUE_TYPE_DMEM 0x0U
/** EMEM engine queue. */
#define QUEUE_TYPE_EMEM 0x1U
/** FB engine queue. */
#define QUEUE_TYPE_FB   0x2U

/** Read accessibility for engine queue.*/
#define OFLAG_READ	0U
/** Write accessibility for engine queue.*/
#define OFLAG_WRITE	1U

/** Set operation on an engine queue.*/
#define QUEUE_SET	(true)
/** Get operation on an engine queue.*/
#define QUEUE_GET	(false)

/** Engine queue alignment requirement. */
#define QUEUE_ALIGNMENT		(4U)

#endif /* NVGPU_ENGINE_QUEUE_H */
