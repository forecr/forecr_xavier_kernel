/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FUSE_GP106_H
#define NVGPU_FUSE_GP106_H

struct gk20a;

u32 gp106_fuse_read_vin_cal_fuse_rev(struct gk20a *g);
int gp106_fuse_read_vin_cal_slope_intercept_fuse(struct gk20a *g,
					     u32 vin_id, u32 *slope,
					     u32 *intercept);
int gp106_fuse_read_vin_cal_gain_offset_fuse(struct gk20a *g,
					     u32 vin_id, s8 *gain,
					     s8 *offset);

#endif /* NVGPU_FUSE_GP106_H */
