/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_SIZES_H
#define NVGPU_POSIX_SIZES_H

/**
 * Define for size equal to 0x100.
 */
#define SZ_256		256UL

/**
 * Define for size equal to 0x400(1K).
 */
#define SZ_1K		(1UL << 10)

/**
 * Define for size equal to 0x1000(4K).
 */
#define SZ_4K		(SZ_1K << 2)

/**
 * Define for size equal to 0x10000(64K).
 */
#define SZ_64K		(SZ_1K << 6)

/**
 * Define for size equal to 0x20000(128K).
 */
#define SZ_128K		(SZ_1K << 7)

/**
 * Define for size equal to 0x100000(1M).
 */
#define SZ_1M		(1UL << 20)

/**
 * Define for size equal to 0x1000000(2M).
 */
#define SZ_2M		(1UL << 21)

/**
 * Define for size equal to 0x1000000(16M).
 */
#define SZ_16M		(SZ_1M << 4)

/**
 * Define for size equal to 0x2000000(32M).
 */
#define SZ_32M		(SZ_1M << 5)

/**
 * Define for size equal to 0x10000000(256M).
 */
#define SZ_256M		(SZ_1M << 8)

/**
 * Define for size equal to 0x20000000(512M).
 */
#define SZ_512M		(SZ_1M << 9)

/**
 * Define for size equal to 0x40000000(1G).
 */
#define SZ_1G		(1UL << 30)

/**
 * Define for size equal to 0x100000000(4G).
 */
#define SZ_4G		(SZ_1G << 2)

#endif /* NVGPU_POSIX_SIZES_H */
