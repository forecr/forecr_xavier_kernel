/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LTC_INTR_GM20B
#define NVGPU_LTC_INTR_GM20B

#include <nvgpu/types.h>

struct gk20a;

void gm20b_ltc_intr_configure(struct gk20a *g);
int gm20b_ltc_intr_isr(struct gk20a *g, u32 ltc);

#endif
