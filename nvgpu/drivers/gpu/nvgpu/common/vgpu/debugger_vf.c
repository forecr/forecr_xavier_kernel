// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/tsg.h>
#include <nvgpu/log.h>
#include <nvgpu/string.h>
#include <nvgpu/gr/ctx.h>
#include <nvgpu/regops.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/vgpu/tegra_vgpu.h>
#include <nvgpu/vgpu/vgpu_ivc.h>

#include "hal/gr/gr/gr_gk20a.h"
#include "debugger_vf.h"
#include "common/vgpu/ivc/comm_vgpu.h"

static int vf_exec_ctx_regops_non_resident(struct nvgpu_tsg *tsg,
	struct nvgpu_dbg_reg_op *ops,
	u32 num_ops,
	u8 *offset_base, u32 offset_size)
{
	struct gk20a *g = tsg->g;
	u8 *pos;
	struct nvgpu_gr_ctx *gr_ctx = tsg->gr_ctx;
	int pass;
	u32 i;
	int err;

	err = g->ops.mm.cache.l2_flush(g, true);
	if (err != 0) {
		nvgpu_err(g, "l2_flush failed");
		return err;
	}

	for (pass = 0; pass < 2; pass++) {
		pos = offset_base;
		while (true) {
			struct tegra_vf_regop_offset *regop_offset =
					(struct tegra_vf_regop_offset *)pos;
			struct nvgpu_mem *current_mem;
			struct nvgpu_dbg_reg_op *op;

			/* ensure regop_offset not exceed boundary */
			nvgpu_assert(pos + sizeof(*regop_offset) < offset_base + offset_size);
			/* ensure regop_offset->offsets not exceed boundary */
			nvgpu_assert(
				(u8 *)(&regop_offset->offsets[regop_offset->num_offsets]) <=
				offset_base + offset_size);

			if (regop_offset->num_offsets == 0U) {
				break; /* end mark */
			}

			if (regop_offset->buffer_type == NVGPU_GR_CTX_CTX) {
				current_mem = nvgpu_gr_ctx_get_ctx_mem(gr_ctx,
								NVGPU_GR_CTX_CTX);
			} else if (regop_offset->buffer_type == NVGPU_GR_CTX_PM_CTX) {
				if (g->support_gpu_tools == 0x0U) {
					nvgpu_err(g, "PM ctx switch is not supported - offset=0x%x",
						regop_offset->offsets[regop_offset->regop_idx].offset);
					continue;
				}
				current_mem = nvgpu_gr_ctx_get_ctx_mem(gr_ctx,
							NVGPU_GR_CTX_PM_CTX);
			} else {
				BUG();
			}
			if (!nvgpu_mem_is_valid(current_mem)) {
				nvgpu_err(g, "Invalid ctx buffer");
				return -EINVAL;
			}

			op = &ops[regop_offset->regop_idx];
			/* only do ctx ops and only on the right pass */
			if ((op->type == REGOP(TYPE_GLOBAL)) ||
				(((pass == 0) && reg_op_is_read(op->op)) ||
				((pass == 1) && !reg_op_is_read(op->op)))) {
				pos = (u8 *)&regop_offset->offsets[regop_offset->num_offsets];
				continue;
			}

			for (i = 0; i < regop_offset->num_offsets; i++) {
				if (pass == 0) { /* write pass */
					gr_gk20a_regops_wr_ctx_mem(tsg, op,
							current_mem,
							regop_offset->offsets[i].offset,
							regop_offset->offsets[i].offset_addr);
				} else { /* read pass */
					gr_gk20a_regops_rd_ctx_mem(tsg, op,
							current_mem,
							regop_offset->offsets[i].offset);
				}
			}
			pos = (u8 *)&regop_offset->offsets[regop_offset->num_offsets];
		}
	}

	return 0;
}

int vf_exec_regops(struct gk20a *g,
		struct nvgpu_tsg *tsg,
		struct nvgpu_dbg_reg_op *ops,
		u32 num_ops,
		u32 ctx_wr_count,
		u32 ctx_rd_count,
		u32 *flags)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_reg_ops_params *p = &msg.params.reg_ops;
	void *oob;
	size_t oob_size, ops_size;
	int ivc = -1;
	bool has_ctx_regops = ctx_wr_count != 0U || ctx_rd_count != 0U;
	int err = 0;

	nvgpu_log_fn(g, " ");
	BUG_ON(sizeof(*ops) != sizeof(struct tegra_vgpu_reg_op));

	if (tsg == NULL && has_ctx_regops) {
		return -EINVAL;
	}

	if (has_ctx_regops) {
		nvgpu_tsg_disable(tsg);
	}

	ivc = vgpu_comm_alloc_ivc_oob(g, (void **)&oob, &oob_size);
	if (ivc < 0) {
		err = ivc;
		goto enable_tsg;
	}
	ops_size = sizeof(*ops) * num_ops;
	if (oob_size < ops_size) {
		vgpu_comm_free_ivc(g, ivc);
		return -ENOMEM;
	}

	nvgpu_memcpy((u8 *)oob, (u8 *)ops, ops_size);

	msg.cmd = TEGRA_VGPU_CMD_VF_REG_OPS;
	msg.handle = vgpu_get_handle(g);
	p->tsg_id = tsg ? tsg->tsgid : U32_MAX;
	p->num_ops = num_ops;
	p->flags = *flags;
	err = vgpu_comm_sendrecv_locked(g, ivc, &msg);
	vgpu_comm_free_ivc(g, ivc);
	err = err ? err : msg.ret;
	if (err != 0) {
		nvgpu_err(g, "vf_exec_regops failed err=%d msg.ret=%d", err, msg.ret);
		return err;
	}

	if (has_ctx_regops && ((p->flags & NVGPU_REG_OP_FLAG_DIRECT_OPS) == 0)) {
		err = vf_exec_ctx_regops_non_resident(tsg, oob, num_ops,
				(u8 *)oob + ops_size, oob_size - ops_size);
	}
	nvgpu_memcpy((u8 *)ops, (u8 *)oob, ops_size);
	*flags = p->flags;

enable_tsg:
	if (has_ctx_regops) {
		nvgpu_tsg_enable(tsg);
	}

	return err;
}

int vf_exec_ctx_ops(struct nvgpu_tsg *tsg,
			struct nvgpu_dbg_reg_op *ctx_ops, u32 num_ops,
			u32 num_ctx_wr_ops, u32 num_ctx_rd_ops,
			u32 *flags)
{
	return vf_exec_regops(tsg->g, tsg, ctx_ops, num_ops,
			num_ctx_wr_ops, num_ctx_rd_ops, flags);
}
