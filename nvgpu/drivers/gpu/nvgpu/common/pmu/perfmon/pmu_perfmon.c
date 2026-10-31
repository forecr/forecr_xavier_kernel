// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/pmu/pmu_perfmon.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/falcon.h>
#include <nvgpu/bug.h>
#include <nvgpu/enabled.h>
#include <nvgpu/pmu.h>
#include <nvgpu/pmu/cmd.h>
#include <nvgpu/pmu/msg.h>
#include <nvgpu/log.h>
#include <nvgpu/bug.h>
#include <nvgpu/pmu/pmuif/nvgpu_cmdif.h>
#include <nvgpu/kmem.h>
#include <nvgpu/string.h>
#include <nvgpu/soc.h>

#include "pmu_perfmon_sw_gm20b.h"
#include "pmu_perfmon_sw_gv11b.h"
#if defined(CONFIG_NVGPU_NON_FUSA)
#include "pmu_perfmon_sw_ga10b.h"
#endif

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
#include <nvgpu_next_perfmon.h>
#endif

static u8 get_perfmon_id(struct nvgpu_pmu *pmu)
{
	struct gk20a *g = pmu->g;
	u32 ver = g->params.gpu_arch + g->params.gpu_impl;
	u8 unit_id;

	switch (ver) {
	case GK20A_GPUID_GK20A:
	case GK20A_GPUID_GM20B:
	case GK20A_GPUID_GM20B_B:
		unit_id = PMU_UNIT_PERFMON;
		break;
	case NVGPU_GPUID_GP10B:
	case NVGPU_GPUID_GV11B:
#if defined(CONFIG_NVGPU_NON_FUSA)
	case NVGPU_GPUID_GA10B:
#endif
		unit_id = PMU_UNIT_PERFMON_T18X;
		break;
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
	case NVGPU_GPUID_GB10B:
	case NVGPU_GPUID_GB10B_NEXT:
		unit_id = PMU_UNIT_PERFMON_T18X;
		break;
#endif
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB20C)
	case NVGPU_GPUID_GB20C:
		unit_id = PMU_UNIT_PERFMON_T18X;
		break;
#endif
	default:
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
		unit_id = nvgpu_next_get_perfmon_id(pmu);
#else
		unit_id = PMU_UNIT_INVALID;
#endif
		break;
	}

	if (unit_id == PMU_UNIT_INVALID) {
		nvgpu_err(g, "no support for %x", ver);
		WARN_ON(true);
	}

	return unit_id;
}

void nvgpu_pmu_perfmon_rpc_handler(struct gk20a *g, struct nvgpu_pmu *pmu,
				   struct nv_pmu_rpc_header *rpc,
				   struct rpc_handler_payload *rpc_payload)
{
	struct nv_pmu_rpc_struct_perfmon_query *rpc_param;

	switch (rpc->function) {
	case NV_PMU_RPC_ID_PERFMON_T18X_INIT:
		nvgpu_pmu_dbg(g,
			"reply NV_PMU_RPC_ID_PERFMON_INIT");
		pmu->pmu_perfmon->perfmon_ready = true;
		break;
	case NV_PMU_RPC_ID_PERFMON_T18X_START:
		nvgpu_pmu_dbg(g,
			"reply NV_PMU_RPC_ID_PERFMON_START");
		break;
	case NV_PMU_RPC_ID_PERFMON_T18X_STOP:
		nvgpu_pmu_dbg(g,
			"reply NV_PMU_RPC_ID_PERFMON_STOP");
		break;
	case NV_PMU_RPC_ID_PERFMON_T18X_QUERY:
		nvgpu_pmu_dbg(g,
			"reply NV_PMU_RPC_ID_PERFMON_QUERY");
		rpc_param = (struct nv_pmu_rpc_struct_perfmon_query *)
			rpc_payload->rpc_buff;
		pmu->pmu_perfmon->load[NVGPU_PMU_PERFMON_CLASS_GR] = rpc_param->sample_buffer[0];
		pmu->pmu_perfmon->load[NVGPU_PMU_PERFMON_CLASS_NVD] = rpc_param->sample_buffer[1];
		pmu->pmu_perfmon->load[NVGPU_PMU_PERFMON_CLASS_NVENC0] = rpc_param->sample_buffer[2];
		pmu->pmu_perfmon->load[NVGPU_PMU_PERFMON_CLASS_NVENC1] = rpc_param->sample_buffer[3];
		pmu->pmu_perfmon->load[NVGPU_PMU_PERFMON_CLASS_NVDEC0] = rpc_param->sample_buffer[4];
		pmu->pmu_perfmon->load[NVGPU_PMU_PERFMON_CLASS_NVDEC1] = rpc_param->sample_buffer[5];
		pmu->pmu_perfmon->load[NVGPU_PMU_PERFMON_CLASS_OFA] = rpc_param->sample_buffer[6];
		pmu->pmu_perfmon->load[NVGPU_PMU_PERFMON_CLASS_NVJPG0] = rpc_param->sample_buffer[7];
		pmu->pmu_perfmon->perfmon_query = 1;
		/* set perfmon_query to 1 after load is copied */
		break;
	default:
		nvgpu_pmu_dbg(g, "invalid reply");
		break;
	}
}

int nvgpu_pmu_initialize_perfmon(struct gk20a *g, struct nvgpu_pmu *pmu,
		struct nvgpu_pmu_perfmon **perfmon_ptr)
{
	struct nvgpu_pmu_perfmon *perfmon;
	int err = 0;
	u32 ver = g->params.gpu_arch + g->params.gpu_impl;

	(void)pmu;

	if (*perfmon_ptr != NULL) {
		/* Not to allocate a new buffer after railgating
		is done. Use the same memory for pmu_perfmon
		   after railgating.
		*/
		nvgpu_pmu_dbg(g, "skip perfmon init for unrailgate sequence");
		goto exit;

	}
	/* One-time memory allocation for pmu_perfmon */
	perfmon = (struct nvgpu_pmu_perfmon *)
		nvgpu_kzalloc(g, sizeof(struct nvgpu_pmu_perfmon));

	if (perfmon == NULL) {
		nvgpu_err(g, "failed to initialize perfmon");
		return -ENOMEM;
	}

	*perfmon_ptr = perfmon;

	switch (ver) {
	case GK20A_GPUID_GM20B:
	case GK20A_GPUID_GM20B_B:
	case NVGPU_GPUID_GP10B:
	case NVGPU_GPUID_GV100:
	case NVGPU_GPUID_TU104:
		nvgpu_gm20b_perfmon_sw_init(g, *perfmon_ptr);
		break;

	case NVGPU_GPUID_GV11B:
		nvgpu_gv11b_perfmon_sw_init(g, *perfmon_ptr);
		break;
#if defined(CONFIG_NVGPU_NON_FUSA)
	case NVGPU_GPUID_GA10B:
		nvgpu_ga10b_perfmon_sw_init(g, *perfmon_ptr);
		break;
#if defined(CONFIG_NVGPU_GB10B)
	case NVGPU_GPUID_GB10B:
	case NVGPU_GPUID_GB10B_NEXT:
		nvgpu_ga10b_perfmon_sw_init(g, *perfmon_ptr);
		break;
#endif
#endif
#ifdef CONFIG_NVGPU_GB20C
	case NVGPU_GPUID_GB20C:
		nvgpu_err(g, "GB20C Perform not implemented");
		break;
#endif
	default:
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
		if (nvgpu_next_pmu_initialize_perfmon(g, pmu, perfmon_ptr))
#endif
		{
			nvgpu_kfree(g, *perfmon_ptr);
			err = -ENODEV;
			nvgpu_err(g, "no support for GPUID %x", ver);
		}
		break;
	}

exit:
	return err;

}

void nvgpu_pmu_deinitialize_perfmon(struct gk20a *g, struct nvgpu_pmu *pmu)
{
	if (pmu->pmu_perfmon == NULL) {
		return;
	} else {
		if (pmu->pmu_perfmon->query_mutex_initialized) {
			pmu->pmu_perfmon->query_mutex_initialized = false;
			nvgpu_mutex_destroy(&pmu->pmu_perfmon->query_mutex);
		}
		nvgpu_kfree(g, pmu->pmu_perfmon);
	}
}

int nvgpu_pmu_init_perfmon(struct nvgpu_pmu *pmu)
{
	struct gk20a *g = pmu->g;
	struct pmu_fw_ver_ops *fw_ops = &pmu->fw->ops;

	struct pmu_cmd cmd;
	struct pmu_payload payload;
	int status;
	u64 tmp_addr, tmp_size;

	if (!nvgpu_is_enabled(g, NVGPU_PMU_PERFMON)) {
		return 0;
	}

	nvgpu_log_fn(g, " ");

	pmu->pmu_perfmon->perfmon_ready = false;

	g->ops.pmu.pmu_init_perfmon_counter(g);

	if (pmu->pmu_perfmon->sample_buffer == 0U) {
		tmp_addr = nvgpu_alloc(&pmu->dmem, 2U * sizeof(u16));
		nvgpu_assert(tmp_addr <= U32_MAX);
		pmu->pmu_perfmon->sample_buffer = (u32)tmp_addr;
	}
	if (pmu->pmu_perfmon->sample_buffer == 0U) {
		nvgpu_err(g, "failed to allocate perfmon sample buffer");
		return -ENOMEM;
	}

	/* init PERFMON */
	(void) memset(&cmd, 0, sizeof(struct pmu_cmd));

	cmd.hdr.unit_id = get_perfmon_id(pmu);
	if (cmd.hdr.unit_id == PMU_UNIT_INVALID) {
		nvgpu_err(g, "failed to get perfmon UNIT ID, command skipped");
		return -EINVAL;
	}

	tmp_size = PMU_CMD_HDR_SIZE +
		(u64)fw_ops->get_perfmon_cmd_init_size();
	nvgpu_assert(tmp_size <= U8_MAX);
	cmd.hdr.size = (u8)tmp_size;
	cmd.cmd.perfmon.cmd_type = PMU_PERFMON_CMD_ID_INIT;
	/* buffer to save counter values for pmu perfmon */
	fw_ops->perfmon_cmd_init_set_sample_buffer(&cmd.cmd.perfmon,
			(u16)pmu->pmu_perfmon->sample_buffer);
	/* number of sample periods below lower threshold
	 * before pmu triggers perfmon decrease event
	 */
	fw_ops->perfmon_cmd_init_set_dec_cnt(&cmd.cmd.perfmon, 15);
	/* index of base counter, aka. always ticking counter */
	fw_ops->perfmon_cmd_init_set_base_cnt_id(&cmd.cmd.perfmon, 6);
	/* microseconds interval between pmu polls perf counters */
	fw_ops->perfmon_cmd_init_set_samp_period_us(&cmd.cmd.perfmon, 16700);
	/* number of perfmon counters
	 * counter #3 (GR and CE2) for gk20a
	 */
	fw_ops->perfmon_cmd_init_set_num_cnt(&cmd.cmd.perfmon, 1);
	/* moving average window for sample periods
	 * TBD: = 3000000 / sample_period_us = 17
	 */
	fw_ops->perfmon_cmd_init_set_mov_avg(&cmd.cmd.perfmon, 17);

	(void) memset(&payload, 0, sizeof(struct pmu_payload));
	payload.in.buf = fw_ops->get_perfmon_cntr_ptr(pmu);
	payload.in.size = fw_ops->get_perfmon_cntr_sz(pmu);
	status = fw_ops->get_perfmon_cmd_init_offset_of_var(COUNTER_ALLOC,
							&payload.in.offset);
	if (status != 0) {
		nvgpu_err(g, "failed to get payload offset, command skipped");
		return status;
	}

	nvgpu_pmu_dbg(g, "cmd post PMU_PERFMON_CMD_ID_INIT");
	status = nvgpu_pmu_cmd_post(g, &cmd, &payload, PMU_COMMAND_QUEUE_LPQ,
			NULL, NULL);
	if (status != 0) {
		nvgpu_err(g, "failed cmd post PMU_PERFMON_CMD_ID_INIT");
		return status;
	}

	return 0;
}


int nvgpu_pmu_perfmon_start_sampling(struct nvgpu_pmu *pmu)
{
	struct gk20a *g = pmu->g;
	struct pmu_fw_ver_ops *fw_ops = &pmu->fw->ops;
	struct pmu_cmd cmd;
	struct pmu_payload payload;
	int status;
	u64 tmp_size;

	if (!nvgpu_is_enabled(g, NVGPU_PMU_PERFMON)) {
		return 0;
	}

	/* PERFMON Start */
	(void) memset(&cmd, 0, sizeof(struct pmu_cmd));
	cmd.hdr.unit_id = get_perfmon_id(pmu);
	if (cmd.hdr.unit_id == PMU_UNIT_INVALID) {
		nvgpu_err(g, "failed to get perfmon UNIT ID, command skipped");
		return -EINVAL;
	}
	tmp_size = PMU_CMD_HDR_SIZE +
		(u64)fw_ops->get_perfmon_cmd_start_size();
	nvgpu_assert(tmp_size <= U8_MAX);
	cmd.hdr.size = (u8)tmp_size;
	fw_ops->perfmon_start_set_cmd_type(&cmd.cmd.perfmon,
		PMU_PERFMON_CMD_ID_START);
	fw_ops->perfmon_start_set_group_id(&cmd.cmd.perfmon,
		PMU_DOMAIN_GROUP_PSTATE);

	fw_ops->perfmon_start_set_state_id(&cmd.cmd.perfmon,
		pmu->pmu_perfmon->perfmon_state_id[PMU_DOMAIN_GROUP_PSTATE]);

	fw_ops->perfmon_start_set_flags(&cmd.cmd.perfmon,
		PMU_PERFMON_FLAG_ENABLE_INCREASE |
		PMU_PERFMON_FLAG_ENABLE_DECREASE |
		PMU_PERFMON_FLAG_CLEAR_PREV);

	(void) memset(&payload, 0, sizeof(struct pmu_payload));

	/* TBD: PMU_PERFMON_PCT_TO_INC * 100 */
	fw_ops->set_perfmon_cntr_ut(pmu, 3000); /* 30% */
	/* TBD: PMU_PERFMON_PCT_TO_DEC * 100 */
	fw_ops->set_perfmon_cntr_lt(pmu, 1000); /* 10% */
	fw_ops->set_perfmon_cntr_valid(pmu, true);

	payload.in.buf = fw_ops->get_perfmon_cntr_ptr(pmu);
	payload.in.size = fw_ops->get_perfmon_cntr_sz(pmu);
	status = fw_ops->get_perfmon_cmd_start_offset_of_var(COUNTER_ALLOC,
							&payload.in.offset);
	if (status != 0) {
		nvgpu_err(g, "failed to get payload offset, command skipped");
		return status;
	}

	nvgpu_pmu_dbg(g, "cmd post PMU_PERFMON_CMD_ID_START");
	status = nvgpu_pmu_cmd_post(g, &cmd, &payload, PMU_COMMAND_QUEUE_LPQ,
			NULL, NULL);
	if (status != 0) {
		nvgpu_err(g, "failed cmd post PMU_PERFMON_CMD_ID_START");
		return status;
	}

	return 0;
}

int nvgpu_pmu_perfmon_stop_sampling(struct nvgpu_pmu *pmu)
{
	struct gk20a *g = pmu->g;
	struct pmu_cmd cmd;
	u64 tmp_size;
	int status;

	if (!nvgpu_is_enabled(g, NVGPU_PMU_PERFMON)) {
		return 0;
	}

	/* PERFMON Stop */
	(void) memset(&cmd, 0, sizeof(struct pmu_cmd));
	cmd.hdr.unit_id = get_perfmon_id(pmu);
	if (cmd.hdr.unit_id == PMU_UNIT_INVALID) {
		nvgpu_err(g, "failed to get perfmon UNIT ID, command skipped");
		return -EINVAL;
	}
	tmp_size = PMU_CMD_HDR_SIZE + sizeof(struct pmu_perfmon_cmd_stop);
	nvgpu_assert(tmp_size <= U8_MAX);
	cmd.hdr.size = (u8)tmp_size;
	cmd.cmd.perfmon.stop.cmd_type = PMU_PERFMON_CMD_ID_STOP;

	nvgpu_pmu_dbg(g, "cmd post PMU_PERFMON_CMD_ID_STOP");
	status = nvgpu_pmu_cmd_post(g, &cmd, NULL, PMU_COMMAND_QUEUE_LPQ,
			NULL, NULL);
	if (status != 0) {
		nvgpu_err(g, "failed cmd post PMU_PERFMON_CMD_ID_STOP");
		return status;
	}
	return 0;
}

/* Do not trigger engine load refresh flow within min interval */
/* in case user space queries load of different engines b2b */
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
static bool nvgpu_next_pmu_do_load_refresh(struct nvgpu_pmu *pmu) {
	s64 now = nvgpu_current_time_us();
	u32 elapse = (u32)(now - pmu->pmu_perfmon->last_load_refresh_time_us);
	if (elapse < pmu->pmu_perfmon->min_load_refresh_interv_us) {
		return false;
	}
	pmu->pmu_perfmon->last_load_refresh_time_us = now;
	return true;
}

static int nvgpu_next_pmu_load_update_from_pmu(struct gk20a *g)
{
	u32 load;
	int status = 0;
	u32 i;

	if (!g->pmu->pmu_perfmon->perfmon_ready) {
		for (i = 0; i < NVGPU_PMU_PERFMON_CLASS_NUM; i++){
			g->pmu->pmu_perfmon->load_shadow[i] = 0;
			g->pmu->pmu_perfmon->load[i] = 0;
			g->pmu->pmu_perfmon->load_avg[i] = 0;
		}
		goto exit;
	}

	if (!nvgpu_next_pmu_do_load_refresh(g->pmu)) {
		goto exit;
	}

	if (g->pmu->pmu_perfmon->get_samples_rpc != NULL) {
		status = g->pmu->pmu_perfmon->get_samples_rpc(g->pmu);
		if (status != 0){
			goto exit;
		}
		for (i = 0; i < NVGPU_PMU_PERFMON_CLASS_NUM; i++){
			load = g->pmu->pmu_perfmon->load[i];
			g->pmu->pmu_perfmon->load_shadow[i] = load / 10U;
			g->pmu->pmu_perfmon->load_avg[i] = (((9U*g->pmu->pmu_perfmon->load_avg[i]) +
				g->pmu->pmu_perfmon->load_shadow[i]) / 10U);
		}
	} else {
		nvgpu_err(g, "failed update engine loads from pmu");
		status = -ENODEV;
		goto exit;
	}

exit:
	return status;
}
#endif

static int pmu_sysfs_load_refresh(struct gk20a *g, u32 *query,
			enum nvgpu_pmu_perfmon_class perfmon_class)
{
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);
	int err = 0;

	(void) query;
	(void) perfmon_class;

	switch (ver) {
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
	case NVGPU_GPUID_GB10B:
	case NVGPU_GPUID_GB10B_NEXT:

		nvgpu_assert(perfmon_class < NVGPU_PMU_PERFMON_CLASS_NUM);
		err = nvgpu_next_pmu_load_update_from_pmu(g);
		if (err != 0){
			goto exit;
		}

		if (query != NULL) {
			*query = g->pmu->pmu_perfmon->load_shadow[perfmon_class];
		}
		break;
#endif
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB20C)
	case NVGPU_GPUID_GB20C:
		nvgpu_err(g, "GB20C Perform not implemented");
		//err = -ENODEV;
		goto exit;
		break;
#endif
	default:
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
		err = nvgpu_next_pmu_sysfs_load_refresh(g, query, perfmon_class);
#else
		err = -ENODEV;
#endif
		break;
	}

#if defined(CONFIG_NVGPU_NON_FUSA)
#if defined(CONFIG_NVGPU_GB10B) || defined(CONFIG_NVGPU_GB20C)
exit:
#endif
#endif
	return err;
}

/* Call this function for sysfs read path, return queried value if caller provides container */
int nvgpu_pmu_sysfs_load_refresh(struct gk20a *g, u32 *query,
		enum nvgpu_pmu_perfmon_class perfmon_class)
{
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);
	int err = 0;

	/*
	 * Bypass PMU load query before PMU is initiailized in the very first
	 * nvgpu_finalize_poweron call in runtime resume cycle.
	 */
	if ((g->pmu == NULL) || !g->pmu->pmu_perfmon->query_mutex_initialized) {
		if (query != NULL) {
			*query = 0;
		}
		return 0;
	}

	/* poweroff sequence may be ongoing when load query is evoked by external thread */
	nvgpu_mutex_acquire(&g->pmu->pmu_perfmon->query_mutex);

	if (ver <= NVGPU_GPUID_GA10B) {
		u32 load = 0;

		nvgpu_assert(perfmon_class == NVGPU_PMU_PERFMON_CLASS_GR);

		if (!g->pmu->pmu_perfmon->perfmon_ready) {
			g->pmu->pmu_perfmon->load_shadow[NVGPU_PMU_PERFMON_CLASS_GR] = 0;
			g->pmu->pmu_perfmon->load[NVGPU_PMU_PERFMON_CLASS_GR] = 0;
			g->pmu->pmu_perfmon->load_avg[NVGPU_PMU_PERFMON_CLASS_GR] = 0;
			if (query != NULL) {
				*query = 0;
			}
			goto exit;
		}

		if (g->pmu->pmu_perfmon->get_samples_rpc != NULL) {
			err = g->pmu->pmu_perfmon->get_samples_rpc(g->pmu);
			if (err != 0) {
				goto exit;
			}
			load = g->pmu->pmu_perfmon->load[NVGPU_PMU_PERFMON_CLASS_GR];
		} else {
#if defined(CONFIG_NVGPU_FALCON_DEBUG) || defined(CONFIG_NVGPU_FALCON_NON_FUSA)
			err = nvgpu_falcon_copy_from_dmem(g->pmu->flcn,
				g->pmu->pmu_perfmon->sample_buffer,	(u8 *)&load, 2 * 1, 0);
			if (err != 0) {
				nvgpu_err(g, "PMU falcon DMEM copy failed");
				goto exit;
			}
#endif
		}

		g->pmu->pmu_perfmon->load_shadow[NVGPU_PMU_PERFMON_CLASS_GR] = load / 10U;
		g->pmu->pmu_perfmon->load_avg[NVGPU_PMU_PERFMON_CLASS_GR] =
			(((9U*g->pmu->pmu_perfmon->load_avg[NVGPU_PMU_PERFMON_CLASS_GR]) +
			g->pmu->pmu_perfmon->load_shadow[NVGPU_PMU_PERFMON_CLASS_GR]) / 10U);

		if (query != NULL) {
			*query = g->pmu->pmu_perfmon->load_shadow[NVGPU_PMU_PERFMON_CLASS_GR];
		}
	} else {
		err = pmu_sysfs_load_refresh(g, query, perfmon_class);
	}

exit:
	nvgpu_mutex_release(&g->pmu->pmu_perfmon->query_mutex);
	return err;
}

static int pmu_load_query(struct gk20a *g, u32 *query,
			enum nvgpu_pmu_perfmon_class perfmon_class)
{
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);
	int err = 0;

	(void) query;
	(void) perfmon_class;

	switch (ver) {
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
	case NVGPU_GPUID_GB10B:
	case NVGPU_GPUID_GB10B_NEXT:

		nvgpu_assert(perfmon_class < NVGPU_PMU_PERFMON_CLASS_NUM);
		err = nvgpu_next_pmu_load_update_from_pmu(g);
		if (err != 0){
			goto exit;
		}

		if (query != NULL) {
			*query = g->pmu->pmu_perfmon->load_shadow[perfmon_class];
		}
		break;
#endif
#ifdef CONFIG_NVGPU_GB20C
	case NVGPU_GPUID_GB20C:
		nvgpu_err(g, "GB20C Perform not implemented");
		//err = -ENODEV;
		goto exit;
		break;
#endif
	default:
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
		err = nvgpu_next_pmu_load_query(g, query, perfmon_class);
#else
		err = -ENODEV;
#endif
		break;
	}

#if defined(CONFIG_NVGPU_NON_FUSA)
#if defined(CONFIG_NVGPU_GB10B) || defined(CONFIG_NVGPU_GB20C)
exit:
#endif
#endif
	return err;
}

/* Call this function for devfreq DVFS scaling, return queried value if caller provides container */
int nvgpu_pmu_load_query(struct gk20a *g, u32 *query, enum nvgpu_pmu_perfmon_class perfmon_class)
{
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);
	int err = 0;

	/*
	 * Bypass PMU load query before PMU is initiailized in the very first
	 * nvgpu_finalize_poweron call in runtime resume cycle.
	 */
	if ((g->pmu == NULL) || !g->pmu->pmu_perfmon->query_mutex_initialized) {
		if (query != NULL) {
			*query = 0;
		}
		return 0;
	}

	/* poweroff sequence may be ongoing when load query is evoked by external thread */
	nvgpu_mutex_acquire(&g->pmu->pmu_perfmon->query_mutex);
	gk20a_busy_noresume(g);
	if (nvgpu_is_powered_off(g)) {
		if (query != NULL) {
			*query = 0;
		}
		goto exit;
	}

	if (ver <= NVGPU_GPUID_GA10B) {
		u64 busy_cycles, total_cycles;
		u32 intr_status;

		nvgpu_assert(perfmon_class == NVGPU_PMU_PERFMON_CLASS_GR);


		if (g->ops.pmu.pmu_read_idle_counter == NULL ||
			g->ops.pmu.pmu_reset_idle_counter == NULL ||
			g->ops.pmu.pmu_read_idle_intr_status == NULL ||
			g->ops.pmu.pmu_clear_idle_intr_status == NULL) {
			g->pmu->pmu_perfmon->load_shadow[NVGPU_PMU_PERFMON_CLASS_GR]
				= PMU_BUSY_CYCLES_NORM_MAX;
			if (query != NULL) {
				*query = PMU_BUSY_CYCLES_NORM_MAX;
			}
			goto exit;
		}

		busy_cycles = g->ops.pmu.pmu_read_idle_counter(g, 4);
		total_cycles = g->ops.pmu.pmu_read_idle_counter(g, 0);
		intr_status = g->ops.pmu.pmu_read_idle_intr_status(g);

		g->ops.pmu.pmu_reset_idle_counter(g, 4);
		g->ops.pmu.pmu_reset_idle_counter(g, 0);

		if (intr_status != 0UL) {
			g->pmu->pmu_perfmon->load_shadow[NVGPU_PMU_PERFMON_CLASS_GR]
				= PMU_BUSY_CYCLES_NORM_MAX;
			g->ops.pmu.pmu_clear_idle_intr_status(g);
		} else if (total_cycles == 0ULL || busy_cycles > total_cycles) {
			g->pmu->pmu_perfmon->load_shadow[NVGPU_PMU_PERFMON_CLASS_GR]
				= PMU_BUSY_CYCLES_NORM_MAX;
		} else {
			g->pmu->pmu_perfmon->load_shadow[NVGPU_PMU_PERFMON_CLASS_GR] =
		(u32)((busy_cycles * PMU_BUSY_CYCLES_NORM_MAX / total_cycles) & 0xFFFFFFFFU);
		}

		if (query != NULL) {
			*query = g->pmu->pmu_perfmon->load_shadow[NVGPU_PMU_PERFMON_CLASS_GR];
		}
	} else {
		err = pmu_load_query(g, query, perfmon_class);
	}

exit:
	gk20a_idle_nosuspend(g);
	nvgpu_mutex_release(&g->pmu->pmu_perfmon->query_mutex);
	return err;
}

/* Call this function to read current value w/o new query */
void nvgpu_pmu_load_read(struct gk20a *g, u32 *value, enum nvgpu_pmu_perfmon_class perfmon_class)
{
	nvgpu_assert(perfmon_class < NVGPU_PMU_PERFMON_CLASS_NUM);

	*value = g->pmu->pmu_perfmon->load_shadow[perfmon_class];
}

static void pmu_get_load_counters(struct gk20a *g, u32 *busy_cycles,
				u32 *total_cycles)
{
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);

	(void) busy_cycles;
	(void) total_cycles;

	switch (ver) {
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
	case NVGPU_GPUID_GB10B:
	case NVGPU_GPUID_GB10B_NEXT:
		*busy_cycles = 0;
		*total_cycles = 0;
		nvgpu_err(g, "sysfs counter is not supported!");
		break;
#endif
#ifdef CONFIG_NVGPU_GB20C
	case NVGPU_GPUID_GB20C:
		*busy_cycles = 0;
		*total_cycles = 0;
		nvgpu_err(g, "sysfs counter is not supported!");
		break;
#endif
	default:
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
		nvgpu_next_pmu_get_load_counters(g, busy_cycles, total_cycles);
#endif
		break;
	}
}

void nvgpu_pmu_get_load_counters(struct gk20a *g, u32 *busy_cycles,
				 u32 *total_cycles)
{
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);

	if (nvgpu_is_powered_off(g) || gk20a_busy(g) != 0) {
		*busy_cycles = 0;
		*total_cycles = 0;
		return;
	}

	if (ver <= NVGPU_GPUID_GA10B) {

		*busy_cycles = g->ops.pmu.pmu_read_idle_counter(g, 1);
		*total_cycles = g->ops.pmu.pmu_read_idle_counter(g, 2);

	} else {
		pmu_get_load_counters(g, busy_cycles, total_cycles);
	}

	gk20a_idle(g);
}

static void pmu_reset_load_counters(struct gk20a *g)
{
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);

	switch (ver) {
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
	case NVGPU_GPUID_GB10B:
	case NVGPU_GPUID_GB10B_NEXT:
		nvgpu_err(g, "sysfs counter_reset is not supported!");
		break;
#endif
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB20C)
	case NVGPU_GPUID_GB20C:
		nvgpu_err(g, "sysfs counter_reset is not supported!");
		break;
#endif
	default:
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
		nvgpu_next_pmu_reset_load_counters(g);
#endif
		break;
	}
}
void nvgpu_pmu_reset_load_counters(struct gk20a *g)
{
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);

	if (nvgpu_is_powered_off(g) || gk20a_busy(g) != 0) {
		return;
	}

	if (ver <= NVGPU_GPUID_GA10B) {

		g->ops.pmu.pmu_reset_idle_counter(g, 2);
		g->ops.pmu.pmu_reset_idle_counter(g, 1);

	} else {
		pmu_reset_load_counters(g);
	}

	gk20a_idle(g);
}

int nvgpu_pmu_handle_perfmon_event(struct gk20a *g,
		struct nvgpu_pmu *pmu, void *pmsg)
{
	struct pmu_msg *msg = (struct pmu_msg *) pmsg;
	struct pmu_perfmon_msg *perfmon_msg = &msg->msg.perfmon;
	nvgpu_log_fn(g, " ");

	switch (perfmon_msg->msg_type) {
	case PMU_PERFMON_MSG_ID_INCREASE_EVENT:
		nvgpu_pmu_dbg(g, "perfmon increase event: ");
		nvgpu_pmu_dbg(g, "state_id %d, ground_id %d, pct %d",
			perfmon_msg->gen.state_id, perfmon_msg->gen.group_id,
			perfmon_msg->gen.data);
		(pmu->pmu_perfmon->perfmon_events_cnt)++;
		break;
	case PMU_PERFMON_MSG_ID_DECREASE_EVENT:
		nvgpu_pmu_dbg(g, "perfmon decrease event: ");
		nvgpu_pmu_dbg(g, "state_id %d, ground_id %d, pct %d",
			perfmon_msg->gen.state_id, perfmon_msg->gen.group_id,
			perfmon_msg->gen.data);
		(pmu->pmu_perfmon->perfmon_events_cnt)++;
		break;
	case PMU_PERFMON_MSG_ID_INIT_EVENT:
		pmu->pmu_perfmon->perfmon_ready = true;
		nvgpu_pmu_dbg(g, "perfmon init event");
		break;
	default:
		nvgpu_pmu_dbg(g, "Invalid msgtype:%u for %s",
				perfmon_msg->msg_type, __func__);
		break;
	}

	/* restart sampling */
	if (pmu->pmu_perfmon->perfmon_sampling_enabled) {
		return nvgpu_pmu_perfmon_start_sample(g, pmu,
					pmu->pmu_perfmon);
	}

	return 0;
}

int nvgpu_pmu_handle_perfmon_event_rpc(struct gk20a *g,
		struct nvgpu_pmu *pmu, void *msg)
{
	struct pmu_nvgpu_rpc_struct *pmurpc =
			(struct pmu_nvgpu_rpc_struct *) msg;
	struct pmu_nvgpu_rpc_perfmon_init *perfmon_rpc =
				&pmurpc->event_rpc.perfmon_init;

	nvgpu_log_fn(g, " ");

	switch (pmurpc->event_rpc.hdr.function) {
	case PMU_RPC_ID_PERFMON_CHANGE_EVENT:
		if (((struct pmu_nvgpu_rpc_perfmon_change *)
				(void *)perfmon_rpc)->b_increase) {
			nvgpu_pmu_dbg(g, "perfmon increase event");
		} else {
			nvgpu_pmu_dbg(g, "perfmon decrease event");
		}
		(pmu->pmu_perfmon->perfmon_events_cnt)++;
		break;
	case PMU_RPC_ID_PERFMON_INIT_EVENT:
		nvgpu_pmu_dbg(g, "perfmon init event");
		pmu->pmu_perfmon->perfmon_ready = true;
		break;
	default:
		nvgpu_pmu_dbg(g, "invalid perfmon event %d",
				perfmon_rpc->rpc_hdr.function);
		break;
	}

	/* restart sampling */
	if (pmu->pmu_perfmon->perfmon_sampling_enabled) {
		return nvgpu_pmu_perfmon_start_sample(g, pmu,
					pmu->pmu_perfmon);
	}

	return 0;
}

static void pmu_init_perfmon_params(struct gk20a *g, struct nv_pmu_rpc_struct_perfmon_init *rpc)
{
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);

	(void) rpc;

	switch (ver) {
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
	case NVGPU_GPUID_GB10B:
	case NVGPU_GPUID_GB10B_NEXT:
#ifdef CONFIG_NVGPU_SIM
		if (!nvgpu_platform_is_silicon(g)) {
			/* microseconds interval between pmu polls perf counters */
			rpc->sample_periodus = PMU_PERFMON_SAMPLE_PERIOD_US_SIM;
			g->pmu->pmu_perfmon->min_load_refresh_interv_us = PMU_PERFMON_SAMPLE_PERIOD_US_SIM;
		} else
#endif
		{
			/* microseconds interval between pmu polls perf counters */
			rpc->sample_periodus = PMU_PERFMON_SAMPLE_PERIOD_US;
			g->pmu->pmu_perfmon->min_load_refresh_interv_us = PMU_PERFMON_SAMPLE_PERIOD_US;
		}

		/* number of sample periods below lower threshold
		* before pmu triggers perfmon decrease event
		*/
		rpc->to_decrease_count = 15;
		/* index of base counter, aka. always ticking counter */
		rpc->base_counter_id = IDLE_COUNTER_BASE;
		/* moving average window for sample periods */
		rpc->samples_in_moving_avg = PMU_PERFMON_MOVING_AVG_SAMPLE_CNT;
		/* number of perfmon counters excluding base counter */
		rpc->num_counters = 8;

		/* Counters used to count clock/engine busy cycles */
		rpc->counter[0].index = IDLE_COUNTER_GR;
		rpc->counter[1].index = IDLE_COUNTER_NVD;
		rpc->counter[2].index = IDLE_COUNTER_NVENC0;
		rpc->counter[3].index = IDLE_COUNTER_NVENC1;
		rpc->counter[4].index = IDLE_COUNTER_NVDEC0;
		rpc->counter[5].index = IDLE_COUNTER_NVDEC1;
		rpc->counter[6].index = IDLE_COUNTER_OFA;
		rpc->counter[7].index = IDLE_COUNTER_NVJPG0;

		break;
#endif
	default:
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
		nvgpu_next_pmu_init_perfmon_params(g, rpc);
#endif
		break;
	}
}

/* Perfmon RPC */
int nvgpu_pmu_init_perfmon_rpc(struct nvgpu_pmu *pmu)
{
	struct gk20a *g = pmu->g;
	struct nv_pmu_rpc_struct_perfmon_init rpc;
	int status = 0;
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);

	if (!nvgpu_is_enabled(g, NVGPU_PMU_PERFMON)) {
		return 0;
	}

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0, sizeof(struct nv_pmu_rpc_struct_perfmon_init));
	pmu->pmu_perfmon->perfmon_ready = false;
	pmu->pmu_perfmon->last_load_refresh_time_us = nvgpu_current_time_us();

	g->ops.pmu.pmu_init_perfmon_counter(g);

	/* This can happen at every runtime resume sequence, only init mutex once */
	if (!pmu->pmu_perfmon->query_mutex_initialized) {
		nvgpu_mutex_init(&pmu->pmu_perfmon->query_mutex);
		pmu->pmu_perfmon->query_mutex_initialized = true;
	}

	if (ver <= NVGPU_GPUID_GA10B) {
#ifdef CONFIG_NVGPU_SIM
		if (!nvgpu_platform_is_silicon(g)) {
			/* microseconds interval between pmu polls perf counters */
			rpc.sample_periodus = PMU_PERFMON_SAMPLE_PERIOD_US_SIM;
		} else
#endif
		{
			/* microseconds interval between pmu polls perf counters */
			rpc.sample_periodus = 16700;
		}

		/* number of sample periods below lower threshold
		 * before pmu triggers perfmon decrease event
		 */
		rpc.to_decrease_count = 15;
		/* index of base counter, aka. always ticking counter */
		rpc.base_counter_id = 6;
		/* moving average window for sample periods */
		rpc.samples_in_moving_avg = 17;
		/* number of perfmon counters
		 * counter #3 (GR and CE2) for gk20a
		 */
		rpc.num_counters = 1;
		/* Counter used to count GR busy cycles */
		rpc.counter[0].index = 3;
	} else {
		pmu_init_perfmon_params(g, &rpc);
	}

	nvgpu_pmu_dbg(g, "RPC post NV_PMU_RPC_ID_PERFMON_INIT");
	PMU_RPC_EXECUTE(status, pmu, PERFMON_T18X, INIT, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC, status=0x%x", status);
		goto exit;
	}

exit:
	return 0;
}

int nvgpu_pmu_perfmon_start_sampling_rpc(struct nvgpu_pmu *pmu)
{
	struct gk20a *g = pmu->g;
	struct nv_pmu_rpc_struct_perfmon_start rpc;
	int status = 0;

	if (!nvgpu_is_enabled(g, NVGPU_PMU_PERFMON)) {
		return 0;
	}

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0, sizeof(struct nv_pmu_rpc_struct_perfmon_start));
	rpc.group_id = PMU_DOMAIN_GROUP_PSTATE;
	rpc.state_id = pmu->pmu_perfmon->perfmon_state_id[PMU_DOMAIN_GROUP_PSTATE];
	rpc.flags = PMU_PERFMON_FLAG_ENABLE_INCREASE |
				PMU_PERFMON_FLAG_ENABLE_DECREASE |
				PMU_PERFMON_FLAG_CLEAR_PREV;

	rpc.counter[0].upper_threshold = 3000;
	rpc.counter[0].lower_threshold = 1000;

	nvgpu_pmu_dbg(g, "RPC post NV_PMU_RPC_ID_PERFMON_START\n");
	PMU_RPC_EXECUTE(status, pmu, PERFMON_T18X, START, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC, status=0x%x", status);
	}

	return status;
}

int nvgpu_pmu_perfmon_stop_sampling_rpc(struct nvgpu_pmu *pmu)
{
	struct gk20a *g = pmu->g;
	struct nv_pmu_rpc_struct_perfmon_stop rpc;
	int status = 0;

	if (!nvgpu_is_enabled(g, NVGPU_PMU_PERFMON)) {
		return 0;
	}

	nvgpu_log_fn(g, " ");

	(void) memset(&rpc, 0, sizeof(struct nv_pmu_rpc_struct_perfmon_stop));
	/* PERFMON Stop */
	nvgpu_pmu_dbg(g, "RPC post NV_PMU_RPC_ID_PERFMON_STOP\n");
	PMU_RPC_EXECUTE(status, pmu, PERFMON_T18X, STOP, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC, status=0x%x", status);
	}

	return status;
}

int nvgpu_pmu_perfmon_get_samples_rpc(struct nvgpu_pmu *pmu)
{
	struct gk20a *g = pmu->g;
	struct nv_pmu_rpc_struct_perfmon_query rpc;
	int status = 0;

	if (!nvgpu_is_enabled(g, NVGPU_PMU_PERFMON)) {
		return 0;
	}

	nvgpu_pmu_dbg(g, " ");
	pmu->pmu_perfmon->perfmon_query = 0;
	(void) memset(&rpc, 0, sizeof(struct nv_pmu_rpc_struct_perfmon_query));
	/* PERFMON QUERY */
	nvgpu_pmu_dbg(g, "RPC post NV_PMU_RPC_ID_PERFMON_QUERY\n");
	PMU_RPC_EXECUTE(status, pmu, PERFMON_T18X, QUERY, &rpc, 0);
	if (status != 0) {
		nvgpu_err(g, "Failed to execute RPC, status=0x%x", status);
	}

	pmu_wait_message_cond(pmu, nvgpu_get_poll_timeout(g),
				      &pmu->pmu_perfmon->perfmon_query, 1);

	return status;
}

int nvgpu_pmu_perfmon_get_sampling_enable_status(struct nvgpu_pmu *pmu)
{
	return pmu->pmu_perfmon->perfmon_sampling_enabled;
}

void nvgpu_pmu_perfmon_set_sampling_enable_status(struct nvgpu_pmu *pmu,
							bool status)
{
	pmu->pmu_perfmon->perfmon_sampling_enabled = status;
}

u64 nvgpu_pmu_perfmon_get_events_count(struct nvgpu_pmu *pmu)
{
	return pmu->pmu_perfmon->perfmon_events_cnt;
}

u32 nvgpu_pmu_perfmon_get_load_avg(struct nvgpu_pmu *pmu)
{
	return pmu->pmu_perfmon->load_avg[NVGPU_PMU_PERFMON_CLASS_GR];
}

int nvgpu_pmu_perfmon_initialization(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_pmu_perfmon *perfmon)
{
	(void)g;
	return perfmon->init_perfmon(pmu);
}

int nvgpu_pmu_perfmon_start_sample(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_pmu_perfmon *perfmon)
{
	(void)g;
	return perfmon->start_sampling(pmu);
}

int nvgpu_pmu_perfmon_stop_sample(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_pmu_perfmon *perfmon)
{
	(void)g;
	return perfmon->stop_sampling(pmu);
}

int nvgpu_pmu_perfmon_get_sample(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_pmu_perfmon *perfmon)
{
	(void)g;
	return perfmon->get_samples_rpc(pmu);
}

int nvgpu_pmu_perfmon_event_handler(struct gk20a *g,
	struct nvgpu_pmu *pmu, void *msg)
{
	return pmu->pmu_perfmon->perfmon_event_handler(g, pmu, msg);
}
