/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef CTX_VGPU_H
#define CTX_VGPU_H

struct gk20a;
struct nvgpu_gr_ctx;
struct vm_gk20a;

void vgpu_gr_free_gr_ctx(struct gk20a *g,
			 struct nvgpu_gr_ctx *gr_ctx);

#endif
