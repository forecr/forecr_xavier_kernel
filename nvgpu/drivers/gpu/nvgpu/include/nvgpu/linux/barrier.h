/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __NVGPU_BARRIER_LINUX_H__
#define __NVGPU_BARRIER_LINUX_H__

#include <asm/barrier.h>

#define nvgpu_mb_impl()	mb()
#define nvgpu_rmb_impl()	rmb()
#define nvgpu_wmb_impl()	wmb()

#define nvgpu_smp_mb_impl()	smp_mb()
#define nvgpu_smp_rmb_impl()	smp_rmb()
#define nvgpu_smp_wmb_impl()	smp_wmb()

#ifdef speculation_barrier
#define nvgpu_speculation_barrier_impl() speculation_barrier()
#else
#define nvgpu_speculation_barrier_impl() ({		\
	asm volatile("dsb sy\n"				\
		      "isb\n" : : : "memory"); })
#endif

#endif /* __NVGPU_BARRIER_LINUX_H__ */
