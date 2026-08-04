/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_TYPES_H
#define NVGPU_POSIX_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdarg.h>

/*
 * For endianness functions.
 */
#include <netinet/in.h>

/**
 * Alias for unsigned 8 bit char.
 */
typedef unsigned char		u8;

/**
 * Alias for unsigned 16 bit short.
 */
typedef unsigned short		u16;

/**
 * Alias for unsigned 32 bit int.
 */
typedef unsigned int		u32;

/**
 * Alias for unsigned 64 bit long long.
 */
typedef unsigned long long	u64;

/**
 * Alias for signed 8 bit char.
 */
typedef signed char		s8;

/**
 * Alias for signed 16 bit short.
 */
typedef signed short		s16;

/**
 * Alias for signed 32 bit int.
 */
typedef signed int		s32;

/**
 * Alias for signed 64 bit long long.
 */
typedef signed long long	s64;

#endif /* NVGPU_POSIX_TYPES_H */
