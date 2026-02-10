/*
 * Copyright (c) 2020-2023, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef NVGPU_GR_INTR_GA10B_H
#define NVGPU_GR_INTR_GA10B_H

struct gk20a;
struct nvgpu_gr_config;
struct nvgpu_gr_tpc_exception;
struct nvgpu_gr_sm_ecc_status;
enum nvgpu_gr_sm_ecc_error_types;
struct nvgpu_gr_intr_info;

int ga10b_gr_intr_handle_sw_method(struct gk20a *g, u32 addr,
			u32 class_num, u32 offset, u32 data);
#if defined(CONFIG_NVGPU_DEBUGGER) && defined(CONFIG_NVGPU_GRAPHICS)
int ga10b_gr_intr_handle_gfx_sw_method(struct gk20a *g, u32 addr,
			u32 class_num, u32 offset, u32 data);
#endif
#if defined(CONFIG_NVGPU_HAL_NON_FUSA)
int ga10b_gr_intr_handle_compute_sw_method(struct gk20a *g, u32 addr,
			u32 class_num, u32 offset, u32 data);
#endif
void ga10b_gr_intr_enable_interrupts(struct gk20a *g, bool enable);
void ga10b_gr_intr_enable_gpc_exceptions(struct gk20a *g,
					 struct nvgpu_gr_config *gr_config);
u32 ga10b_gr_intr_get_tpc_exception(struct gk20a *g, u32 offset,
				   struct nvgpu_gr_tpc_exception *pending_tpc);
void ga10b_gr_intr_set_hww_esr_report_mask(struct gk20a *g);
void ga10b_gr_intr_enable_exceptions(struct gk20a *g,
			struct nvgpu_gr_config *gr_config, bool enable);
bool ga10b_gr_intr_handle_exceptions(struct gk20a *g, bool *is_gpc_exception);
void ga10b_gr_intr_handle_gpc_gpcmmu_exception(struct gk20a *g, u32 gpc,
		u32 gpc_exception, u32 *corrected_err, u32 *uncorrected_err);
void ga10b_gr_intr_handle_tpc_sm_ecc_exception(struct gk20a *g, u32 gpc,
					       u32 tpc);
bool ga10b_gr_intr_sm_ecc_status_errors(struct gk20a *g,
	u32 ecc_status_reg, enum nvgpu_gr_sm_ecc_error_types err_type,
	struct nvgpu_gr_sm_ecc_status *ecc_status);
int ga10b_gr_intr_retrigger(struct gk20a *g);
void ga10b_gr_intr_enable_gpc_crop_hww(struct gk20a *g);
void ga10b_gr_intr_enable_gpc_zrop_hww(struct gk20a *g);
void ga10b_gr_intr_handle_gpc_crop_hww(struct gk20a *g, u32 gpc, u32 exception);
void ga10b_gr_intr_handle_gpc_zrop_hww(struct gk20a *g, u32 gpc, u32 exception);
void ga10b_gr_intr_handle_gpc_rrh_hww(struct gk20a *g, u32 gpc, u32 exception);
u32 ga10b_gr_intr_read_pending_interrupts(struct gk20a *g,
					struct nvgpu_gr_intr_info *intr_info);
u32 ga10b_gr_intr_enable_mask(struct gk20a *g);
void ga10b_gr_intr_report_tpc_sm_rams_ecc_err(struct gk20a *g,
		struct nvgpu_gr_sm_ecc_status *ecc_status, u32 gpc, u32 tpc);

#endif /* NVGPU_GR_INTR_GA10B_H */
