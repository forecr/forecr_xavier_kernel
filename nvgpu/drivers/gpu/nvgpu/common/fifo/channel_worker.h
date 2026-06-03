/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_COMMON_FIFO_CHANNEL_WORKER_H
#define NVGPU_COMMON_FIFO_CHANNEL_WORKER_H

#include <nvgpu/gk20a.h>

void nvgpu_channel_worker_enqueue(struct nvgpu_channel *ch);

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
static inline struct nvgpu_channel_worker *
nvgpu_channel_worker_from_worker(struct nvgpu_worker *worker)
{
	return (struct nvgpu_channel_worker *)
	   ((uintptr_t)worker - offsetof(struct nvgpu_channel_worker, worker));
};
#endif

#endif /* NVGPU_COMMON_FIFO_CHANNEL_WORKER_H */
