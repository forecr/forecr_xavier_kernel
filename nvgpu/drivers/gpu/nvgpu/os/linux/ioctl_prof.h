/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef LINUX_IOCTL_PROF_H
#define LINUX_IOCTL_PROF_H

#include <nvgpu/types.h>

struct inode;
struct file;
#if defined(CONFIG_NVGPU_NON_FUSA)
struct nvgpu_profiler_object;
#endif

int nvgpu_prof_dev_fops_open(struct inode *inode, struct file *filp);
int nvgpu_prof_ctx_fops_open(struct inode *inode, struct file *filp);
int nvgpu_prof_fops_release(struct inode *inode, struct file *filp);
long nvgpu_prof_fops_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg);

#endif /* LINUX_IOCTL_PROF_H */
