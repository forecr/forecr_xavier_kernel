// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/gr/gr_falcon.h>

#include "gr_falcon_gm20b.h"
#include "gr_falcon_gv11b.h"
#include "gr_falcon_tu104.h"
#include "common/gr/gr_falcon_priv.h"

#include <nvgpu/hw/tu104/hw_gr_tu104.h>

int tu104_gr_falcon_ctrl_ctxsw(struct gk20a *g, u32 fecs_method,
		u32 data, u32 *ret_val)
{
#if defined(CONFIG_NVGPU_DEBUGGER) || defined(CONFIG_NVGPU_PROFILER)
	u32 flags = 0U;

	struct nvgpu_fecs_method_op op = {
		.mailbox = { .id = 1U, .data = 0U, .ret = NULL,
			.clr = ~U32(0U)},
		.method.data = ~U32(0U),
		.mailbox.ok = gr_fecs_ctxsw_mailbox_value_pass_v(),
		.mailbox.fail = gr_fecs_ctxsw_mailbox_value_fail_v(),
		.cond.ok = GR_IS_UCODE_OP_EQUAL,
		.cond.fail = GR_IS_UCODE_OP_EQUAL,
	};

	flags |= NVGPU_GR_FALCON_SUBMIT_METHOD_F_SLEEP;

	if (fecs_method == NVGPU_GR_FALCON_METHOD_START_SMPC_GLOBAL_MODE) {
		op.method.addr =
			gr_fecs_method_push_adr_smpc_global_mode_start_v();
		return gm20b_gr_falcon_submit_fecs_method_op(g, op, flags, fecs_method);
	}

	if (fecs_method == NVGPU_GR_FALCON_METHOD_STOP_SMPC_GLOBAL_MODE) {
		op.method.addr =
			gr_fecs_method_push_adr_smpc_global_mode_stop_v();
		return gm20b_gr_falcon_submit_fecs_method_op(g, op, flags, fecs_method);
	}
#endif

	return gv11b_gr_falcon_ctrl_ctxsw(g, fecs_method, data, ret_val);
}
