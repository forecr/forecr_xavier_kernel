/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_FENCE_SEMA_H
#define NVGPU_FENCE_SEMA_H

#ifdef CONFIG_NVGPU_SW_SEMAPHORE

void nvgpu_fence_from_semaphore(
		struct nvgpu_fence_type *f,
		struct nvgpu_semaphore *semaphore,
		struct nvgpu_cond *semaphore_wq,
		struct nvgpu_os_fence os_fence);

#endif /* CONFIG_NVGPU_SW_SEMAPHORE */

#endif
