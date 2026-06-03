// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/mc.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/nvgpu_err.h>

#include "ce_gp10b.h"

#include <nvgpu/hw/gp10b/hw_ce_gp10b.h>

u32 gp10b_ce_nonstall_isr(struct gk20a *g, u32 inst_id, u32 pri_base)
{
	u32 nonstall_ops = 0U;
	u32 ce_intr = nvgpu_readl(g, ce_intr_status_r(inst_id));

	(void)pri_base;

	nvgpu_log(g, gpu_dbg_intr, "ce nonstall isr %08x %08x",
			ce_intr, inst_id);

	if ((ce_intr & ce_intr_status_nonblockpipe_pending_f()) != 0U) {
		nvgpu_writel(g, ce_intr_status_r(inst_id),
			ce_intr_status_nonblockpipe_pending_f());
		nonstall_ops |= (NVGPU_CIC_NONSTALL_OPS_WAKEUP_SEMAPHORE |
			NVGPU_CIC_NONSTALL_OPS_POST_EVENTS);
	}

	return nonstall_ops;
}
