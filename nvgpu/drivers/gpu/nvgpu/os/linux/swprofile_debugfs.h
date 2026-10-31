/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __NVGPU_SWPROFILE_DEBUGFS_H__
#define __NVGPU_SWPROFILE_DEBUGFS_H__

struct dentry;

struct gk20a;
struct nvgpu_swprofiler;

void nvgpu_debugfs_swprofile_init(struct gk20a *g,
				  struct dentry *root,
				  struct nvgpu_swprofiler *p,
				  const char *name);

#endif /* __NVGPU_SWPROFILE_DEBUGFS_H__ */
