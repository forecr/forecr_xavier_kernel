// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "engine_status_gb20c.h"
#include <nvgpu/hw/gb20c/hw_runlist_gb20c.h>

u32 gb20c_rleng_status0_reg_off(u32 rleng_id)
{
	return runlist_engine_status0_r(rleng_id);
}

u32 gb20c_rleng_status1_reg_off(u32 rleng_id)
{
	return runlist_engine_status1_r(rleng_id);
}
