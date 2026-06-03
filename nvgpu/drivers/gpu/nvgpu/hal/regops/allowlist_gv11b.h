/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GV11B_REGOPS_ALLOWLIST_H
#define NVGPU_GV11B_REGOPS_ALLOWLIST_H

#include <nvgpu/types.h>

struct nvgpu_pm_resource_register_range;

u32 gv11b_get_hwpm_perfmon_register_stride(void);
u32 gv11b_get_hwpm_router_register_stride(void);
u32 gv11b_get_hwpm_pma_channel_register_stride(void);
u32 gv11b_get_hwpm_pma_trigger_register_stride(void);
u32 gv11b_get_smpc_register_stride(void);

const u32 *gv11b_get_hwpm_perfmon_register_offset_allowlist(u32 *count);
const u32 *gv11b_get_hwpm_router_register_offset_allowlist(u32 *count);
const u32 *gv11b_get_hwpm_pma_channel_register_offset_allowlist(u32 *count);
const u32 *gv11b_get_hwpm_pma_trigger_register_offset_allowlist(u32 *count);
const u32 *gv11b_get_smpc_register_offset_allowlist(u32 *count);

const struct nvgpu_pm_resource_register_range
	*gv11b_get_hwpm_perfmon_register_ranges(u32 *count);
const struct nvgpu_pm_resource_register_range
	*gv11b_get_hwpm_router_register_ranges(u32 *count);
const struct nvgpu_pm_resource_register_range
	*gv11b_get_hwpm_pma_channel_register_ranges(u32 *count);
const struct nvgpu_pm_resource_register_range
	*gv11b_get_hwpm_pma_trigger_register_ranges(u32 *count);
const struct nvgpu_pm_resource_register_range
	*gv11b_get_smpc_register_ranges(u32 *count);
const struct nvgpu_pm_resource_register_range
	*gv11b_get_hwpm_perfmux_register_ranges(u32 *count);
const struct nvgpu_pm_resource_register_range
	*gv11b_get_hwpm_pc_sampler_register_ranges(u32 *count);

#endif /* NVGPU_GV11B_REGOPS_ALLOWLIST_H */
