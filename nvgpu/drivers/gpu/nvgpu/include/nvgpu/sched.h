/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_SCHED_COMMON_H
#define NVGPU_SCHED_COMMON_H

#include <nvgpu/types.h>
#include <nvgpu/lock.h>
#include <nvgpu/cond.h>

struct nvgpu_sched_ctrl {
	struct nvgpu_mutex control_lock;
	bool control_locked;
	bool sw_ready;
	struct nvgpu_mutex status_lock;
	struct nvgpu_mutex busy_lock;

	u64 status;

	size_t bitmap_size;
	u64 *active_tsg_bitmap;
	u64 *recent_tsg_bitmap;
	u64 *ref_tsg_bitmap;

	struct nvgpu_cond readout_wq;
};

#endif /* NVGPU_SCHED_COMMON_H */
