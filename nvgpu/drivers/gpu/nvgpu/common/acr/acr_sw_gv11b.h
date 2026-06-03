/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef ACR_SW_GV11B_H
#define ACR_SW_GV11B_H

struct gk20a;
struct nvgpu_acr;
struct hs_acr;

/** @cond DOXYGEN_SHOULD_SKIP_THIS */
void nvgpu_gv11b_acr_sw_init(struct gk20a *g, struct nvgpu_acr *acr);
u32 gv11b_acr_lsf_config(struct gk20a *g, struct nvgpu_acr *acr);
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

#endif /* ACR_SW_GV11B_H */

