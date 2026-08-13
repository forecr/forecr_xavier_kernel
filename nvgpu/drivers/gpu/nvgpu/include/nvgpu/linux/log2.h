/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __LOG2_LINUX_H__
#define __LOG2_LINUX_H__

#include <linux/log2.h>

/**
 * @brief Integer logarithm for base 2.
 *
 * Calculates the log to the base 2 of input value \a x and returns the
 * integer value of the same.
 * Macro performs validation of the parameter.
 *
 * @param x [in]	The number to get the log for.
 *
 * @return Integer value of log to the base 2 of input \a x.
 */

#define nvgpu_ilog2(x)	({	\
				unsigned long result;	\
				nvgpu_assert(x > 0ULL);	\
				result = nvgpu_safe_cast_s32_to_u64(ilog2(x));	\
				result;				\
			})
#endif
