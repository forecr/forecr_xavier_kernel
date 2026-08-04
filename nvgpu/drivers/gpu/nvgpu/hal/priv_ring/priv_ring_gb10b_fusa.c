// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/log.h>
#include <nvgpu/timers.h>
#include <nvgpu/enabled.h>
#include <nvgpu/io.h>
#include <nvgpu/utils.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/gin.h>

#include "priv_ring_gb10b.h"

#include <nvgpu/hw/gb10b/hw_func_gb10b.h>
#include <nvgpu/hw/gb10b/hw_proj_gb10b.h>
#include <nvgpu/hw/gb10b/hw_pri_ringmaster_gb10b.h>
#include <nvgpu/hw/gb10b/hw_pri_ringstation_sys_gb10b.h>
#include <nvgpu/hw/gb10b/hw_pri_sys_gb10b.h>
#include <nvgpu/hw/gb10b/hw_pri_gpc_gb10b.h>
#include <nvgpu/hw/gb10b/hw_pri_fbp_gb10b.h>

#ifdef CONFIG_NVGPU_MIG
#include <nvgpu/grmgr.h>
#endif

#define PRIV_INIT_POLL_MAX_RETRIES 		(60U)
#define PRIV_INIT_POLL_DELAY_US			(500U)

static void gb10b_priv_ring_stall_handler(struct gk20a *g, u64 cookie)
{
	(void)cookie;
	g->ops.priv_ring.isr(g);
}

static void gb10b_priv_ring_intr_enable(struct gk20a *g)
{
	u32 vector = 0U;
	u32 intr_ctrl_msg = 0U;
	u32 old_vector = 0U;
	u32 old_intr_ctrl_msg = 0U;

	vector = nvgpu_gin_get_unit_stall_vector(g, NVGPU_GIN_INTR_UNIT_PRIV_RING, 0);
	intr_ctrl_msg = nvgpu_gin_get_intr_ctrl_msg(g, vector,
			NVGPU_GIN_CPU_ENABLE, NVGPU_GIN_GSP_DISABLE);

	old_intr_ctrl_msg = nvgpu_readl(g, pri_ringmaster_intr_ctrl_r());
	old_vector = nvgpu_gin_get_vector_from_intr_ctrl_msg(g, old_intr_ctrl_msg);
	nvgpu_writel(g, pri_ringmaster_intr_ctrl_r(), intr_ctrl_msg);
	g->ops.priv_ring.intr_retrigger(g);
	if (vector != old_vector) {
		nvgpu_gin_clear_pending_intr(g, old_vector);
	}

	nvgpu_gin_set_stall_handler(g, vector, &gb10b_priv_ring_stall_handler, 0);
	nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_PRIV_RING, 0, NVGPU_GIN_INTR_ENABLE);
}

int gb10b_priv_ring_enable(struct gk20a *g)
{
	u32 status = 0U;
	struct nvgpu_timeout to;

#ifdef CONFIG_NVGPU_SIM
	if (nvgpu_is_enabled(g, NVGPU_IS_FMODEL)) {
		nvgpu_log_info(g, "priv ring is already enabled");
		gb10b_priv_ring_intr_enable(g);
		return 0;
	}
#endif

	nvgpu_log_info(g, "enabling priv ring");

	nvgpu_cg_slcg_priring_load_enable(g);

	/*
	 * Enable interrupt early on.
	 */
	gb10b_priv_ring_intr_enable(g);

	if (nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
		/* Priv sec is enabled, PRI will be enabled by secure firmware  */
		return status;
	}

	/*
	 * Skip the priv ring init if it is already enabled.
	 */
	status = nvgpu_readl(g, pri_ringstation_sys_pri_ring_init_r());
	if (pri_ringstation_sys_pri_ring_init_status_v(status) ==
			pri_ringstation_sys_pri_ring_init_status_alive_v()) {
		nvgpu_log_info(g, "priv ring is already enabled");
		return 0;
	}

	/*
	 * Make sure NV_PPRIV_SYS_PRI_RING_INIT_STATUS != _CMD_RDY
	 */
	if (pri_ringstation_sys_pri_ring_init_status_v(status) ==
			pri_ringstation_sys_pri_ring_init_status_cmd_rdy_v()) {
		nvgpu_err(g, "priv ring is not ready to accept ring commands");
		return -EAGAIN;
	}

	/*
	 * Start the priv ring init.
	 */
	nvgpu_writel(g, pri_ringstation_sys_pri_ring_init_r(),
			pri_ringstation_sys_pri_ring_init_cmd_enumerate_and_start_f());

	/*
	 * Wait for enumeration to complete and verify it has passed.
	 */
	nvgpu_timeout_init_cpu_timer(g, &to,
			(PRIV_INIT_POLL_MAX_RETRIES * PRIV_INIT_POLL_DELAY_US));
	do {
		status = nvgpu_readl(g, pri_ringstation_sys_pri_ring_init_r());
		if (pri_ringstation_sys_pri_ring_init_cmd_v(status) ==
			pri_ringstation_sys_pri_ring_init_cmd_none_v()) {
			break;
		}
		nvgpu_udelay(PRIV_INIT_POLL_DELAY_US);
	} while (nvgpu_timeout_expired(&to) == 0);

	if (nvgpu_timeout_peek_expired(&to)) {
		nvgpu_err(g, "priv ring enumeration timedout");
		return -ETIMEDOUT;
	}

	/*
	 * Wait for init_status to be not equal to dead and command ready.
	 */
	nvgpu_timeout_init_cpu_timer(g, &to,
			(PRIV_INIT_POLL_MAX_RETRIES * PRIV_INIT_POLL_DELAY_US));
	do {
		status = nvgpu_readl(g, pri_ringstation_sys_pri_ring_init_r());
		if ((pri_ringstation_sys_pri_ring_init_status_v(status) !=
			pri_ringstation_sys_pri_ring_init_status_dead_v()) &&
		 	(pri_ringstation_sys_pri_ring_init_status_v(status) !=
			pri_ringstation_sys_pri_ring_init_status_cmd_rdy_v())) {
			break;
		}
		nvgpu_udelay(PRIV_INIT_POLL_DELAY_US);
	} while (nvgpu_timeout_expired(&to) == 0);

	if (nvgpu_timeout_peek_expired(&to)) {
		nvgpu_err(g, "priv ring enumeration status_dead timedout");
		return -ETIMEDOUT;
	}

	/*
	 * At this point, init_status should report status_alive. Otherwise,
	 * this scenario needs to debugged. For debugging purpose, log
	 * ring_intr_status.
	 */
	if (pri_ringstation_sys_pri_ring_init_status_v(status) !=
			pri_ringstation_sys_pri_ring_init_status_alive_v()) {
		status = nvgpu_readl(g, pri_ringmaster_intr_status0_r());
		nvgpu_err(g, "priv ring init failed. ring_intr_status(0x%x)",
				status);
		return -1;
	}

	return 0;
}

void gb10b_priv_ring_intr_retrigger(struct gk20a *g)
{
	nvgpu_writel(g, pri_ringmaster_intr_retrigger_r(),
			pri_ringmaster_intr_retrigger_trigger_true_f());
}

#ifdef CONFIG_NVGPU_MIG
int gb10b_priv_ring_config_gpc_rs_map(struct gk20a *g, bool enable)
{
	u32 reg_val;
	u32 index;
	u32 local_id;
	u32 logical_gpc_id = 0U;
	struct nvgpu_gr_syspipe *gr_syspipe;

	for (index = 0U; index < g->mig.num_gpu_instances; index++) {
		if (!nvgpu_grmgr_is_mig_type_gpu_instance(
				&g->mig.gpu_instance[index])) {
			nvgpu_log(g, gpu_dbg_mig, "skip physical instance[%u]",
				index);
			continue;
		}
		gr_syspipe = &g->mig.gpu_instance[index].gr_syspipe;

		for (local_id = 0U; local_id < gr_syspipe->num_gpc; local_id++) {
			logical_gpc_id = gr_syspipe->gpcs[local_id].logical_id;
			reg_val = nvgpu_readl(g, pri_ringmaster_gpc_rs_map_r(
				logical_gpc_id));

			if (enable) {
				reg_val = set_field(reg_val,
					pri_ringmaster_gpc_rs_map_smc_engine_id_m(),
					pri_ringmaster_gpc_rs_map_smc_engine_id_f(
						gr_syspipe->gr_syspipe_id));
				reg_val = set_field(reg_val,
					pri_ringmaster_gpc_rs_map_smc_engine_local_cluster_id_m(),
					pri_ringmaster_gpc_rs_map_smc_engine_local_cluster_id_f(
						local_id));
				reg_val = set_field(reg_val,
					pri_ringmaster_gpc_rs_map_smc_enable_m(),
					pri_ringmaster_gpc_rs_map_smc_enable_f(
						pri_ringmaster_gpc_rs_map_smc_enable_true_v()));
			} else {
				reg_val = set_field(reg_val,
					pri_ringmaster_gpc_rs_map_smc_enable_m(),
					pri_ringmaster_gpc_rs_map_smc_enable_f(
						pri_ringmaster_gpc_rs_map_smc_enable_false_v()));
			}

			nvgpu_writel(g, pri_ringmaster_gpc_rs_map_r(logical_gpc_id),
				reg_val);

			nvgpu_log(g, gpu_dbg_mig,
				"[%d] gpu_instance_id[%u] gr_syspipe_id[%u] gr_instance_id[%u] "
					"local_gpc_id[%u] physical_id[%u] logical_id[%u] "
					"gpcgrp_id[%u] reg_val[%x] enable[%d] ",
				index,
				g->mig.gpu_instance[index].gpu_instance_id,
				gr_syspipe->gr_syspipe_id,
				gr_syspipe->gr_instance_id,
				local_id,
				gr_syspipe->gpcs[local_id].physical_id,
				gr_syspipe->gpcs[local_id].logical_id,
				gr_syspipe->gpcs[local_id].gpcgrp_id,
				reg_val,
				enable);
		}
		/*
		 * Do a dummy read on last written GPC to ensure that RS_MAP has been acked
		 * by all slave ringstations.
		 */
		reg_val = nvgpu_readl(g, pri_ringmaster_gpc_rs_map_r(
			logical_gpc_id));
	}

	return 0;
}
#endif
