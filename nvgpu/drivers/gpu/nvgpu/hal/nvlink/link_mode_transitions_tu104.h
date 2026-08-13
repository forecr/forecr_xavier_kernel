/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef LINK_MODE_TRANSITIONS_TU104_H
#define LINK_MODE_TRANSITIONS_TU104_H

#include <nvgpu/types.h>
struct gk20a;

/* API */
int tu104_nvlink_setup_pll(struct gk20a *g, unsigned long link_mask);
u32 tu104_nvlink_link_get_tx_sublink_state(struct gk20a *g, u32 link_id);
u32 tu104_nvlink_link_get_rx_sublink_state(struct gk20a *g, u32 link_id);
int tu104_nvlink_data_ready_en(struct gk20a *g, unsigned long link_mask,
								bool sync);
#endif
