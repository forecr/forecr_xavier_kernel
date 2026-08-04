// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2013-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/kmem.h>
#include <nvgpu/log.h>
#include <nvgpu/vm.h>
#include <nvgpu/atomic.h>
#include <nvgpu/mm.h>
#include <nvgpu/bug.h>
#include <nvgpu/io.h>
#include <nvgpu/utils.h>
#include <nvgpu/channel.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/debugger.h>
#include <nvgpu/profiler.h>
#include <nvgpu/power_features/power_features.h>

#include "hal/gr/gr/gr_gk20a.h"
#include <nvgpu/regops.h>
#include "common/gr/gr_priv.h"
#include "common/gr/gr_config_priv.h"
#include <nvgpu/gr/gr_instances.h>
#include <nvgpu/gr/ctx.h>
#include <nvgpu/regops.h>
#include <nvgpu/gr/config.h>
#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/gr/gr_instances.h>
#include <nvgpu/grmgr.h>
#if defined(CONFIG_NVGPU_GB10B)
#include <nvgpu/hw/gb10b/hw_gr_gb10b.h>
#elif
#include <nvgpu/hw/ga10b/hw_gr_ga10b.h>
#endif

/*
 * API to get first channel from the list of all channels
 * bound to the debug session
 */
struct nvgpu_channel *
nvgpu_dbg_gpu_get_session_channel(struct dbg_session_gk20a *dbg_s)
{
	struct dbg_session_channel_data *ch_data;
	struct nvgpu_channel *ch;
	struct gk20a *g = dbg_s->g;

	nvgpu_mutex_acquire(&dbg_s->ch_list_lock);
	if (nvgpu_list_empty(&dbg_s->ch_list)) {
		nvgpu_mutex_release(&dbg_s->ch_list_lock);
		return NULL;
	}

	ch_data = nvgpu_list_first_entry(&dbg_s->ch_list,
				   dbg_session_channel_data,
				   ch_entry);
	ch = g->fifo.channel + ch_data->chid;

	nvgpu_mutex_release(&dbg_s->ch_list_lock);

	return ch;
}

static void nvgpu_dbg_gpu_post_events_for_ch(struct gk20a *g, struct nvgpu_channel *ch)
{
	struct dbg_session_data *session_data;
	struct dbg_session_gk20a *dbg_s;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gpu_dbg, " ");

	/* guard against the session list being modified */
	nvgpu_mutex_acquire(&ch->dbg_s_lock);

	nvgpu_list_for_each_entry(session_data, &ch->dbg_s_list,
				dbg_session_data, dbg_s_entry) {
		dbg_s = session_data->dbg_s;
		if (dbg_s->dbg_events.events_enabled) {
			nvgpu_log(g, gpu_dbg_gpu_dbg, "posting event on session id %d",
					dbg_s->id);
			nvgpu_log(g, gpu_dbg_gpu_dbg, "%d events pending",
					dbg_s->dbg_events.num_pending_events);

			dbg_s->dbg_events.num_pending_events++;

			nvgpu_dbg_session_post_event(dbg_s);
		}
	}

	nvgpu_mutex_release(&ch->dbg_s_lock);
}

void nvgpu_dbg_gpu_post_events(struct gk20a *g, u32 gfid, u32 tsgid)
{
	struct nvgpu_tsg *tsg = NULL;
	struct nvgpu_channel *ch = NULL;

	(void)gfid;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gpu_dbg, " ");

	tsg = nvgpu_tsg_check_and_get_from_id(g, tsgid);
	if (tsg == NULL) {
		nvgpu_err(g, "invalid tsgid %u", tsgid);
		return;
	}

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);
	nvgpu_list_for_each_entry(ch, &tsg->ch_list, nvgpu_channel, ch_entry) {
		if (nvgpu_channel_get(ch) != NULL) {
			nvgpu_dbg_gpu_post_events_for_ch(g, ch);
			nvgpu_channel_put(ch);
		}
	}
	nvgpu_rwsem_up_read(&tsg->ch_list_lock);
}

u32 nvgpu_set_powergate_locked(struct dbg_session_gk20a *dbg_s,
				bool mode)
{
	u32 err = 0U;
	struct gk20a *g = dbg_s->g;

	if (dbg_s->is_pg_disabled != mode) {
		if (mode == false) {
			g->dbg_powergating_disabled_refcount--;
		}

		/*
		 * Allow powergate disable or enable only if
		 * the global pg disabled refcount is zero
		 */
		if (g->dbg_powergating_disabled_refcount == 0) {
			err = (u32)g->ops.debugger.dbg_set_powergate(dbg_s,
									mode);
		}

		if (mode) {
			g->dbg_powergating_disabled_refcount++;
		}

		dbg_s->is_pg_disabled = mode;
	}

	return err;
}

int nvgpu_dbg_set_powergate(struct dbg_session_gk20a *dbg_s, bool disable_powergate)
{
	int err = 0;
	struct gk20a *g = dbg_s->g;

	 /* This function must be called with g->dbg_sessions_lock held */

	nvgpu_log(g, gpu_dbg_fn|gpu_dbg_gpu_dbg, "%s powergate mode = %s",
		   g->name, disable_powergate ? "disable" : "enable");

	/*
	 * Powergate mode here refers to railgate+powergate+clockgate
	 * so in case slcg/blcg/elcg are disabled and railgating is enabled,
	 * disable railgating and then set is_pg_disabled = true
	 * Similarly re-enable railgating and not other features if they are not
	 * enabled when powermode=MODE_ENABLE
	 */
	if (disable_powergate) {
		/* save off current powergate, clk state.
		 * set gpu module's can_powergate = 0.
		 * set gpu module's clk to max.
		 * while *a* debug session is active there will be no power or
		 * clocking state changes allowed from mainline code (but they
		 * should be saved).
		 */

		nvgpu_log(g, gpu_dbg_gpu_dbg | gpu_dbg_fn,
						"module busy");
		err = gk20a_busy(g);
		if (err != 0) {
			return err;
		}

#ifdef CONFIG_NVGPU_NON_FUSA
		err = nvgpu_cg_pg_disable(g);
#endif
		if (err == 0) {
			dbg_s->is_pg_disabled = true;
			nvgpu_log(g, gpu_dbg_gpu_dbg | gpu_dbg_fn,
					"pg disabled");
		}
	} else {
		/* restore (can) powergate, clk state */
		/* release pending exceptions to fault/be handled as usual */
		/*TBD: ordering of these? */

#ifdef CONFIG_NVGPU_NON_FUSA
		err = nvgpu_cg_pg_enable(g);
#endif
		if (err == 0) {
			dbg_s->is_pg_disabled = false;
			nvgpu_log(g, gpu_dbg_gpu_dbg | gpu_dbg_fn,
					"pg enabled");
		}

		nvgpu_log(g, gpu_dbg_gpu_dbg | gpu_dbg_fn, "module idle");

		gk20a_idle(g);
	}

	nvgpu_log(g, gpu_dbg_fn|gpu_dbg_gpu_dbg, "%s powergate mode = %s done",
		   g->name, disable_powergate ? "disable" : "enable");
	return err;
}

int nvgpu_dbg_gpu_set_sm_dbgr_ctrl_single_step_mode(
		struct dbg_session_gk20a *dbg_s, u32 sm_id, bool enable)
{
	u32 gpc, tpc, sm, offset;
	u32 reg_mask, reg_val;
	u32 flags = NVGPU_REG_OP_FLAG_MODE_ALL_OR_NONE;
	int err = 0;
	struct gk20a *g = dbg_s->g;
	struct nvgpu_channel *ch = NULL;
	struct nvgpu_tsg *tsg = NULL;
	struct nvgpu_sm_info *sm_info = NULL;
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr(g);
	struct nvgpu_gr_config *gr_config_local =
		nvgpu_gr_get_gpu_instance_config_ptr(g, dbg_s->gpu_instance_id);
	struct nvgpu_dbg_reg_op *ops = NULL;
	u32 debugger_ctrl_base = g->ops.gr.get_sm_dbgr_ctrl_base();

	ch = nvgpu_dbg_gpu_get_session_channel(dbg_s);
	if (ch == NULL) {
		nvgpu_err(g, "no bound channel for sm dbg single step mode");
		return -EINVAL;
	}

	tsg = nvgpu_tsg_from_ch(ch);
	if (tsg == NULL) {
		nvgpu_err(g, "Channel not bound to TSG");
		return -EINVAL;
	}

	if (sm_id >= nvgpu_gr_config_get_no_of_sm(gr_config_local)) {
		return -EINVAL;
	}
	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gpu_dbg, "sm_id %d enable %u", sm_id, enable);

	ops = nvgpu_kzalloc(g, sizeof(*ops));
	if (ops == NULL) {
		nvgpu_err(g, "not enough memory for allocation");
		return -ENOMEM;
	}

	err = gk20a_busy(g);
	if (err) {
		nvgpu_err(g, "failed to poweron");
		goto busy_err;
	}

	nvgpu_mutex_acquire(&g->dbg_sessions_lock);

#ifdef CONFIG_NVGPU_SM_DIVERSITY
	if (nvgpu_gr_ctx_get_sm_diversity_config(tsg->gr_ctx) ==
			NVGPU_DEFAULT_SM_DIVERSITY_CONFIG) {
		sm_info =
			nvgpu_gr_config_get_sm_info(
					gr->config, sm_id);
	} else {
		sm_info =
			nvgpu_gr_config_get_redex_sm_info(
					gr->config, sm_id);
	}
#else
	sm_info = nvgpu_gr_config_get_sm_info(gr->config, sm_id);
#endif
	if (sm_info == NULL) {
		nvgpu_err(g, "Failed to get SM info");
		goto sm_info_err;
	}

	gpc = nvgpu_gr_config_get_sm_info_gpc_index(sm_info);
	tpc = nvgpu_gr_config_get_sm_info_tpc_index(sm_info);
	sm = nvgpu_gr_config_get_sm_info_sm_index(sm_info);
	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gpu_dbg,
			"sm_id %d gpc %d tpc %d sm %d", sm_id, gpc, tpc, sm);

	offset = nvgpu_safe_add_u32(nvgpu_safe_add_u32(nvgpu_gr_gpc_offset(g, gpc),
				nvgpu_gr_tpc_offset(g, tpc)), nvgpu_gr_sm_offset(g, sm));

	ops->op = REGOP(WRITE_32);
	ops->type = REGOP(TYPE_GR_CTX);
	ops->offset = debugger_ctrl_base + offset;

	reg_mask = 0;
	reg_val = 0;
	if (enable) {
		nvgpu_log(g, gpu_dbg_gpu_dbg,
				"SM:%d debugger single step mode ON", sm_id);
		reg_mask |=
			gr_gpc0_tpc0_sm0_dbgr_control0_single_step_mode_m();
		reg_val |=
			gr_gpc0_tpc0_sm0_dbgr_control0_single_step_mode_enable_f();
	} else {
		nvgpu_log(g, gpu_dbg_gpu_dbg,
				"SM:%d debuggger single step mode Off", sm_id);
		reg_mask |=
			gr_gpc0_tpc0_sm0_dbgr_control0_single_step_mode_m();
		reg_val |=
			gr_gpc0_tpc0_sm0_dbgr_control0_single_step_mode_disable_f();
	}

	ops->and_n_mask_lo = reg_mask;
	ops->value_lo = reg_val;

	err = gr_gk20a_exec_ctx_ops(tsg, ops, 1, 1, 0, &flags);
	if (err != 0) {
		nvgpu_err(g, "Failed to access register");
	}
	nvgpu_err(g, "ATUL register successfully updated");

sm_info_err:
	nvgpu_mutex_release(&g->dbg_sessions_lock);
	gk20a_idle(g);
busy_err:
	nvgpu_kfree(g, ops);
	return  err;
}

