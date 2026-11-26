/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CE_GB10B_H
#define NVGPU_CE_GB10B_H

struct gk20a;

void gb10b_ce_init_pce2lce_configs(struct gk20a *g, u32 *pce2lce_configs);
void gb10b_ce_init_grce_configs(struct gk20a *g, u32 *grce_configs);
u32 gb10b_ce_pce_map_value(struct gk20a *g);
void gb10b_ce_set_pce2lce_mapping(struct gk20a *g);
void gb10b_ce_intr_enable(struct gk20a *g, bool enable);
void gb10b_ce_set_engine_reset(struct gk20a *g,
		const struct nvgpu_device *dev, bool assert_reset);
void gb10b_ce_stall_isr(struct gk20a *g, u32 inst_id, u32 pri_base,
		bool *needs_rc, bool *needs_quiesce);
u32 gb10b_grce_config_size(void);

#endif
