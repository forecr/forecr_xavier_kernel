/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GB10B_PPRIV
#define NVGPU_GB10B_PPRIV

struct gk20a;
void gb10b_init_fbp_physical_logical_id_map(struct gk20a *g, u32 *fbp_physical_id_map);

#endif
