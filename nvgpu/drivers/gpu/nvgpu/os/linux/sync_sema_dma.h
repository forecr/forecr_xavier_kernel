/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_OS_LINUX_SYNC_SEMA_DMA_H
#define NVGPU_OS_LINUX_SYNC_SEMA_DMA_H

#ifdef CONFIG_NVGPU_SYNCFD_STABLE
#include <nvgpu/types.h>

struct nvgpu_channel;
struct nvgpu_semaphore;
struct dma_fence;

u64 nvgpu_sync_dma_context_create(void);

struct dma_fence *nvgpu_sync_dma_create(struct nvgpu_channel *c,
		struct nvgpu_semaphore *sema);

void nvgpu_sync_dma_signal(struct dma_fence *fence);
u32 nvgpu_dma_fence_length(struct dma_fence *fence);
struct nvgpu_semaphore *nvgpu_dma_fence_nth(struct dma_fence *fence, u32 i);

struct dma_fence *nvgpu_sync_dma_fence_fdget(int fd);
#endif /* CONFIG_NVGPU_SYNCFD_STABLE */

#endif
