// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/vgpu/tegra_vgpu.h>
#include <nvgpu/vgpu/vgpu.h>

#include "common/vgpu/ivc/comm_vgpu.h"
#include "fb_vgpu.h"

#ifdef CONFIG_NVGPU_DEBUGGER

void vgpu_fb_set_mmu_debug_mode(struct gk20a *g, bool enable)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_fb_set_mmu_debug_mode_params *p =
				&msg.params.fb_set_mmu_debug_mode;
	int err;

	msg.cmd = TEGRA_VGPU_CMD_FB_SET_MMU_DEBUG_MODE;
	msg.handle = vgpu_get_handle(g);
	p->enable = enable ? 1U : 0U;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err != 0 ? err : msg.ret;
	if (err != 0) {
		nvgpu_err(g,
			"fb set mmu debug mode failed err %d", err);
	}
}

#endif
