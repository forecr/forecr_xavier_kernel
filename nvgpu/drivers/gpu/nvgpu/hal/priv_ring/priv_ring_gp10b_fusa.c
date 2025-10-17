// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/timers.h>
#include <nvgpu/enabled.h>
#include <nvgpu/io.h>
#include <nvgpu/utils.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/static_analysis.h>

#include <nvgpu/hw/gp10b/hw_pri_ringmaster_gp10b.h>
#include <nvgpu/hw/gp10b/hw_pri_ringstation_sys_gp10b.h>
#include <nvgpu/hw/gp10b/hw_pri_ringstation_gpc_gp10b.h>

#include "priv_ring_gp10b.h"

static const u32 poll_retries = 100;
static const u32 poll_delay = 20;

void gp10b_priv_ring_isr(struct gk20a *g)
{
	u32 retries = poll_retries;
	u32 status0, status1;
	u32 cmd;

	status0 = nvgpu_readl(g, pri_ringmaster_intr_status0_r());
	status1 = nvgpu_readl(g, pri_ringmaster_intr_status1_r());

	nvgpu_err(g, "ringmaster intr status0: 0x%08x, status1: 0x%08x",
			status0, status1);

	g->ops.priv_ring.isr_handle_0(g, status0);
	g->ops.priv_ring.isr_handle_1(g, status1);

	/* clear interrupt */
	cmd = nvgpu_readl(g, pri_ringmaster_command_r());
	cmd = set_field(cmd, pri_ringmaster_command_cmd_m(),
		pri_ringmaster_command_cmd_ack_interrupt_f());
	nvgpu_writel(g, pri_ringmaster_command_r(), cmd);

	/* poll for clear interrupt done */

	cmd = pri_ringmaster_command_cmd_v(
		nvgpu_readl(g, pri_ringmaster_command_r()));
	while ((cmd != pri_ringmaster_command_cmd_no_cmd_v()) && (retries != 0U)) {
		nvgpu_udelay(poll_delay);
		cmd = pri_ringmaster_command_cmd_v(
			nvgpu_readl(g, pri_ringmaster_command_r()));
		retries--;
	}

	if (retries == 0U) {
		nvgpu_err(g, "priv ringmaster intr ack failed");
	}

	if (g->ops.priv_ring.intr_retrigger != NULL) {
		g->ops.priv_ring.intr_retrigger(g);
	}
}
