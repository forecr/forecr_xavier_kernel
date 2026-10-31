/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_SYSFS_H
#define NVGPU_SYSFS_H

struct device;

int nvgpu_create_sysfs(struct device *dev);
void nvgpu_remove_sysfs(struct device *dev);

#endif
