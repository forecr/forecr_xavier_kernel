/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_FENCE_H
#define NVGPU_FENCE_H

#include <nvgpu/types.h>
#include <nvgpu/kref.h>
#include <nvgpu/os_fence.h>

struct gk20a;
struct nvgpu_channel;
struct platform_device;
#ifdef CONFIG_NVGPU_SW_SEMAPHORE
struct nvgpu_semaphore;
#endif
struct nvgpu_os_fence;
struct nvgpu_user_fence;
struct nvgpu_fence_ops;

struct nvgpu_fence_type_priv {
	/* Valid for all fence types: */
	struct nvgpu_ref ref;
	const struct nvgpu_fence_ops *ops;

	struct nvgpu_os_fence os_fence;

#ifdef CONFIG_NVGPU_SW_SEMAPHORE
	/* Valid for fences created from semaphores: */
	struct nvgpu_semaphore *semaphore;
	struct nvgpu_cond *semaphore_wq;
#endif

#ifdef CONFIG_TEGRA_GK20A_NVHOST
	/* Valid for fences created from syncpoints: */
	struct nvgpu_nvhost_dev *nvhost_device;
	u32 syncpt_id;
	u32 syncpt_value;
#endif
};

struct nvgpu_fence_type {
	/*
	 * struct nvgpu_fence_type needs to be allocated outside fence code for
	 * performance. It's technically possible to peek inside this priv
	 * data, but it's called priv for a reason. Don't touch it; use the
	 * public API (nvgpu_fence_*()).
	 */
	struct nvgpu_fence_type_priv priv;
};

/* Fence operations */
void nvgpu_fence_put(struct nvgpu_fence_type *f);
struct nvgpu_fence_type *nvgpu_fence_get(struct nvgpu_fence_type *f);
int  nvgpu_fence_wait(struct gk20a *g, struct nvgpu_fence_type *f, u32 timeout);
bool nvgpu_fence_is_expired(struct nvgpu_fence_type *f);
struct nvgpu_user_fence nvgpu_fence_extract_user(struct nvgpu_fence_type *f);

#endif /* NVGPU_FENCE_H */
