/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef SUPER_SURFACE_H
#define SUPER_SURFACE_H

#include <nvgpu/types.h>

struct nvgpu_mem;
struct nvgpu_pmu;
struct nvgpu_pmu_super_surface;

/* 1MB Bytes for SUPER_SURFACE_SIZE on PMU 1.0 */
#define SUPER_SURFACE_SIZE     (1024U * 1024U)
/* 11MB Bytes for SUPER_SURFACE_SIZE on PMU 2.0 */
#define SUPER_SURFACE_SIZE_20  (1024U * 1024U * 11U)

/*
 * Super surface member BIT identification used in member_mask indicating
 * which members in the super surface are valid.
 *
 * The ordering here is very important because it defines the order of
 * processing in the PMU and takes dependencies into consideration.
 */
#define NV_PMU_SUPER_SURFACE_MEMBER_FBQ_CMD                        0x0000U
#define NV_PMU_SUPER_SURFACE_MEMBER_FBQ_MSG                        0x0001U
#define NV_PMU_SUPER_SURFACE_MEMBER_CLK_DOMAIN_SET                 0x0002U
#define NV_PMU_SUPER_SURFACE_MEMBER_CLK_NAFLL_DEVICE_SET           0x0003U
#define NV_PMU_SUPER_SURFACE_MEMBER_CLK_NAFLL_DEVICE_GET_STATUS    0x0004U
#define NV_PMU_SUPER_SURFACE_MEMBER_CLK_ADC_DEVICE_SET             0x0005U
#define NV_PMU_SUPER_SURFACE_MEMBER_CLK_ADC_DEVICE_GET_STATUS      0x0006U
#define NV_PMU_SUPER_SURFACE_MEMBER_CLK_VF_POINT_SET               0x0007U
#define NV_PMU_SUPER_SURFACE_MEMBER_CLK_VF_POINT_GET_STATUS        0x0008U
#define NV_PMU_SUPER_SURFACE_MEMBER_CLK_FREQ_DOMAIN_SET            0x0009U
#define NV_PMU_SUPER_SURFACE_MEMBER_CLK_ENUM_SET                   0x000AU
#define NV_PMU_SUPER_SURFACE_MEMBER_CLK_VF_REL_SET                 0x000BU
#define NV_PMU_SUPER_SURFACE_MEMBER_CLK_PROP_REGIME_SET            0x000CU
#define NV_PMU_SUPER_SURFACE_MEMBER_CLK_PROP_TOP_SET               0x000DU
#define NV_PMU_SUPER_SURFACE_MEMBER_CLK_PROP_TOP_REL_SET           0x000EU
#define NV_PMU_SUPER_SURFACE_MEMBER_VOLT_RAIL_SET                  0x000FU
#define NV_PMU_SUPER_SURFACE_MEMBER_VOLT_RAIL_GET_STATUS           0x0010U
#define NV_PMU_SUPER_SURFACE_MEMBER_VOLT_DEVICE_SET                0x0011U
#define NV_PMU_SUPER_SURFACE_MEMBER_VOLT_POLICY_SET                0x0012U
#define NV_PMU_SUPER_SURFACE_MEMBER_VFE_VAR_SET                    0x0013U
#define NV_PMU_SUPER_SURFACE_MEMBER_VFE_VAR_GET_STATUS             0x0014U
#define NV_PMU_SUPER_SURFACE_MEMBER_VFE_EQU_SET                    0x0015U
#define NV_PMU_SUPER_SURFACE_MEMBER_VPSTATE_SET                    0x0016U
#define NV_PMU_SUPER_SURFACE_MEMBER_PERF_LIMIT_SET                 0x0017U
#define NV_PMU_SUPER_SURFACE_MEMBER_CHANGE_SEQ_SET                 0x0018U
#define NV_PMU_SUPER_SURFACE_MEMBER_CHANGE_SEQ_GET_STATUS          0x0019U
#define NV_PMU_SUPER_SURFACE_MEMBER_THERM_DEVICE_SET               0x001AU
#define NV_PMU_SUPER_SURFACE_MEMBER_THERM_DEVICE_GET_STATUS        0x001BU
#define NV_PMU_SUPER_SURFACE_MEMBER_THERM_CHANNEL_SET              0x001CU
#define NV_PMU_SUPER_SURFACE_MEMBER_THERM_CHANNEL_GET_STATUS       0x001DU
#define NV_PMU_SUPER_SURFACE_MEMBER_PWR_DEVICE_SET                 0x001EU
#define NV_PMU_SUPER_SURFACE_MEMBER_PWR_DEVICE_V2_GET_STATUS       0x001FU
#define NV_PMU_SUPER_SURFACE_MEMBER_PWR_CHANNEL_V2_GET_STATUS      0x0020U
#define NV_PMU_SUPER_SURFACE_MEMBER_PERF_CF_TOPOLOGY_SET           0x0021U
#define NV_PMU_SUPER_SURFACE_MEMBER_PERF_CF_TOPOLOGY_GET_STATUS    0x0022U
#define NV_PMU_SUPER_SURFACE_MEMBER_MAX                            0x0023U

u32 nvgpu_pmu_get_ss_member_offset(struct gk20a *g,
	struct nvgpu_pmu *pmu, u32 member_id);
u32 nvgpu_pmu_get_ss_member_size(struct gk20a *g,
	struct nvgpu_pmu *pmu, u32 member_id);

int nvgpu_pmu_ss_create_ssmd_lookup_table(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_pmu_super_surface *ss);
struct nvgpu_mem *nvgpu_pmu_super_surface_mem(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_pmu_super_surface *ss);
int nvgpu_pmu_super_surface_buf_alloc(struct gk20a *g, struct nvgpu_pmu *pmu,
	struct nvgpu_pmu_super_surface *ss);

u32 nvgpu_pmu_get_ss_cmd_fbq_offset(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_pmu_super_surface *ss, u32 id);
u32 nvgpu_pmu_get_ss_msg_fbq_offset(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_pmu_super_surface *ss);
u32 nvgpu_pmu_get_ss_msg_fbq_element_offset(struct gk20a *g,
	struct nvgpu_pmu *pmu, struct nvgpu_pmu_super_surface *ss, u32 idx);
void nvgpu_pmu_ss_fbq_flush(struct gk20a *g, struct nvgpu_pmu *pmu);

void nvgpu_pmu_super_surface_deinit(struct gk20a *g, struct nvgpu_pmu *pmu,
	struct nvgpu_pmu_super_surface *ss);

int nvgpu_pmu_super_surface_init(struct gk20a *g, struct nvgpu_pmu *pmu,
	struct nvgpu_pmu_super_surface **super_suface);
u32 nvgpu_pmu_get_fbq_ptcb_ss_offset(struct gk20a *g, struct nvgpu_pmu *pmu,
	struct nvgpu_pmu_super_surface *ss, u32 ptcb_index, u32 queue_pos);
#endif /* SUPER_SURFACE_H */