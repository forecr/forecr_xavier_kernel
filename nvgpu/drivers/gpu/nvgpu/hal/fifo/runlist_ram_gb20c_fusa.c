// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "runlist_ram_gb20c.h"

#include <nvgpu/hw/gb20c/hw_ram_gb20c.h>

u32 gb20c_runlist_get_ch_entry_veid(u32 value)
{
	/*
	 * Starting with gb20x, VEID field should be populated
	 * in runlist entry. Follow RM and set CASID as VEID
	 */
	return ram_rl_entry_chan_casid_f(value);
}

u32 gb20c_chan_userd_ptr_hi(u32 addr_hi)
{
	return ram_rl_entry_chan_userd_ptr_hi_f(addr_hi);
}
