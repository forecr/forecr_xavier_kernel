/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GA10B_PERF
#define NVGPU_GA10B_PERF

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_mem;

bool ga10b_perf_get_membuf_overflow_status(struct gk20a *g, u32 pma_channel_id);
u32 ga10b_perf_get_membuf_pending_bytes(struct gk20a *g, u32 pma_channel_id);
void ga10b_perf_set_membuf_handled_bytes(struct gk20a *g, u32 pma_channel_id,
					 u32 entries, u32 entry_size);

void ga10b_perf_membuf_reset_streaming(struct gk20a *g, u32 pma_channel_id);

void ga10b_perf_enable_membuf(struct gk20a *g, u32 pma_channel_id, u32 size,
			      u64 buf_addr);
void ga10b_perf_disable_membuf(struct gk20a *g, u32 pma_channel_id);
void ga10b_perf_bind_mem_bytes_buffer_addr(struct gk20a *g, u32 pma_channel_id,
					   u64 buf_addr);

int ga10b_perf_init_inst_block(struct gk20a *g, u32 pma_channel_id,
			       u32 inst_blk_ptr, u32 aperture, u32 gfid);
void ga10b_perf_deinit_inst_block(struct gk20a *g, u32 pma_channel_id);

u32 ga10b_perf_get_pmmsys_per_chiplet_offset(void);
u32 ga10b_perf_get_pmmgpc_per_chiplet_offset(void);
u32 ga10b_perf_get_pmmfbp_per_chiplet_offset(void);
u32 ga10b_perf_get_pmmgpcrouter_per_chiplet_offset(void);
u32 ga10b_perf_get_pmmfbprouter_per_chiplet_offset(void);

const u32 *ga10b_perf_get_hwpm_sys_perfmon_regs(u32 *count);
const u32 *ga10b_perf_get_hwpm_gpc_perfmon_regs(u32 *count);
const u32 *ga10b_perf_get_hwpm_fbp_perfmon_regs(u32 *count);

u32 ga10b_get_hwpm_fbp_perfmon_regs_base(struct gk20a *g);
u32 ga10b_get_hwpm_gpc_perfmon_regs_base(struct gk20a *g);

void ga10b_perf_get_num_hwpm_perfmon(struct gk20a *g, u32 *num_sys_perfmon,
		u32 *num_fbp_perfmon, u32 *num_gpc_perfmon);
void ga10b_perf_init_hwpm_pmm_register(struct gk20a *g, u32 gr_instance_id);
void ga10b_perf_disable_all_perfmons(struct gk20a *g, u32 gr_instance_id);
void ga10b_perf_pma_stream_enable(struct gk20a *g, u32 pma_channel_id,
				  bool enable);
int ga10b_perf_wait_for_idle_pma(struct gk20a *g);
int ga10b_perf_update_get_put(struct gk20a *g, u32 pma_channel_id,
			      u64 bytes_consumed, bool update_available_bytes,
			      u64 *put_ptr, bool *overflowed);
void ga10b_perf_enable_hs_streaming(struct gk20a *g, bool enable);
void ga10b_perf_reset_hs_streaming_credits(struct gk20a *g);
void ga10b_perf_enable_pmasys_legacy_mode(struct gk20a *g, bool enable);
void ga10b_perf_reset_hwpm_pma_registers(struct gk20a *g);
void ga10b_perf_reset_hwpm_pma_trigger_registers(struct gk20a *g);
void ga10b_perf_reset_pmasys_channel_registers(struct gk20a *g);
u32 ga10b_perf_get_pma_channel_count(struct gk20a *g);
u32 ga10b_perf_get_hwpm_fbp_fbpgs_ltcs_base_addr(void);
u32 ga10b_perf_get_hwpm_gpcgs_gpctpca_base_addr(void);
u32 ga10b_perf_get_hwpm_gpcgs_gpctpcb_base_addr(void);
u32 ga10b_perf_get_hwpm_gpcs_base_addr(void);
u32 ga10b_perf_get_hwpm_gpcsrouter_base_addr(void);
u32 ga10b_perf_get_hwpm_fbps_base_addr(void);
u32 ga10b_perf_get_hwpm_fbpsrouter_base_addr(void);
#endif /* CONFIG_NVGPU_DEBUGGER */
#endif
