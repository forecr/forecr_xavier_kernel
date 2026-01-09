/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_INTR_GP10B_H
#define NVGPU_GR_INTR_GP10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_channel;
struct nvgpu_gr_isr_data;

#define NVC097_SET_GO_IDLE_TIMEOUT		0x022cU
#define NVC097_SET_ALPHA_CIRCULAR_BUFFER_SIZE	0x02dcU
#define NVC097_SET_COALESCE_BUFFER_SIZE		0x1028U
#define NVC097_SET_RD_COALESCE			0x102cU
#define NVC097_SET_CIRCULAR_BUFFER_SIZE		0x1280U
#define NVC097_SET_SHADER_EXCEPTIONS		0x1528U
#define NVC097_SET_BES_CROP_DEBUG3		0x10c4U
#define NVC097_SET_BES_CROP_DEBUG4		0x10b0U
#define NVC0C0_SET_SHADER_EXCEPTIONS		0x1528U
#define NVC0C0_SET_RD_COALESCE			0x0228U

int gp10b_gr_intr_handle_fecs_error(struct gk20a *g,
			struct nvgpu_channel *ch_ptr,
			struct nvgpu_gr_isr_data *isr_data);
void gp10b_gr_intr_handle_class_error(struct gk20a *g, u32 chid,
				      struct nvgpu_gr_isr_data *isr_data);
#if defined(CONFIG_NVGPU_DEBUGGER) && defined(CONFIG_NVGPU_GRAPHICS)
void gp10b_gr_intr_set_coalesce_buffer_size(struct gk20a *g, u32 data);
void gp10b_gr_intr_set_go_idle_timeout(struct gk20a *g, u32 data);
#endif
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gp10b_gr_intr_handle_tex_exception(struct gk20a *g, u32 gpc, u32 tpc);
int gp10b_gr_intr_handle_sw_method(struct gk20a *g, u32 addr,
				     u32 class_num, u32 offset, u32 data);
int gp10b_gr_intr_handle_sm_exception(struct gk20a *g,
			u32 gpc, u32 tpc, u32 sm,
			bool *post_event, struct nvgpu_gr_isr_data *isr_data,
			u32 *hww_global_esr);
#endif /* CONFIG_NVGPU_HAL_NON_FUSA */
#endif /* NVGPU_GR_INTR_GP10B_H */
