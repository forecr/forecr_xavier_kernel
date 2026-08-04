/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_REGOPS_GA100_H
#define NVGPU_REGOPS_GA100_H

#ifdef CONFIG_NVGPU_DEBUGGER

const struct regop_offset_range *ga100_get_global_whitelist_ranges(void);
u64 ga100_get_global_whitelist_ranges_count(void);
const struct regop_offset_range *ga100_get_context_whitelist_ranges(void);
u64 ga100_get_context_whitelist_ranges_count(void);
const u32 *ga100_get_runcontrol_whitelist(void);
u64 ga100_get_runcontrol_whitelist_count(void);
const struct regop_offset_range *ga100_get_runcontrol_whitelist_ranges(void);
u64 ga100_get_runcontrol_whitelist_ranges_count(void);

#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_REGOPS_GA100_H */
