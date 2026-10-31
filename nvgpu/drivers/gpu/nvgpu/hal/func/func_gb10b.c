// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/hw/gb10b/hw_func_gb10b.h>
#include <nvgpu/io.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/trace.h>
#include <nvgpu/utils.h>

#include "hal/func/func_gb10b.h"

int gb10b_func_tlb_flush(struct gk20a *g)
{
	struct nvgpu_timeout timeout;
	struct mm_gk20a *mm = &g->mm;
	struct nvgpu_mem *pdb;
	struct vm_gk20a *vm;
	u64 addr;
	u32 addr_lo;
	u32 addr_hi;
	u32 data;
	int err = 0;

	nvgpu_log(g, gpu_dbg_mm, " ");

	/**
	 * prepare_poweroff. When gk20a deinit releases those pagetables,
	 * common code in vm unmap path calls tlb invalidate that touches
	 * hw. Use the power_on flag to skip tlb invalidation when gpu
	 * power is turned off.
	 */

	if (nvgpu_is_powered_off(g)) {
		return err;
	}

	vm = mm->bar2.vm;

	if (!vm) {
		nvgpu_log(g, gpu_dbg_mm, "tlb flush is called before bar2 is bound");
		return err;
	}

	pdb = vm->pdb.mem;

	addr = nvgpu_mem_get_addr(g, pdb) >>
		func_priv_mmu_invalidate_pdb_addr_alignment_v();
	addr_lo = u64_lo32(addr);
	addr_hi = u64_hi32(addr);

	nvgpu_mutex_acquire(&g->mm.tlb_lock);

#ifdef CONFIG_NVGPU_TRACE
	trace_gk20a_mm_tlb_invalidate(g->name);
#endif

	nvgpu_func_writel(g, func_priv_mmu_invalidate_pdb_r(),
		func_priv_mmu_invalidate_pdb_addr_f(addr_lo) |
		nvgpu_aperture_mask(g, pdb,
			func_priv_mmu_invalidate_pdb_aperture_sys_mem_f(),
			func_priv_mmu_invalidate_pdb_aperture_sys_mem_f(),
			func_priv_mmu_invalidate_pdb_aperture_vid_mem_f()));
	nvgpu_func_writel(g, func_priv_mmu_invalidate_upper_pdb_r(),
		func_priv_mmu_invalidate_upper_pdb_addr_f(addr_hi));
	nvgpu_func_writel(g, func_priv_mmu_invalidate_r(),
		func_priv_mmu_invalidate_hubtlb_only_true_f() |
		func_priv_mmu_invalidate_scope_non_link_tlbs_f() |
		func_priv_mmu_invalidate_trigger_true_f());

	nvgpu_timeout_init_retry(g, &timeout, 1000);
	do {
		data = nvgpu_func_readl(g, func_priv_mmu_invalidate_r());
		if (func_priv_mmu_invalidate_trigger_v(data) ==
			func_priv_mmu_invalidate_trigger_false_v()) {
			break;
		}
		nvgpu_udelay(2);
	} while (nvgpu_timeout_expired_msg(&timeout,
					 "wait mmu invalidate") == 0);

	if (nvgpu_timeout_peek_expired(&timeout)) {
		err = -ETIMEDOUT;
		goto out;
	}

#ifdef CONFIG_NVGPU_TRACE
	trace_gk20a_mm_tlb_invalidate_done(g->name);
#endif

out:
	nvgpu_mutex_release(&g->mm.tlb_lock);
	return err;
}

int gb10b_func_tlb_invalidate(struct gk20a *g, struct nvgpu_mem *pdb)
{
	struct nvgpu_timeout timeout;
	u64 addr;
	u32 addr_lo;
	u32 addr_hi;
	u32 data;
	int err = 0;

	nvgpu_log(g, gpu_dbg_mm, " ");

	/**
	 * prepare_poweroff. When gk20a deinit releases those pagetables,
	 * common code in vm unmap path calls tlb invalidate that touches
	 * hw. Use the power_on flag to skip tlb invalidation when gpu
	 * power is turned off.
	 */

	if (nvgpu_is_powered_off(g)) {
		return err;
	}

	addr = nvgpu_mem_get_addr(g, pdb) >>
		func_priv_mmu_invalidate_pdb_addr_alignment_v();
	addr_lo = u64_lo32(addr);
	addr_hi = u64_hi32(addr);

	nvgpu_mutex_acquire(&g->mm.tlb_lock);

#ifdef CONFIG_NVGPU_TRACE
	trace_gk20a_mm_tlb_invalidate(g->name);
#endif

	nvgpu_func_writel(g, func_priv_mmu_invalidate_pdb_r(),
		func_priv_mmu_invalidate_pdb_addr_f(addr_lo) |
		nvgpu_aperture_mask(g, pdb,
			func_priv_mmu_invalidate_pdb_aperture_sys_mem_f(),
			func_priv_mmu_invalidate_pdb_aperture_sys_mem_f(),
			func_priv_mmu_invalidate_pdb_aperture_vid_mem_f()));
	nvgpu_func_writel(g, func_priv_mmu_invalidate_upper_pdb_r(),
		func_priv_mmu_invalidate_upper_pdb_addr_f(addr_hi));
	nvgpu_func_writel(g, func_priv_mmu_invalidate_r(),
		func_priv_mmu_invalidate_all_va_true_f() |
		func_priv_mmu_invalidate_trigger_true_f());

	nvgpu_timeout_init_retry(g, &timeout, 1000);
	do {
		data = nvgpu_func_readl(g, func_priv_mmu_invalidate_r());
		if (func_priv_mmu_invalidate_trigger_v(data) ==
			func_priv_mmu_invalidate_trigger_false_v()) {
			break;
		}
		nvgpu_udelay(2);
	} while (nvgpu_timeout_expired_msg(&timeout,
					 "wait mmu invalidate") == 0);

	if (nvgpu_timeout_peek_expired(&timeout)) {
		err = -ETIMEDOUT;
		goto out;
	}

#ifdef CONFIG_NVGPU_TRACE
	trace_gk20a_mm_tlb_invalidate_done(g->name);
#endif

out:
	nvgpu_mutex_release(&g->mm.tlb_lock);
	return err;
}

/* TODO: add to dev_nv_xal_addendum.h */
#define MEMOP_MAX_OUTSTANDING 140U

void gb10b_func_l2_invalidate_locked(struct gk20a *g)
{
	u32 data, token_mask, start_token, complete_token;
	struct nvgpu_timeout timeout;
	u32 retries = 200;

#ifdef CONFIG_NVGPU_TRACE
	trace_gk20a_mm_l2_invalidate(g->name);
#endif
	if (g->ops.mm.get_flush_retries != NULL) {
		retries = g->ops.mm.get_flush_retries(g, NVGPU_FLUSH_L2_INV);
	}

	token_mask = BIT32(func_priv_func_l2_system_invalidate_token_s()) - 1U;

	nvgpu_timeout_init_retry(g, &timeout, retries);

	/* Invalidate any clean lines from the L2 so subsequent reads go to
	   DRAM. Dirty lines are not affected by this operation. */
	data = nvgpu_func_readl(g, func_priv_func_l2_system_invalidate_r());
	start_token = func_priv_func_l2_system_invalidate_token_v(data);

	do {
		data = nvgpu_func_readl(g, func_priv_func_l2_system_invalidate_completed_r());

		if (func_priv_func_l2_system_invalidate_completed_status_v(data) ==
			func_priv_func_l2_system_invalidate_completed_status_idle_v()) {
			break;
		}

		complete_token = func_priv_func_l2_system_invalidate_completed_token_v(data);
		/* The loop will wait only when completedToken in the range of
		 * [startToken-NV_XAL_EP_MEMOP_MAX_OUTSTANDING, startToken].
		 */
		if (((start_token - complete_token) & token_mask) > MEMOP_MAX_OUTSTANDING) {
			break;
		}

		nvgpu_udelay(5);
	} while (nvgpu_timeout_expired(&timeout) == 0);

	if (nvgpu_timeout_peek_expired(&timeout)) {
		nvgpu_warn(g, "l2_system_invalidate too many retries");
	}

#ifdef CONFIG_NVGPU_TRACE
	trace_gk20a_mm_l2_invalidate_done(g->name);
#endif
}

void gb10b_func_l2_invalidate(struct gk20a *g)
{
	struct mm_gk20a *mm = &g->mm;
	gk20a_busy_noresume(g);
	if (!nvgpu_is_powered_off(g)) {
		nvgpu_mutex_acquire(&mm->l2_op_lock);
		gb10b_func_l2_invalidate_locked(g);
		nvgpu_mutex_release(&mm->l2_op_lock);
	}
	gk20a_idle_nosuspend(g);
}

int gb10b_func_bar1_bind(struct gk20a *g, struct nvgpu_mem *bar1_inst)
{
	struct nvgpu_timeout timeout;
	int err = 0;
	u64 iova = nvgpu_inst_block_addr(g, bar1_inst);
	/*
	 * BAR1 address is 4K-aligned. So, lower 12-bits of address are not
	 * specified. We are storing the upper 32-bit of the 64-bit address
	 * pointer as BAR1_BLOCK_HIGH_ADDR[31:0] and the lower 20-bit of the
	 * 64-bit pointer as BAR1_BLOCK_LOW_ADDR[31:12].
	 */
	u32 ptr_low_v = u64_lo32(iova >> func_bar1_block_ptr_shift_v());
	u32 ptr_high_v = u64_hi32(iova);

	nvgpu_log_info(g, "bar1 inst block high_addr_ptr(0x%08x), "
			"low_addr_ptr(0x%08x)", ptr_high_v, ptr_low_v);

	nvgpu_func_writel(g, func_bar1_block_low_addr_r(),
		     nvgpu_aperture_mask(g, bar1_inst,
					 func_bar1_block_low_addr_target_sys_mem_ncoh_f(),
					 func_bar1_block_low_addr_target_sys_mem_coh_f(),
					 func_bar1_block_low_addr_target_vid_mem_f()) |
		     func_bar1_block_low_addr_mode_virtual_f() |
		     func_bar1_block_low_addr_ptr_f(ptr_low_v));

	nvgpu_func_writel(g, func_bar1_block_high_addr_r(),
		     func_bar1_block_high_addr_ptr_f(ptr_high_v));

	nvgpu_timeout_init_retry(g, &timeout, 1000);

	do {
		u32 val = nvgpu_func_readl(g, func_bar1_block_low_addr_r());
		u32 pending = func_bar1_block_low_addr_bar1_pending_v(val);
		u32 outstanding = func_bar1_block_low_addr_bar1_outstanding_v(val);
		if ((pending == 0U) && (outstanding == 0U)) {
			break;
		}

		nvgpu_udelay(5);
	} while (nvgpu_timeout_expired(&timeout) == 0);

	if (nvgpu_timeout_peek_expired(&timeout)) {
		err = -EINVAL;
	}

	return err;
}

int gb10b_func_bar2_bind(struct gk20a *g, struct nvgpu_mem *bar2_inst)
{
	struct nvgpu_timeout timeout;
	int err = 0;
	u64 iova = nvgpu_inst_block_addr(g, bar2_inst);
	/*
	 * BAR2 address is 4K-aligned. So, lower 12-bits of address are not
	 * specified. We are storing the upper 32-bit of the 64-bit address
	 * pointer as BAR2_BLOCK_HIGH_ADDR[31:0] and the lower 20-bit of the
	 * 64-bit pointer as BAR2_BLOCK_LOW_ADDR[31:12].
	 */
	u32 ptr_low_v = u64_lo32(iova >> func_bar2_block_ptr_shift_v());
	u32 ptr_high_v = u64_hi32(iova);

	nvgpu_log_info(g, "bar2 inst block high_addr_ptr(0x%08x), "
			"low_addr_ptr(0x%08x)", ptr_high_v, ptr_low_v);

	nvgpu_func_writel(g, func_bar2_block_low_addr_r(),
		     nvgpu_aperture_mask(g, bar2_inst,
					 func_bar2_block_low_addr_target_sys_mem_ncoh_f(),
					 func_bar2_block_low_addr_target_sys_mem_coh_f(),
					 func_bar2_block_low_addr_target_vid_mem_f()) |
		     func_bar2_block_low_addr_mode_virtual_f() |
		     func_bar2_block_low_addr_ptr_f(ptr_low_v));

	nvgpu_func_writel(g, func_bar2_block_high_addr_r(),
		     func_bar2_block_high_addr_ptr_f(ptr_high_v));

	nvgpu_timeout_init_retry(g, &timeout, 1000);

	do {
		u32 val = nvgpu_func_readl(g, func_bar2_block_low_addr_r());
		u32 pending = func_bar2_block_low_addr_bar2_pending_v(val);
		u32 outstanding = func_bar2_block_low_addr_bar2_outstanding_v(val);
		if ((pending == 0U) && (outstanding == 0U)) {
			break;
		}

		nvgpu_udelay(5);
	} while (nvgpu_timeout_expired(&timeout) == 0);

	if (nvgpu_timeout_peek_expired(&timeout)) {
		err = -EINVAL;
	}

	return err;
}

void gb10b_func_write_mmu_fault_buffer_lo_hi(struct gk20a *g, u32 index,
		u32 addr_lo, u32 addr_hi)
{
	nvgpu_func_writel(g, func_priv_mmu_fault_buffer_lo_r(index), addr_lo);
	nvgpu_func_writel(g, func_priv_mmu_fault_buffer_hi_r(index), addr_hi);
}

u32 gb10b_func_read_mmu_fault_buffer_get(struct gk20a *g, u32 index)
{
	return nvgpu_func_readl(g, func_priv_mmu_fault_buffer_get_r(index));
}

void gb10b_func_write_mmu_fault_buffer_get(struct gk20a *g, u32 index,
		u32 reg_val)
{
	nvgpu_func_writel(g, func_priv_mmu_fault_buffer_get_r(index), reg_val);
}

u32 gb10b_func_read_mmu_fault_buffer_put(struct gk20a *g, u32 index)
{
	return nvgpu_func_readl(g, func_priv_mmu_fault_buffer_put_r(index));
}

u32 gb10b_func_read_mmu_fault_buffer_size(struct gk20a *g, u32 index)
{
	return nvgpu_func_readl(g, func_priv_mmu_fault_buffer_size_r(index));
}

void gb10b_func_write_mmu_fault_buffer_size(struct gk20a *g, u32 index,
		u32 reg_val)
{
	nvgpu_func_writel(g, func_priv_mmu_fault_buffer_size_r(index), reg_val);
}

void gb10b_func_read_mmu_fault_addr_lo_hi(struct gk20a *g,
		u32 *addr_lo, u32 *addr_hi)
{
	*addr_lo = nvgpu_func_readl(g, func_priv_mmu_fault_addr_lo_r());
	*addr_hi = nvgpu_func_readl(g, func_priv_mmu_fault_addr_hi_r());
}

void gb10b_func_read_mmu_fault_inst_lo_hi(struct gk20a *g,
		u32 *inst_lo, u32 *inst_hi)
{
	*inst_lo = nvgpu_func_readl(g, func_priv_mmu_fault_inst_lo_r());
	*inst_hi = nvgpu_func_readl(g, func_priv_mmu_fault_inst_hi_r());
}

u32 gb10b_func_read_mmu_fault_info(struct gk20a *g)
{
	return nvgpu_func_readl(g, func_priv_mmu_fault_info_r());
}

u32 gb10b_func_read_mmu_fault_status(struct gk20a *g)
{
	return nvgpu_func_readl(g, func_priv_mmu_fault_status_r());
}

void gb10b_func_write_mmu_fault_status(struct gk20a *g, u32 reg_val)
{
	nvgpu_func_writel(g, func_priv_mmu_fault_status_r(), reg_val);
}
