/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_BITOPS_H
#define NVGPU_BITOPS_H

#include <nvgpu/types.h>
#include <nvgpu/bug.h>
#include <nvgpu/utils.h>

/*
 * Explicit sizes for bit definitions. Please use these instead of BIT().
 */
#define BIT8(i)		(U8(1) << U8(i))
#define BIT16(i)	(U16(1) << U16(i))
#define BIT32(i)	(U32(1) << U32(i))
#define BIT64(i)	(U64(1) << U64(i))

#ifdef __KERNEL__
#include <nvgpu/linux/bitops.h>
#else
#include <nvgpu/posix/bitops.h>
#endif

/*
 * BITS_PER_BYTE is U64 data type.
 * Casting U64 to U32 results in certc_violation.
 * To avoid violation, define BITS_PER_BYTE_U32 as U32 data type
 */
#define BITS_PER_BYTE_U32	8U

#endif /* NVGPU_BITOPS_H */
