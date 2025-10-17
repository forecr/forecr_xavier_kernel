/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef LINUX_VGPU_COMMON_H
#define LINUX_VGPU_COMMON_H

struct device;
struct nvgpu_os_linux;

#ifdef CONFIG_NVGPU_GR_VIRTUALIZATION

int vgpu_pm_prepare_poweroff(struct device *dev);
int vgpu_pm_finalize_poweron(struct device *dev);
int vgpu_probe_common(struct nvgpu_os_linux *l);

#else

static inline int vgpu_pm_prepare_poweroff(struct device *dev)
{
	return -ENOSYS;
}
static inline int vgpu_pm_finalize_poweron(struct device *dev)
{
	return -ENOSYS;
}

#endif

#endif