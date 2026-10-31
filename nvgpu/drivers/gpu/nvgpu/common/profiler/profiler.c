// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/pm_reservation.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/profiler.h>
#include <nvgpu/atomic.h>
#include <nvgpu/log.h>
#include <nvgpu/lock.h>
#include <nvgpu/kmem.h>
#include <nvgpu/tsg.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/gr/ctx.h>
#include <nvgpu/perfbuf.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/regops_allowlist.h>
#include <nvgpu/regops.h>
#include <nvgpu/sort.h>
#include <nvgpu/gr/gr_instances.h>
#include <nvgpu/grmgr.h>
#include <nvgpu/power_features/pg.h>

static int nvgpu_profiler_build_regops_allowlist(struct nvgpu_profiler_object *prof);
static void nvgpu_profiler_destroy_regops_allowlist(struct nvgpu_profiler_object *prof);

static nvgpu_atomic_t unique_id = NVGPU_ATOMIC_INIT(0);
static int generate_unique_id(void)
{
	return nvgpu_atomic_add_return(1, &unique_id);
}

int nvgpu_profiler_alloc(struct gk20a *g,
	struct nvgpu_profiler_object **_prof,
	enum nvgpu_profiler_pm_reservation_scope scope,
	u32 gpu_instance_id)
{
	struct nvgpu_profiler_object *prof;
	*_prof = NULL;

	nvgpu_log(g, gpu_dbg_prof, " ");

	prof = nvgpu_kzalloc(g, sizeof(*prof));
	if (prof == NULL) {
		return -ENOMEM;
	}

	prof->prof_handle = (u32)generate_unique_id();
	prof->scope = scope;
	prof->gpu_instance_id = gpu_instance_id;
	prof->g = g;

	nvgpu_mutex_init(&prof->ioctl_lock);
	nvgpu_mutex_init(&prof->bind_lock);
	nvgpu_init_list_node(&prof->prof_obj_entry);
	nvgpu_mutex_acquire(&g->prof_obj_lock);
	nvgpu_list_add(&prof->prof_obj_entry, &g->profiler_objects);
	nvgpu_mutex_release(&g->prof_obj_lock);

	nvgpu_log(g, gpu_dbg_prof, "Allocated profiler handle %u",
		prof->prof_handle);

	*_prof = prof;
	return 0;
}

void nvgpu_profiler_free(struct nvgpu_profiler_object *prof)
{
	struct gk20a *g = prof->g;

	nvgpu_log(g, gpu_dbg_prof, "Free profiler handle %u",
		prof->prof_handle);

	nvgpu_profiler_unbind_context(prof);
	nvgpu_profiler_free_pma_stream(prof, prof->pma_channel_id);
	nvgpu_profiler_pm_resource_release_all(prof);

	nvgpu_list_del(&prof->prof_obj_entry);
	prof->gpu_instance_id = 0U;
	nvgpu_kfree(g, prof);
}

int nvgpu_profiler_bind_context(struct nvgpu_profiler_object *prof,
		struct nvgpu_tsg *tsg)
{
	struct gk20a *g = prof->g;

	nvgpu_log(g, gpu_dbg_prof, "Request to bind tsgid %u with profiler handle %u",
		tsg->tsgid, prof->prof_handle);

	if (tsg->prof != NULL) {
		nvgpu_err(g, "TSG %u is already bound", tsg->tsgid);
		return -EINVAL;
	}

	if (prof->tsg != NULL) {
		nvgpu_err(g, "Profiler object %u already bound!", prof->prof_handle);
		return -EINVAL;
	}

	prof->tsg = tsg;
	tsg->prof = prof;

	nvgpu_log(g, gpu_dbg_prof, "Bind tsgid %u with profiler handle %u successful",
		tsg->tsgid, prof->prof_handle);

	prof->context_init = true;
	return 0;
}

void nvgpu_profiler_pm_resource_release_all(struct nvgpu_profiler_object *prof)
{
	int i;

	for (i = 0; i < NVGPU_PROFILER_PM_RESOURCE_TYPE_COUNT; i++) {
		if (prof->reserved[i]) {
			nvgpu_warn(NULL,
				"Releasing reserved resource %u for handle %u",
				i, prof->prof_handle);
			nvgpu_profiler_pm_resource_release(prof, i,
				i == NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM ?
				prof->pma_channel_id : 0U);
		}
	}
}

int nvgpu_profiler_unbind_context(struct nvgpu_profiler_object *prof)
{
	struct gk20a *g = prof->g;
	struct nvgpu_tsg *tsg = prof->tsg;

	if (prof->bound) {
		nvgpu_warn(g, "Unbinding resources for handle %u",
			prof->prof_handle);
		nvgpu_profiler_unbind_pm_resources(prof);
	}

	if (!prof->context_init) {
		return -EINVAL;
	}

	if (tsg != NULL && !(prof->bound)) {
		tsg->prof = NULL;
		prof->tsg = NULL;

		nvgpu_log(g, gpu_dbg_prof, "Unbind profiler handle %u and tsgid %u",
			prof->prof_handle, tsg->tsgid);
	}

	prof->context_init = false;
	return 0;
}

int nvgpu_profiler_pm_resource_reserve(struct nvgpu_profiler_object *prof,
		enum nvgpu_profiler_pm_resource_type pm_resource,
		u32 *pma_channel_id)
{
	struct gk20a *g = prof->g;
	enum nvgpu_profiler_pm_reservation_scope scope = prof->scope;
	u32 reservation_id = prof->prof_handle;
	int err;
	bool is_ctx_switched = true;

	nvgpu_log(g, gpu_dbg_prof,
		"Request reservation for profiler handle %u, resource %u, scope %u",
		prof->prof_handle, pm_resource, prof->scope);

	if (prof->reserved[pm_resource]) {
		nvgpu_err(g, "Profiler handle %u already has the reservation",
			prof->prof_handle);
		return -EEXIST;
	}

	if (prof->bound) {
		nvgpu_err(g, "PM resources already bound with profiler handle"
			" %u, rejecting reserve request", prof->prof_handle);
		return -EEXIST;
	}

	err = g->ops.pm_reservation.acquire(g, prof->gpu_instance_id, reservation_id,
			pm_resource, scope, 0, pma_channel_id);
	if (err != 0) {
		nvgpu_err(g, "Profiler handle %u denied the reservation, err %d",
			prof->prof_handle, err);
		return err;
	}

	prof->reserved[pm_resource] = true;
	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC) {
		if (prof->ctxsw[NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC]) {
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_SMPC] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
		} else {
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_SMPC] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
		}
	}

	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY) {
		if (prof->ctxsw[NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY]) {
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMON] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_ROUTER] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_TRIGGER] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_CAU] =
				NVGPU_DBG_REG_OP_TYPE_GR_CTX;

			// Refer Bug 4719304
			// PMA registers are not part of context buffer offset
			// map yet. They are to be executed as device level
			// regops till then irrespective of context switch mode.
			if (g->ops.regops.is_hwpm_pma_reg_context_switched != NULL) {
				is_ctx_switched =
					g->ops.regops.is_hwpm_pma_reg_context_switched();
				if (!is_ctx_switched) {
					prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_ROUTER] =
						NVGPU_DBG_REG_OP_TYPE_GLOBAL;
					prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_TRIGGER] =
						NVGPU_DBG_REG_OP_TYPE_GLOBAL;
				}
			}
		} else {
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMON] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_ROUTER] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_TRIGGER] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
			prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_CAU] =
				NVGPU_DBG_REG_OP_TYPE_GLOBAL;
		}
		prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_MM] =
			NVGPU_DBG_REG_OP_TYPE_GLOBAL;
	}

	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM) {
		prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_CHANNEL] =
			NVGPU_DBG_REG_OP_TYPE_GLOBAL;
	}

	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_PC_SAMPLER) {
		prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_PC_SAMPLER] =
			NVGPU_DBG_REG_OP_TYPE_GR_CTX;
	}

	/* ctxsw support does not exist for hes right now */
	if (pm_resource == NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES) {
		prof->reg_op_type[NVGPU_HWPM_REGISTER_TYPE_HES] =
			NVGPU_DBG_REG_OP_TYPE_GLOBAL;
	}

	nvgpu_log(g, gpu_dbg_prof,
		"Granted reservation for profiler handle %u, resource %u, scope %u",
		prof->prof_handle, pm_resource, prof->scope);

	return 0;
}

int nvgpu_profiler_pm_resource_release(struct nvgpu_profiler_object *prof,
	enum nvgpu_profiler_pm_resource_type pm_resource, u32 pma_channel_id)
{
	struct gk20a *g = prof->g;
	u32 reservation_id = prof->prof_handle;
	int err;

	nvgpu_log(g, gpu_dbg_prof,
		"Release reservation for profiler handle %u, resource %u, scope %u",
		prof->prof_handle, pm_resource, prof->scope);

	if (!prof->reserved[pm_resource]) {
		nvgpu_log(g, gpu_dbg_prof,
			"Profiler handle %u resource is not reserved",
			prof->prof_handle);
		return -EINVAL;
	}

	if (prof->bound) {
		nvgpu_log(g, gpu_dbg_prof,
			"PM resources alredy bound with profiler handle %u,"
			" unbinding for reservation release",
			prof->prof_handle);
		err = nvgpu_profiler_unbind_pm_resources(prof);
		if (err != 0) {
			nvgpu_err(g, "Profiler handle %u failed to unbound, err %d",
				prof->prof_handle, err);
			return err;
		}
	}

	err = g->ops.pm_reservation.release(g, prof->gpu_instance_id, reservation_id,
					    pm_resource, 0, pma_channel_id);
	if (err != 0) {
		nvgpu_err(g, "Profiler handle %u does not have valid reservation, err %d",
			prof->prof_handle, err);
		prof->reserved[pm_resource] = false;
		return err;
	}

	prof->reserved[pm_resource] = false;

	nvgpu_log(g, gpu_dbg_prof,
		"Released reservation for profiler handle %u, resource %u, scope %u",
		prof->prof_handle, pm_resource, prof->scope);

	return 0;
}

static bool nvgpu_profiler_is_context_resource(
			struct nvgpu_profiler_object *prof,
			enum nvgpu_profiler_pm_resource_type pm_resource)
{
	return (prof->scope != NVGPU_PROFILER_PM_RESERVATION_SCOPE_DEVICE) ||
		prof->ctxsw[pm_resource];
}

int nvgpu_profiler_bind_smpc(struct gk20a *g,
		u32 gr_instance_id, bool is_ctxsw,
		struct nvgpu_tsg *tsg, u32 reservation_id)
{
	int err = 0;
	(void) reservation_id;

	if (!is_ctxsw) {
		if (nvgpu_is_enabled(g, NVGPU_SUPPORT_SMPC_GLOBAL_MODE)) {
			err = nvgpu_gr_exec_with_err_for_instance(g,
				gr_instance_id,
				g->ops.gr.update_smpc_global_mode(g, true));
		} else {
			err = -EINVAL;
		}
		if (g->ops.gr.init_cau != NULL) {
			err = nvgpu_gr_exec_with_err_for_instance(g,
					gr_instance_id, g->ops.gr.init_cau(g, gr_instance_id));
			if (err != 0) {
				return err;
			}
		}
	} else {
		err = g->ops.gr.update_smpc_ctxsw_mode(g, tsg, true);
		if (err != 0) {
			goto done;
		}
		if (nvgpu_is_enabled(g, NVGPU_SUPPORT_SMPC_GLOBAL_MODE)) {
			err = nvgpu_gr_exec_with_err_for_instance(g,
				gr_instance_id,
				g->ops.gr.update_smpc_global_mode(g, false));
		}
	}

done:
	if (err != 0) {
		nvgpu_err(g, "nvgpu bind smpc failed, err=%d", err);
	}
	return err;
}

int nvgpu_profiler_disable_cau_and_smpc(struct gk20a *g, u32 gr_instance_id)
{
	int err = 0;

	/* Disable CAUs */
	if (g->ops.gr.disable_cau != NULL) {
		err = g->ops.gr.disable_cau(g, gr_instance_id);
		if (err != 0) {
			return err;
		}
	}

	/* Disable SMPC */
	if (g->ops.gr.disable_smpc != NULL) {
		g->ops.gr.disable_smpc(g);
	}

	return 0;
}

int nvgpu_profiler_unbind_smpc(struct gk20a *g, u32 gr_instance_id,
		bool is_ctxsw, struct nvgpu_tsg *tsg, u32 reservation_id)
{
	int err;
	(void) reservation_id;

	if (!is_ctxsw) {
		if (nvgpu_is_enabled(g, NVGPU_SUPPORT_SMPC_GLOBAL_MODE)) {
			err = nvgpu_gr_exec_with_err_for_instance(g,
				gr_instance_id,
				g->ops.gr.update_smpc_global_mode(g, false));
		} else {
			err = -EINVAL;
			return err;
		}
		err = nvgpu_gr_exec_with_err_for_instance(g, gr_instance_id,
				nvgpu_profiler_disable_cau_and_smpc(g, gr_instance_id));
		if (err != 0) {
			return err;
		}
	} else {
		err = g->ops.gr.update_smpc_ctxsw_mode(g, tsg, false);
	}

	if (err != 0) {
		nvgpu_err(g, "nvgpu unbind smpc failed, err=%d", err);
	}
	return err;
}

int nvgpu_profiler_bind_hwpm(struct gk20a *g, u32 gr_instance_id,
		bool is_ctxsw, struct nvgpu_tsg *tsg, bool streamout, u32 reservation_id)
{
	int err = 0;
	u32 mode = streamout ? NVGPU_GR_CTX_HWPM_CTXSW_MODE_STREAM_OUT_CTXSW :
			       NVGPU_GR_CTX_HWPM_CTXSW_MODE_CTXSW;
	(void) reservation_id;

	if (!is_ctxsw) {
		if (g->ops.perf.reset_hwpm_pmm_registers != NULL) {
			g->ops.perf.reset_hwpm_pmm_registers(g, gr_instance_id);
		}
		g->ops.perf.init_hwpm_pmm_register(g, gr_instance_id);
	} else {
		err = g->ops.gr.update_hwpm_ctxsw_mode(
				g, gr_instance_id, tsg, mode);
	}

	return err;
}

int nvgpu_profiler_unbind_hwpm(struct gk20a *g, u32 gr_instance_id,
		bool is_ctxsw, struct nvgpu_tsg *tsg)
{
	int err = 0;
	u32 mode = NVGPU_GR_CTX_HWPM_CTXSW_MODE_NO_CTXSW;

	if (is_ctxsw) {
		err = g->ops.gr.update_hwpm_ctxsw_mode(
				g, gr_instance_id, tsg, mode);
	}

	return err;
}

void nvgpu_profiler_init_pmasys_state(struct gk20a *g,
					u32 gr_instance_id)
{
	/* Once MIG support gets added to Profiler,
	 * gr_instance_id will get consumed
	 */
	(void)gr_instance_id;

	nvgpu_log(g, gpu_dbg_prof, "HWPM PMA being reset");

	if (g->ops.perf.reset_hwpm_pma_registers != NULL) {
		g->ops.perf.reset_hwpm_pma_registers(g);
	}

	if (g->ops.perf.reset_hwpm_pma_trigger_registers != NULL) {
		g->ops.perf.reset_hwpm_pma_trigger_registers(g);
	}
}

int nvgpu_profiler_quiesce_hwpm_streamout_resident(struct gk20a *g,
					u32 gr_instance_id, u32 pma_channel_id)
{
	int err = 0;

	nvgpu_log(g, gpu_dbg_prof,
		"HWPM streamout quiesce in resident state started");

	/* Enable streamout */
	g->ops.perf.pma_stream_enable(g, pma_channel_id, true);

	/* Disable all perfmons */
	g->ops.perf.disable_all_perfmons(g, gr_instance_id);

	/* Wait for routers to idle/quiescent */
	err = g->ops.perf.wait_for_idle_pmm_routers(g, gr_instance_id, pma_channel_id);
	if (err != 0) {
		goto fail;
	}

	/* Wait for PMA to idle/quiescent */
	err = g->ops.perf.wait_for_idle_pma(g);
	if (err != 0) {
		goto fail;
	}

#ifdef CONFIG_NVGPU_NON_FUSA
	nvgpu_profiler_hs_stream_quiesce(g);
#endif

	/* Disable streamout */
	g->ops.perf.pma_stream_enable(g, pma_channel_id, false);

fail:
	if (err != 0) {
		nvgpu_err(g, "Failed to quiesce HWPM streamout in resident state");
	} else {
		nvgpu_log(g, gpu_dbg_prof,
			"HWPM streamout quiesce in resident state successfull");
	}

	return 0;
}

int nvgpu_profiler_quiesce_hwpm_streamout_non_resident(struct gk20a *g,
		struct nvgpu_tsg *tsg)
{
	struct nvgpu_mem *pm_ctx_mem;
	int err;

	nvgpu_log(g, gpu_dbg_prof,
		"HWPM streamout quiesce in non-resident state started");

	if (tsg == NULL || tsg->gr_ctx == NULL) {
		return -EINVAL;
	}

	pm_ctx_mem = nvgpu_gr_ctx_get_ctx_mem(tsg->gr_ctx, NVGPU_GR_CTX_PM_CTX);
	if (pm_ctx_mem == NULL) {
		nvgpu_err(g, "No PM context");
		return -EINVAL;
	}

	err = g->ops.mm.cache.l2_flush(g, true);
	if (err != 0) {
		nvgpu_err(g, "l2_flush failed");
		return err;
	}

	nvgpu_memset(g, pm_ctx_mem, 0U, 0U, pm_ctx_mem->size);
	nvgpu_log(g, gpu_dbg_prof,
		"HWPM streamout quiesce in non-resident state successfull");

	return 0;
}

static int nvgpu_profiler_disable_ctxsw_and_check_is_tsg_ctx_resident(
	struct nvgpu_tsg *tsg)
{
	struct gk20a *g = tsg->g;
	int err;

	err = nvgpu_gr_disable_ctxsw(g);
	if (err != 0) {
		nvgpu_err(g, "unable to stop gr ctxsw");
		return err;
	}

	return g->ops.gr.is_tsg_ctx_resident(tsg);
}

static int nvgpu_profiler_quiesce_hwpm_streamout_ctx(struct gk20a *g,
		u32 gr_instance_id, u32 pma_channel_id,
		struct nvgpu_tsg *tsg,
		void *pma_bytes_available_buffer_cpuva)
{
	bool ctx_resident;
	int err, ctxsw_err;
	u64 bytes_available;

	ctx_resident = nvgpu_gr_exec_with_err_for_instance(g, gr_instance_id,
		nvgpu_profiler_disable_ctxsw_and_check_is_tsg_ctx_resident(tsg));

	if (ctx_resident) {
		err = nvgpu_profiler_quiesce_hwpm_streamout_resident(g,
				gr_instance_id, pma_channel_id);
		if (err == 0) {
			/* wait for all the inflight records from fb-hub to stream out */
			err = nvgpu_perfbuf_update_get_put(g, pma_channel_id, 0U,
					&bytes_available,
					pma_bytes_available_buffer_cpuva, true,
					NULL, NULL);
			if (err != 0) {
				nvgpu_err(g, "Failed to quiesce resident HWPM streamout");
			}
		}
	} else {
		err = nvgpu_profiler_quiesce_hwpm_streamout_non_resident(g, tsg);
		if (err != 0) {
			nvgpu_err(g, "Failed to quiesce non-resident HWPM streamout");
		}
	}

	ctxsw_err = nvgpu_gr_exec_with_err_for_instance(g, gr_instance_id,
					nvgpu_gr_enable_ctxsw(g));
	if (ctxsw_err != 0) {
		nvgpu_err(g, "unable to restart ctxsw!");
		err = ctxsw_err;
	}

	return err;
}

static int nvgpu_profiler_quiesce_hwpm_streamout(struct gk20a *g,
		u32 gr_instance_id, u32 pma_channel_id,
		bool is_ctxsw,
		struct nvgpu_tsg *tsg,
		void *pma_bytes_available_buffer_cpuva)
{
	int ret;
	u64 bytes_available;
	nvgpu_profiler_init_pmasys_state(g, gr_instance_id);

	if (!is_ctxsw) {
		ret = nvgpu_profiler_quiesce_hwpm_streamout_resident(g,
						gr_instance_id, pma_channel_id);
		if (ret == 0) {
			/* wait for all the inflight records from fb-hub to stream out */
			return nvgpu_perfbuf_update_get_put(g, pma_channel_id,
					0U, &bytes_available,
					pma_bytes_available_buffer_cpuva, true,
					NULL, NULL);
		}
	} else {
		ret = nvgpu_profiler_quiesce_hwpm_streamout_ctx(g,
						gr_instance_id, pma_channel_id,
						tsg,
						pma_bytes_available_buffer_cpuva);
	}

	return ret;
}

int nvgpu_profiler_bind_pma_streamout(struct gk20a *g,
		u32 gr_instance_id,
		u32 pma_channel_id,
		u64 pma_buffer_va,
		u32 pma_buffer_size,
		u64 pma_bytes_available_buffer_va,
		u32 reservation_id)
{
	int err;

	(void)gr_instance_id;
	(void)reservation_id;

	err = g->ops.perfbuf.perfbuf_enable(g, pma_channel_id, pma_buffer_va,
					    pma_buffer_size);
	if (err) {
		return err;
	}

	g->ops.perf.bind_mem_bytes_buffer_addr(g, pma_channel_id,
					       pma_bytes_available_buffer_va);

	return 0;
}

int nvgpu_profiler_unbind_pma_streamout(struct gk20a *g,
		u32 gr_instance_id,
		u32 pma_channel_id,
		bool is_ctxsw,
		struct nvgpu_tsg *tsg,
		void *pma_bytes_available_buffer_cpuva,
		u32 reservation_id)
{
	int err;
	(void)reservation_id;

	err = nvgpu_profiler_quiesce_hwpm_streamout(g,
			gr_instance_id, pma_channel_id,
			is_ctxsw, tsg,
			pma_bytes_available_buffer_cpuva);
	if (err) {
		return err;
	}

	g->ops.perf.bind_mem_bytes_buffer_addr(g, pma_channel_id, 0ULL);

	err = g->ops.perfbuf.perfbuf_disable(g, pma_channel_id);
	if (err) {
		return err;
	}

	if (g->ops.perf.reset_pmasys_channel_registers != NULL) {
		g->ops.perf.reset_pmasys_channel_registers(g);
	}

	return 0;
}

int nvgpu_profiler_set_secure_config_hwpm(struct gk20a *g,  u32 gr_instance_id,
					  u32 pma_channel_id, bool enable)
{
	int err;

	if (g->ops.perf.set_secure_config_for_hwpm_dg != NULL) {
		err = g->ops.perf.set_secure_config_for_hwpm_dg(g,
				gr_instance_id, pma_channel_id, enable);
		if (err != 0) {
			if (enable) {
				nvgpu_err(g,
					"set_secure_config_for_hwpm_dg failed for channel: 0x%x",
										pma_channel_id);
				(void)g->ops.perf.set_secure_config_for_hwpm_dg(g,
						gr_instance_id, pma_channel_id, false);
			}
			return err;
		}
	}

	return 0;
}

int nvgpu_profiler_set_secure_config_cau(struct gk20a *g, u32 gr_instance_id,
					 u32 pma_channel_id, bool enable)
{
	int err;

	if (g->ops.perf.set_secure_config_for_cau != NULL) {
		err =  nvgpu_gr_exec_with_err_for_instance(g, gr_instance_id,
				g->ops.perf.set_secure_config_for_cau(g, gr_instance_id,
							pma_channel_id, enable));
		if (err != 0) {
			if (enable)
				nvgpu_gr_exec_for_instance(g, gr_instance_id,
						g->ops.perf.set_secure_config_for_cau(g,
					gr_instance_id, pma_channel_id, false));
			return err;
		}
	}

	return 0;
}

int nvgpu_profiler_set_secure_config_ccuprof(struct gk20a *g, u32 gr_instance_id,
					     u32 pma_channel_id, bool enable)
{
	int err;

	if (g->ops.perf.set_secure_config_for_ccu_prof != NULL) {
		err = g->ops.perf.set_secure_config_for_ccu_prof(g,
				gr_instance_id, pma_channel_id, enable);
		if (err != 0) {
			if (enable)
				(void)g->ops.perf.set_secure_config_for_ccu_prof(g,
						gr_instance_id,
						pma_channel_id, false);
			return err;
		}
	}

	return 0;
}

int nvgpu_profiler_set_secure_config_cwd_hem(struct gk20a *g, u32 gr_instance_id,
						u32 pma_channel_id, bool enable)
{
	int err;

	if (g->ops.perf.set_secure_config_for_cwd_hem != NULL) {
		err = g->ops.perf.set_secure_config_for_cwd_hem(g, gr_instance_id,
								pma_channel_id, enable);
		if (err != 0) {
			if (enable)
				(void)g->ops.perf.set_secure_config_for_cwd_hem(g, gr_instance_id,
									pma_channel_id, false);
			return err;
		}
	}

	return 0;
}

static void nvgpu_profiler_configure_hs_credits(struct gk20a *g, u32 gr_instance_id,
				struct nvgpu_pma_stream_hs_credits *hs_credits, bool reset)
{
	u32 i;

	nvgpu_assert(hs_credits->initialized == true);
	nvgpu_assert(g->ops.profiler.set_hs_credit_per_chiplet != NULL);

	for (i = 0U; i < hs_credits->gpc_credit_pool_sz; i++) {
		g->ops.profiler.set_hs_credit_per_chiplet(g, gr_instance_id,
				hs_credits->pma_channel_id, NVGPU_PROFILER_CHANNEL_CHIPLET_TYPE_GPC,
				i, reset ? 0U : hs_credits->gpc_credit_pool[i]);
	}

	for (i = 0U; i < hs_credits->fbp_credit_pool_sz; i++) {
		g->ops.profiler.set_hs_credit_per_chiplet(g, gr_instance_id,
				hs_credits->pma_channel_id, NVGPU_PROFILER_CHANNEL_CHIPLET_TYPE_FBP,
				i, reset ? 0U : hs_credits->fbp_credit_pool[i]);
	}

	g->ops.profiler.set_hs_credit_per_chiplet(g, gr_instance_id,
				hs_credits->pma_channel_id, NVGPU_PROFILER_CHANNEL_CHIPLET_TYPE_SYS,
				i, reset ? 0U : hs_credits->sys_credit_pool);
}

bool nvgpu_profiler_hs_credit_allocation_supported(struct gk20a *g)
{
	if (g->ops.profiler.get_hs_credit_per_gpc_chiplet == NULL ||
		g->ops.profiler.get_hs_credit_per_fbp_chiplet == NULL ||
		g->ops.profiler.get_hs_credit_per_sys_pipe_for_profiling == NULL ||
		g->ops.profiler.get_hs_credit_per_sys_pipe_for_hes == NULL ||
		g->ops.profiler.set_hs_credit_per_chiplet == NULL) {
		/* Does not support the HS Credit */
		return false;
	}
	return true;
}

static void nvgpu_profiler_free_hs_credits(struct gk20a *g, u32 gr_instance_id,
					struct nvgpu_profiler_object *prof)
{
	struct nvgpu_pma_stream_hs_credits *hs_credits;

	if (!nvgpu_profiler_hs_credit_allocation_supported(g)) {
		return;
	}

	hs_credits = &g->hs_credits[prof->pma_channel_id];
	if (hs_credits->initialized != true) {
		return;
	}

	nvgpu_profiler_configure_hs_credits(g, gr_instance_id, hs_credits, true);

	hs_credits->initialized = false;

	nvgpu_kfree(g, hs_credits->fbp_credit_pool);
	nvgpu_kfree(g, hs_credits->gpc_credit_pool);

	hs_credits->fbp_credit_pool_sz = 0;
	hs_credits->gpc_credit_pool_sz = 0;

	hs_credits->gpc_pool_max_total_credits = 0;
	hs_credits->fbp_pool_max_total_credits = 0;

	hs_credits->sys_credit_pool = 0;
}

static int nvgpu_profiler_init_hs_credits(struct gk20a *g, u32 gr_instance_id,
				struct nvgpu_profiler_object *prof)
{
	u32 gpu_instance_id = nvgpu_grmgr_get_gpu_instance_id(g, gr_instance_id);
	struct nvgpu_gpu_instance *gpu_instance = &g->mig.gpu_instance[gpu_instance_id];
	struct nvgpu_pma_stream_hs_credits *hs_credits;
	u32 num_gpcs, num_fbps, credits_per_chiplet, i;
	int err;

	if (!nvgpu_profiler_hs_credit_allocation_supported(g)) {
		return 0;
	}

	hs_credits = &g->hs_credits[prof->pma_channel_id];
	hs_credits->pma_channel_id = prof->pma_channel_id;

	num_gpcs = nvgpu_grmgr_get_gr_num_gpcs(g, gr_instance_id);
	hs_credits->gpc_credit_pool_sz = num_gpcs;

	num_fbps = nvgpu_grmgr_get_gr_num_fbps(g, gr_instance_id);
	hs_credits->fbp_credit_pool_sz = num_fbps;

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY]) {
		hs_credits->gpc_credit_pool =
			nvgpu_kzalloc(g, nvgpu_safe_mult_u32(num_gpcs, sizeof(u32)));
		if (hs_credits->gpc_credit_pool == NULL) {
			err = -ENOMEM;
			goto gpc_pool_alloc_failed;
		}

		hs_credits->fbp_credit_pool =
			nvgpu_kzalloc(g, nvgpu_safe_mult_u32(num_fbps, sizeof(u32)));
		if (hs_credits->fbp_credit_pool == NULL) {
			err = -ENOMEM;
			goto fbp_pool_alloc_failed;
		}

		credits_per_chiplet = g->ops.profiler.get_hs_credit_per_gpc_chiplet(g);
		for (i = 0; i < hs_credits->gpc_credit_pool_sz; i++) {
			hs_credits->gpc_credit_pool[i] = credits_per_chiplet;
			nvgpu_log(g, gpu_dbg_prof, "gr_instance_id: %u, gpc[%u] = %u",
					gr_instance_id, i, hs_credits->gpc_credit_pool[i]);
		}

		hs_credits->gpc_pool_max_total_credits =
				nvgpu_safe_mult_u32(num_gpcs, credits_per_chiplet);

		credits_per_chiplet = g->ops.profiler.get_hs_credit_per_fbp_chiplet(g);
		for (i = 0; i < hs_credits->fbp_credit_pool_sz; i++) {
			hs_credits->fbp_credit_pool[i] = credits_per_chiplet;
			nvgpu_log(g, gpu_dbg_prof, "gr_instance_id: %u, fbp[%u] = %u",
					gr_instance_id, i, hs_credits->fbp_credit_pool[i]);
		}

		hs_credits->fbp_pool_max_total_credits =
					nvgpu_safe_mult_u32(num_fbps, credits_per_chiplet);
	} else {
		hs_credits->fbp_credit_pool_sz = 0U;
		hs_credits->gpc_credit_pool_sz = 0U;
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES]) {
		hs_credits->sys_credit_pool =
			g->ops.profiler.get_hs_credit_per_sys_pipe_for_hes(g);
	} else {
		/* for HWPM */
		hs_credits->sys_credit_pool =
			g->ops.profiler.get_hs_credit_per_sys_pipe_for_profiling(g);
	}


	if (!nvgpu_grmgr_is_mig_type_gpu_instance(gpu_instance)) {
		hs_credits->sys_credit_pool *= 2U;
	}

	nvgpu_log(g, gpu_dbg_prof, "gr_instance_id: %u, sys credits: %u", gr_instance_id,
						hs_credits->sys_credit_pool);
	hs_credits->sys_max_credits = hs_credits->sys_credit_pool;
	hs_credits->initialized = true;

	nvgpu_profiler_configure_hs_credits(g, gr_instance_id, hs_credits, false);

	return 0;

fbp_pool_alloc_failed:
	nvgpu_kfree(g, hs_credits->gpc_credit_pool);
gpc_pool_alloc_failed:
	hs_credits->fbp_credit_pool_sz = 0;
	hs_credits->gpc_credit_pool_sz = 0;

	return err;
}

int nvgpu_profiler_bind_pm_resources(struct nvgpu_profiler_object *prof)
{
	struct gk20a *g = prof->g;
	bool is_ctxsw, pma_stream = false;
	u32 reservation_id = prof->prof_handle;
	int err;
	u32 gr_instance_id;

	nvgpu_log(g, gpu_dbg_prof,
		"Request to bind PM resources with profiler handle %u",
		prof->prof_handle);

	nvgpu_mutex_acquire(&prof->bind_lock);
	if (prof->bound) {
		nvgpu_err(g, "PM resources are already bound with profiler handle %u",
			prof->prof_handle);
		nvgpu_mutex_release(&prof->bind_lock);
		return -EINVAL;
	}

	if (!prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY] &&
	    !prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC] &&
	    !prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES] &&
	    !prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CCU_PROF]) {
		nvgpu_err(g, "No PM resources reserved for profiler handle %u",
			prof->prof_handle);
		nvgpu_mutex_release(&prof->bind_lock);
		return -EINVAL;
	}

	err = gk20a_busy(g);
	if (err) {
		nvgpu_err(g, "failed to poweron");
		nvgpu_mutex_release(&prof->bind_lock);
		return err;
	}

	gr_instance_id = nvgpu_grmgr_get_gr_instance_id(g, prof->gpu_instance_id);

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM])
		pma_stream = true;

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY]) {
		is_ctxsw = nvgpu_profiler_is_context_resource(prof,
				  NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY);
		err = g->ops.profiler.bind_hwpm(prof->g, gr_instance_id,
				is_ctxsw, prof->tsg, pma_stream, reservation_id);
		if (err != 0) {
			nvgpu_err(g,
				"failed to bind HWPM with profiler handle %u",
				prof->prof_handle);
			goto fail;
		}

		nvgpu_log(g, gpu_dbg_prof,
			"HWPM bound with profiler handle %u",
			prof->prof_handle);
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC]) {
		is_ctxsw = nvgpu_profiler_is_context_resource(prof,
				NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC);
		err = g->ops.profiler.bind_smpc(g, gr_instance_id,
				is_ctxsw, prof->tsg, reservation_id);
		if (err) {
			nvgpu_err(g, "failed to bind SMPC with profiler handle %u",
				prof->prof_handle);
			goto fail;
		}

		nvgpu_log(g, gpu_dbg_prof,
			"SMPC bound with profiler handle %u", prof->prof_handle);
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CCU_PROF] &&
	    prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM]) {
		is_ctxsw = nvgpu_profiler_is_context_resource(prof,
				NVGPU_PROFILER_PM_RESOURCE_TYPE_CCU_PROF);
		if (g->ops.profiler.bind_ccuprof != NULL) {
			g->ops.profiler.bind_ccuprof(g, gr_instance_id, reservation_id,
				prof->pma_channel_id);
		}

		nvgpu_log(g, gpu_dbg_prof,
			"CCU prof bound with profiler handle %u",
			prof->prof_handle);
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES] &&
			prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM]) {
		is_ctxsw = nvgpu_profiler_is_context_resource(prof,
				NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES);

		if (g->ops.perf.reset_pm_trigger_masks != NULL) {
			g->ops.perf.reset_pm_trigger_masks(g,
				prof->pma_channel_id, gr_instance_id,
				reservation_id);
		}
		g->ops.profiler.bind_hes_cwd(g, gr_instance_id, reservation_id,
				prof->pma_channel_id);

		if (g->ops.perf.enable_hes_event_trigger != NULL)
			g->ops.perf.enable_hes_event_trigger(g, gr_instance_id,
					prof->pma_channel_id);

		nvgpu_log(g, gpu_dbg_prof,
			"HES CWD bound with profiler handle %u",
			prof->prof_handle);
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM]) {
		if ((prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY]) ||
			(prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC]) ||
			(prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CCU_PROF])) {
			if (g->ops.perf.reset_pm_trigger_masks != NULL) {
				g->ops.perf.reset_pm_trigger_masks(g,
					prof->pma_channel_id, gr_instance_id,
					reservation_id);
			}
			if (g->ops.perf.enable_pm_trigger != NULL) {
				g->ops.perf.enable_pm_trigger(g,
					gr_instance_id, prof->pma_channel_id,
					reservation_id);
			}
		}

		err = g->ops.profiler.bind_pma_streamout(g,
				gr_instance_id,
				prof->pma_channel_id,
				prof->pma_buffer_va,
				prof->pma_buffer_size,
				prof->pma_bytes_available_buffer_va,
				reservation_id);
		if (err != 0) {
			nvgpu_err(g,
				"failed to bind pma streamout with profiler handle %u",
				prof->prof_handle);
			goto fail;
		}

		if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY]) {
			err = nvgpu_profiler_set_secure_config_hwpm(g, gr_instance_id,
								    prof->pma_channel_id, true);
			if (err != 0) {
				nvgpu_err(g, "set_secure_config_hwpm failed. %d", err);
				goto fail;
			}
		}

		if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC]) {
			err = nvgpu_profiler_set_secure_config_cau(g, gr_instance_id,
							prof->pma_channel_id, true);
			if (err != 0) {
				nvgpu_err(g, "set_secure_config_cau failed. %d", err);
				goto fail;
			}
		}

		if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES]) {
			err = nvgpu_profiler_set_secure_config_cwd_hem(g, gr_instance_id,
								prof->pma_channel_id, true);
			if (err != 0) {
				nvgpu_err(g, "set_secure_config_cwd_hem failed. %d", err);
				goto fail;
			}
		}

		if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CCU_PROF]) {
			err = nvgpu_profiler_set_secure_config_ccuprof(g,
					gr_instance_id,
					prof->pma_channel_id,
					true);
			if (err != 0) {
				nvgpu_err(g,
					"set_secure_config_ccuprof failed. %d",
					err);
				goto fail;
			}
		}

		if (g->ops.perf.enable_pma_trigger != NULL)
			g->ops.perf.enable_pma_trigger(g, prof->pma_channel_id);

		err = nvgpu_profiler_init_hs_credits(g, gr_instance_id,  prof);
		if (err != 0) {
			goto fail_unbind;
		}

		nvgpu_log(g, gpu_dbg_prof,
			"PMA streamout bound with profiler handle %u",
			prof->prof_handle);
	}

	err = nvgpu_profiler_build_regops_allowlist(prof);
	if (err != 0) {
		nvgpu_err(g, "failed to build allowlist");
		goto fail_unbind;
	}

	prof->bound = true;
	nvgpu_mutex_release(&prof->bind_lock);
	gk20a_idle(g);
	return 0;

fail_unbind:
	nvgpu_mutex_release(&prof->bind_lock);
	nvgpu_profiler_unbind_pm_resources(prof);
fail:
	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM]) {
		if (g->ops.perf.reset_pm_trigger_masks != NULL) {
			g->ops.perf.reset_pm_trigger_masks(g,
					prof->pma_channel_id, gr_instance_id,
					reservation_id);
		}
	}
	gk20a_idle(g);
	return err;
}

int nvgpu_profiler_unbind_pm_resources(struct nvgpu_profiler_object *prof)
{
	struct gk20a *g = prof->g;
	bool is_ctxsw = false;
	u32 reservation_id = prof->prof_handle;
	int err;
	u32 gr_instance_id;

	nvgpu_mutex_acquire(&prof->bind_lock);

	if (!prof->bound) {
		nvgpu_err(g, "No PM resources bound to profiler handle %u",
			prof->prof_handle);
		nvgpu_mutex_release(&prof->bind_lock);
		return -EINVAL;
	}

	err = gk20a_busy(g);
	if (err) {
		nvgpu_err(g, "failed to poweron");
		nvgpu_mutex_release(&prof->bind_lock);
		return err;
	}

	gr_instance_id = nvgpu_grmgr_get_gr_instance_id(g, prof->gpu_instance_id);

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM]) {

		nvgpu_profiler_free_hs_credits(g, gr_instance_id, prof);

		if (g->ops.perf.reset_pm_trigger_masks != NULL) {
			g->ops.perf.reset_pm_trigger_masks(g,
					prof->pma_channel_id, gr_instance_id,
					reservation_id);
		}
		/* Assuming pma stream will be enabled only with one resource */
		if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY]) {
			is_ctxsw = nvgpu_profiler_is_context_resource(prof,
				NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY);
		} else if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC]) {
			is_ctxsw = nvgpu_profiler_is_context_resource(prof,
				NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC);
		} else if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CCU_PROF]) {
			is_ctxsw = nvgpu_profiler_is_context_resource(prof,
				NVGPU_PROFILER_PM_RESOURCE_TYPE_CCU_PROF);
		} else if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES] &&
			prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM]) {
			is_ctxsw = nvgpu_profiler_is_context_resource(prof,
				NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES);
		}

		if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY]) {
			err = nvgpu_profiler_set_secure_config_hwpm(g, gr_instance_id,
							prof->pma_channel_id, false);
			if (err != 0) {
				nvgpu_err(g, "set_secure_config_hwpm failed. %d", err);
				goto fail;
			}
		}

		if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC]) {
			err = nvgpu_profiler_set_secure_config_cau(g, gr_instance_id,
							prof->pma_channel_id, false);
			if (err != 0) {
				nvgpu_err(g, "set_secure_config_cau failed. %d", err);
				goto fail;
			}
		}

		if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CCU_PROF]) {
			err = nvgpu_profiler_set_secure_config_ccuprof(g, gr_instance_id,
								prof->pma_channel_id, false);
			if (err != 0) {
				nvgpu_err(g, "set_secure_config_ccu_prof failed. %d", err);
				goto fail;
			}
		}

		if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES]) {
			err = nvgpu_profiler_set_secure_config_cwd_hem(g, gr_instance_id,
								prof->pma_channel_id, false);
			if (err != 0) {
				nvgpu_err(g, "set_secure_config_cwd_hem failed. %d", err);
				goto fail;
			}
		}

		err = g->ops.profiler.unbind_pma_streamout(g,
			gr_instance_id, prof->pma_channel_id,
			is_ctxsw,
			prof->tsg,
			prof->pma_bytes_available_buffer_cpuva,
			reservation_id);
		if (err) {
			nvgpu_err(g,
				"failed to unbind PMA streamout from profiler handle %u",
				prof->prof_handle);
			goto fail;
		}

		nvgpu_log(g, gpu_dbg_prof,
			"PMA streamout unbound from profiler handle %u",
			prof->prof_handle);
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY]) {
		is_ctxsw = nvgpu_profiler_is_context_resource(prof,
				  NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY);
		err = g->ops.profiler.unbind_hwpm(g, gr_instance_id,
				is_ctxsw, prof->tsg);
		if (err) {
			nvgpu_err(g,
				"failed to unbind HWPM from profiler handle %u",
				prof->prof_handle);
			goto fail;
		}

		nvgpu_log(g, gpu_dbg_prof,
			"HWPM unbound from profiler handle %u",
			prof->prof_handle);
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC]) {
		is_ctxsw = nvgpu_profiler_is_context_resource(prof,
				NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC);
		err = g->ops.profiler.unbind_smpc(g, gr_instance_id,
				is_ctxsw, prof->tsg, reservation_id);
		if (err) {
			nvgpu_err(g,
				"failed to unbind SMPC from profiler handle %u",
				prof->prof_handle);
			goto fail;
		}

		nvgpu_log(g, gpu_dbg_prof,
			"SMPC unbound from profiler handle %u",	prof->prof_handle);
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CCU_PROF]) {
		is_ctxsw = nvgpu_profiler_is_context_resource(prof,
				NVGPU_PROFILER_PM_RESOURCE_TYPE_CCU_PROF);
		if (g->ops.profiler.unbind_ccuprof != NULL) {
			err = g->ops.profiler.unbind_ccuprof(g, gr_instance_id, is_ctxsw);
			if (err) {
				nvgpu_err(g, "failed to unbind CCU prof with profiler handle %u",
					prof->prof_handle);
				goto fail;
			}
		}

		nvgpu_log(g, gpu_dbg_prof,
			"CCU prof unbound from profiler handle %u",
			prof->prof_handle);
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES] &&
			prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM]) {
		is_ctxsw = nvgpu_profiler_is_context_resource(prof,
				NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES);
		/* Revisit argument list based on usecase.
		 * TODO
		 * cblock information logistics
		 * MIG specific stuff
		 * ctxsw support
		 */
		if (g->ops.profiler.unbind_hes_cwd != NULL) {
			err = g->ops.profiler.unbind_hes_cwd(g, gr_instance_id, is_ctxsw);
			if (err) {
				nvgpu_err(g,
					"failed to unbind HES CWD from profiler handle %u",
					prof->prof_handle);
				goto fail;
			}
		}

		nvgpu_log(g, gpu_dbg_prof,
			"HES CWD unbound from profiler handle %u",
			prof->prof_handle);
	}

fail:
	nvgpu_profiler_destroy_regops_allowlist(prof);
	prof->bound = false;
	nvgpu_mutex_release(&prof->bind_lock);
	gk20a_idle(g);
	return err;
}

int nvgpu_profiler_alloc_pma_stream(struct nvgpu_profiler_object *prof,
					u32 *pma_channel_id)
{
	struct gk20a *g = prof->g;
	int err;

	err = nvgpu_profiler_pm_resource_reserve(prof,
			NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM,
			pma_channel_id);
	if (err) {
		nvgpu_err(g, "failed to reserve PMA stream");
		return err;
	}

	err = nvgpu_perfbuf_init_vm(g, *pma_channel_id);
	if (err) {
		nvgpu_err(g, "failed to initialize perfbuf VM");
		nvgpu_profiler_pm_resource_release(prof,
			NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM,
			*pma_channel_id);
		return err;
	}

	return 0;
}

void nvgpu_profiler_free_pma_stream(struct nvgpu_profiler_object *prof,
					u32 pma_channel_id)
{
	struct gk20a *g = prof->g;

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM]) {
		nvgpu_perfbuf_deinit_vm(g, pma_channel_id);
		nvgpu_profiler_pm_resource_release(prof,
			NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM,
			pma_channel_id);
	}
}

static int map_cmp(const void *a, const void *b)
{
	const struct nvgpu_pm_resource_register_range_map *e1;
	const struct nvgpu_pm_resource_register_range_map *e2;

	e1 = (const struct nvgpu_pm_resource_register_range_map *)a;
	e2 = (const struct nvgpu_pm_resource_register_range_map *)b;

	if (e1->start < e2->start) {
		return -1;
	}

	if (e1->start > e2->start) {
		return 1;
	}

	return 0;
}

static u32 get_pm_resource_register_range_map_entry_count(struct nvgpu_profiler_object *prof)
{
	struct gk20a *g = prof->g;
	u32 count = 0U;
	u32 range_count;

	/* Account for TYPE_TEST entries added in add_test_range_to_map() */
	count += 2U;

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC]) {
		g->ops.regops.get_smpc_register_ranges(&range_count);
		count += range_count;
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY]) {
		g->ops.regops.get_hwpm_perfmon_register_ranges(&range_count);
		count += range_count;

		g->ops.regops.get_hwpm_router_register_ranges(&range_count);
		count += range_count;

		g->ops.regops.get_hwpm_pma_trigger_register_ranges(&range_count);
		count += range_count;

		g->ops.regops.get_hwpm_perfmux_register_ranges(&range_count);
		count += range_count;

		if (g->ops.regops.get_cau_register_ranges != NULL) {
			g->ops.regops.get_cau_register_ranges(&range_count);
			count += range_count;
		}

		if (g->ops.regops.get_hwpm_mm_register_ranges != NULL) {
			g->ops.regops.get_hwpm_mm_register_ranges(&range_count);
			count += range_count;
		}
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM]) {
		g->ops.regops.get_hwpm_pma_channel_register_ranges(&range_count);
		count += range_count;
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_PC_SAMPLER]) {
		g->ops.regops.get_hwpm_pc_sampler_register_ranges(&range_count);
		count += range_count;
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES]) {
		if (g->ops.regops.get_hes_register_ranges != NULL) {
			g->ops.regops.get_hes_register_ranges(&range_count);
			count += range_count;
		}
	}

	return count;
}

static void add_range_to_map(const struct nvgpu_pm_resource_register_range *range,
		u32 range_count, struct nvgpu_pm_resource_register_range_map *map,
		u32 *map_index, enum nvgpu_pm_resource_hwpm_register_type type)
{
	u32 index = *map_index;
	u32 i;

	for (i = 0U; i < range_count; i++) {
		map[index].start = range[i].start;
		map[index].end = range[i].end;
		map[index].type = type;
		index++;
	}

	*map_index = index;
}

static void add_test_range_to_map(struct gk20a *g,
		struct nvgpu_pm_resource_register_range_map *map,
		u32 *map_index, enum nvgpu_pm_resource_hwpm_register_type type)
{
	u32 index = *map_index;
	u32 timer0_offset, timer1_offset;

	g->ops.ptimer.get_timer_reg_offsets(&timer0_offset, &timer1_offset);

	map[index].start = timer0_offset;
	map[index].end = timer0_offset;
	map[index].type = type;
	index++;

	map[index].start = timer1_offset;
	map[index].end = timer1_offset;
	map[index].type = type;
	index++;

	*map_index = index;
}

static int nvgpu_profiler_build_regops_allowlist(struct nvgpu_profiler_object *prof)
{
	struct nvgpu_pm_resource_register_range_map *map;
	const struct nvgpu_pm_resource_register_range *range;
	u32 map_count, map_index = 0U;
	u32 range_count;
	struct gk20a *g = prof->g;
	u32 i;

	map_count = get_pm_resource_register_range_map_entry_count(prof);
	if (map_count == 0U) {
		return -EINVAL;
	}

	nvgpu_log(g, gpu_dbg_prof, "Allowlist map number of entries %u for handle %u",
		map_count, prof->prof_handle);

	map = nvgpu_kzalloc(g, sizeof(*map) * map_count);
	if (map == NULL) {
		return -ENOMEM;
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_SMPC]) {
		range = g->ops.regops.get_smpc_register_ranges(&range_count);
		add_range_to_map(range, range_count, map, &map_index,
			NVGPU_HWPM_REGISTER_TYPE_SMPC);
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_HWPM_LEGACY]) {
		range = g->ops.regops.get_hwpm_perfmon_register_ranges(&range_count);
		add_range_to_map(range, range_count, map, &map_index,
			NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMON);

		range = g->ops.regops.get_hwpm_router_register_ranges(&range_count);
		add_range_to_map(range, range_count, map, &map_index,
			NVGPU_HWPM_REGISTER_TYPE_HWPM_ROUTER);

		range = g->ops.regops.get_hwpm_pma_trigger_register_ranges(&range_count);
		add_range_to_map(range, range_count, map, &map_index,
			NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_TRIGGER);

		range = g->ops.regops.get_hwpm_perfmux_register_ranges(&range_count);
		add_range_to_map(range, range_count, map, &map_index,
			NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX);

		if (g->ops.regops.get_cau_register_ranges != NULL) {
			range = g->ops.regops.get_cau_register_ranges(&range_count);
			add_range_to_map(range, range_count, map, &map_index,
				NVGPU_HWPM_REGISTER_TYPE_CAU);
		}

		if (g->ops.regops.get_hwpm_mm_register_ranges != NULL) {
			range = g->ops.regops.get_hwpm_mm_register_ranges(&range_count);
			add_range_to_map(range, range_count, map, &map_index,
				NVGPU_HWPM_REGISTER_TYPE_HWPM_MM);
		}
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_PMA_STREAM]) {
		range = g->ops.regops.get_hwpm_pma_channel_register_ranges(&range_count);
		add_range_to_map(range, range_count, map, &map_index,
			NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_CHANNEL);
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_PC_SAMPLER]) {
		range = g->ops.regops.get_hwpm_pc_sampler_register_ranges(&range_count);
		add_range_to_map(range, range_count, map, &map_index,
				NVGPU_HWPM_REGISTER_TYPE_PC_SAMPLER);
	}

	if (prof->reserved[NVGPU_PROFILER_PM_RESOURCE_TYPE_CWD_HES]) {
		if (g->ops.regops.get_hes_register_ranges != NULL) {
			range = g->ops.regops.get_hes_register_ranges(&range_count);
			add_range_to_map(range, range_count, map, &map_index,
			NVGPU_HWPM_REGISTER_TYPE_HES);
		}
	}

	add_test_range_to_map(g, map, &map_index, NVGPU_HWPM_REGISTER_TYPE_TEST);

	nvgpu_log(g, gpu_dbg_prof, "Allowlist map created successfully for handle %u",
		prof->prof_handle);

	nvgpu_assert(map_count == map_index);

	sort(map, map_count, sizeof(*map), map_cmp, NULL);

	for (i = 0; i < map_count; i++) {
		nvgpu_log(g, gpu_dbg_prof, "allowlist[%u]: 0x%x-0x%x : type %u",
			i, map[i].start, map[i].end, map[i].type);
	}

	prof->map = map;
	prof->map_count = map_count;
	return 0;
}

static void nvgpu_profiler_destroy_regops_allowlist(struct nvgpu_profiler_object *prof)
{
	nvgpu_log(prof->g, gpu_dbg_prof, "Allowlist map destroy for handle %u",
		prof->prof_handle);

	nvgpu_kfree(prof->g, prof->map);
}

bool nvgpu_profiler_allowlist_range_search(struct gk20a *g,
		struct nvgpu_pm_resource_register_range_map *map,
		u32 map_count, u32 offset,
		struct nvgpu_pm_resource_register_range_map *entry)
{
	s32 start = 0;
	s32 mid = 0;
	s32 end = nvgpu_safe_sub_s32((s32)map_count, 1);
	bool found = false;

	if (map == NULL) {
		return false;
	}

	while (start <= end) {
		mid = start + (end - start) / 2;

		if (offset < map[mid].start) {
			end = mid - 1;
		} else if (offset > map[mid].end) {
			start = mid + 1;
		} else {
			found = true;
			break;
		}
	}

	if (found) {
		*entry = map[mid];
		nvgpu_log(g, gpu_dbg_prof, "Offset 0x%x found in range 0x%x-0x%x, type: %u",
			offset, map[mid].start, map[mid].end, map[mid].type);
	} else {
		nvgpu_log(g, gpu_dbg_prof, "Offset 0x%x not found in range search", offset);
	}

	return found;
}

static bool allowlist_offset_search(struct gk20a *g,
		const u32 *offset_allowlist, u32 count, u32 offset)
{
	s32 start = 0;
	s32 mid = 0;
	s32 end = nvgpu_safe_sub_s32((s32)count, 1);
	bool found = false;

	while (start <= end) {
		mid = start + (end - start) / 2;
		if (offset_allowlist[mid] == offset) {
			found = true;
			break;
		}

		if (offset < offset_allowlist[mid]) {
			end = mid - 1;
		} else {
			start = mid + 1;
		}
	}

	if (found) {
		nvgpu_log(g, gpu_dbg_prof, "Offset 0x%x found in offset allowlist",
			offset);
	} else {
		nvgpu_log(g, gpu_dbg_prof, "Offset 0x%x not found in offset allowlist",
			offset);
	}

	return found;
}

bool nvgpu_profiler_validate_regops_allowlist(struct nvgpu_profiler_object *prof,
		u32 offset, enum nvgpu_pm_resource_hwpm_register_type type)
{
	struct gk20a *g = prof->g;
	const u32 *offset_allowlist;
	u32 count;
	u32 stride;

	if ((type == NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMUX) ||
	    (type == NVGPU_HWPM_REGISTER_TYPE_TEST)) {
		return true;
	}

	switch ((u32)type) {
	case NVGPU_HWPM_REGISTER_TYPE_HWPM_PERFMON:
		offset_allowlist = g->ops.regops.get_hwpm_perfmon_register_offset_allowlist(&count);
		stride = g->ops.regops.get_hwpm_perfmon_register_stride();
		break;

	case NVGPU_HWPM_REGISTER_TYPE_HWPM_ROUTER:
		offset_allowlist = g->ops.regops.get_hwpm_router_register_offset_allowlist(&count);
		stride = g->ops.regops.get_hwpm_router_register_stride();
		break;

	case NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_TRIGGER:
		offset_allowlist = g->ops.regops.get_hwpm_pma_trigger_register_offset_allowlist(&count);
		stride = g->ops.regops.get_hwpm_pma_trigger_register_stride();
		break;

	case NVGPU_HWPM_REGISTER_TYPE_SMPC:
		offset_allowlist = g->ops.regops.get_smpc_register_offset_allowlist(&count);
		stride = g->ops.regops.get_smpc_register_stride();
		break;

	case NVGPU_HWPM_REGISTER_TYPE_CAU:
		offset_allowlist = g->ops.regops.get_cau_register_offset_allowlist(&count);
		stride = g->ops.regops.get_cau_register_stride();
		break;

	case NVGPU_HWPM_REGISTER_TYPE_HWPM_PMA_CHANNEL:
		offset_allowlist = g->ops.regops.get_hwpm_pma_channel_register_offset_allowlist(&count);
		stride = g->ops.regops.get_hwpm_pma_channel_register_stride();
		break;

	default:
		return false;
	}

	offset = offset & (stride - 1U);
	return allowlist_offset_search(g, offset_allowlist, count, offset);
}

#ifdef CONFIG_NVGPU_NON_FUSA
void nvgpu_profiler_hs_stream_quiesce(struct gk20a *g)
{
	if (g->ops.perf.reset_hs_streaming_credits != NULL) {
		/* Reset high speed streaming credits to 0. */
		g->ops.perf.reset_hs_streaming_credits(g);
	}

	if (g->ops.perf.enable_hs_streaming != NULL) {
		/* Disable high speed streaming */
		g->ops.perf.enable_hs_streaming(g, false);
	}
}
#endif /* CONFIG_NVGPU_NON_FUSA */

void nvgpu_profiler_bind_hes_cwd(struct gk20a *g, u32 gr_instance_id,
		u32 reservation_id, u32 pma_channel_id)
{
	(void) reservation_id;
	(void) pma_channel_id;
	if (g->ops.perf.reset_cwd_hem_registers != NULL) {
		g->ops.perf.reset_cwd_hem_registers(g, gr_instance_id);
	}
}
