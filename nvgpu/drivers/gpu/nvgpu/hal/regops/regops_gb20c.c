// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/regops.h>

#include "regops_gb20c.h"
#include "user_access_map_gb20c.h"

/* context */

/* runcontrol */
static const u32 gb20c_runcontrol_allowlist[] = {
};
static const u64 gb20c_runcontrol_allowlist_count =
	ARRAY_SIZE(gb20c_runcontrol_allowlist);

const struct regop_offset_range *gb20c_get_context_allowlist_ranges(void)
{
	return gb20c_get_global_allowlist_ranges();
}

u64 gb20c_get_context_allowlist_ranges_count(void)
{
	return gb20c_get_global_allowlist_ranges_count();
}

const u32 *gb20c_get_runcontrol_allowlist(void)
{
	return gb20c_runcontrol_allowlist;
}

u64 gb20c_get_runcontrol_allowlist_count(void)
{
	return gb20c_runcontrol_allowlist_count;
}
