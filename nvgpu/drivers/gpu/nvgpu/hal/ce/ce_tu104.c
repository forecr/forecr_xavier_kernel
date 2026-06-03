// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/io.h>

#include "ce_tu104.h"

#include <nvgpu/hw/tu104/hw_ce_tu104.h>

void tu104_ce_set_pce2lce_mapping(struct gk20a *g)
{
	/*
	 * By default GRCE0 and GRCE1 share PCE0.
	 * Do not change PCE0 config until GRCEs are remapped to PCE1/PCE3.
	 */

	/* PCE1 (HSHUB) is assigned to LCE4 */
	nvgpu_writel(g, ce_pce2lce_config_r(1),
		ce_pce2lce_config_pce_assigned_lce_f(4));
	/* GRCE1 shares with LCE4 */
	nvgpu_writel(g, ce_grce_config_r(1),
		ce_grce_config_shared_lce_f(4) |
		ce_grce_config_shared_f(1));

	/* PCE2 (FBHUB) is assigned to LCE2 */
	nvgpu_writel(g, ce_pce2lce_config_r(2),
		ce_pce2lce_config_pce_assigned_lce_f(2));

	/* PCE3 (FBHUB) is assigned to LCE3 */
	nvgpu_writel(g, ce_pce2lce_config_r(3),
		ce_pce2lce_config_pce_assigned_lce_f(3));
	/* GRCE0 shares with LCE3 */
	nvgpu_writel(g, ce_grce_config_r(0),
		ce_grce_config_shared_lce_f(3) |
		ce_grce_config_shared_f(1));

	/* PCE0 (HSHUB) is unconnected */
	nvgpu_writel(g, ce_pce2lce_config_r(0),
		ce_pce2lce_config_pce_assigned_lce_none_f());

}
