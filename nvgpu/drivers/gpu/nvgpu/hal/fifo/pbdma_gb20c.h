/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PBDMA_GB20C_H
#define NVGPU_PBDMA_GB20C_H

#include <nvgpu/types.h>

#define PBDMA_METHOD_SUBCH_FIVE 	5U
#define PBDMA_METHOD_SUBCH_SIX		6U
#define PBDMA_METHOD_SUBCH_SEVEN	7U

void gb20c_pbdma_dump_status(struct gk20a *g, struct nvgpu_debug_context *o);

u32 gb20c_pbdma_read_data(struct gk20a *g, u32 pbdma_id);
u32 gb20c_pbdma_get_mmu_fault_id(struct gk20a *g, u32 pbdma_id);
u32 gb20c_pbdma_get_fc_target(const struct nvgpu_device *dev);
u32 gb20c_pbdma_get_config_auth_level_privileged(void);
u32 gb20c_pbdma_get_num_of_pbdmas(void);
void gb20c_pbdma_intr_enable(struct gk20a *g, bool enable);
void gb20c_pbdma_reset_method(struct gk20a *g, u32 pbdma_id,
			u32 pbdma_method_index);
void gb20c_pbdma_dump_intr_0(struct gk20a *g, u32 pbdma_id,
				u32 pbdma_intr_0);
void gb20c_pbdma_reset_header(struct gk20a *g, u32 pbdma_id);
bool gb20c_pbdma_is_sw_method_subch(struct gk20a *g, u32 pbdma_id,
					u32 pbdma_method_index);
u32 gb20c_intr_0_reg_addr(struct gk20a *g, u32 pbdma_id);
u32 gb20c_intr_1_reg_addr(struct gk20a *g, u32 pbdma_id);
u32 gb20c_hce_dbg0_reg_addr(struct gk20a *g, u32 pbdma_id);
u32 gb20c_hce_dbg1_reg_addr(struct gk20a *g, u32 pbdma_id);

#endif /* NVGPU_PBDMA_GB20C_H */