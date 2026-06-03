/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_CONFIG_GA10B_H
#define NVGPU_GR_CONFIG_GA10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_config;

u32 ga10b_gr_config_get_gpc_rop_mask(struct gk20a *g,
		struct nvgpu_gr_config *config, u32 gpc_index);
#endif /* NVGPU_GR_CONFIG_GA10B_H */
