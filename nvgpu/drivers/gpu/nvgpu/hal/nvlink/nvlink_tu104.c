// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#ifdef CONFIG_NVGPU_NVLINK

#include <nvgpu/nvgpu_common.h>
#include <nvgpu/bitops.h>
#include <nvgpu/nvlink.h>
#include <nvgpu/enabled.h>
#include <nvgpu/io.h>
#include <nvgpu/timers.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvlink_minion.h>

#include "nvlink_gv100.h"
#include "nvlink_tu104.h"

#include <nvgpu/hw/tu104/hw_nvl_tu104.h>

int tu104_nvlink_rxdet(struct gk20a *g, u32 link_id)
{
	int ret = 0;
	u32 reg;
	struct nvgpu_timeout timeout;

	ret = g->ops.nvlink.minion.send_dlcmd(g, link_id,
				NVGPU_NVLINK_MINION_DLCMD_INITRXTERM, true);
	if (ret != 0) {
		nvgpu_err(g, "Error during INITRXTERM minion DLCMD on link %u",
				link_id);
		return ret;
	}

	ret = g->ops.nvlink.minion.send_dlcmd(g, link_id,
			NVGPU_NVLINK_MINION_DLCMD_TURING_RXDET, true);
	if (ret != 0) {
		nvgpu_err(g, "Error during RXDET minion DLCMD on link %u",
				link_id);
		return ret;
	}

	nvgpu_timeout_init_cpu_timer(g, &timeout, NV_NVLINK_REG_POLL_TIMEOUT_MS);

	do {
		reg = DLPL_REG_RD32(g, link_id, nvl_sl0_link_rxdet_status_r());
		if (nvl_sl0_link_rxdet_status_sts_v(reg) ==
				nvl_sl0_link_rxdet_status_sts_found_v()) {
			nvgpu_log(g, gpu_dbg_nvlink,
					"RXDET successful on link %u", link_id);
			return ret;
		}
		if (nvl_sl0_link_rxdet_status_sts_v(reg) ==
				nvl_sl0_link_rxdet_status_sts_timeout_v()) {
			nvgpu_log(g, gpu_dbg_nvlink,
					"RXDET failed on link %u", link_id);
			break;
		}
		nvgpu_udelay(NV_NVLINK_TIMEOUT_DELAY_US);
	} while (nvgpu_timeout_expired_msg(
				&timeout,
				"RXDET status check timed out on link %u",
				link_id) == 0);
	return -ETIMEDOUT;
}

void tu104_nvlink_get_connected_link_mask(u32 *link_mask)
{
	*link_mask = TU104_CONNECTED_LINK_MASK;
}

#endif /* CONFIG_NVGPU_NVLINK */
