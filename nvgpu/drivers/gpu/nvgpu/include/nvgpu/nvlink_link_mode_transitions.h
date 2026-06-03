/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_NVLINK_LINK_MODE_TRANSITIONS_H
#define NVGPU_NVLINK_LINK_MODE_TRANSITIONS_H

struct gk20a;


enum nvgpu_nvlink_link_mode {
	nvgpu_nvlink_link_off,
	nvgpu_nvlink_link_hs,
	nvgpu_nvlink_link_safe,
	nvgpu_nvlink_link_fault,
	nvgpu_nvlink_link_rcvy_ac,
	nvgpu_nvlink_link_rcvy_sw,
	nvgpu_nvlink_link_rcvy_rx,
	nvgpu_nvlink_link_detect,
	nvgpu_nvlink_link_reset,
	nvgpu_nvlink_link_enable_pm,
	nvgpu_nvlink_link_disable_pm,
	nvgpu_nvlink_link_disable_err_detect,
	nvgpu_nvlink_link_lane_disable,
	nvgpu_nvlink_link_lane_shutdown,
	nvgpu_nvlink_link__last,
};

enum nvgpu_nvlink_sublink_mode {
	nvgpu_nvlink_sublink_tx_hs,
	nvgpu_nvlink_sublink_tx_enable_pm,
	nvgpu_nvlink_sublink_tx_disable_pm,
	nvgpu_nvlink_sublink_tx_single_lane,
	nvgpu_nvlink_sublink_tx_safe,
	nvgpu_nvlink_sublink_tx_off,
	nvgpu_nvlink_sublink_tx_common,
	nvgpu_nvlink_sublink_tx_common_disable,
	nvgpu_nvlink_sublink_tx_data_ready,
	nvgpu_nvlink_sublink_tx_prbs_en,
	nvgpu_nvlink_sublink_tx__last,
	/* RX */
	nvgpu_nvlink_sublink_rx_hs,
	nvgpu_nvlink_sublink_rx_enable_pm,
	nvgpu_nvlink_sublink_rx_disable_pm,
	nvgpu_nvlink_sublink_rx_single_lane,
	nvgpu_nvlink_sublink_rx_safe,
	nvgpu_nvlink_sublink_rx_off,
	nvgpu_nvlink_sublink_rx_rxcal,
	nvgpu_nvlink_sublink_rx__last,
};

enum nvgpu_nvlink_link_mode nvgpu_nvlink_get_link_mode(struct gk20a *g);
u32 nvgpu_nvlink_get_link_state(struct gk20a *g);
int nvgpu_nvlink_set_link_mode(struct gk20a *g,
					enum nvgpu_nvlink_link_mode mode);
void nvgpu_nvlink_get_tx_sublink_state(struct gk20a *g, u32 *state);
void nvgpu_nvlink_get_rx_sublink_state(struct gk20a *g, u32 *state);
enum nvgpu_nvlink_sublink_mode nvgpu_nvlink_get_sublink_mode(struct gk20a *g,
					bool is_rx_sublink);
int nvgpu_nvlink_set_sublink_mode(struct gk20a *g, bool is_rx_sublink,
					enum nvgpu_nvlink_sublink_mode mode);

#endif /* NVGPU_NVLINK_LINK_MODE_TRANSITIONS_H */
