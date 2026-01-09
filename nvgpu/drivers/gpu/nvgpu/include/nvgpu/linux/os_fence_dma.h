/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LINUX_OS_FENCE_DMA_H
#define NVGPU_LINUX_OS_FENCE_DMA_H

struct gk20a;
struct nvgpu_os_fence;
struct dma_fence;
struct nvgpu_channel;

struct dma_fence *nvgpu_get_dma_fence(struct nvgpu_os_fence *s);

void nvgpu_os_fence_dma_drop_ref(struct nvgpu_os_fence *s);

int nvgpu_os_fence_sema_fdget(struct nvgpu_os_fence *fence_out,
		struct nvgpu_channel *c, int fd);

int nvgpu_os_fence_dma_install_fd(struct nvgpu_os_fence *s, int fd);
void nvgpu_os_fence_dma_dup(struct nvgpu_os_fence *s);

int nvgpu_os_fence_syncpt_fdget(struct nvgpu_os_fence *fence_out,
		struct nvgpu_channel *c, int fd);

#endif /* NVGPU_LINUX_OS_FENCE_DMA_H */

