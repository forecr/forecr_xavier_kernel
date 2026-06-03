/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef LINK_MODE_TRANSITIONS_GV100_H
#define LINK_MODE_TRANSITIONS_GV100_H

#include <nvgpu/types.h>

struct gk20a;

#define NVLINK_PLL_ON_TIMEOUT_MS        30
#define NVLINK_SUBLINK_TIMEOUT_MS       200

int gv100_nvlink_setup_pll(struct gk20a *g, unsigned long link_mask);
int gv100_nvlink_data_ready_en(struct gk20a *g,
					unsigned long link_mask, bool sync);
enum nvgpu_nvlink_link_mode gv100_nvlink_get_link_mode(struct gk20a *g,
								u32 link_id);
u32 gv100_nvlink_get_link_state(struct gk20a *g, u32 link_id);
int gv100_nvlink_set_link_mode(struct gk20a *g, u32 link_id,
					enum nvgpu_nvlink_link_mode mode);
enum nvgpu_nvlink_sublink_mode gv100_nvlink_link_get_sublink_mode(
	struct gk20a *g, u32 link_id, bool is_rx_sublink);
u32 gv100_nvlink_link_get_tx_sublink_state(struct gk20a *g, u32 link_id);
u32 gv100_nvlink_link_get_rx_sublink_state(struct gk20a *g, u32 link_id);
int gv100_nvlink_link_set_sublink_mode(struct gk20a *g, u32 link_id,
	bool is_rx_sublink, enum nvgpu_nvlink_sublink_mode mode);
#endif
