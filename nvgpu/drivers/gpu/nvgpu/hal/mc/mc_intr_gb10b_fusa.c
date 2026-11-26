// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/io.h>
#include <nvgpu/utils.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/engines.h>
#include <nvgpu/device.h>
#include <nvgpu/mc.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/power_features/pg.h>
#include <nvgpu/gr/gr_instances.h>
#include <nvgpu/ce.h>
#ifdef CONFIG_NVGPU_GSP_SCHEDULER
#include <nvgpu/gsp.h>
#endif

#include "hal/mc/mc_intr_ga10b.h"
#include "mc_intr_gb10b.h"

#include <nvgpu/hw/gb10b/hw_func_gb10b.h>
#include <nvgpu/hw/gb10b/hw_ctrl_gb10b.h>

#ifdef CONFIG_NVGPU_NON_FUSA
void gb10b_intr_log_pending_intrs(struct gk20a *g)
{
	u32 intr_top, intr_leaf, i, j;

	for (i = 0U; i < func_priv_cpu_intr_top__size_1_v(); i++) {
		intr_top = nvgpu_func_readl(g,
			func_priv_cpu_intr_top_r(i));

		/* Each top reg contains intr status for leaf__size */
		for (j = 0U; j < func_priv_cpu_intr_leaf__size_1_v(); j++) {
			intr_leaf = nvgpu_func_readl(g,
					func_priv_cpu_intr_leaf_r(j));
			if (intr_leaf == 0U) {
				continue;
			}
			nvgpu_err(g,
				"Pending TOP[%d]: 0x%08x, LEAF[%d]: 0x%08x",
				i, intr_top, j, intr_leaf);
		}
	}
}
#endif
