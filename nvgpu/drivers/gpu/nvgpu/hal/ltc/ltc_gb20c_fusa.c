// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/hw/gb20c/hw_ltc_gb20c.h>

#include "ltc_gb20c.h"

#if defined(CONFIG_NVGPU_NON_FUSA)
u32 gb20c_ltcs_ltss_tstg_cfg1_active_sets(u32 reg_val)
{
	return ltc_ltcs_ltss_tstg_cfg1_active_sets_v(reg_val);
}
#endif
