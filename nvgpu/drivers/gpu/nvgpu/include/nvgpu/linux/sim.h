/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __SIM_LINUX_H__
#define __SIM_LINUX_H__

struct platform_device;
struct gk20a_platform;

struct sim_nvgpu_linux {
	struct sim_nvgpu sim;
	struct resource *reg_mem;
	void (*remove_support_linux)(struct gk20a *g);
};

int nvgpu_init_sim_support_linux(struct gk20a *g,
		struct platform_device *dev);
int nvgpu_init_sim_support_linux_igpu_pci(struct gk20a *g,
		struct gk20a_platform *platform);
void nvgpu_remove_sim_support_linux(struct gk20a *g);
#endif
