/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_FENCE_SYNCPT_H
#define NVGPU_FENCE_SYNCPT_H

#ifdef CONFIG_TEGRA_GK20A_NVHOST

#include <nvgpu/types.h>
#include <nvgpu/os_fence.h>

struct nvgpu_fence_type;
struct nvgpu_nvhost_dev;

void nvgpu_fence_from_syncpt(
		struct nvgpu_fence_type *f,
		struct nvgpu_nvhost_dev *nvhost_device,
		u32 id, u32 value,
		struct nvgpu_os_fence os_fence);

#endif /* CONFIG_TEGRA_GK20A_NVHOST */

#endif
