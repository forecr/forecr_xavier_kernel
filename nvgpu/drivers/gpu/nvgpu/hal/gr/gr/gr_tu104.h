/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_TU104_H
#define NVGPU_GR_TU104_H

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>

struct gk20a;

int gr_tu104_init_sw_bundle64(struct gk20a *g);
void gr_tu10x_create_sysfs(struct gk20a *g);
void gr_tu10x_remove_sysfs(struct gk20a *g);
int gr_tu104_get_offset_in_gpccs_segment(struct gk20a *g,
	enum ctxsw_addr_type addr_type, u32 num_tpcs, u32 num_ppcs,
	u32 reg_list_ppc_count, u32 *__offset_in_segment);
void gr_tu104_init_sm_dsm_reg_info(void);
void gr_tu104_get_sm_dsm_perf_ctrl_regs(struct gk20a *g,
	u32 *num_sm_dsm_perf_ctrl_regs, u32 **sm_dsm_perf_ctrl_regs,
	u32 *ctrl_register_stride);
int tu104_gr_update_smpc_global_mode(struct gk20a *g, bool enable);

int tu104_gr_disable_cau(struct gk20a *g, u32 gr_instance_id);
void tu104_gr_disable_smpc(struct gk20a *g);
const u32 *tu104_gr_get_hwpm_cau_init_data(u32 *count);
int tu104_gr_init_cau(struct gk20a *g, u32 gr_instance_id);

int gr_tu104_decode_priv_addr(struct gk20a *g, u32 addr,
	enum ctxsw_addr_type *addr_type,
	u32 *gpc_num, u32 *tpc_num, u32 *ppc_num, u32 *be_num,
	u32 *broadcast_flags);

#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_GR_TU104_H */
