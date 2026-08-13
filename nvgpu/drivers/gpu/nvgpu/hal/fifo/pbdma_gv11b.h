/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PBDMA_GV11B_H
#define NVGPU_PBDMA_GV11B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_device;

void gv11b_pbdma_setup_hw(struct gk20a *g);
void gv11b_pbdma_intr_enable(struct gk20a *g, bool enable);
bool gv11b_pbdma_handle_intr_0(struct gk20a *g, u32 pbdma_id, u32 pbdma_intr_0,
			u32 *error_notifier);
bool gv11b_pbdma_handle_intr_1(struct gk20a *g, u32 pbdma_id, u32 pbdma_intr_1,
			u32 *error_notifier);
u32 gv11b_pbdma_channel_fatal_0_intr_descs(void);
u32 gv11b_pbdma_get_fc_pb_header(void);
u32 gv11b_pbdma_get_fc_target(const struct nvgpu_device *dev);
u32 gv11b_pbdma_set_channel_info_veid(u32 subctx_id);
u32 gv11b_pbdma_config_userd_writeback_enable(u32 v);
void gv11b_pbdma_report_error(struct gk20a *g, u32 pbdma_id,
		u32 pbdma_intr_0);

#endif /* NVGPU_PBDMA_GV11B_H */
