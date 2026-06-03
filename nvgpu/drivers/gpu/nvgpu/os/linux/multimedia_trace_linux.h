/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_MULTIMEDIA_TRACE_LINUX_H
#define NVGPU_MULTIMEDIA_TRACE_LINUX_H

#include <nvgpu/types.h>

#define NVGPU_MULTIMEDIA_TRACE_MAX_VM_RING_SIZE	(128*NVGPU_CPU_PAGE_SIZE)

struct file;
struct inode;
struct gk20a;
struct vm_area_struct;
struct poll_table_struct;

int nvgpu_multimedia_trace_init(struct gk20a *g);
void nvgpu_multimedia_trace_cleanup(struct gk20a *g);

int nvgpu_multimedia_dev_mmap(struct file *filp, struct vm_area_struct *vma);

int nvgpu_multimedia_dev_release(struct inode *inode, struct file *filp);
int nvgpu_multimedia_dev_open(struct inode *inode, struct file *filp);
long nvgpu_multimedia_dev_ioctl(struct file *filp,
			 unsigned int cmd, unsigned long arg);
ssize_t nvgpu_multimedia_dev_read(struct file *filp, char __user *buf,
			     size_t size, loff_t *offs);
unsigned int nvgpu_multimedia_dev_poll(struct file *filp,
				  struct poll_table_struct *pts);

#endif /*NVGPU_MULTIMEDIA_TRACE_LINUX_H */
