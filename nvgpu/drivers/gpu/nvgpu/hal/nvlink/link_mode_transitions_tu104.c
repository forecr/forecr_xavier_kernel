// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#ifdef CONFIG_NVGPU_NVLINK

#include <nvgpu/io.h>
#include <nvgpu/timers.h>
#include <nvgpu/gk20a.h>

#include "link_mode_transitions_tu104.h"

#include <nvgpu/hw/tu104/hw_nvl_tu104.h>

int tu104_nvlink_setup_pll(struct gk20a *g, unsigned long link_mask)
{
	int ret = 0;
	u32 link_id;
	u32 reg;
	struct nvgpu_timeout timeout;
	unsigned long bit;

	for_each_set_bit(bit, &link_mask, NVLINK_MAX_LINKS_SW) {
		link_id = (u32)bit;
		ret = g->ops.nvlink.minion.send_dlcmd(g, link_id,
			NVGPU_NVLINK_MINION_DLCMD_TXCLKSWITCH_PLL, true);
		if (ret != 0) {
			nvgpu_err(g, "Error: TXCLKSWITCH_PLL dlcmd on link %u",
								link_id);
			return ret;
		}

		nvgpu_timeout_init_cpu_timer(g, &timeout, NV_NVLINK_REG_POLL_TIMEOUT_MS);

		do {
			reg = DLPL_REG_RD32(g, link_id, nvl_clk_status_r());
			if (nvl_clk_status_txclk_sts_v(reg) ==
					nvl_clk_status_txclk_sts_pll_clk_v()) {
				nvgpu_log(g, gpu_dbg_nvlink,
					"PLL SETUP successful on link %u",
					link_id);
				break;
			}
			nvgpu_udelay(NV_NVLINK_TIMEOUT_DELAY_US);
		} while (nvgpu_timeout_expired_msg(&timeout,
					"Timed out setting pll on link %u",
					link_id) == 0);
		if (nvgpu_timeout_peek_expired(&timeout)) {
			return -ETIMEDOUT;
		}
	}
	return ret;
}

u32 tu104_nvlink_link_get_tx_sublink_state(struct gk20a *g, u32 link_id)
{
	u32 reg;
	struct nvgpu_timeout timeout;

	nvgpu_timeout_init_cpu_timer(g, &timeout, NV_NVLINK_REG_POLL_TIMEOUT_MS);

	/* Poll till substate value becomes STABLE */
	do {
		reg = DLPL_REG_RD32(g, link_id, nvl_sl0_slsm_status_tx_r());
		if (nvl_sl0_slsm_status_tx_substate_v(reg) ==
				nvl_sl0_slsm_status_tx_substate_stable_v()) {
			return nvl_sl0_slsm_status_tx_primary_state_v(reg);
		}
		nvgpu_udelay(NV_NVLINK_TIMEOUT_DELAY_US);
	} while (nvgpu_timeout_expired_msg(&timeout,
			"Timeout on TX SLSM substate = stable check") == 0);

	nvgpu_log(g, gpu_dbg_nvlink, "TX SLSM primary state :%u, substate:%u",
				nvl_sl0_slsm_status_tx_primary_state_v(reg),
				nvl_sl0_slsm_status_tx_substate_v(reg));

	return nvl_sl0_slsm_status_tx_primary_state_unknown_v();
}

u32 tu104_nvlink_link_get_rx_sublink_state(struct gk20a *g, u32 link_id)
{
	u32 reg;
	struct nvgpu_timeout timeout;

	nvgpu_timeout_init_cpu_timer(g, &timeout, NV_NVLINK_REG_POLL_TIMEOUT_MS);

	/* Poll till substate value becomes STABLE */
	do {
		reg = DLPL_REG_RD32(g, link_id, nvl_sl1_slsm_status_rx_r());
		if (nvl_sl1_slsm_status_rx_substate_v(reg) ==
				nvl_sl1_slsm_status_rx_substate_stable_v()) {
			return nvl_sl1_slsm_status_rx_primary_state_v(reg);
		}
		nvgpu_udelay(NV_NVLINK_TIMEOUT_DELAY_US);
	} while (nvgpu_timeout_expired_msg(&timeout,
			"Timeout on RX SLSM substate = stable check") == 0);

	nvgpu_log(g, gpu_dbg_nvlink, "RX SLSM primary state :%u, substate:%u",
				nvl_sl1_slsm_status_rx_primary_state_v(reg),
				nvl_sl1_slsm_status_rx_substate_v(reg));

	return nvl_sl1_slsm_status_rx_primary_state_unknown_v();
}

int tu104_nvlink_data_ready_en(struct gk20a *g,
					unsigned long link_mask, bool sync)
{
	int ret = 0;
	u32 link_id;
	unsigned long bit;

	/* On Volta, the order of INIT* DLCMDs was arbitrary.
	 * On Turing, the INIT* DLCMDs need to be executed in the following
	 * order -
	 * INITDLPL -> INITL -> INITLANEENABLE.
	 * INITDLPL_TO_CHIPA is needed additionally when connected  to 2.0 dev.
	 */
	for_each_set_bit(bit, &link_mask, NVLINK_MAX_LINKS_SW) {
		link_id = (u32)bit;
		ret = g->ops.nvlink.minion.send_dlcmd(g, link_id,
			NVGPU_NVLINK_MINION_DLCMD_INITDLPL, sync);
		if (ret != 0) {
			nvgpu_err(g, "Minion initdlpl failed on link %u",
								link_id);
			return ret;
		}
	}
	for_each_set_bit(bit, &link_mask, NVLINK_MAX_LINKS_SW) {
		link_id = (u32)bit;
		ret = g->ops.nvlink.minion.send_dlcmd(g, link_id,
			NVGPU_NVLINK_MINION_DLCMD_TURING_INITDLPL_TO_CHIPA,
			sync);
		if (ret != 0) {
			nvgpu_err(g, "Minion initdlpl_to_chipA failed on link\
								%u", link_id);
			return ret;
		}
	}
	for_each_set_bit(bit, &link_mask, NVLINK_MAX_LINKS_SW) {
		link_id = (u32)bit;
		ret = g->ops.nvlink.minion.send_dlcmd(g, link_id,
			NVGPU_NVLINK_MINION_DLCMD_INITTL, sync);
		if (ret != 0) {
			nvgpu_err(g, "Minion inittl failed on link %u",
								link_id);
			return ret;
		}
	}
	for_each_set_bit(bit, &link_mask, NVLINK_MAX_LINKS_SW) {
		link_id = (u32)bit;
		ret = g->ops.nvlink.minion.send_dlcmd(g, link_id,
			NVGPU_NVLINK_MINION_DLCMD_INITLANEENABLE, sync);
		if (ret != 0) {
			nvgpu_err(g, "Minion initlaneenable failed on link %u",
								link_id);
			return ret;
		}
	}
	return ret;
}
#endif /* CONFIG_NVGPU_NVLINK */
