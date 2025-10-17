// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/gmmu.h>

#include "gmmu_gv11b.h"

/**
 * The GPU determines whether to do specific action by checking
 * the specific bit (bit number depends on soc) of the physical address.
 *
 * L3 alloc bit is used to allocate lines in L3.
 * TEGRA_RAW bit is used to read buffers in TEGRA_RAW format.
 */
u64 gv11b_gpu_phys_addr(struct gk20a *g,
			struct nvgpu_gmmu_attrs *attrs, u64 phys)
{
	if (attrs == NULL) {
		return phys;
	}

	if (attrs->l3_alloc && (g->ops.mm.gmmu.get_iommu_bit != NULL)) {
		phys |= BIT64(g->ops.mm.gmmu.get_iommu_bit(g));
	}

	if (attrs->tegra_raw &&
		(g->ops.mm.gmmu.get_gpu_phys_tegra_raw_bit != NULL)) {
		phys |= BIT64(g->ops.mm.gmmu.get_gpu_phys_tegra_raw_bit(g));
	}

	return phys;
}
