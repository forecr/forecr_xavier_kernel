/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_OS_LINUX_IOCTL_NVS_H
#define NVGPU_OS_LINUX_IOCTL_NVS_H

#include <nvgpu/types.h>

struct inode;
struct file;

int     nvgpu_nvs_dev_open(struct inode *inode, struct file *filp);
int     nvgpu_nvs_dev_release(struct inode *inode, struct file *filp);
long    nvgpu_nvs_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
ssize_t nvgpu_nvs_dev_read(struct file *filp, char __user *buf,
			   size_t size, loff_t *off);
struct nvgpu_nvs_domain *nvgpu_nvs_domain_get_from_file(int fd);

int     nvgpu_nvs_ctrl_fifo_ops_open(struct inode *inode, struct file *filp);
int     nvgpu_nvs_ctrl_fifo_ops_release(struct inode *inode, struct file *filp);
long    nvgpu_nvs_ctrl_fifo_ops_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

#endif
