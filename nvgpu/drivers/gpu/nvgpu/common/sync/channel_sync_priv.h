/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CHANNEL_SYNC_PRIV_H
#define NVGPU_CHANNEL_SYNC_PRIV_H

/*
 * These APIs are used for job synchronization that we know about in the
 * driver. If submits happen in userspace only, none of this will be needed and
 * won't be included. This is here just to double check for now.
 */
#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT

#include <nvgpu/atomic.h>
#include <nvgpu/types.h>

struct priv_cmd_entry;
struct nvgpu_fence_type;
struct nvgpu_channel_sync_ops;

/*
 * This struct is private and should not be used directly. Users should
 * instead use the public APIs starting with nvgpu_channel_sync_*
 */
struct nvgpu_channel_sync {
	nvgpu_atomic_t refcount;
	const struct nvgpu_channel_sync_ops *ops;
};

/*
 * This struct is private and should not be used directly. Users should
 * instead use the public APIs starting with nvgpu_channel_sync_*
 */
struct nvgpu_channel_sync_ops {
	int (*wait_fence_raw)(struct nvgpu_channel_sync *s, u32 id, u32 thresh,
			   struct priv_cmd_entry **entry);

	int (*wait_fence_fd)(struct nvgpu_channel_sync *s, int fd,
		       struct priv_cmd_entry **entry, u32 max_wait_cmds);

	int (*incr)(struct nvgpu_channel_sync *s,
		    struct priv_cmd_entry **entry,
		    struct nvgpu_fence_type *fence,
		    bool need_sync_fence);

	int (*incr_user)(struct nvgpu_channel_sync *s,
			 struct priv_cmd_entry **entry,
			 struct nvgpu_fence_type *fence,
			 bool wfi,
			 bool need_sync_fence);

	void (*mark_progress)(struct nvgpu_channel_sync *s,
			     bool register_irq);

	void (*set_min_eq_max)(struct nvgpu_channel_sync *s);

	void (*destroy)(struct nvgpu_channel_sync *s);
};

#endif /* CONFIG_NVGPU_KERNEL_MODE_SUBMIT */

#endif /* NVGPU_CHANNEL_SYNC_PRIV_H */
