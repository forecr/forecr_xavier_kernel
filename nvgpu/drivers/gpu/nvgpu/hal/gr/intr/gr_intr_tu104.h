/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_INTR_TU104_H
#define NVGPU_GR_INTR_TU104_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_config;

#define NVC5C0_SET_SHADER_EXCEPTIONS            0x1528U
#define NVC5C0_SET_SKEDCHECK                    0x23cU
#define NVC5C0_SET_SHADER_CUT_COLLECTOR         0x254U

#define NVC5C0_SET_SM_DISP_CTRL                             0x250U
#define NVC5C0_SET_SM_DISP_CTRL_COMPUTE_SHADER_QUAD_MASK    0x1U
#define NVC5C0_SET_SM_DISP_CTRL_COMPUTE_SHADER_QUAD_DISABLE 0U
#define NVC5C0_SET_SM_DISP_CTRL_COMPUTE_SHADER_QUAD_ENABLE  1U

#define NVC597_SET_SHADER_EXCEPTIONS            0x1528U
#define NVC597_SET_CIRCULAR_BUFFER_SIZE         0x1280U
#define NVC597_SET_ALPHA_CIRCULAR_BUFFER_SIZE   0x02dcU
#define NVC597_SET_GO_IDLE_TIMEOUT              0x022cU
#define NVC597_SET_TEX_IN_DBG                   0x10bcU
#define NVC597_SET_SKEDCHECK                    0x10c0U
#define NVC597_SET_BES_CROP_DEBUG3              0x10c4U
#define NVC597_SET_BES_CROP_DEBUG4              0x10b0U
#define NVC597_SET_SM_DISP_CTRL                 0x10c8U
#define NVC597_SET_SHADER_CUT_COLLECTOR         0x10d0U

int tu104_gr_intr_handle_sw_method(struct gk20a *g, u32 addr,
			      u32 class_num, u32 offset, u32 data);
void tu104_gr_intr_enable_gpc_exceptions(struct gk20a *g,
					 struct nvgpu_gr_config *gr_config);
void tu104_gr_intr_log_mme_exception(struct gk20a *g);

#endif /* NVGPU_GR_INTR_TU104_H */
