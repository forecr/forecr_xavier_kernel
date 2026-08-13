// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/gr/gr_falcon.h>

#include "hal/gr/falcon/gr_falcon_gm20b.h"
#include "hal/gr/falcon/gr_falcon_tu104.h"
#include "hal/gr/falcon/gr_falcon_ga100.h"
#include "common/gr/gr_falcon_priv.h"

#include <nvgpu/hw/ga100/hw_gr_ga100.h>

int ga100_gr_falcon_ctrl_ctxsw(struct gk20a *g, u32 fecs_method,
		u32 data, u32 *ret_val)
{
	struct nvgpu_fecs_method_op op = {
		.mailbox = { .id = 0U, .data = 0U, .ret = NULL,
			     .clr = ~U32(0U), .ok = 0U, .fail = 0U},
		.method.data = 0U,
		.cond.ok = GR_IS_UCODE_OP_NOT_EQUAL,
		.cond.fail = GR_IS_UCODE_OP_SKIP,
	};
	u32 flags = 0U;

	int ret;

	nvgpu_log_info(g, "fecs method %d data 0x%x ret_val %p",
				fecs_method, data, ret_val);

	switch (fecs_method) {
	case NVGPU_GR_FALCON_METHOD_SET_WATCHDOG_TIMEOUT:
		op.method.addr =
			gr_fecs_method_push_adr_set_watchdog_timeout_f();
		op.method.data = data;
		op.cond.ok = GR_IS_UCODE_OP_SKIP;
		op.mailbox.ok = gr_fecs_ctxsw_mailbox_value_pass_v();
		flags |= NVGPU_GR_FALCON_SUBMIT_METHOD_F_LOCKED;

		ret = gm20b_gr_falcon_submit_fecs_method_op(g, op, flags, fecs_method);
		break;

	default:
		ret = tu104_gr_falcon_ctrl_ctxsw(g, fecs_method,
				data, ret_val);
		break;
	}
	return ret;
}
