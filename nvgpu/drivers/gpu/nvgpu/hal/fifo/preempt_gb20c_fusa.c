// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "preempt_gb20c.h"

#include <nvgpu/hw/gb20c/hw_runlist_gb20c.h>

u32 gb20c_runlist_preempt_reg(void)
{
	return runlist_preempt_r();
}
