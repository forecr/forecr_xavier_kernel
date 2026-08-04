/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

/* This file contains NVGPU_* high-level abstractions for various
 * memor-barrier operations available in linux/kernel. Every OS
 * should provide their own OS specific calls under this common API
 */

#ifndef NVGPU_BARRIER_H
#define NVGPU_BARRIER_H

#ifdef __KERNEL__
#include <nvgpu/linux/barrier.h>
#elif defined(__NVGPU_POSIX__)
#include <nvgpu/posix/barrier.h>
#elif defined(NVGPU_HVRTOS)
#include <nvgpu_hvrtos/barrier.h>
#else
#include <nvgpu/qnx/barrier.h>
#endif

#define nvgpu_mb()	nvgpu_mb_impl()
#define nvgpu_rmb()	nvgpu_rmb_impl()
#define nvgpu_wmb()	nvgpu_wmb_impl()

#define nvgpu_smp_mb()	nvgpu_smp_mb_impl()
#define nvgpu_smp_rmb()	nvgpu_smp_rmb_impl()
#define nvgpu_smp_wmb()	nvgpu_smp_wmb_impl()

/**
 * @brief Compilers can do optimizations assuming there is a single thread
 * executing the code. For example, a variable read in a loop from one thread
 * may not see the update from another thread because compiler has assumed that
 * it's value cannot change from the one initialized before the loop. There are
 * other possibilities like multiple references to a variable when the code
 * assumes that it should see a constant value. In general, this macro should
 * never be used by nvgpu driver code, and many of the current uses in driver
 * are likely wrong.
 * For more info see: URL = lwn.net/Articles/508991/
 *
 * @param x [in] variable that needs to turn into a volatile type temporarily.
 */
#define NV_READ_ONCE(x)		(*((volatile typeof(x) *)&x))

/**
 * @brief Ensure ordered writes.
 *
 * @param x    Variable to be updated.
 * @param y    Value to be written.
 *
 * Prevent compiler optimizations from mangling writes.
 */
#define NV_WRITE_ONCE(x, y)	(*((volatile typeof(x) *)(&(x))) = (y))

/*
 * Sometimes we want to prevent speculation.
 */
#ifdef __NVGPU_PREVENT_UNTRUSTED_SPECULATION
#define nvgpu_speculation_barrier()	nvgpu_speculation_barrier_impl()
#else
#define nvgpu_speculation_barrier()
#endif

#endif /* NVGPU_BARRIER_H */
