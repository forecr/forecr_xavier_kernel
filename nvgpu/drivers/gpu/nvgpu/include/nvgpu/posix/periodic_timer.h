/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_PERIODIC_TIMER_H
#define NVGPU_POSIX_PERIODIC_TIMER_H

#include <nvgpu/cond.h>
#include <signal.h>
#include <time.h>

struct nvgpu_periodic_timer {
	timer_t timerid;
	void (*fn)(void *arg);
	void *arg;
	struct itimerspec ts;
	bool enabled;
	bool last_run_done;
	struct nvgpu_cond cond;
};

#endif
