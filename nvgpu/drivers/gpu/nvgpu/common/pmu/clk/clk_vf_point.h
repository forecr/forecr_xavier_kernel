/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLK_VF_POINT_H
#define NVGPU_CLK_VF_POINT_H

int clk_vf_point_init_pmupstate(struct gk20a *g);
void clk_vf_point_free_pmupstate(struct gk20a *g);
int clk_vf_point_sw_setup(struct gk20a *g);
int clk_vf_point_pmu_setup(struct gk20a *g);

#endif /* NVGPU_CLK_VF_POINT_H */
