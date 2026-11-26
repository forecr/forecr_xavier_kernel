/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LINUX_OS_FENCE_ANDROID_H
#define NVGPU_LINUX_OS_FENCE_ANDROID_H

struct gk20a;
struct nvgpu_os_fence;
struct sync_fence;
struct nvgpu_channel;

struct sync_fence *nvgpu_get_sync_fence(struct nvgpu_os_fence *s);

void nvgpu_os_fence_android_drop_ref(struct nvgpu_os_fence *s);

int nvgpu_os_fence_sema_fdget(struct nvgpu_os_fence *fence_out,
	struct nvgpu_channel *c, int fd);

void nvgpu_os_fence_android_dup(struct nvgpu_os_fence *s);
int nvgpu_os_fence_android_install_fd(struct nvgpu_os_fence *s, int fd);

int nvgpu_os_fence_syncpt_fdget(
	struct nvgpu_os_fence *fence_out,
	struct nvgpu_channel *c, int fd);

#endif /* NVGPU_LINUX_OS_FENCE_ANDROID_H */
