// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/class.h>
#include <nvgpu/channel.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/debug.h>

#include <nvgpu/gr/config.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/gr/ctx.h>
#include <nvgpu/gr/gr_falcon.h>
#include <nvgpu/gr/gr_intr.h>
#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/gr/gr_instances.h>

#include "common/gr/gr_priv.h"
#include "common/gr/gr_intr_priv.h"

#include "gr_intr_gp10b.h"

#include <nvgpu/hw/gp10b/hw_gr_gp10b.h>

void gp10b_gr_intr_handle_class_error(struct gk20a *g, u32 chid,
				      struct nvgpu_gr_isr_data *isr_data)
{
	u32 gr_class_error;
	u32 offset_bit_shift = 2U;
	u32 data_hi_set = 0U;

	gr_class_error =
		gr_class_error_code_v(nvgpu_readl(g, gr_class_error_r()));

	nvgpu_err(g, "class error 0x%08x, offset 0x%08x,"
		"sub channel 0x%08x mme generated %d,"
		" mme pc 0x%08xdata high %d priv status %d"
		" unhandled intr 0x%08x for channel %u",
		isr_data->class_num, (isr_data->offset << offset_bit_shift),
		gr_trapped_addr_subch_v(isr_data->addr),
		gr_trapped_addr_mme_generated_v(isr_data->addr),
		gr_trapped_data_mme_pc_v(
			nvgpu_readl(g, gr_trapped_data_mme_r())),
		gr_trapped_addr_datahigh_v(isr_data->addr),
		gr_trapped_addr_priv_v(isr_data->addr),
		gr_class_error, chid);

	nvgpu_err(g, "trapped data low 0x%08x",
		nvgpu_readl(g, gr_trapped_data_lo_r()));
	data_hi_set = gr_trapped_addr_datahigh_v(isr_data->addr);
	if (data_hi_set != 0U) {
		nvgpu_err(g, "trapped data high 0x%08x",
		nvgpu_readl(g, gr_trapped_data_hi_r()));
	}
}

#ifdef CONFIG_NVGPU_CILP
static int gp10b_gr_intr_clear_cilp_preempt_pending(struct gk20a *g,
					       u32 tsgid)
{
	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gpu_dbg | gpu_dbg_intr, " ");

	/*
	 * The ucode is self-clearing, so all we need to do here is
	 * to clear cilp_preempt_pending.
	 */
	if (!nvgpu_gr_get_cilp_preempt_pending(g)) {
		nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gpu_dbg | gpu_dbg_intr,
				"CILP is already cleared for tsgid %d\n",
				tsgid);
		return 0;
	}

	nvgpu_gr_set_cilp_preempt_pending(g, false);
	nvgpu_gr_clear_cilp_preempt_pending_tsgid(g);

	return 0;
}

static int gp10b_gr_intr_get_cilp_preempt_pending_tsgid(struct gk20a *g,
					u32 *tsgid_ptr)
{
	u32 tsgid;
	int ret = -EINVAL;

	tsgid = nvgpu_gr_get_cilp_preempt_pending_tsgid(g);
	if (tsgid == NVGPU_INVALID_TSG_ID) {
		return ret;
	}

	if (nvgpu_gr_get_cilp_preempt_pending(g)) {
		*tsgid_ptr = tsgid;
		ret = 0;
	}

	return ret;
}
#endif /* CONFIG_NVGPU_CILP */

int gp10b_gr_intr_handle_fecs_error(struct gk20a *g,
				struct nvgpu_channel *ch_ptr,
				struct nvgpu_gr_isr_data *isr_data)
{
#ifdef CONFIG_NVGPU_CILP
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr(g);
	u32 tsgid = NVGPU_INVALID_TSG_ID;
	int ret = 0;
	struct nvgpu_fecs_host_intr_status *fecs_host_intr;
#endif
	u32 gr_fecs_intr = isr_data->fecs_intr;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gpu_dbg | gpu_dbg_intr, " ");

	if (gr_fecs_intr == 0U) {
		return 0;
	}


#ifdef CONFIG_NVGPU_CILP
	fecs_host_intr = &isr_data->fecs_host_intr_status;
	/*
	 * INTR1 (bit 1 of the HOST_INT_STATUS_CTXSW_INTR)
	 * indicates that a CILP ctxsw save has finished
	 */
	if (fecs_host_intr->ctxsw_intr1 != 0U) {
		nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gpu_dbg | gpu_dbg_intr,
				"CILP: ctxsw save completed!\n");

		gr->can_broadcast_stop_trigger = true;

		/* now clear the interrupt */
		g->ops.gr.falcon.fecs_host_clear_intr(g,
					fecs_host_intr->ctxsw_intr1);
		/**
		 * clear the interrupt from isr_data too. This is
		 * for nvgpu_gr_intr_handle_fecs_error to not handle
		 * already handled interrupt.
		 */
		isr_data->fecs_intr &= ~(fecs_host_intr->ctxsw_intr1);
		fecs_host_intr->ctxsw_intr1 = 0U;

		ret = gp10b_gr_intr_get_cilp_preempt_pending_tsgid(g, &tsgid);
		if ((ret != 0) || (tsgid == NVGPU_INVALID_TSG_ID)) {
			nvgpu_err(g, "CILP: Failed to get preempt pending TSG ID");
			g->ops.gr.falcon.dump_stats(g);
			gk20a_fecs_gr_debug_dump(g);
			goto clean_up;
		}

		/* set preempt_pending to false */
		ret = gp10b_gr_intr_clear_cilp_preempt_pending(g, tsgid);
		if (ret != 0) {
			nvgpu_err(g, "CILP: error while unsetting CILP preempt pending!");
			g->ops.gr.falcon.dump_stats(g);
			gk20a_fecs_gr_debug_dump(g);
			goto clean_up;
		}

#ifdef CONFIG_NVGPU_DEBUGGER
		g->ops.gr.intr.handle_cilp_save_completed(g, gr->cilp_preempt_pending_gfid, tsgid);
#endif
	}

clean_up:
#endif /* CONFIG_NVGPU_CILP */

	/* handle any remaining interrupts */
	return nvgpu_gr_intr_handle_fecs_error(g, ch_ptr, isr_data);
}

#if defined(CONFIG_NVGPU_DEBUGGER) && defined(CONFIG_NVGPU_GRAPHICS)
void gp10b_gr_intr_set_go_idle_timeout(struct gk20a *g, u32 data)
{
	nvgpu_writel(g, gr_fe_go_idle_timeout_r(), data);
}

void gp10b_gr_intr_set_coalesce_buffer_size(struct gk20a *g, u32 data)
{
	u32 val;

	nvgpu_log_fn(g, " ");

	val = nvgpu_readl(g, gr_gpcs_tc_debug0_r());
	val = set_field(val, gr_gpcs_tc_debug0_limit_coalesce_buffer_size_m(),
			gr_gpcs_tc_debug0_limit_coalesce_buffer_size_f(data));
	nvgpu_writel(g, gr_gpcs_tc_debug0_r(), val);

	nvgpu_log_fn(g, "done");
}
#endif
