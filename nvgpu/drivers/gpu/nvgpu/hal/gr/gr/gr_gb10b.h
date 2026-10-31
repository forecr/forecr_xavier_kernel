/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_GB10B_H
#define NVGPU_GR_GB10B_H

struct gk20a;
struct nvgpu_debug_context;
struct nvgpu_vab_range_checker;
struct nvgpu_warpstate;

int gb10b_gr_dump_gr_status_regs(struct gk20a *g,
			struct nvgpu_debug_context *o);
int gb10b_gr_dump_fecs_gr_status_regs(struct gk20a *g,
			struct nvgpu_debug_context *o);
void gb10b_gr_set_circular_buffer_size(struct gk20a *g, u32 data);
int gb10b_gr_wait_for_sm_lock_down(struct gk20a *g,
		u32 gpc, u32 tpc, u32 sm,
		u32 global_esr_mask, bool check_errors);
void gb10b_gr_bpt_reg_info(struct gk20a *g, struct nvgpu_warpstate *w_state);
void gb10b_gr_set_gpcs_rops_crop_debug4(struct gk20a *g, u32 data);
const u32 *gb10b_gr_get_hwpm_cau_init_data(u32 *count);
int gb10b_gr_init_cau(struct gk20a *g, u32 gr_instance_id);
int gb10b_gr_disable_cau(struct gk20a *g, u32 gr_instance_id);

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gb10b_gr_vab_reserve(struct gk20a *g, u32 vab_reg, u32 num_range_checkers,
	struct nvgpu_vab_range_checker *vab_range_checker);
void gb10b_gr_vab_configure(struct gk20a *g, u32 vab_reg);
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
u32 gb10b_get_sm_dbgr_ctrl_base(void);
u32 gb10b_get_sm_dbgr_status_base(void);
u32 gb10b_get_sm_hww_global_esr_base(void);
u32 gb10b_get_sm_hww_warp_esr_base(void);
u32 gb10b_get_gpcs_pri_mmu_debug_ctrl_reg(void);
u32 gb10b_get_gpcs_tpcs_tex_in_dbg_reg(void);
u32 gb10b_get_gpcs_tpcs_sm_l1tag_ctrl_reg(void);
u32 gb10b_gr_gpc0_ppc0_cbm_alpha_cb_size(void);
int gb10b_gr_set_sched_wait_for_errbar(struct gk20a *g,
	struct nvgpu_channel *ch, bool enable);
u32 gb10b_gr_get_egpc_base(struct gk20a *g);
u32 gb10b_gr_get_egpc_shared_base(struct gk20a *g);
int gb10b_gr_find_priv_offset_in_buffer(struct gk20a *g, u32 addr,
		u32 *context_buffer, u32 max_offsets, u32 context_buffer_size,
		u32 *priv_offset, u32 *num_offsets);
void gb10b_gr_disable_smpc(struct gk20a *g);
int gr_gb10b_decode_priv_addr(struct gk20a *g, u32 addr,
		enum ctxsw_addr_type *addr_type,
		u32 *gpc_num, u32 *tpc_num, u32 *ppc_num, u32 *be_num,
		u32 *broadcast_flags);
int gr_gb10b_create_priv_addr_table(struct gk20a *g, u32 addr,
		u32 *priv_addr_table, u32 *num_registers);
#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_GR_GB10B_H */
