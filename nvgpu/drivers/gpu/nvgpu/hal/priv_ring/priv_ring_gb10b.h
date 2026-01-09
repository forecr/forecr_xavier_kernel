/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PRIV_RING_GB10B_H
#define NVGPU_PRIV_RING_GB10B_H

#include <nvgpu/types.h>
#include <nvgpu/static_analysis.h>

int gb10b_priv_ring_enable(struct gk20a *g);
void gb10b_priv_ring_intr_retrigger(struct gk20a *g);
#ifdef CONFIG_NVGPU_MIG
int gb10b_priv_ring_config_gpc_rs_map(struct gk20a *g, bool enable);
#endif
#endif
