/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef THERM_GV11B_H
#define THERM_GV11B_H

struct gk20a;
int gv11b_elcg_init_idle_filters(struct gk20a *g);
int gv11b_init_therm_setup_hw(struct gk20a *g);
void gv11b_therm_init_elcg_mode(struct gk20a *g, u32 mode, u32 engine);
#ifdef CONFIG_NVGPU_NON_FUSA
u32 gv11b_therm_max_fpdiv_factor(void);
u32 gv11b_therm_grad_stepping_pdiv_duration(void);
#endif
#endif /* THERM_GV11B_H */
