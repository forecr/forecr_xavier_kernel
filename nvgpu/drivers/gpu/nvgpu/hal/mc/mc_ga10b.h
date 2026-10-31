/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_MC_GA10B_H
#define NVGPU_MC_GA10B_H

#include <nvgpu/types.h>

#define MC_UNIT_RESET_DELAY_US		20U
#define MC_ENGINE_RESET_DELAY_US	500U

struct gk20a;

int ga10b_mc_poll_device_enable(struct gk20a *g, u32 reg_idx,
			u32 poll_val);
int ga10b_mc_enable_units(struct gk20a *g, u32 units, bool enable);
int ga10b_mc_enable_dev(struct gk20a *g, const struct nvgpu_device *dev,
			bool enable);
int ga10b_mc_enable_devtype(struct gk20a *g, u32 devtype, bool enable);
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void ga10b_mc_elpg_enable(struct gk20a *g);
#endif

#endif /* NVGPU_MC_GA10B_H */
