/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef __NVGPU_SCHED_H
#define __NVGPU_SCHED_H

struct gk20a;
struct gpu_ops;
struct nvgpu_tsg;
struct poll_table_struct;

int gk20a_sched_dev_release(struct inode *inode, struct file *filp);
int gk20a_sched_dev_open(struct inode *inode, struct file *filp);
long gk20a_sched_dev_ioctl(struct file *, unsigned int, unsigned long);
ssize_t gk20a_sched_dev_read(struct file *, char __user *, size_t, loff_t *);
unsigned int gk20a_sched_dev_poll(struct file *, struct poll_table_struct *);

void gk20a_sched_ctrl_tsg_added(struct gk20a *, struct nvgpu_tsg *);
void gk20a_sched_ctrl_tsg_removed(struct gk20a *, struct nvgpu_tsg *);
int gk20a_sched_ctrl_init(struct gk20a *);

void gk20a_sched_ctrl_cleanup(struct gk20a *g);

#endif /* __NVGPU_SCHED_H */
