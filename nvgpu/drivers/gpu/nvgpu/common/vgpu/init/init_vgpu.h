/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef INIT_VGPU_H
#define INIT_VGPU_H

struct gk20a;

u64 vgpu_connect(struct gk20a *g);
void vgpu_remove_support_common(struct gk20a *g);
int vgpu_init_gpu_characteristics(struct gk20a *g);
int vgpu_get_constants(struct gk20a *g);
int vgpu_finalize_poweron_common(struct gk20a *g);

#endif
