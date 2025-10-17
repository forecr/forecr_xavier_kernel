/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_INTR_GA100_H
#define NVGPU_GR_INTR_GA100_H

struct gk20a;
struct nvgpu_gr_config;

int ga100_gr_intr_handle_sw_method(struct gk20a *g, u32 addr,
			u32 class_num, u32 offset, u32 data);
void ga100_gr_intr_enable_exceptions(struct gk20a *g,
			struct nvgpu_gr_config *gr_config, bool enable);
bool ga100_gr_intr_handle_exceptions(struct gk20a *g, bool *is_gpc_exception);
void ga100_gr_intr_enable_gpc_exceptions(struct gk20a *g,
					 struct nvgpu_gr_config *gr_config);
u32 ga100_gr_intr_enable_mask(struct gk20a *g);
u32 ga100_gr_intr_read_pending_interrupts(struct gk20a *g,
					struct nvgpu_gr_intr_info *intr_info);

#endif /* NVGPU_GR_INTR_GA100_H */
