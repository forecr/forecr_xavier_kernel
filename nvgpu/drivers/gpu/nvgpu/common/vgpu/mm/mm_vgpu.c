// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/ce_app.h>

#include <nvgpu/vgpu/vgpu.h>

#include "mm_vgpu.h"

static int vgpu_init_mm_setup_sw(struct gk20a *g)
{
	struct mm_gk20a *mm = &g->mm;
	int err;

	nvgpu_log_fn(g, " ");

	if (mm->sw_ready) {
		nvgpu_log_fn(g, "skip init");
		return 0;
	}

	nvgpu_mutex_init(&mm->tlb_lock);

	mm->g = g;

	/*TBD: make channel vm size configurable */
	g->ops.mm.get_default_va_sizes(NULL, &mm->channel.user_size,
		&mm->channel.kernel_size);

	nvgpu_log_info(g, "channel vm size: user %dMB  kernel %dMB",
		       (int)(mm->channel.user_size >> 20),
		       (int)(mm->channel.kernel_size >> 20));

	if (g->ops.mm.init_bar2_vm != NULL) {
		err = g->ops.mm.init_bar2_vm(g);
		if (err != 0) {
			return err;
		}
	}

#ifdef CONFIG_NVGPU_COMPRESSION_RAW
	mm->cbc.ce_ctx_id = NVGPU_CE_INVAL_CTX_ID;
	err = nvgpu_init_ce_vm(mm);
	if (err != 0) {
		nvgpu_err(g, "nvgpu_init_ce_vm failed");
		return err;
	}
#endif

	if (g->ops.mm.mmu_fault.setup_sw != NULL) {
		err = g->ops.mm.mmu_fault.setup_sw(g);
		if (err != 0) {
			return err;
		}
	}

	mm->sw_ready = true;

	return 0;
}

static int vgpu_init_mm_setup_hw(struct gk20a *g)
{
	struct mm_gk20a *mm = &g->mm;
	int err = 0;

	nvgpu_log_fn(g, " ");

	if (g->ops.fb.init_hw != NULL) {
		g->ops.fb.init_hw(g);
	}

	if (g->ops.bus.bar2_bind != NULL) {
		err = g->ops.bus.bar2_bind(g, &mm->bar2.inst_block);
		if (err != 0) {
			return err;
		}
	}

	if (g->ops.mm.mmu_fault.setup_hw != NULL) {
		g->ops.mm.mmu_fault.setup_hw(g);
	}

	return 0;
}

int vgpu_init_mm_support(struct gk20a *g)
{
	int err;

	nvgpu_log_fn(g, " ");

	err = vgpu_init_mm_setup_sw(g);
	if (err) {
		return err;
	}

	err = vgpu_init_mm_setup_hw(g);
	if (err) {
		return err;
	}

	return err;
}

static int vgpu_cache_maint(struct gk20a *g, u8 op)
{
	struct tegra_vgpu_cmd_msg msg;
	u64 handle = vgpu_get_handle(g);
	struct tegra_vgpu_cache_maint_params *p = &msg.params.cache_maint;
	int err;

	msg.cmd = TEGRA_VGPU_CMD_CACHE_MAINT;
	msg.handle = handle;
	p->op = op;
	err = vgpu_comm_sendrecv(g, &msg);
	WARN_ON(err || msg.ret);
	return err;
}

int vgpu_mm_fb_flush(struct gk20a *g)
{

	nvgpu_log_fn(g, " ");

	return vgpu_cache_maint(g, TEGRA_VGPU_FB_FLUSH);
}

int vgpu_mm_l2_flush(struct gk20a *g, bool invalidate)
{
	u8 op;

	nvgpu_log_fn(g, " ");

	if (invalidate) {
		op = TEGRA_VGPU_L2_MAINT_FLUSH_INV;
	} else {
		op =  TEGRA_VGPU_L2_MAINT_FLUSH;
	}

	return vgpu_cache_maint(g, op);
}

#ifdef CONFIG_NVGPU_DEBUGGER
void vgpu_mm_mmu_set_debug_mode(struct gk20a *g, bool enable)
{
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_mmu_debug_mode *p = &msg.params.mmu_debug_mode;
	int err;

	nvgpu_log_fn(g, " ");

	msg.cmd = TEGRA_VGPU_CMD_SET_MMU_DEBUG_MODE;
	msg.handle = vgpu_get_handle(g);
	p->enable = (u32)enable;
	err = vgpu_comm_sendrecv(g, &msg);
	WARN_ON(err || msg.ret);
}
#endif
