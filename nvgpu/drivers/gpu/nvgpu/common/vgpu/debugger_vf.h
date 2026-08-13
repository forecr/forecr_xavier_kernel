/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_DEBUGGER_VF_H
#define NVGPU_DEBUGGER_VF_H

#include <nvgpu/types.h>

struct nvgpu_tsg;
struct nvgpu_dbg_reg_op;

int vf_exec_regops(struct gk20a *g,
		struct nvgpu_tsg *tsg,
		struct nvgpu_dbg_reg_op *ops,
		u32 num_ops,
		u32 ctx_wr_count,
		u32 ctx_rd_count,
		u32 *flags);
int vf_exec_ctx_ops(struct nvgpu_tsg *tsg,
			  struct nvgpu_dbg_reg_op *ctx_ops, u32 num_ops,
			  u32 num_ctx_wr_ops, u32 num_ctx_rd_ops,
			  u32 *flags);

#endif