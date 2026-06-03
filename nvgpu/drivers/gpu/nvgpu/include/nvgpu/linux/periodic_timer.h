/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LINUX_PERIODIC_TIMER_H
#define NVGPU_LINUX_PERIODIC_TIMER_H

#include <linux/async.h>
#include <linux/hrtimer.h>

struct nvgpu_periodic_timer {
	struct hrtimer timer;
	void (*fn)(void *arg);
	void *arg;
	ktime_t interval;
	async_cookie_t async_cookie;
	bool enabled;
	raw_spinlock_t lock;

};

#endif