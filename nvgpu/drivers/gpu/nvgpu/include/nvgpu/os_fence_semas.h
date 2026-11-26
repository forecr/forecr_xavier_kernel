/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_OS_FENCE_SEMA_H
#define NVGPU_OS_FENCE_SEMA_H

#include <nvgpu/errno.h>

struct nvgpu_os_fence;
struct nvgpu_semaphore;

struct nvgpu_os_fence_sema {
	struct nvgpu_os_fence *fence;
};

#if defined(CONFIG_NVGPU_SW_SEMAPHORE) && !defined(CONFIG_NVGPU_SYNCFD_NONE)

int nvgpu_os_fence_sema_create(
	struct nvgpu_os_fence *fence_out,
	struct nvgpu_channel *c,
	struct nvgpu_semaphore *sema);

/*
 * Return a struct of nvgpu_os_fence_sema only if the underlying os_fence
 * object is backed by semaphore, else return empty object.
 */
int nvgpu_os_fence_get_semas(struct nvgpu_os_fence_sema *fence_sema_out,
	struct nvgpu_os_fence *fence_in);

/*
 * This method returns the nth semaphore as *semaphore_out
 * and should be only called on a valid instance of type
 * nvgpu_os_fence_sema.
 */
void nvgpu_os_fence_sema_extract_nth_semaphore(
	struct nvgpu_os_fence_sema *fence, u32 n,
		struct nvgpu_semaphore **semaphore_out);


/*
 * This method returns the number of underlying semaphores
 * and should be only called on a valid instance of type
 * nvgpu_os_fence_sema instance.
 */
u32 nvgpu_os_fence_sema_get_num_semaphores(
	struct nvgpu_os_fence_sema *fence);

#else /* CONFIG_NVGPU_SW_SEMAPHORE && !CONFIG_NVGPU_SYNCFD_NONE */

static inline int nvgpu_os_fence_sema_create(
	struct nvgpu_os_fence *fence_out,
	struct nvgpu_channel *c,
	struct nvgpu_semaphore *sema)
{
	(void)fence_out;
	(void)c;
	(void)sema;
	return -ENOSYS;
}

static inline int nvgpu_os_fence_get_semas(
	struct nvgpu_os_fence_sema *fence_sema_out,
	struct nvgpu_os_fence *fence_in)
{
	(void)fence_sema_out;
	(void)fence_in;
	return -EINVAL;
}

static inline void nvgpu_os_fence_sema_extract_nth_semaphore(
	struct nvgpu_os_fence_sema *fence, u32 n,
		struct nvgpu_semaphore **semaphore_out)
{
	(void)fence;
	(void)n;
	(void)semaphore_out;
}

static inline u32 nvgpu_os_fence_sema_get_num_semaphores(
	struct nvgpu_os_fence_sema *fence)
{
	(void)fence;
	return 0;
}

#endif /* CONFIG_NVGPU_SW_SEMAPHORE && !CONFIG_NVGPU_SYNCFD_NONE */

#endif /* NVGPU_OS_FENCE_SEMAS_H */
