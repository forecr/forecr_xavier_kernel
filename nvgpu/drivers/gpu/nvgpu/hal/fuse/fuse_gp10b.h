/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FUSE_GP10B_H
#define NVGPU_FUSE_GP10B_H

struct gk20a;

int gp10b_fuse_check_priv_security(struct gk20a *g);
bool gp10b_fuse_is_opt_ecc_enable(struct gk20a *g);
bool gp10b_fuse_is_opt_feature_override_disable(struct gk20a *g);

#endif /* NVGPU_FUSE_GP10B_H */
