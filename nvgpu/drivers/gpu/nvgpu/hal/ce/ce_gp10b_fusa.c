// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/mc.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/nvgpu_err.h>

#include "ce_gp10b.h"

#include <nvgpu/hw/gp10b/hw_ce_gp10b.h>

void gp10b_ce_stall_isr(struct gk20a *g, u32 inst_id, u32 pri_base,
				bool *needs_rc, bool *needs_quiesce)
{
	u32 ce_intr = nvgpu_readl(g, ce_intr_status_r(inst_id));
	u32 clear_intr = 0U;

	(void)needs_quiesce;
	(void)pri_base;
	(void)needs_rc;

	nvgpu_log(g, gpu_dbg_intr, "ce isr %08x %08x", ce_intr, inst_id);

	/* clear blocking interrupts: they exibit broken behavior */
	if ((ce_intr & ce_intr_status_blockpipe_pending_f()) != 0U) {
		nvgpu_err(g, "ce blocking pipe interrupt");
		clear_intr |= ce_intr_status_blockpipe_pending_f();
	}

	nvgpu_writel(g, ce_intr_status_r(inst_id), clear_intr);
	return;
}
