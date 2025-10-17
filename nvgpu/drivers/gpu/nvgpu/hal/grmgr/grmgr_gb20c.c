// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/hw/gb20c/hw_runlist_gb20c.h>

#include "grmgr_gb20c.h"

#ifdef CONFIG_NVGPU_MIG

u32 gb20c_runlist_config_reg_off(void)
{
	return runlist_config_r();
}

#endif
