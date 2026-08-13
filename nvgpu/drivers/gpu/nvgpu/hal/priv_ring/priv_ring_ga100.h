/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PRIV_RING_GA100_H
#define NVGPU_PRIV_RING_GA100_H

#include <nvgpu/types.h>

#ifdef CONFIG_NVGPU_MIG
int ga100_priv_ring_config_gpc_rs_map(struct gk20a *g, bool enable);
#endif

#endif /* NVGPU_PRIV_RING_GA100_H */
