/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_CONFIG_GV100_H
#define NVGPU_GR_CONFIG_GV100_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_config;

int gv100_gr_config_init_sm_id_table(struct gk20a *g,
		struct nvgpu_gr_config *gr_config);

#endif /* NVGPU_GR_CONFIG_GV100_H */
