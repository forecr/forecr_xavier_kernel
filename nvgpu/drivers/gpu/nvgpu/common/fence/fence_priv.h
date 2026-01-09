/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_FENCE_PRIV_H
#define NVGPU_FENCE_PRIV_H

#include <nvgpu/os_fence.h>

struct nvgpu_fence_type;

struct nvgpu_fence_ops {
	int (*wait)(struct nvgpu_fence_type *f, u32 timeout);
	bool (*is_expired)(struct nvgpu_fence_type *f);
	void (*release)(struct nvgpu_fence_type *f);
};

void nvgpu_fence_init(struct nvgpu_fence_type *f,
		const struct nvgpu_fence_ops *ops,
		struct nvgpu_os_fence os_fence);

#endif
