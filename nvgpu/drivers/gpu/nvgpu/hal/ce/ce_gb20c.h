/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CE_GB20C_H
#define NVGPU_CE_GB20C_H

struct gk20a;

void gb20c_ce_init_pce2lce_configs(struct gk20a *g, u32 *pce2lce_configs);
void gb20c_ce_init_grce_configs(struct gk20a *g, u32 *grce_configs);
u32 gb20c_grce_config_size(void);

#endif
