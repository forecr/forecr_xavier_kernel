/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_INTR_GB10B_H
#define NVGPU_GR_INTR_GB10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_gr_config;
struct nvgpu_channel;
struct nvgpu_gr_isr_data;
struct nvgpu_gr_tpc_exception;
struct nvgpu_gr_sm_ecc_status;
enum nvgpu_gr_sm_ecc_error_types;

void gb10b_gr_intr_handle_gpc_gpccs_exception(struct gk20a *g, u32 gpc,
		u32 gpc_exception, u32 *corrected_err, u32 *uncorrected_err);
bool gb10b_gr_intr_is_valid_sw_method(struct gk20a *g, u32 class_num, u32 offset);
int gb10b_gr_intr_handle_sw_method(struct gk20a *g, u32 addr,
		u32 class_num, u32 offset, u32 data);
u32 gb10b_gr_intr_read_gpc_tpc_exception(u32 gpc_exception);
void gb10b_gr_intr_handle_tpc_tpccs_exception(struct gk20a *g, u32 gpc, u32 tpc);
void gb10b_gr_intr_handle_tpc_mpc_exception(struct gk20a *g, u32 gpc, u32 tpc);
void gb10b_gr_intr_clear_sm_hww(struct gk20a *g, u32 gpc, u32 tpc, u32 sm,
				u32 global_esr);
u32 gb10b_gr_intr_get_warp_esr_sm_hww(struct gk20a *g,
			u32 gpc, u32 tpc, u32 sm);
u32 gb10b_gr_intr_get_sm_hww_global_esr(struct gk20a *g,
			u32 gpc, u32 tpc, u32 sm);
u32 gb10b_gr_intr_get_sm_hww_cga_esr(struct gk20a *g,
			u32 gpc, u32 tpc, u32 sm);
void gb10b_gr_intr_handle_sm_hww_cga_esr(struct gk20a *g, u32 cga_esr,
			u32 gpc, u32 tpc, u32 sm);
void gb10b_gr_intr_set_hww_esr_report_mask(struct gk20a *g);
u64 gb10b_gr_intr_get_warp_esr_pc_sm_hww(struct gk20a *g, u32 offset);
void gb10b_gr_intr_get_esr_sm_sel(struct gk20a *g, u32 gpc, u32 tpc,
				u32 *esr_sm_sel);
u32 gb10b_gr_intr_get_tpc_exception(struct gk20a *g, u32 offset,
				   struct nvgpu_gr_tpc_exception *pending_tpc);
void gb10b_gr_intr_enable_gpc_exceptions(struct gk20a *g,
					 struct nvgpu_gr_config *gr_config);
void gb10b_gr_intr_handle_tpc_sm_ecc_exception(struct gk20a *g,
					u32 gpc, u32 tpc);
u32 gb10b_gr_intr_record_sm_error_state(struct gk20a *g, u32 gpc, u32 tpc, u32 sm,
				struct nvgpu_gr_isr_data *isr_data);
void gb10b_gr_intr_handle_gcc_exception(struct gk20a *g, u32 gpc,
				u32 gpc_exception, u32 *corrected_err, u32 *uncorrected_err);
void gb10b_gr_intr_enable_gpc_crop_hww(struct gk20a *g);
void gb10b_gr_intr_enable_gpc_zrop_hww(struct gk20a *g);
void gb10b_gr_intr_enable_interrupts(struct gk20a *g, bool enable);
void gb10b_handle_sm_poison_error(struct gk20a *g, u32 global_esr);
bool gb10b_gr_intr_sm_rams_ecc_status_errors(struct gk20a *g,
	u32 rams_ecc_status, struct nvgpu_gr_sm_ecc_status *ecc_status);
bool gb10b_gr_intr_sm_l1_tag_ecc_status_errors(struct gk20a *g,
	u32 l1_tag_ecc_status, struct nvgpu_gr_sm_ecc_status *ecc_status);
bool gb10b_gr_intr_sm_l1_data_ecc_status_errors(struct gk20a *g,
	u32 l1_data_ecc_status, struct nvgpu_gr_sm_ecc_status *ecc_status);
void gb10b_gr_intr_enable_hww_exceptions(struct gk20a *g);
bool gb10b_gr_intr_handle_exceptions(struct gk20a *g, bool *is_gpc_exception);
void gb10b_gr_intr_enable_exceptions(struct gk20a *g,
	struct nvgpu_gr_config *gr_config, bool enable);
void gb10b_gr_intr_handle_gpc_wdxps_hww(struct gk20a *g, u32 gpc,
		u32 gpc_exception);
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gb10b_gr_intr_set_shader_exceptions(struct gk20a *g, u32 data);
void gb10b_gr_intr_tpc_exception_sm_enable(struct gk20a *g);
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
u64 gb10b_gr_intr_tpc_enabled_exceptions(struct gk20a *g);
void gb10b_gr_intr_tpc_exception_sm_disable(struct gk20a *g, u32 offset);
#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_GR_INTR_GB10B_H */
