// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>
#include <nvgpu/io.h>
#include <nvgpu/pmu/cmd.h>
#include <nvgpu/pmu/pmu_model_20.h>

#ifdef CONFIG_NVGPU_GB20C
#include <nvgpu/hw/gb20c/hw_pwr_gb20c.h>
#endif

#ifdef CONFIG_NVGPU_LS_PMU
#ifdef CONFIG_NVGPU_GB20C
/* Register PMU riscv writes PMU2.0 FW lifetime init tracking state */
#define NV_PMU_20_LIFETIME_INIT_TRACKING_r \
			pwr_falcon_common_scratch_group_0_r(0)
#endif
/* PMU2.0 FW lifetime init tracking state */
#define NV_PMU_20_LIFETIME_INIT_TRACKING_STATE_MASK  (0xFF)
#define NV_PMU_20_LIFETIME_INIT_TRACKING_STATE_REGISTER_READY	(0x4U)

int nvgpu_pmu_wait_fw_driver_register_ready(struct gk20a *g)
{
	int status = 0;
	struct nvgpu_timeout timeout;
	unsigned int delay = POLL_DELAY_MIN_US;
	u32 regVal = 0;
	u32 cur_pmu20_lifetime_state = 0xFFFFFFFF;

	nvgpu_timeout_init_cpu_timer(g, &timeout, nvgpu_get_poll_timeout(g));

	do {
		nvgpu_rmb();

		nvgpu_usleep_range(delay, delay * 2U);
#ifdef CONFIG_NVGPU_GB20C
		regVal = nvgpu_readl(g, NV_PMU_20_LIFETIME_INIT_TRACKING_r);
#endif
		if ((regVal & NV_PMU_20_LIFETIME_INIT_TRACKING_STATE_MASK) !=
			cur_pmu20_lifetime_state) {
			cur_pmu20_lifetime_state =
				(regVal & NV_PMU_20_LIFETIME_INIT_TRACKING_STATE_MASK);
			nvgpu_pmu_dbg(g, "cur_pmu20_lifetime_state 0x%x",
							cur_pmu20_lifetime_state);
		}

		if ((regVal & NV_PMU_20_LIFETIME_INIT_TRACKING_STATE_MASK) ==
				NV_PMU_20_LIFETIME_INIT_TRACKING_STATE_REGISTER_READY) {
			return status;
		}

		delay = min_t(u32, delay << 1, POLL_DELAY_MAX_US);

	} while (nvgpu_timeout_expired(&timeout) == 0);

	return -ETIMEDOUT;
}

int nvgpu_pmu_driver_register_send(struct gk20a *g,
		enum  driver_register_type register_type, u32 client_mask)
{
	int  err = 0;
	bool send_rpc = true;

	struct pmu_rpc_struct_driver_register_event rpc;

	(void) memset(&rpc, 0,
			sizeof(struct pmu_rpc_struct_driver_register_event));

	switch (register_type) {
	case register_start:
		rpc.registerEvent = NV_PMU_DRIVER_REGISTER_EVENT_REGISTER_START;
		g->pmu->driver_register_client_mask = client_mask;
		break;
	case register_complete:
		rpc.registerEvent = NV_PMU_DRIVER_REGISTER_EVENT_REGISTER_COMPLETE;
		g->pmu->driver_register_client_mask &= ~client_mask;
		if (g->pmu->driver_register_client_mask) {
			send_rpc = false;
		}
		break;
	case unregister_start:
		rpc.registerEvent = NV_PMU_DRIVER_REGISTER_EVENT_UNREGISTER_START;
		break;
	case unregister_complete:
		rpc.registerEvent = NV_PMU_DRIVER_REGISTER_EVENT_UNREGISTER_COMPLETE;
		break;
	default:
		err = -EINVAL;
		goto exit;
	}

	if (send_rpc) {
		nvgpu_log_info(g, "Send driver_register rpc, type %d, client_mask 0x%x",
			register_type, g->pmu->driver_register_client_mask);
		PMU_RPC_EXECUTE(err, g->pmu, CMDMGMT, DRIVER_REGISTER_EVENT, &rpc, 0);
	} else {
		nvgpu_log_info(g, "Skip driver_register rpc, type %d, client_mask 0x%x",
			register_type, g->pmu->driver_register_client_mask);
	}

exit:
	return err;
}
#endif
