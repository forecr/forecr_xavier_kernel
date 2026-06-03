// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "nvdec_ga100.h"

#include <nvgpu/hw/ga100/hw_pnvdec_ga100.h>

u32 ga100_nvdec_falcon_base_addr(u32 inst_id)
{
	nvgpu_assert(inst_id == 0U);
	return pnvdec_falcon_irqsset_r(0);
}
