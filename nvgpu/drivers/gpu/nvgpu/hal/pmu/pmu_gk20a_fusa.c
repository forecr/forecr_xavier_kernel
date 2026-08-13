/*
 * Copyright (c) 2011-2023, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <nvgpu/pmu/debug.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>

#include <nvgpu/hw/gk20a/hw_pwr_gk20a.h>

#include "pmu_gk20a.h"

u32 gk20a_pmu_get_irqmask(struct gk20a *g)
{
	u32 mask = 0U;

	mask = nvgpu_readl(g, pwr_falcon_irqmask_r());
	mask &= nvgpu_readl(g, pwr_falcon_irqdest_r());

	return mask;
}

void gk20a_pmu_set_mailbox1(struct gk20a *g, u32 val)
{
	nvgpu_writel(g, pwr_falcon_mailbox1_r(), val);
}

u32 gk20a_pmu_get_irqstat(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_falcon_irqstat_r());
}

void gk20a_pmu_set_irqsclr(struct gk20a *g, u32 intr)
{
	nvgpu_writel(g, pwr_falcon_irqsclr_r(), intr);
}

void gk20a_pmu_set_irqsset(struct gk20a *g, u32 intr)
{
	nvgpu_writel(g, pwr_falcon_irqsset_r(), intr);
}

u32 gk20a_pmu_get_exterrstat(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_falcon_exterrstat_r());
}

void gk20a_pmu_set_exterrstat(struct gk20a *g, u32 intr)
{
	nvgpu_writel(g, pwr_falcon_exterrstat_r(), intr);
}

u32 gk20a_pmu_get_exterraddr(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_falcon_exterraddr_r());
}

u32 gk20a_pmu_get_bar0_addr(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_bar0_addr_r());
}

u32 gk20a_pmu_get_bar0_data(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_bar0_data_r());
}

u32 gk20a_pmu_get_bar0_timeout(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_bar0_timeout_r());
}

u32 gk20a_pmu_get_bar0_ctl(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_bar0_ctl_r());
}

u32 gk20a_pmu_get_bar0_error_status(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_bar0_error_status_r());
}

void gk20a_pmu_set_bar0_error_status(struct gk20a *g, u32 val)
{
	return nvgpu_writel(g, pwr_pmu_bar0_error_status_r(), val);
}

u32 gk20a_pmu_get_bar0_fecs_error(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_bar0_fecs_error_r());
}

void gk20a_pmu_set_bar0_fecs_error(struct gk20a *g, u32 val)
{
	return nvgpu_writel(g, pwr_pmu_bar0_fecs_error_r(), val);
}

u32 gk20a_pmu_get_mailbox(struct gk20a *g, u32 i)
{
	return nvgpu_readl(g, pwr_pmu_mailbox_r(i));
}

u32 gk20a_pmu_get_pmu_debug(struct gk20a *g, u32 i)
{
	return nvgpu_readl(g, pwr_pmu_debug_r(i));
}

u32 gk20a_pmu_get_mutex_reg(struct gk20a *g, u32 i)
{
	return nvgpu_readl(g, pwr_pmu_mutex_r(i));
}

void gk20a_pmu_set_mutex_reg(struct gk20a *g, u32 i, u32 data)
{
	nvgpu_writel(g, pwr_pmu_mutex_r(i), data);
}

u32 gk20a_pmu_get_mutex_id(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_mutex_id_r());
}

u32 gk20a_pmu_get_mutex_id_release(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_mutex_id_release_r());
}

void gk20a_pmu_set_mutex_id_release(struct gk20a *g, u32 data)
{
	nvgpu_writel(g, pwr_pmu_mutex_id_release_r(), data);
}

u32 gk20a_pmu_get_pmu_msgq_head(struct gk20a *g)
{
	return nvgpu_readl(g, pwr_pmu_msgq_head_r());
}

void gk20a_pmu_set_pmu_msgq_head(struct gk20a *g, u32 data)
{
	nvgpu_writel(g, pwr_pmu_msgq_head_r(), data);
}

void gk20a_pmu_set_new_instblk(struct gk20a *g, u32 data)
{
	nvgpu_writel(g, pwr_pmu_new_instblk_r(), data);
}

void gk20a_pmu_isr(struct gk20a *g)
{
	struct nvgpu_pmu *pmu = g->pmu;
	u32 intr = 0U;
	u32 mask = 0U;

	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&pmu->isr_mutex);

	intr = g->ops.pmu.get_irqstat(g);
	mask = g->ops.pmu.get_irqmask(g);
	nvgpu_pmu_dbg(g, "received PMU interrupt: stat:0x%08x mask:0x%08x",
			intr, mask);

	if (!pmu->isr_enabled || ((intr & mask) == 0U)) {
		nvgpu_log_info(g,
			"clearing unhandled interrupt: stat:0x%08x mask:0x%08x",
			intr, mask);
		g->ops.pmu.set_irqsclr(g, intr);
		nvgpu_mutex_release(&pmu->isr_mutex);
		return;
	}

	intr = intr & mask;

	if (g->ops.pmu.handle_ext_irq != NULL) {
		g->ops.pmu.handle_ext_irq(g, intr);
	}

	g->ops.pmu.set_irqsclr(g, intr);

#ifdef CONFIG_NVGPU_LS_PMU
	if (nvgpu_pmu_get_fw_state(g, pmu) == PMU_FW_STATE_OFF) {
		nvgpu_mutex_release(&pmu->isr_mutex);
		return;
	}

	gk20a_pmu_handle_interrupts(g, intr);
#endif

	nvgpu_mutex_release(&pmu->isr_mutex);
}
