// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/io.h>
#include <nvgpu/mc.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/engines.h>

#include "mc_gv11b.h"

#include <nvgpu/hw/gv11b/hw_mc_gv11b.h>

bool gv11b_mc_is_intr_hub_pending(struct gk20a *g, u32 mc_intr_0)
{
	(void)g;
	return ((mc_intr_0 & mc_intr_hub_pending_f()) != 0U);
}

bool gv11b_mc_is_stall_and_eng_intr_pending(struct gk20a *g, u32 engine_id,
			u32 *eng_intr_pending)
{
	u32 mc_intr_0 = nvgpu_readl(g, mc_intr_r(NVGPU_CIC_INTR_STALLING));
	u32 stall_intr, eng_intr_mask;

	eng_intr_mask = nvgpu_engine_act_interrupt_mask(g, engine_id);
	*eng_intr_pending = mc_intr_0 & eng_intr_mask;

	stall_intr = mc_intr_pfifo_pending_f() |
			mc_intr_hub_pending_f() |
			mc_intr_priv_ring_pending_f() |
			mc_intr_ltc_pending_f();

	nvgpu_log(g, gpu_dbg_info | gpu_dbg_intr,
		"mc_intr_0 = 0x%08x, eng_intr = 0x%08x",
		mc_intr_0 & stall_intr, *eng_intr_pending);

	return (mc_intr_0 & (eng_intr_mask | stall_intr)) != 0U;
}

bool gv11b_mc_is_mmu_fault_pending(struct gk20a *g)
{
	return g->ops.fb.intr.is_mmu_fault_pending(g);
}
