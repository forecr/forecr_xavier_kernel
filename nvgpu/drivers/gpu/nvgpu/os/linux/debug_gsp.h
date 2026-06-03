/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef __NVGPU_DEBUG_GSP_H__
#define __NVGPU_DEBUG_GSP_H__

struct gk20a;

void nvgpu_gsp_debugfs_fini(struct gk20a *g);
int nvgpu_gsp_debugfs_init(struct gk20a *g);

#endif /* __NVGPU_DEBUG_GSP_H__ */
