/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ERRNO_H
#define NVGPU_ERRNO_H

/*
 * Explicit include to get all the -E* error messages. Useful for header files
 * with static inlines that return error messages. In actual C code normally
 * enough Linux/QNX headers bleed in to get the error messages but header files
 * with sparse includes do not have this luxury.
 */

#ifdef __KERNEL__
#include <linux/errno.h>
#endif

/*
 * TODO: add else path above for QNX.
 */

#endif /* NVGPU_ERRNO_H */
