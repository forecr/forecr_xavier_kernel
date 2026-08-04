/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FECS_TRACE_LINUX_H
#define NVGPU_FECS_TRACE_LINUX_H

#include <nvgpu/types.h>

#define GK20A_CTXSW_TRACE_NUM_DEVS			1
#define GK20A_CTXSW_TRACE_MAX_VM_RING_SIZE	(128*NVGPU_CPU_PAGE_SIZE)

struct file;
struct inode;
struct gk20a;
struct nvgpu_tsg;
struct nvgpu_channel;
struct vm_area_struct;
struct poll_table_struct;

int gk20a_ctxsw_trace_init(struct gk20a *g);
void gk20a_ctxsw_trace_cleanup(struct gk20a *g);

int gk20a_ctxsw_dev_mmap(struct file *filp, struct vm_area_struct *vma);

int gk20a_ctxsw_dev_release(struct inode *inode, struct file *filp);
int gk20a_ctxsw_dev_open(struct inode *inode, struct file *filp);
long gk20a_ctxsw_dev_ioctl(struct file *filp,
			 unsigned int cmd, unsigned long arg);
ssize_t gk20a_ctxsw_dev_read(struct file *filp, char __user *buf,
			     size_t size, loff_t *offs);
unsigned int gk20a_ctxsw_dev_poll(struct file *filp,
				  struct poll_table_struct *pts);

#endif /*NVGPU_FECS_TRACE_LINUX_H */
