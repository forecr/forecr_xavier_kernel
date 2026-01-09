/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef _VGPU_PLATFORM_H_
#define _VGPU_PLATFORM_H_

long vgpu_plat_clk_round_rate(struct device *dev, unsigned long rate);
int vgpu_plat_clk_get_freqs(struct device *dev, unsigned long **freqs,
			int *num_freqs);
int vgpu_plat_clk_cap_rate(struct device *dev, unsigned long rate);
#endif
