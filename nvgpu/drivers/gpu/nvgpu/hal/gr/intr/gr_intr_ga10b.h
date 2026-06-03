/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
int ga10b_gr_intr_is_valid_gfx_sw_method(struct gk20a *g, u32 offset);
int ga10b_gr_intr_handle_gfx_sw_method(struct gk20a *g, u32 addr,
			u32 class_num, u32 offset, u32 data);
#endif
#if defined(CONFIG_NVGPU_HAL_NON_FUSA)
bool ga10b_gr_intr_is_valid_compute_sw_method(struct gk20a *g, u32 offset);
int ga10b_gr_intr_handle_compute_sw_method(struct gk20a *g, u32 addr,
			u32 class_num, u32 offset, u32 data);
#endif
void ga10b_gr_intr_enable_interrupts(struct gk20a *g, bool enable);
void ga10b_gr_intr_configure_sked_interrupts(struct gk20a *g);
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
u32 ga10b_gpc0_rop0_crop_hww_esr_offset(void);
void ga10b_gr_intr_handle_gpc_crop_hww(struct gk20a *g, u32 gpc, u32 gpc_exception);
void ga10b_gr_intr_handle_gpc_zrop_hww(struct gk20a *g, u32 gpc, u32 gpc_exception);
u32 ga10b_gpc0_rop0_rrh_status_offset(void);
void ga10b_gr_intr_handle_gpc_rrh_hww(struct gk20a *g, u32 gpc, u32 gpc_exception);
bool ga10b_gr_intr_is_illegal_method_pending(struct gk20a *g);
u32 ga10b_gr_intr_read_pending_interrupts(struct gk20a *g,
					struct nvgpu_gr_intr_info *intr_info);
u32 ga10b_gr_intr_enable_mask(struct gk20a *g);
void ga10b_gr_intr_report_tpc_sm_rams_ecc_err(struct gk20a *g,
		struct nvgpu_gr_sm_ecc_status *ecc_status, u32 gpc, u32 tpc);
bool ga10b_gr_intr_sm_rams_ecc_status_errors(struct gk20a *g,
	u32 rams_ecc_status, struct nvgpu_gr_sm_ecc_status *ecc_status);
bool ga10b_gr_intr_sm_l1_tag_ecc_status_errors(struct gk20a *g,
	u32 l1_tag_ecc_status, struct nvgpu_gr_sm_ecc_status *ecc_status);
bool ga10b_gr_intr_sm_l1_data_ecc_status_errors(struct gk20a *g,
	u32 l1_data_ecc_status, struct nvgpu_gr_sm_ecc_status *ecc_status);
#endif /* NVGPU_GR_INTR_GA10B_H */
