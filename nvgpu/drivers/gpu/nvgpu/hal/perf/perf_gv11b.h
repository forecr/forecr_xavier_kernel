/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GV11B_PERF
#define NVGPU_GV11B_PERF

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_mem;

bool gv11b_perf_get_membuf_overflow_status(struct gk20a *g, u32 pma_channel_id);
u32 gv11b_perf_get_membuf_pending_bytes(struct gk20a *g, u32 pma_channel_id);
void gv11b_perf_set_membuf_handled_bytes(struct gk20a *g, u32 pma_channel_id,
					 u32 entries, u32 entry_size);

void gv11b_perf_membuf_reset_streaming(struct gk20a *g, u32 pma_channel_id);

void gv11b_perf_enable_membuf(struct gk20a *g, u32 pma_channel_id, u32 size,
			      u64 buf_addr);
void gv11b_perf_disable_membuf(struct gk20a *g, u32 pma_channel_id);

void gv11b_perf_bind_mem_bytes_buffer_addr(struct gk20a *g, u32 pma_channel_id,
					   u64 buf_addr);

int gv11b_perf_update_get_put(struct gk20a *g, u32 pma_channel_id,
			      u64 bytes_consumed, bool update_available_bytes,
			      u64 *put_ptr, bool *overflowed);

int gv11b_perf_init_inst_block(struct gk20a *g, u32 pma_channel_id,
			       u32 inst_blk_ptr, u32 aperture, u32 gfid);
void gv11b_perf_deinit_inst_block(struct gk20a *g, u32 pma_channel_id);

u32 gv11b_perf_get_pmmsys_per_chiplet_offset(void);
u32 gv11b_perf_get_pmmgpc_per_chiplet_offset(void);
u32 gv11b_perf_get_pmmgpcrouter_per_chiplet_offset(void);
u32 gv11b_perf_get_pmmfbp_per_chiplet_offset(void);
u32 gv11b_perf_get_pmmfbprouter_per_chiplet_offset(void);
u32 gv11b_get_hwpm_fbprouter_perfmon_regs_base(struct gk20a *g);
u32 gv11b_get_hwpm_gpcrouter_perfmon_regs_base(struct gk20a *g);

const u32 *gv11b_perf_get_hwpm_sys_perfmon_regs(u32 *count);
const u32 *gv11b_perf_get_hwpm_gpc_perfmon_regs(u32 *count);
const u32 *gv11b_perf_get_hwpm_fbp_perfmon_regs(u32 *count);

void gv11b_perf_set_pmm_register(struct gk20a *g, u32 offset, u32 val,
		u32 num_chiplets, u32 chiplet_stride, u32 num_perfmons);
void gv11b_perf_get_num_hwpm_perfmon(struct gk20a *g, u32 *num_sys_perfmon,
		u32 *num_fbp_perfmon, u32 *num_gpc_perfmon);
void gv11b_perf_reset_hwpm_pmm_registers(struct gk20a *g, u32 gr_instance_id);
void gv11b_perf_init_hwpm_pmm_register(struct gk20a *g, u32 gr_instance_id);

void gv11b_perf_pma_stream_enable(struct gk20a *g, u32 pma_channel_id,
				  bool enable);
void gv11b_perf_disable_all_perfmons(struct gk20a *g, u32 gr_instance_id);
int gv11b_perf_wait_for_idle_pmm_routers(struct gk20a *g, u32 gr_instance_id, u32 pma_channel_id);
int gv11b_perf_wait_for_idle_pma(struct gk20a *g);
u32 gv11b_perf_get_hwpm_fbp_fbpgs_ltcs_base_addr(void);
u32 gv11b_perf_get_hwpm_gpcgs_gpctpca_base_addr(void);
u32 gv11b_perf_get_hwpm_gpcgs_gpctpcb_base_addr(void);
u32 gv11b_perf_get_hwpm_gpcs_base_addr(void);
u32 gv11b_perf_get_hwpm_gpcsrouter_base_addr(void);
u32 gv11b_perf_get_hwpm_fbps_base_addr(void);
u32 gv11b_perf_get_hwpm_fbpsrouter_base_addr(void);
#endif /* CONFIG_NVGPU_DEBUGGER */
#endif
