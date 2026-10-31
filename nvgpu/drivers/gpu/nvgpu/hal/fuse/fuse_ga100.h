/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FUSE_GA100_H
#define NVGPU_FUSE_GA100_H

struct gk20a;

u32 ga100_fuse_status_opt_l2_fbp(struct gk20a *g, u32 fbp);
int ga100_fuse_read_per_device_identifier(struct gk20a *g, u64 *pdi);
int ga100_read_ucode_version(struct gk20a *g, u32 falcon_id,
		u32 *ucode_version);

#endif /* NVGPU_FUSE_GA100_H */
