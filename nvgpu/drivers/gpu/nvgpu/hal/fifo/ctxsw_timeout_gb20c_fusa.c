// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>

#include "ctxsw_timeout_gb20c.h"

#include <nvgpu/hw/gb20c/hw_runlist_gb20c.h>

u32 gb20c_fifo_ctxsw_timeout_config(u32 rleng_id)
{
	return runlist_engine_ctxsw_timeout_config_r(rleng_id);
}

u32 gb20c_rleng_ctxsw_timeout_info_reg_off(u32 rleng_id)
{
	return runlist_engine_ctxsw_timeout_info_r(rleng_id);
}

u32 gb20c_ctxsw_timeout_config_size_1(void)
{
	return runlist_engine_ctxsw_timeout_config__size_1_v();
}
