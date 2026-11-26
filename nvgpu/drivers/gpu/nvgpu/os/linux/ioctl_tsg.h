/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_IOCTL_TSG_H
#define NVGPU_IOCTL_TSG_H

struct inode;
struct file;
struct gk20a;
struct nvgpu_ref;
struct nvgpu_cdev;
struct gk20a_ctrl_priv;

struct nvgpu_tsg *nvgpu_tsg_get_from_file(int fd);

int nvgpu_ioctl_tsg_dev_release(struct inode *inode, struct file *filp);
int nvgpu_ioctl_tsg_dev_open(struct inode *inode, struct file *filp);
int nvgpu_ioctl_tsg_open(struct gk20a *g, struct gk20a_ctrl_priv *ctrl_priv,
			 struct nvgpu_cdev *cdev, struct file *filp,
			 bool open_share, u64 source_device_instance_id,
			 u64 target_device_instance_id, u64 token);
long nvgpu_ioctl_tsg_dev_ioctl(struct file *filp,
			       unsigned int cmd, unsigned long arg);
void nvgpu_ioctl_tsg_release(struct nvgpu_ref *ref);

#endif
