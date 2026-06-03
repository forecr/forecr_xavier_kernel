/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_FALCON_GV11B_H
#define NVGPU_GR_FALCON_GV11B_H

struct gk20a;
struct nvgpu_fecs_ecc_status;

void gv11b_gr_falcon_handle_fecs_ecc_error(struct gk20a *g,
			struct nvgpu_fecs_ecc_status *fecs_ecc_status);
void gv11b_gr_falcon_fecs_host_int_enable(struct gk20a *g);
int gv11b_gr_falcon_ctrl_ctxsw(struct gk20a *g, u32 fecs_method,
			u32 data, u32 *ret_val);
u32 gv11b_gr_falcon_get_current_ctx_id(struct gk20a *g);

#endif /* NVGPU_GR_FALCON_GV11B_H */
