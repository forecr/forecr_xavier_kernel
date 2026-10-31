/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FIFO_VGPU_H
#define NVGPU_FIFO_VGPU_H

#include <nvgpu/types.h>

struct gk20a;
struct tegra_vgpu_fifo_intr_info;

int vgpu_fifo_setup_sw(struct gk20a *g);
void vgpu_fifo_cleanup_sw(struct gk20a *g);
int vgpu_fifo_isr(struct gk20a *g, struct tegra_vgpu_fifo_intr_info *info);

#endif /* NVGPU_FIFO_VGPU_H */
