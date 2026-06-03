/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FUSE_TU104_H
#define NVGPU_FUSE_TU104_H

struct gk20a;

int tu104_fuse_read_per_device_identifier(struct gk20a *g, u64 *pdi);

#endif /* NVGPU_FUSE_TU104_H */
