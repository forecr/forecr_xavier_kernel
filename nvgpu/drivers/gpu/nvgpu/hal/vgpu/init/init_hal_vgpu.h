/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef INIT_HAL_VGPU_H
#define INIT_HAL_VGPU_H

struct gk20a;

int vgpu_init_hal(struct gk20a *g);
void vgpu_detect_chip(struct gk20a *g);

#endif
