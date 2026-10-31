// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/cic_mon.h>
#include <nvgpu/cic_rm.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/bug.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/trace.h>

#include "cic_mon_priv.h"

void nvgpu_cic_mon_intr_mask(struct gk20a *g)
{
	unsigned long flags = 0;

	if (nvgpu_gin_is_present(g)) {
		nvgpu_gin_intr_mask_all(g);
	} else {
		if (g->ops.mc.intr_mask != NULL) {
			nvgpu_spinlock_irqsave(&g->mc.intr_lock, flags);
			g->ops.mc.intr_mask(g);
			nvgpu_spinunlock_irqrestore(&g->mc.intr_lock, flags);
		}
	}
}

void nvgpu_cic_mon_intr_stall_unit_config(struct gk20a *g, u32 unit, bool enable)
{
	unsigned long flags = 0;

	nvgpu_spinlock_irqsave(&g->mc.intr_lock, flags);
	g->ops.mc.intr_stall_unit_config(g, unit, enable);
	nvgpu_spinunlock_irqrestore(&g->mc.intr_lock, flags);
}

#ifdef CONFIG_NVGPU_NONSTALL_INTR
void nvgpu_cic_mon_intr_nonstall_unit_config(struct gk20a *g, u32 unit, bool enable)
{
	unsigned long flags = 0;

	nvgpu_spinlock_irqsave(&g->mc.intr_lock, flags);
	g->ops.mc.intr_nonstall_unit_config(g, unit, enable);
	nvgpu_spinunlock_irqrestore(&g->mc.intr_lock, flags);
}
#endif

#ifndef CONFIG_NVGPU_MON_PRESENT
void nvgpu_cic_mon_intr_stall_pause(struct gk20a *g, u64 intr_mask)
{
	unsigned long flags = 0;

	if (nvgpu_gin_is_present(g)) {
		nvgpu_gin_intr_stall_pause(g, intr_mask);
	} else {
		nvgpu_spinlock_irqsave(&g->mc.intr_lock, flags);
		g->ops.mc.intr_stall_pause(g);
		nvgpu_spinunlock_irqrestore(&g->mc.intr_lock, flags);
	}
}

void nvgpu_cic_mon_intr_stall_resume(struct gk20a *g, u64 intr_mask)
{
	unsigned long flags = 0;

	if (nvgpu_gin_is_present(g)) {
		nvgpu_gin_intr_stall_resume(g, intr_mask);
	} else {
		nvgpu_spinlock_irqsave(&g->mc.intr_lock, flags);
		g->ops.mc.intr_stall_resume(g);
		nvgpu_spinunlock_irqrestore(&g->mc.intr_lock, flags);
	}
}
#endif

#ifdef CONFIG_NVGPU_NONSTALL_INTR
void nvgpu_cic_mon_intr_nonstall_pause(struct gk20a *g)
{
	unsigned long flags = 0;

	if (nvgpu_gin_is_present(g)) {
		nvgpu_gin_intr_nonstall_pause(g);
	} else {
		nvgpu_spinlock_irqsave(&g->mc.intr_lock, flags);
		g->ops.mc.intr_nonstall_pause(g);
		nvgpu_spinunlock_irqrestore(&g->mc.intr_lock, flags);
	}
}

void nvgpu_cic_mon_intr_nonstall_resume(struct gk20a *g)
{
	unsigned long flags = 0;

	if (nvgpu_gin_is_present(g)) {
		nvgpu_gin_intr_nonstall_resume(g);
	} else {
		nvgpu_spinlock_irqsave(&g->mc.intr_lock, flags);
		g->ops.mc.intr_nonstall_resume(g);
		nvgpu_spinunlock_irqrestore(&g->mc.intr_lock, flags);
	}
}

static void nvgpu_cic_mon_intr_nonstall_work(struct gk20a *g, u32 work_ops)
{
	bool semaphore_wakeup, post_events;

	semaphore_wakeup =
		(((work_ops & NVGPU_CIC_NONSTALL_OPS_WAKEUP_SEMAPHORE) != 0U) ?
					true : false);
	post_events = (((work_ops & NVGPU_CIC_NONSTALL_OPS_POST_EVENTS) != 0U) ?
					true : false);

	if (semaphore_wakeup) {
		g->ops.semaphore_wakeup(g, post_events);
	}
}

static u32 nvgpu_cic_mon_intr_nonstall_isr_with_mask(struct gk20a *g, u64 intr_mask)
{
	u64 non_stall_intr_val = 0U;

	if (nvgpu_is_powered_off(g)) {
		return NVGPU_CIC_INTR_UNMASK;
	}

	/* not from gpu when sharing irq with others */
	if (nvgpu_gin_is_present(g)) {
		non_stall_intr_val = nvgpu_gin_nonstall_isr(g, intr_mask);
	} else {
		non_stall_intr_val = (u64)g->ops.mc.intr_nonstall(g);
	}
	if (non_stall_intr_val == 0U) {
		return NVGPU_CIC_INTR_NONE;
	}

	nvgpu_cic_mon_intr_nonstall_pause(g);
	if (g->sw_quiesce_pending) {
		return NVGPU_CIC_INTR_QUIESCE_PENDING;
	}

	nvgpu_cic_rm_set_irq_nonstall(g, 1);

	return NVGPU_CIC_INTR_HANDLE;
}

u32 nvgpu_cic_mon_intr_nonstall_isr(struct gk20a *g)
{
	u64 intr_mask = 0UL;

	if (nvgpu_gin_is_present(g)) {
		intr_mask = g->gin.config->nonstall_intr_top_mask;
	}

	return nvgpu_cic_mon_intr_nonstall_isr_with_mask(g, intr_mask);
}

static void nvgpu_cic_mon_intr_nonstall_handle_with_mask(struct gk20a *g, u64 intr_mask)
{
	u32 nonstall_ops = 0;

	if (nvgpu_gin_is_present(g)) {
		nonstall_ops = nvgpu_gin_nonstall_isr_thread(g, intr_mask);
	} else {
		nonstall_ops = g->ops.mc.isr_nonstall(g);
	}
	if (nonstall_ops != 0U) {
		nvgpu_cic_mon_intr_nonstall_work(g, nonstall_ops);
	}

	/* sync handled irq counter before re-enabling interrupts */
	nvgpu_cic_rm_set_irq_nonstall(g, 0);

	nvgpu_cic_mon_intr_nonstall_resume(g);

	(void)nvgpu_cic_rm_broadcast_last_irq_nonstall(g);
}

void nvgpu_cic_mon_intr_nonstall_handle(struct gk20a *g)
{
	u64 intr_mask = 0UL;

	if (nvgpu_gin_is_present(g)) {
		intr_mask = g->gin.config->nonstall_intr_top_mask;
	}

	nvgpu_cic_mon_intr_nonstall_handle_with_mask(g, intr_mask);
}
#endif
#ifdef CONFIG_NVGPU_MON_PRESENT
int nvgpu_cic_mon_handle_fatal_intr(struct gk20a *g)
{
	if (nvgpu_is_powered_off(g)) {
		nvgpu_err(g, "GPU is already powered off");
		return -ENODEV;
	}
	if (nvgpu_gin_is_present(g)) {
		nvgpu_gin_stall_isr_thread(g, g->gin.config->stall_intr_top_mask);
	} else {
		g->ops.mc.isr_stall(g);
	}

	return 0;
}
#endif

static u32 nvgpu_cic_mon_intr_stall_isr_with_mask(struct gk20a *g, u64 intr_mask)
{
#ifdef CONFIG_NVGPU_MON_PRESENT
	(void)g;
	(void)(intr_mask);
	return NVGPU_CIC_INTR_HANDLE;
#else
	u64 intr_status = 0U;
	bool handled = false;

	nvgpu_trace_intr_stall_start(g);

	if (nvgpu_is_powered_off(g)) {
		return NVGPU_CIC_INTR_UNMASK;
	}

	/* not from gpu when sharing irq with others */
	if (nvgpu_gin_is_present(g)) {
		intr_status = nvgpu_gin_stall_isr(g, intr_mask);
	} else {
		intr_status = (u64)g->ops.mc.intr_stall(g);
	}
	if (intr_status == 0U) {
		return NVGPU_CIC_INTR_NONE;
	}

	nvgpu_cic_mon_intr_stall_pause(g, intr_status);

	if (g->sw_quiesce_pending) {
		return NVGPU_CIC_INTR_QUIESCE_PENDING;
	}

	nvgpu_cic_rm_set_irq_stall(g, 1);

	if (nvgpu_gin_is_present(g)) {
		handled = nvgpu_gin_handle_gr_illegal_method(g, intr_status);

		handled = handled || nvgpu_gin_handle_nvdec_swgen0(g, intr_status);

		if (handled) {
			nvgpu_cic_rm_set_irq_stall(g, 0);
			nvgpu_cic_mon_intr_stall_resume(g, intr_status);

			(void)nvgpu_cic_rm_broadcast_last_irq_stall(g);

			return NVGPU_CIC_INTR_NONE;
		}
	}

	nvgpu_trace_intr_stall_done(g);

	return NVGPU_CIC_INTR_HANDLE;
#endif
}

u32 nvgpu_cic_mon_intr_stall_isr(struct gk20a *g)
{
#ifdef CONFIG_NVGPU_MON_PRESENT
	(void)g;
	return NVGPU_CIC_INTR_HANDLE;
#else
	u64 intr_mask = 0UL;

	if (nvgpu_gin_is_present(g)) {
		intr_mask = g->gin.config->stall_intr_top_mask;
	}

	return nvgpu_cic_mon_intr_stall_isr_with_mask(g, intr_mask);
#endif
}

static void nvgpu_cic_mon_intr_stall_handle_with_mask(struct gk20a *g, u64 intr_mask)
{
#ifdef CONFIG_NVGPU_MON_PRESENT
	(void)g;
	(void)(intr_mask);
#else
	if (nvgpu_gin_is_present(g)) {
		nvgpu_mutex_acquire(&g->gin.intr_thread_mutex);
		nvgpu_gin_stall_isr_thread(g, intr_mask);
	} else {
		nvgpu_mutex_acquire(&g->mc.intr_thread_mutex);
		g->ops.mc.isr_stall(g);
	}

	/* sync handled irq counter before re-enabling interrupts */
	nvgpu_cic_rm_set_irq_stall(g, 0);

	nvgpu_cic_mon_intr_stall_resume(g, intr_mask);

	if (nvgpu_gin_is_present(g)) {
		nvgpu_mutex_release(&g->gin.intr_thread_mutex);
	} else {
		nvgpu_mutex_release(&g->mc.intr_thread_mutex);
	}

	(void)nvgpu_cic_rm_broadcast_last_irq_stall(g);
#endif
}

void nvgpu_cic_mon_intr_stall_handle(struct gk20a *g)
{
#ifdef CONFIG_NVGPU_MON_PRESENT
	(void)g;
#else
	u64 intr_mask = 0UL;

	if (nvgpu_gin_is_present(g)) {
		intr_mask = g->gin.config->stall_intr_top_mask;
	}

	nvgpu_cic_mon_intr_stall_handle_with_mask(g, intr_mask);
#endif
}

u32 nvgpu_cic_mon_intr_msi_isr(struct gk20a *g, u32 id)
{
	u64 intr_mask = 0UL;
	u32 ret = NVGPU_CIC_INTR_NONE;

	nvgpu_assert(id < 64U);
	intr_mask = BIT64(id);

	if (!nvgpu_gin_is_present(g)) {
		nvgpu_err(g, "MSI/MSI-X interrupts not supported without GIN");
		return NVGPU_CIC_INTR_NONE;
	}

	if (nvgpu_gin_msi_is_stall(g, id)) {
		ret = nvgpu_cic_mon_intr_stall_isr_with_mask(g, intr_mask);
	}
#if defined(CONFIG_NVGPU_NONSTALL_INTR)
	else {
		nvgpu_assert(nvgpu_gin_msi_is_nonstall(g, id));
		ret = nvgpu_cic_mon_intr_nonstall_isr_with_mask(g, intr_mask);
	}
#endif

	return ret;
}

void nvgpu_cic_mon_intr_msi_handle(struct gk20a *g, u32 id)
{
	u64 intr_mask = 0UL;

	nvgpu_assert(id < 64U);
	intr_mask = BIT64(id);

	if (!nvgpu_gin_is_present(g)) {
		nvgpu_err(g, "MSI/MSI-X interrupts not supported without GIN");
		return;
	}

	if (nvgpu_gin_msi_is_stall(g, id)) {
		nvgpu_cic_mon_intr_stall_handle_with_mask(g, intr_mask);
	}
#if defined(CONFIG_NVGPU_NONSTALL_INTR)
	else {
		nvgpu_assert(nvgpu_gin_msi_is_nonstall(g, id));
		nvgpu_cic_mon_intr_nonstall_handle_with_mask(g, intr_mask);
	}
#endif
}

#ifdef CONFIG_NVGPU_NON_FUSA
void nvgpu_cic_mon_intr_enable(struct gk20a *g)
{
	unsigned long flags = 0;

	if (g->ops.mc.intr_enable != NULL) {
		nvgpu_spinlock_irqsave(&g->mc.intr_lock, flags);
		g->ops.mc.intr_enable(g);
		nvgpu_spinunlock_irqrestore(&g->mc.intr_lock, flags);
	}
}
#endif

void nvgpu_cic_mon_intr_unit_vectorid_init(struct gk20a *g, u32 unit, u32 *vectorid,
		u32 num_entries)
{
	unsigned long flags = 0;
	u32 i = 0U;
	struct nvgpu_intr_unit_info *intr_unit_info;

	nvgpu_assert(num_entries <= NVGPU_CIC_INTR_VECTORID_SIZE_MAX);

	nvgpu_log(g, gpu_dbg_intr, "UNIT=%d, nvecs=%d", unit, num_entries);

	intr_unit_info = g->mc.intr_unit_info;

	nvgpu_spinlock_irqsave(&g->mc.intr_lock, flags);

	if (intr_unit_info[unit].valid == false) {
		for (i = 0U; i < num_entries; i++) {
			nvgpu_log(g, gpu_dbg_intr, " vec[%d] = %d", i,
					*(vectorid + i));
			intr_unit_info[unit].vectorid[i] = *(vectorid + i);
		}
		intr_unit_info[unit].vectorid_size = num_entries;
	}
	nvgpu_spinunlock_irqrestore(&g->mc.intr_lock, flags);
}

bool nvgpu_cic_mon_intr_is_unit_info_valid(struct gk20a *g, u32 unit)
{
	struct nvgpu_intr_unit_info *intr_unit_info;
	bool info_valid = false;

	if (unit >= NVGPU_CIC_INTR_UNIT_MAX) {
		nvgpu_err(g, "invalid unit(%d)", unit);
		return false;
	}

	intr_unit_info = g->mc.intr_unit_info;

	if (intr_unit_info[unit].valid == true) {
		info_valid = true;
	}

	return info_valid;
}

bool nvgpu_cic_mon_intr_get_unit_info(struct gk20a *g, u32 unit, u32 *subtree,
		u64 *subtree_mask)
{
	if (unit >= NVGPU_CIC_INTR_UNIT_MAX) {
		nvgpu_err(g, "invalid unit(%d)", unit);
		return false;
	}
	if (nvgpu_cic_mon_intr_is_unit_info_valid(g, unit) != true) {
		if (g->ops.mc.intr_get_unit_info(g, unit) != true) {
			nvgpu_err(g, "failed to fetch info for unit(%d)", unit);
			return false;
		}
	}
	*subtree = g->mc.intr_unit_info[unit].subtree;
	*subtree_mask = g->mc.intr_unit_info[unit].subtree_mask;
	nvgpu_log(g, gpu_dbg_intr, "subtree(%d) subtree_mask(%llx)",
			*subtree, *subtree_mask);

	return true;
}
