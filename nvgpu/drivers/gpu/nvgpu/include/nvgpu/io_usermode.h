/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_IO_USERMODE_H
#define NVGPU_IO_USERMODE_H

/**
 * @file
 *
 * Interface for usermode ops.
 */

/**
 * @brief Write a value to an usermode mapped io-region.
 *
 * @param g [in]	GPU super structure.
 * @param r [in]	Register offset in io-region.
 * @param v [in]	Value to write at the offset.
 *
 * Write a 32-bit value to register offset in usermode region.
 * @return None.
 */
void nvgpu_usermode_writel(struct gk20a *g, u32 r, u32 v);

#endif /* NVGPU_IO_USERMODE_H */
