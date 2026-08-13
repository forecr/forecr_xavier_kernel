/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef THERM_GM20B_H
#define THERM_GM20B_H

struct gk20a;
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
int gm20b_init_therm_setup_hw(struct gk20a *g);
int gm20b_elcg_init_idle_filters(struct gk20a *g);
void gm20b_therm_init_elcg_mode(struct gk20a *g, u32 mode, u32 engine);
void gm20b_therm_throttle_enable(struct gk20a *g, u32 val);
u32 gm20b_therm_throttle_disable(struct gk20a *g);
void gm20b_therm_idle_slowdown_enable(struct gk20a *g, u32 val);
u32 gm20b_therm_idle_slowdown_disable(struct gk20a *g);
#endif /* CONFIG_NVGPU_HAL_NON_FUSA */
void gm20b_therm_init_blcg_mode(struct gk20a *g, u32 mode, u32 engine);

#endif /* THERM_GM20B_H */
