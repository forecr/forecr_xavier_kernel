/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ECC_GP10B_H
#define NVGPU_ECC_GP10B_H

struct gk20a;

void gp10b_ecc_detect_enabled_units(struct gk20a *g);
int gp10b_gr_ecc_init(struct gk20a *g);
void gp10b_gr_ecc_deinit(struct gk20a *g);

#endif /* NVGPU_ECC_GP10B_H */
