/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_NVLINK_TU104_H
#define NVGPU_NVLINK_TU104_H

#define TU104_CONNECTED_LINK_MASK		0x1

struct gk20a;

/* API */
int tu104_nvlink_rxdet(struct gk20a *g, u32 link_id);
void tu104_nvlink_get_connected_link_mask(u32 *link_mask);
#endif
