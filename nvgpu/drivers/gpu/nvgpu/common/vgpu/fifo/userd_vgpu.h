/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef USERD_VGPU_H
#define USERD_VGPU_H

struct gk20a;

int vgpu_userd_setup_sw(struct gk20a *g);
void vgpu_userd_cleanup_sw(struct gk20a *g);

#endif /* USERD_VGPU_H */
