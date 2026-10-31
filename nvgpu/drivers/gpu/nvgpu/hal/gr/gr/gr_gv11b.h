/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_GV11B_H
#define NVGPU_GR_GV11B_H

#ifdef CONFIG_NVGPU_DEBUGGER

struct gk20a;
struct nvgpu_warpstate;
struct nvgpu_debug_context;

u32 gv11b_gr_gpc0_ppc0_cbm_alpha_cb_size(void);
void gr_gv11b_set_alpha_circular_buffer_size(struct gk20a *g, u32 data);
void gr_gv11b_set_circular_buffer_size(struct gk20a *g, u32 data);
int gr_gv11b_dump_gr_status_regs(struct gk20a *g,
			   struct nvgpu_debug_context *o);
#ifdef CONFIG_NVGPU_TEGRA_FUSE
void gr_gv11b_set_gpc_tpc_mask(struct gk20a *g, u32 gpc_index);
#endif
int gr_gv11b_pre_process_sm_exception(struct gk20a *g,
		u32 gpc, u32 tpc, u32 sm, u32 global_esr, u32 warp_esr,
		u32 cga_esr,
		bool sm_debugger_attached, struct nvgpu_gr_isr_data *isr_data,
		bool *early_exit, bool *ignore_debugger);
void gv11b_gr_bpt_reg_info(struct gk20a *g, struct nvgpu_warpstate *w_state);
int gv11b_gr_set_sm_debug_mode(struct gk20a *g,
	struct nvgpu_channel *ch, u64 sms, bool enable);
u32 gv11b_get_sm_dbgr_ctrl_base(void);
u32 gv11b_get_sm_dbgr_status_base(void);
u32 gv11b_get_sm_hww_global_esr_base(void);
u32 gv11b_get_sm_hww_warp_esr_base(void);
u32 gv11b_get_gpcs_tpcs_tex_in_dbg_reg(void);
u32 gv11b_get_gpcs_tpcs_sm_l1tag_ctrl_reg(void);
int gv11b_gr_clear_sm_error_state(struct gk20a *g,
		struct nvgpu_channel *ch, u32 sm_id);
bool gv11b_gr_sm_debugger_attached(struct gk20a *g);
bool gv11b_gr_check_warp_esr_error(struct gk20a *g, u32 warp_esr_error);
void gv11b_gr_suspend_single_sm(struct gk20a *g,
		u32 gpc, u32 tpc, u32 sm,
		u32 global_esr_mask, bool check_errors);
void gv11b_gr_suspend_all_sms(struct gk20a *g,
		u32 global_esr_mask, bool check_errors);
void gv11b_gr_resume_single_sm(struct gk20a *g,
		u32 gpc, u32 tpc, u32 sm);
void gv11b_gr_resume_all_sms(struct gk20a *g);
int gv11b_gr_wait_for_sm_lock_down(struct gk20a *g,
		u32 gpc, u32 tpc, u32 sm,
		u32 global_esr_mask, bool check_errors);
int gv11b_gr_lock_down_sm(struct gk20a *g,
			 u32 gpc, u32 tpc, u32 sm, u32 global_esr_mask,
			 bool check_errors);
void gv11b_gr_init_ovr_sm_dsm_perf(void);
void gv11b_gr_init_sm_dsm_reg_info(void);
void gv11b_gr_get_sm_dsm_perf_regs(struct gk20a *g,
					  u32 *num_sm_dsm_perf_regs,
					  u32 **sm_dsm_perf_regs,
					  u32 *perf_register_stride);
void gv11b_gr_get_sm_dsm_perf_ctrl_regs(struct gk20a *g,
					       u32 *num_sm_dsm_perf_ctrl_regs,
					       u32 **sm_dsm_perf_ctrl_regs,
					       u32 *ctrl_register_stride);
void gv11b_gr_get_ovr_perf_regs(struct gk20a *g, u32 *num_ovr_perf_regs,
					       u32 **ovr_perf_regs);
bool gv11b_gr_pri_is_egpc_addr(struct gk20a *g, u32 addr);
bool gv11b_gr_pri_is_etpc_addr(struct gk20a *g, u32 addr);
void gv11b_gr_get_egpc_etpc_num(struct gk20a *g, u32 addr,
			u32 *egpc_num, u32 *etpc_num);
int gv11b_gr_decode_egpc_addr(struct gk20a *g, u32 addr,
	enum ctxsw_addr_type *addr_type, u32 *gpc_num, u32 *tpc_num,
	u32 *broadcast_flags);
void gv11b_gr_egpc_etpc_priv_addr_table(struct gk20a *g, u32 addr,
				u32 gpc_num, u32 tpc_num, u32 broadcast_flags,
				u32 *priv_addr_table, u32 *t);
u32 gv11b_gr_get_egpc_base(struct gk20a *g);
u32 gv11b_gr_get_egpc_shared_base(struct gk20a *g);
int gr_gv11b_decode_priv_addr(struct gk20a *g, u32 addr,
	enum ctxsw_addr_type *addr_type,
	u32 *gpc_num, u32 *tpc_num, u32 *ppc_num, u32 *be_num,
	u32 *broadcast_flags);
u32 gr_gv11b_pri_pmmgpc_addr(struct gk20a *g, u32 gpc_num,
	u32 domain_idx, u32 offset);
void gr_gv11b_split_pmm_fbp_broadcast_address(struct gk20a *g,
	u32 offset, u32 *priv_addr_table, u32 *t,
	u32 domain_start, u32 num_domains);
int gr_gv11b_create_priv_addr_table(struct gk20a *g,
	u32 addr,
	u32 *priv_addr_table,
	u32 *num_registers);
bool gv11b_gr_esr_bpt_pending_events(u32 global_esr,
				enum nvgpu_event_id_type bpt_event);
int gv11b_gr_handle_warp_esr_error_mmu_nack(struct gk20a *g,
	u32 gpc, u32 tpc, u32 sm,
	u32 warp_esr_error);
#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_GR_GV11B_H */
