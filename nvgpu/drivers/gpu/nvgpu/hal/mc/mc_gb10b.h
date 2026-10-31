/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_MC_GB10B_H
#define NVGPU_MC_GB10B_H

#include <nvgpu/types.h>

#define MC_ENGINE_RESET_DELAY_US	500U

struct gk20a;

int gb10b_mc_poll_device_enable(struct gk20a *g, u32 reg_idx,
			u32 poll_val);
int gb10b_mc_enable_dev(struct gk20a *g, const struct nvgpu_device *dev,
			bool enable);
int gb10b_mc_enable_devtype(struct gk20a *g, u32 devtype, bool enable);
bool gb10b_mc_is_enabled(struct gk20a *g, u32 unit);

#endif /* NVGPU_MC_GB10B_H */
