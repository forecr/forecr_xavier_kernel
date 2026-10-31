// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>

#include "ce_gb20c.h"

#include <nvgpu/hw/gb20c/hw_ce_gb20c.h>

void gb20c_ce_init_pce2lce_configs(struct gk20a *g, u32 *pce2lce_configs)
{
	(void)g;

	pce2lce_configs[0] = ce_pce2lce_config_pce_assigned_lce_config_0_init_f();
	pce2lce_configs[1] = ce_pce2lce_config_pce_assigned_lce_config_1_init_f();
}

void gb20c_ce_init_grce_configs(struct gk20a *g, u32 *grce_configs)
{
	(void)g;

	grce_configs[0] = (ce_grce_config_shared_config_0_init_f() |
			ce_grce_config_shared_lce_config_0_init_f());
}

u32 gb20c_grce_config_size(void)
{
	return ce_grce_config__size_1_v();
}
