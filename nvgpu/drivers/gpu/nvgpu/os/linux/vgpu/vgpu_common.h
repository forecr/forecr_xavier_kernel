/*
  * Copyright (c) 2023, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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