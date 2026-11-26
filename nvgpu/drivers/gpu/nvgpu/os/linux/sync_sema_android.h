/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_OS_LINUX_SYNC_SEMA_ANDROID_H
#define NVGPU_OS_LINUX_SYNC_SEMA_ANDROID_H

struct sync_timeline;
struct sync_fence;
struct sync_pt;
struct nvgpu_semaphore;
struct fence;

#ifdef CONFIG_NVGPU_SYNCFD_ANDROID
struct sync_timeline *gk20a_sync_timeline_create(const char *name);
void gk20a_sync_timeline_destroy(struct sync_timeline *);
void gk20a_sync_timeline_signal(struct sync_timeline *);
struct sync_fence *gk20a_sync_fence_create(
		struct nvgpu_channel *c,
		struct nvgpu_semaphore *,
		const char *fmt, ...);
struct sync_fence *gk20a_sync_fence_fdget(int fd);
struct nvgpu_semaphore *gk20a_sync_pt_sema(struct sync_pt *spt);
#else
static inline void gk20a_sync_timeline_destroy(struct sync_timeline *obj) {}
static inline void gk20a_sync_timeline_signal(struct sync_timeline *obj) {}
static inline struct sync_fence *gk20a_sync_fence_fdget(int fd)
{
	return NULL;
}
static inline struct sync_timeline *gk20a_sync_timeline_create(
	const char *name) {
		return NULL;
}
#endif

#endif
