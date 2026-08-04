/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_JOB_H
#define NVGPU_JOB_H

#include <nvgpu/list.h>
#include <nvgpu/fence.h>

struct priv_cmd_entry;
struct nvgpu_mapped_buf;
struct priv_cmd_entry;
struct nvgpu_channel;

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT

struct nvgpu_channel_job {
	struct nvgpu_mapped_buf **mapped_buffers;
	u32 num_mapped_buffers;
	struct nvgpu_fence_type post_fence;
	struct nvgpu_semaphore *gpfifo_sema;
	struct priv_cmd_entry *wait_cmd;
	struct priv_cmd_entry *incr_cmd;
	struct priv_cmd_entry *gpfifo_incr_cmd;
	struct nvgpu_list_node list;
};

int nvgpu_channel_alloc_job(struct nvgpu_channel *c,
		struct nvgpu_channel_job **job_out);
void nvgpu_channel_free_job(struct nvgpu_channel *c,
		struct nvgpu_channel_job *job);

void nvgpu_channel_joblist_lock(struct nvgpu_channel *c);
void nvgpu_channel_joblist_unlock(struct nvgpu_channel *c);
struct nvgpu_channel_job *nvgpu_channel_joblist_peek(struct nvgpu_channel *c);
void nvgpu_channel_joblist_add(struct nvgpu_channel *c,
		struct nvgpu_channel_job *job);
void nvgpu_channel_joblist_delete(struct nvgpu_channel *c,
		struct nvgpu_channel_job *job);

int nvgpu_channel_joblist_init(struct nvgpu_channel *c, u32 num_jobs);
void nvgpu_channel_joblist_deinit(struct nvgpu_channel *c);
#endif /* CONFIG_NVGPU_KERNEL_MODE_SUBMIT */

#endif
