/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef THERM_GA10B_H
#define THERM_GA10B_H

struct gk20a;

u32 ga10b_therm_max_fpdiv_factor(void);
u32 ga10b_therm_grad_stepping_pdiv_duration(void);
int ga10b_elcg_init_idle_filters(struct gk20a *g);

#endif /* THERM_GA10B_H */
