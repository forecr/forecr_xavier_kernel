// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "nvdec_gp106.h"

#include <nvgpu/hw/gp106/hw_pnvdec_gp106.h>

u32 gp106_nvdec_falcon_base_addr(u32 inst_id)
{
	nvgpu_assert(inst_id == 0U);
	return pnvdec_falcon_irqsset_r();
}
