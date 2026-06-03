/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef THERM_GP10B_H
#define THERM_GP10B_H

struct gk20a;
int gp10b_init_therm_setup_hw(struct gk20a *g);
int gp10b_elcg_init_idle_filters(struct gk20a *g);

#endif /* THERM_GP10B_H */
