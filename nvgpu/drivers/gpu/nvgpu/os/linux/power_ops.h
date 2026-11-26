/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POWER_LINUX_H
#define NVGPU_POWER_LINUX_H

#include <nvgpu/types.h>

int gk20a_power_open(struct inode *inode, struct file *filp);
ssize_t gk20a_power_read(struct file *filp, char __user *buf,
		                size_t size, loff_t *off);
ssize_t gk20a_power_write(struct file *filp, const char __user *buf,
		                size_t size, loff_t *off);
int gk20a_power_release(struct inode *inode, struct file *filp);

#endif
