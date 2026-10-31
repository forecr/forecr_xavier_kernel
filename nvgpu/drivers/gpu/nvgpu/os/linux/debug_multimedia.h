/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __NVGPU_DEBUG_MULTIMEDIA_H__
#define __NVGPU_DEBUG_MULTIMEDIA_H__

struct gk20a;
struct nvgpu_device;
struct nvgpu_debug_context;

int nvgpu_multimedia_debugfs_init(struct gk20a *g);
int nvgpu_multimedia_debug_common(struct gk20a *g, const struct nvgpu_device *dev,
				struct nvgpu_debug_context *o);

#endif /* __NVGPU_DEBUG_MULTIMEDIA_H__ */
