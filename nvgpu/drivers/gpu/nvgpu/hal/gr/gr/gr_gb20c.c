// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/debug.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>
#include <nvgpu/regops.h>
#include <nvgpu/gr/ctx.h>
#include <nvgpu/gr/config.h>

#include "gr_gb20c.h"
#include "hal/gr/gr/gr_gk20a.h"
#include "hal/gr/gr/gr_gb20c.h"

#include <nvgpu/hw/gb20c/hw_gr_gb20c.h>

int gb20c_gr_set_mmu_debug_mode(struct gk20a *g, struct nvgpu_channel *ch,
				bool enable, bool enable_gcc)
{
	struct nvgpu_dbg_reg_op ctx_ops = {
		.op = REGOP(WRITE_32),
		.type = REGOP(TYPE_GR_CTX),
		.offset = g->ops.gr.get_gpcs_pri_mmu_debug_ctrl_reg(),
		.value_lo = enable ?
			gr_gpcs_pri_mmu_debug_ctrl_debug_enabled_f() :
			gr_gpcs_pri_mmu_debug_ctrl_debug_disabled_f(),
	};
	int err;
	struct nvgpu_tsg *tsg = nvgpu_tsg_from_ch(ch);
	u32 flags = NVGPU_REG_OP_FLAG_MODE_ALL_OR_NONE;
	(void)enable_gcc;

	if (tsg == NULL) {
		return enable ? -EINVAL : 0;
	}

	ctx_ops.value_lo |= enable_gcc ? gr_gpcs_pri_mmu_debug_ctrl_gcc_debug_enabled_f() :
					gr_gpcs_pri_mmu_debug_ctrl_gcc_debug_disabled_f();

	err = gr_gk20a_exec_ctx_ops(tsg, &ctx_ops, 1, 1, 0, &flags);
	if (err != 0) {
		nvgpu_err(g, "update MMU debug mode failed");
	}
	return err;
}
