/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PERIODIC_TIMER_H
#define NVGPU_PERIODIC_TIMER_H

#ifdef CONFIG_NVGPU_FECS_TRACE
#ifdef __KERNEL__
#include <nvgpu/linux/periodic_timer.h>
#elif defined(NVGPU_HVRTOS)
#include <nvgpu_hvrtos/periodic_timer.h>
#else
#include <nvgpu/posix/periodic_timer.h>
#endif

#include <nvgpu/types.h>

/**
 * @brief Initialize a nvgpu_periodic_timer
 *
 * @param timer The timer to be initialized
 * @param fn Timer callback function, must not be NULL
 * @param arg The argument of the timer callback function
 * @return 0 Success
 * @return -EAGAIN Temporary error during kernel allocation of timer structure
 * @return -EINVAL OS specific implementation error
 * @return -ENOMEM out of memory
 */
int nvgpu_periodic_timer_init(struct nvgpu_periodic_timer *timer,
			void (*fn)(void *arg), void *arg);
/**
 * @brief start a nvgpu_periodic_timer. Not thread safe.
 *
 * @param timer timer to start, must not be NULL
 * @param interval_ns periodic timer interval in the unit of ns
 * @return 0 Success
 * @return -EINVAL invalid timer or interval_ns
 */
int nvgpu_periodic_timer_start(struct nvgpu_periodic_timer *timer,
				u64 interval_ns);
/**
 * @brief stop a timer and wait for any timer callback to be finished.
 * Not thread safe.
 *
 * @param timer timer to stop
 * @return 0 Success
 * @return -EINVAL invalid timer
 */
int nvgpu_periodic_timer_stop(struct nvgpu_periodic_timer *timer);
/**
 * @brief destroy a nvgpu_periodic_timer
 *
 * @param timer timer to destroy
 * @return 0 Success
 * @return -EINVAL invalid timer
 */
int nvgpu_periodic_timer_destroy(struct nvgpu_periodic_timer *timer);
#endif
#endif
