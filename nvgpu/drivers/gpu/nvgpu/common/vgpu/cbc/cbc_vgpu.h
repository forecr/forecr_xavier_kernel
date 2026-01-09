/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CBC_VGPU_H
#define NVGPU_CBC_VGPU_H

#ifdef CONFIG_NVGPU_COMPRESSION

struct gk20a;
struct nvgpu_cbc;

int vgpu_cbc_alloc_comptags(struct gk20a *g, struct nvgpu_cbc *cbc);

#endif
#endif /* NVGPU_CBC_VGPU_H */
