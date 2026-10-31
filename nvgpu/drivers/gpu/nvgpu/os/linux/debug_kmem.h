/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __NVGPU_DEBUG_KMEM_H__
#define __NVGPU_DEBUG_KMEM_H__

struct gk20a;
#ifdef CONFIG_NVGPU_TRACK_MEM_USAGE
void nvgpu_kmem_debugfs_init(struct gk20a *g);
#endif

#endif /* __NVGPU_DEBUG_KMEM_H__ */
