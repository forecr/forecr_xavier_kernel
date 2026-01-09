/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_OS_FENCE_PRIV_H
#define NVGPU_OS_FENCE_PRIV_H

#include <nvgpu/os_fence.h>

static inline void nvgpu_os_fence_clear(struct nvgpu_os_fence *fence_out)
{
	fence_out->priv = NULL;
	fence_out->g = NULL;
	fence_out->ops = NULL;
}

static inline void nvgpu_os_fence_init(struct nvgpu_os_fence *fence_out,
		struct gk20a *g, const struct nvgpu_os_fence_ops *fops,
		void *fence)
{
	fence_out->g = g;
	fence_out->ops = fops;
	fence_out->priv = fence;
}

#endif /* NVGPU_OS_FENCE_PRIV_H */
