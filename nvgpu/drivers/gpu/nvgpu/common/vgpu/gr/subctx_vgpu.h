/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_SUBCTX_VGPU_H
#define NVGPU_SUBCTX_VGPU_H

struct nvgpu_channel;

void vgpu_gr_setup_free_subctx(struct nvgpu_channel *c);

#endif /* NVGPU_SUBCTX_VGPU_H */
