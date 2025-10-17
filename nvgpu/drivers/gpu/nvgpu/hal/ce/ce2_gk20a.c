// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/kmem.h>
#include <nvgpu/dma.h>
#include <nvgpu/log.h>
#include <nvgpu/enabled.h>
#include <nvgpu/io.h>
#include <nvgpu/utils.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/mc.h>
#include <nvgpu/channel.h>
#include <nvgpu/engines.h>

#include "ce2_gk20a.h"

#include <nvgpu/hw/gk20a/hw_ce2_gk20a.h>

void gk20a_ce2_stall_isr(struct gk20a *g, u32 inst_id, u32 pri_base,
				bool *needs_rc, bool *needs_quiesce)
{
	u32 ce2_intr = nvgpu_readl(g, ce2_intr_status_r());
	u32 clear_intr = 0U;

	(void)needs_quiesce;
	(void)inst_id;
	(void)pri_base;

	nvgpu_log(g, gpu_dbg_intr, "ce2 isr %08x", ce2_intr);

	/* clear blocking interrupts: they exibit broken behavior */
	if ((ce2_intr & ce2_intr_status_blockpipe_pending_f()) != 0U) {
		nvgpu_log(g, gpu_dbg_intr, "ce2 blocking pipe interrupt");
		clear_intr |= ce2_intr_status_blockpipe_pending_f();
	}
	if ((ce2_intr & ce2_intr_status_launcherr_pending_f()) != 0U) {
		nvgpu_log(g, gpu_dbg_intr, "ce2 launch error interrupt");
		*needs_rc = true;
		clear_intr |= ce2_intr_status_launcherr_pending_f();
	}

	nvgpu_writel(g, ce2_intr_status_r(), clear_intr);
}

u32 gk20a_ce2_nonstall_isr(struct gk20a *g, u32 inst_id, u32 pri_base)
{
	u32 ops = 0U;
	u32 ce2_intr = nvgpu_readl(g, ce2_intr_status_r());

	(void)inst_id;
	(void)pri_base;

	nvgpu_log(g, gpu_dbg_intr, "ce2 nonstall isr %08x", ce2_intr);

	if ((ce2_intr & ce2_intr_status_nonblockpipe_pending_f()) != 0U) {
		nvgpu_log(g, gpu_dbg_intr, "ce2 non-blocking pipe interrupt");
		nvgpu_writel(g, ce2_intr_status_r(),
			ce2_intr_status_nonblockpipe_pending_f());
		ops |= (NVGPU_CIC_NONSTALL_OPS_WAKEUP_SEMAPHORE |
			NVGPU_CIC_NONSTALL_OPS_POST_EVENTS);
	}
	return ops;
}
