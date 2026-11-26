// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "top_gb20c.h"

#include <nvgpu/hw/gb20c/hw_top_gb20c.h>

u32 gb20c_get_expected_chip_info_cfg_version(void)
{
	return top_device_info_cfg_version_init_v();
}
