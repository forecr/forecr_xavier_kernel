// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/gmmu.h>

#include <nvgpu/hw/gm20b/hw_gmmu_gm20b.h>

#include "gmmu_gm20b.h"

u64 gm20b_gpu_phys_addr(struct gk20a *g,
			struct nvgpu_gmmu_attrs *attrs, u64 phys)
{
	(void)g;
	(void)attrs;
	return phys;
}
