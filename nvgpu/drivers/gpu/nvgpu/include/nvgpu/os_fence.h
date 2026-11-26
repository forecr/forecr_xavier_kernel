/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_OS_FENCE_H
#define NVGPU_OS_FENCE_H

#include <nvgpu/types.h>

/*
 * struct nvgpu_os_fence adds an abstraction to the earlier Android Sync
 * Framework, specifically the sync-fence mechanism and the newer DMA sync
 * APIs from linux-4.9. This abstraction provides the high-level definition
 * as well as APIs that can be used by other OSes in future to have their own
 * alternatives for the sync-framework.
 */
struct nvgpu_os_fence;

/*
 * struct nvgpu_os_fence depends on the following ops structure
 */
struct nvgpu_os_fence_ops {
	/*
	 * This should be the last operation on the OS fence. The
	 * OS fence acts as a place-holder for the underlying fence
	 * implementation e.g. sync_fences. For each construct/fdget call
	 * there needs to be a drop_ref call. This reduces a reference count
	 * for the underlying sync_fence.
	 */
	void (*drop_ref)(struct nvgpu_os_fence *s);

	/*
	 * Used to install the fd in the corresponding OS. The underlying
	 * implementation varies from OS to OS.
	 */
	int (*install_fence)(struct nvgpu_os_fence *s, int fd);
	/*
	 * Increment a refcount of the underlying sync object. After this the
	 * struct nvgpu_os_fence object can be copied once. This call must be
	 * matched with a drop_ref as usual.
	 */
	void (*dup)(struct nvgpu_os_fence *s);
};

#if !defined(CONFIG_NVGPU_SYNCFD_NONE)

struct gk20a;
struct nvgpu_channel;

/*
 * The priv field contains the actual backend:
 * - struct sync_fence for semas on LINUX_VERSION <= 4.14
 * - struct dma_fence for semas on LINUX_VERSION > 4.14
 * - struct dma_fence syncpt-backed fences on all kernel versions
 */
struct nvgpu_os_fence {
	void *priv;
	struct gk20a *g;
	const struct nvgpu_os_fence_ops *ops;
};

/*
 * This API is used to validate the nvgpu_os_fence
 */
static inline bool nvgpu_os_fence_is_initialized(struct nvgpu_os_fence *fence)
{
	return (fence->ops != NULL);
}

int nvgpu_os_fence_fdget(
	struct nvgpu_os_fence *fence_out,
	struct nvgpu_channel *c, int fd);

#else /* CONFIG_NVGPU_SYNCFD_NONE */

struct nvgpu_os_fence {
	const struct nvgpu_os_fence_ops *ops;
};

static inline bool nvgpu_os_fence_is_initialized(struct nvgpu_os_fence *fence)
{
	(void)fence;
	return false;
}

#endif /* CONFIG_NVGPU_SYNCFD_NONE */

#endif /* NVGPU_OS_FENCE_H */
