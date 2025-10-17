/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_XAL_EP_GB10B_H
#define NVGPU_XAL_EP_GB10B_H

#include <nvgpu/types.h>

struct gk20a;

int gb10b_xal_ep_init_hw(struct gk20a *g);
void gb10b_xal_ep_isr(struct gk20a *g);

#endif /* NVGPU_XAL_EP_GB10B_H */
