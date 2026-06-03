/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef __NVGPU_COMMON_LINUX_AS_H__
#define __NVGPU_COMMON_LINUX_AS_H__

struct inode;
struct file;
struct vm_gk20a;

/* MAP_BUFFER_BATCH_LIMIT: the upper limit for num_unmaps and
 * num_maps */
#define NVGPU_IOCTL_AS_MAP_BUFFER_BATCH_LIMIT	256

/* struct file_operations driver interface */
int gk20a_as_dev_open(struct inode *inode, struct file *filp);
int gk20a_as_dev_release(struct inode *inode, struct file *filp);
struct vm_gk20a *nvgpu_vm_get_from_file(int fd);
long gk20a_as_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

extern const struct file_operations gk20a_as_ops;

#endif
