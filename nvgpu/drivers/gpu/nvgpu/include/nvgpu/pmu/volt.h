/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_VOLT_H
#define NVGPU_PMU_VOLT_H

#include <nvgpu/types.h>
#include <nvgpu/pmu/msg.h>

struct gk20a;
struct nvgpu_pmu_volt_metadata;

#define CTRL_VOLT_DOMAIN_LOGIC			0x01U
#define CLK_PROG_VFE_ENTRY_LOGIC		0x00U
#define CTRL_VOLT_VOLT_RAIL_CLIENT_MAX_RAILS	0x04U

struct nvgpu_pmu_volt {
	struct nvgpu_pmu_volt_metadata *volt_metadata;
	void (*volt_rpc_handler)(struct gk20a *g,
			struct nv_pmu_rpc_header *rpc);
};

u8 nvgpu_pmu_volt_rail_volt_domain_convert_to_idx(struct gk20a *g, u8 volt_domain);
int nvgpu_pmu_volt_get_vmin_vmax_ps35(struct gk20a *g, u32 *vmin_uv, u32 *vmax_uv);
u8 nvgpu_pmu_volt_get_vmargin_ps35(struct gk20a *g);
int nvgpu_pmu_volt_get_curr_volt_ps35(struct gk20a *g, u32 *vcurr_uv);
int nvgpu_pmu_volt_sw_setup(struct gk20a *g);
int nvgpu_pmu_volt_pmu_setup(struct gk20a *g);
void nvgpu_pmu_volt_deinit(struct gk20a *g);
int nvgpu_pmu_volt_init(struct gk20a *g);

#endif /* NVGPU_PMU_VOLT_H */
