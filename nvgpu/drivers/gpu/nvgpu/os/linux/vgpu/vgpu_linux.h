/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __VGPU_LINUX_H__
#define __VGPU_LINUX_H__

struct device;
struct platform_device;

#ifdef CONFIG_NVGPU_GR_VIRTUALIZATION

#include <nvgpu/vgpu/vgpu.h>

int vgpu_probe(struct platform_device *dev);
int vgpu_remove(struct platform_device *dev);

void vgpu_create_sysfs(struct device *dev);
void vgpu_remove_sysfs(struct device *dev);

int vgpu_tegra_suspend(struct device *dev);
int vgpu_tegra_resume(struct device *dev);
#else
/* define placeholders for functions used outside of vgpu */

static inline int vgpu_probe(struct platform_device *dev)
{
	return -ENOSYS;
}
static inline int vgpu_remove(struct platform_device *dev)
{
	return -ENOSYS;
}
static inline int vgpu_tegra_suspend(struct device *dev)
{
	return -ENOSYS;
}
static inline int vgpu_tegra_resume(struct device *dev)
{
	return -ENOSYS;
}
#endif

#endif
