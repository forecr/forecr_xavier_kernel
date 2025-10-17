/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_NVS_IMPL_H
#define NVGPU_NVS_IMPL_H

#include <nvgpu/kmem.h>
#include <nvgpu/string.h>
#include <nvgpu/timers.h>
#include <nvgpu/log.h>

#define nvs_malloc(sched, size)					\
	nvgpu_kmalloc((struct gk20a *)(sched)->priv, (size))

#define nvs_free(sched, ptr)					\
	nvgpu_kfree((struct gk20a *)(sched)->priv, (ptr))

#define nvs_memset(ptr, value, length)				\
	memset((ptr), (value), (length))

#define nvs_timestamp()						\
	nvgpu_current_time_ns()

#define nvs_log(sched, fmt, args...)				\
	nvgpu_log((struct gk20a *)(sched)->priv,		\
		  gpu_dbg_nvs_internal, fmt, ##args)


#endif
