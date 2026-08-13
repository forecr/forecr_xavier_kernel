// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include "riscv_gb10b.h"

#include <nvgpu/hw/gb10b/hw_priscv_gb10b.h>

u32 gb10b_riscv_cpuctl_offset(void)
{
	return priscv_priscv_cpuctl_r();
}

void gb10b_riscv_dump_debug_regs(struct nvgpu_falcon *flcn,
				struct nvgpu_debug_context *o)
{
	gk20a_debug_output(o, "NV_PRISCV_RISCV_BCR_CTRL : 0x%x",
		nvgpu_riscv_readl(flcn, priscv_priscv_bcr_ctrl_r()));
	gk20a_debug_output(o, "NV_PRISCV_RISCV_CPUCTL : 0x%x",
		nvgpu_riscv_readl(flcn, priscv_priscv_cpuctl_r()));
	gk20a_debug_output(o, "NV_PRISCV_RISCV_BR_RETCODE : 0x%x",
		nvgpu_riscv_readl(flcn, priscv_priscv_br_retcode_r()));
	gk20a_debug_output(o, "NV_PRISCV_RISCV_IRQMASK : 0x%x",
		nvgpu_riscv_readl(flcn, priscv_riscv_irqmask_r()));
	gk20a_debug_output(o, "NV_PRISCV_RISCV_IRQDEST : 0x%x",
		nvgpu_riscv_readl(flcn, priscv_riscv_irqdest_r()));
	gk20a_debug_output(o, "NV_PRISCV_RISCV_INTR_STATUS : 0x%x",
		nvgpu_riscv_readl(flcn, priscv_riscv_intr_status_r()));
	gk20a_debug_output(o, "NV_PRISCV_RISCV_FAULT_CONTAINMENT_SRCSTAT : 0x%x",
		nvgpu_riscv_readl(flcn, priscv_riscv_fault_containment_srcstat_r()));
}

void gb10b_riscv_dump_trace_info(struct nvgpu_falcon *flcn,
				struct nvgpu_debug_context *o)
{
	u32 traceidx, depth, trace_count, idx;
	u32 tracepc_lo, tracepc_hi;

	gk20a_debug_output(o, "\nNV_PRISCV_RISCV_TRACECTL: : 0x%x",
			priscv_riscv_tracectl_r());
	traceidx = priscv_riscv_trace_wtidx_wtidx_v(
			nvgpu_riscv_readl(flcn, priscv_riscv_trace_wtidx_r()));
	depth = priscv_riscv_trace_rdidx_maxidx_v(
			nvgpu_riscv_readl(flcn, priscv_riscv_trace_rdidx_r()));
	trace_count = min(32U, depth);

	for (idx = 0U; idx < trace_count; idx++) {
		traceidx = (nvgpu_safe_add_u32(traceidx, depth - 1U)) % depth;
		nvgpu_riscv_writel(flcn, priscv_riscv_trace_rdidx_r(),
				priscv_riscv_trace_rdidx_rdidx_f(traceidx));
		tracepc_lo = nvgpu_riscv_readl(flcn, priscv_riscv_tracepc_lo_r());
		tracepc_hi = nvgpu_riscv_readl(flcn, priscv_riscv_tracepc_hi_r());
		gk20a_debug_output(o, "TRACE_PC[%02x]: 0x%08x 0x%08x",
				idx, tracepc_hi, tracepc_lo);
	}
}
