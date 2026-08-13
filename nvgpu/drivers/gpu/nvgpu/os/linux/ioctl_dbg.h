/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef DBG_GPU_IOCTL_GK20A_H
#define DBG_GPU_IOCTL_GK20A_H

struct inode;
struct file;
typedef struct poll_table_struct poll_table;

/* NVGPU_DBG_GPU_IOCTL_REG_OPS: the upper limit for the number
 * of regops */
#define NVGPU_IOCTL_DBG_REG_OPS_LIMIT 1024

/* module debug driver interface */
int gk20a_dbg_gpu_dev_release(struct inode *inode, struct file *filp);
int gk20a_dbg_gpu_dev_open(struct inode *inode, struct file *filp);
long gk20a_dbg_gpu_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
unsigned int gk20a_dbg_gpu_dev_poll(struct file *filep, poll_table *wait);

/* used by profiler driver interface */
int gk20a_prof_gpu_dev_open(struct inode *inode, struct file *filp);

u32 nvgpu_get_regops_op_values_common(u32 regops_op);
u32 nvgpu_get_regops_status_values_common(u32 regops_status);
u32 nvgpu_get_regops_op_values_linux(u32 regops_op);
u32 nvgpu_get_regops_status_values_linux(u32 regops_status);

#endif
