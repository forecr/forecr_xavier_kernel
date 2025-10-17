/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_DBG_VGPU_H
#define NVGPU_DBG_VGPU_H

struct dbg_session_gk20a;
struct nvgpu_dbg_reg_op;
struct gk20a;
struct nvgpu_channel;

int vgpu_exec_regops(struct gk20a *g,
		struct nvgpu_tsg *tsg,
		struct nvgpu_dbg_reg_op *ops,
		u32 num_ops,
		u32 ctx_wr_count,
		u32 ctx_rd_count,
		u32 *flags);
int vgpu_dbg_set_powergate(struct dbg_session_gk20a *dbg_s,
			bool disable_powergate);
int vgpu_tsg_set_long_timeslice(struct nvgpu_tsg *tsg, u32 timeslice_us);

#endif /* NVGPU_DBG_VGPU_H */
