/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __NVGPU_NVHOST_PRIV_H__
#define __NVGPU_NVHOST_PRIV_H__

#include <nvgpu/os_fence_syncpts.h>

struct dma_fence;
struct nvgpu_nvhost_dev {
	struct platform_device *host1x_pdev;
	struct gk20a *g;
};

int nvgpu_nvhost_fence_install(struct dma_fence *f, int fd);
void nvgpu_nvhost_fence_put(struct dma_fence *f);
void nvgpu_nvhost_fence_dup(struct dma_fence *f);
struct dma_fence *nvgpu_nvhost_fence_create(struct platform_device *pdev,
					struct nvhost_ctrl_sync_fence_info *pts,
					u32 num_pts, const char *name);
u32 nvgpu_nvhost_fence_num_pts(struct dma_fence *fence);
int nvgpu_nvhost_fence_foreach_pt(struct dma_fence *fence,
	int (*iter)(struct nvhost_ctrl_sync_fence_info, void *),
	void *data);
bool nvgpu_nvhost_fence_is_signaled(struct dma_fence *fence);
#endif /* __NVGPU_NVHOST_PRIV_H__ */
