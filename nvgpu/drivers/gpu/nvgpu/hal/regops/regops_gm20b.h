/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2013-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_REGOPS_GM20B_H
#define NVGPU_REGOPS_GM20B_H

#ifdef CONFIG_NVGPU_DEBUGGER

const struct regop_offset_range *gm20b_get_global_whitelist_ranges(void);
u64 gm20b_get_global_whitelist_ranges_count(void);
const struct regop_offset_range *gm20b_get_context_whitelist_ranges(void);
u64 gm20b_get_context_whitelist_ranges_count(void);
const u32 *gm20b_get_runcontrol_whitelist(void);
u64 gm20b_get_runcontrol_whitelist_count(void);
const struct regop_offset_range *gm20b_get_runcontrol_whitelist_ranges(void);
u64 gm20b_get_runcontrol_whitelist_ranges_count(void);

#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_REGOPS_GM20B_H */
