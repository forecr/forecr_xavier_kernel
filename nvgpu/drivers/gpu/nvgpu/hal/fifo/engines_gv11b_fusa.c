// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/hw/gv11b/hw_gmmu_gv11b.h>

#include "engines_gv11b.h"

bool gv11b_is_fault_engine_subid_gpc(struct gk20a *g, u32 engine_subid)
{
	(void)g;
	return (engine_subid == gmmu_fault_client_type_gpc_v());
}
