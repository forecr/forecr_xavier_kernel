/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef IOCTL_GSP_VM_SCHED_H
#define IOCTL_GSP_VM_SCHED_H

struct gk20a;

int nvgpu_gsp_vm_sched_dev_open(struct inode *inode, struct file *filp);
int nvgpu_gsp_vm_sched_dev_release(struct inode *inode, struct file *filp);
long nvgpu_gsp_vm_sched_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

#endif /* IOCTL_GSP_VM_SCHED_H */
