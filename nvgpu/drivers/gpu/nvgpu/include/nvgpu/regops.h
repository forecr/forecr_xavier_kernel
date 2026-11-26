/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2013-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_REGOPS_H
#define NVGPU_REGOPS_H

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_tsg;
struct nvgpu_profiler_object;

/*
 * Register operations
 * All operations are targeted towards first channel
 * attached to debug session
 */
/* valid op values */
#define NVGPU_DBG_REG_OP_READ_32                             0x00000000U
#define NVGPU_DBG_REG_OP_WRITE_32                            0x00000001U
#define NVGPU_DBG_REG_OP_READ_64                             0x00000002U
#define NVGPU_DBG_REG_OP_WRITE_64                            0x00000003U
/* note: 8b ops are unsupported */
#define NVGPU_DBG_REG_OP_READ_08                             0x00000004U
#define NVGPU_DBG_REG_OP_WRITE_08                            0x00000005U

/* valid type values */
#define NVGPU_DBG_REG_OP_TYPE_GLOBAL                         0x00000000U
#define NVGPU_DBG_REG_OP_TYPE_GR_CTX                         0x00000001U
#define NVGPU_DBG_REG_OP_TYPE_GR_CTX_TPC                     0x00000002U
#define NVGPU_DBG_REG_OP_TYPE_GR_CTX_SM                      0x00000004U
#define NVGPU_DBG_REG_OP_TYPE_GR_CTX_CROP                    0x00000008U
#define NVGPU_DBG_REG_OP_TYPE_GR_CTX_ZROP                    0x00000010U
#define NVGPU_DBG_REG_OP_TYPE_GR_CTX_QUAD                    0x00000040U

/* valid status values */
#define NVGPU_DBG_REG_OP_STATUS_SUCCESS                      0x00000000U
#define NVGPU_DBG_REG_OP_STATUS_INVALID_OP                   0x00000001U
#define NVGPU_DBG_REG_OP_STATUS_INVALID_TYPE                 0x00000002U
#define NVGPU_DBG_REG_OP_STATUS_INVALID_OFFSET               0x00000004U
#define NVGPU_DBG_REG_OP_STATUS_UNSUPPORTED_OP               0x00000008U
#define NVGPU_DBG_REG_OP_STATUS_INVALID_MASK                 0x00000010U

#define NVGPU_REG_OP_FLAG_MODE_ALL_OR_NONE		BIT32(1U)
#define NVGPU_REG_OP_FLAG_MODE_CONTINUE_ON_ERROR	BIT32(2U)
#define NVGPU_REG_OP_FLAG_ALL_PASSED			BIT32(3U)
#define NVGPU_REG_OP_FLAG_DIRECT_OPS			BIT32(4U)

struct nvgpu_dbg_reg_op {
	u8    op;
	u8    type;
	u8    status;
	u8    quad;
	u32   group_mask;
	u32   sub_group_mask;
	u32   offset;
	u32   value_lo;
	u32   value_hi;
	u32   and_n_mask_lo;
	u32   and_n_mask_hi;
};

struct regop_offset_range {
	u32 base:24;
	u32 count:8;
};
int exec_global_regops(struct gk20a *g,
				struct nvgpu_dbg_reg_op *ops, u32 num_ops);
int exec_regops_gk20a(struct gk20a *g,
		      struct nvgpu_tsg *tsg,
		      struct nvgpu_dbg_reg_op *ops,
		      u32 num_ops,
		      u32 ctx_wr_count,
		      u32 ctx_rd_count,
		      u32 *flags);
int nvgpu_regops_exec(struct gk20a *g,
		struct nvgpu_tsg *tsg,
		struct nvgpu_profiler_object *prof,
		struct nvgpu_dbg_reg_op *ops,
		u32 num_ops,
		u32 *flags);

/* turn seriously unwieldy names -> something shorter */
#define REGOP(x) NVGPU_DBG_REG_OP_##x

bool reg_op_is_gr_ctx(u8 type);
bool reg_op_is_read(u8 op);
bool is_bar0_global_offset_whitelisted_gk20a(struct gk20a *g, u32 offset);

#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_REGOPS_H */
