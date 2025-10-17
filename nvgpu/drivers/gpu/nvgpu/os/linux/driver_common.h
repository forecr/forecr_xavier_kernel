/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LINUX_DRIVER_COMMON
#define NVGPU_LINUX_DRIVER_COMMON

extern int nvgpu_lpwr_enable;
extern char *nvgpu_devfreq_timer;
extern char *nvgpu_devfreq_gov;

struct gk20a;

int nvgpu_probe(struct gk20a *g,
		const char *debugfs_symlink);

void nvgpu_init_gk20a(struct gk20a *g);
void nvgpu_read_support_gpu_tools(struct gk20a *g);
void nvgpu_devfreq_init(struct gk20a *g);

#endif
