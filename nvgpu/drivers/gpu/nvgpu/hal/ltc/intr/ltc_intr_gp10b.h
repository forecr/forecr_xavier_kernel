/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LTC_INTR_GP10B
#define NVGPU_LTC_INTR_GP10B

#include <nvgpu/types.h>

struct gk20a;

void gp10b_ltc_intr_handle_lts_interrupts(struct gk20a *g, u32 ltc, u32 slice);
#ifdef CONFIG_NVGPU_FALCON_NON_FUSA
void gp10b_ltc_intr_configure(struct gk20a *g);
int gp10b_ltc_intr_isr(struct gk20a *g, u32 ltc);
#endif

#endif
