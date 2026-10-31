/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_REGOPS_GB10B_H
#define NVGPU_REGOPS_GB10B_H

#ifdef CONFIG_NVGPU_DEBUGGER

const struct regop_offset_range *gb10b_get_global_allowlist_ranges(void);
u64 gb10b_get_global_allowlist_ranges_count(void);
const struct regop_offset_range *gb10b_get_context_allowlist_ranges(void);
u64 gb10b_get_context_allowlist_ranges_count(void);
const u32 *gb10b_get_runcontrol_allowlist(void);
u64 gb10b_get_runcontrol_allowlist_count(void);
const struct regop_offset_range *gb10b_get_runcontrol_allowlist_ranges(void);
u64 gb10b_get_runcontrol_allowlist_ranges_count(void);

#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_REGOPS_GB10B_H */
