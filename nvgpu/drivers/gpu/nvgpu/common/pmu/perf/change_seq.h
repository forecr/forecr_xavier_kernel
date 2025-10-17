/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CHANGE_SEQ_H
#define NVGPU_CHANGE_SEQ_H

#include "ucode_perf_change_seq_inf.h"

#define SEQ_SCRIPT_CURR		0x0U
#define SEQ_SCRIPT_LAST		0x1U
#define SEQ_SCRIPT_QUERY	0x2U

struct change_seq_pmu_script {
	struct perf_change_seq_pmu_script buf;
	u32 super_surface_offset;
};

struct change_seq {
	u8 version;
	bool b_enabled_pmu_support;
	u32 thread_seq_id_last;
	u64 thread_carry_over_timens;
	struct ctrl_perf_change_seq_change last_pstate_values;
	struct boardobjgrpmask_e32 clk_domains_exclusion_mask;
	struct boardobjgrpmask_e32 clk_domains_inclusion_mask;
	u32 client_lock_mask;
};

struct change_seq_pmu {
	struct change_seq super;
	bool b_lock;
	bool b_vf_point_check_ignore;
	u32 cpu_adverised_step_id_mask;
	u32 cpu_step_id_mask;
	u32 event_mask_pending;
	u32 event_mask_received;
	u32 last_completed_change_Seq_id;
	struct change_seq_pmu_script script_curr;
	struct change_seq_pmu_script script_last;
	struct change_seq_pmu_script script_query;
	u32 change_state;
	s64 start_time;
	s64 stop_time;
};

int perf_change_seq_sw_setup(struct gk20a *g);
int perf_change_seq_pmu_setup(struct gk20a *g);

#endif /* NVGPU_CHANGE_SEQ_H */
