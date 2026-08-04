// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/io.h>
#include <nvgpu/timers.h>
#include <nvgpu/gk20a.h>

#include "hal/fb/fb_mmu_fault_gb10b.h"

#include <nvgpu/hw/gb10b/hw_fb_gb10b.h>

void gb10b_fb_write_mmu_fault_buffer_lo_hi(struct gk20a *g, u32 index,
		u32 addr_lo, u32 addr_hi)
{
	nvgpu_writel(g, fb_mmu_fault_buffer_lo_r(index), addr_lo);
	nvgpu_writel(g, fb_mmu_fault_buffer_hi_r(index), addr_hi);
}

u32 gb10b_fb_read_mmu_fault_buffer_get(struct gk20a *g, u32 index)
{
	return nvgpu_readl(g, fb_mmu_fault_buffer_get_r(index));
}

void gb10b_fb_write_mmu_fault_buffer_get(struct gk20a *g, u32 index,
		u32 reg_val)
{
	nvgpu_writel(g, fb_mmu_fault_buffer_get_r(index), reg_val);
}

u32 gb10b_fb_read_mmu_fault_buffer_put(struct gk20a *g, u32 index)
{
	return nvgpu_readl(g, fb_mmu_fault_buffer_put_r(index));
}

u32 gb10b_fb_read_mmu_fault_buffer_size(struct gk20a *g, u32 index)
{
	return nvgpu_readl(g, fb_mmu_fault_buffer_size_r(index));
}

void gb10b_fb_write_mmu_fault_buffer_size(struct gk20a *g, u32 index,
		u32 reg_val)
{
	nvgpu_writel(g, fb_mmu_fault_buffer_size_r(index), reg_val);
}

void gb10b_fb_read_mmu_fault_addr_lo_hi(struct gk20a *g,
		u32 *addr_lo, u32 *addr_hi)
{
	*addr_lo = nvgpu_readl(g, fb_mmu_fault_addr_lo_r());
	*addr_hi = nvgpu_readl(g, fb_mmu_fault_addr_hi_r());
}

void gb10b_fb_read_mmu_fault_inst_lo_hi(struct gk20a *g,
		u32 *inst_lo, u32 *inst_hi)
{
	*inst_lo = nvgpu_readl(g, fb_mmu_fault_inst_lo_r());
	*inst_hi = nvgpu_readl(g, fb_mmu_fault_inst_hi_r());
}

u32 gb10b_fb_read_mmu_fault_info(struct gk20a *g)
{
	return nvgpu_readl(g, fb_mmu_fault_info_r());
}

u32 gb10b_fb_read_mmu_fault_status(struct gk20a *g)
{
	return nvgpu_readl(g, fb_mmu_fault_status_r());
}

void gb10b_fb_write_mmu_fault_status(struct gk20a *g, u32 reg_val)
{

	nvgpu_writel(g, fb_mmu_fault_status_r(), reg_val);
}

int gb10b_fb_mmu_invalidate_replay(struct gk20a *g,
		u32 invalidate_replay_val)
{
	int err = -ETIMEDOUT;
	u32 reg_val;
	struct nvgpu_timeout timeout;

	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&g->mm.tlb_lock);

	reg_val = nvgpu_readl(g, fb_mmu_invalidate_r());

	reg_val |= fb_mmu_invalidate_all_va_true_f() |
		fb_mmu_invalidate_all_pdb_true_f() |
		invalidate_replay_val |
		fb_mmu_invalidate_trigger_true_f();

	nvgpu_writel(g, fb_mmu_invalidate_r(), reg_val);

	nvgpu_timeout_init_retry(g, &timeout, 200U);

	do {
		reg_val = nvgpu_readl(g, fb_mmu_ctrl_r());
		if (fb_mmu_ctrl_pri_fifo_empty_v(reg_val) !=
			fb_mmu_ctrl_pri_fifo_empty_false_f()) {
			err = 0;
			break;
		}
		nvgpu_udelay(5);
	} while (nvgpu_timeout_expired_msg(&timeout,
			"invalidate replay failed 0x%x",
			invalidate_replay_val) == 0);
	if (err != 0) {
		nvgpu_err(g, "invalidate replay timedout");
	}

	nvgpu_mutex_release(&g->mm.tlb_lock);

	return err;
}
