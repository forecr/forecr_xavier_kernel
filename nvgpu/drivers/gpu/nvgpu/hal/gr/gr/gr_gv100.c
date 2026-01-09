// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/debug.h>
#include <nvgpu/enabled.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/gr/ctx.h>
#include <nvgpu/gr/config.h>
#include <nvgpu/fbp.h>

#include "gr_gk20a.h"
#include "gr_pri_gk20a.h"
#include "gr_gv100.h"

#include "common/gr/gr_priv.h"

#include <nvgpu/hw/gv100/hw_gr_gv100.h>

#ifdef CONFIG_NVGPU_TEGRA_FUSE
void gr_gv100_set_gpc_tpc_mask(struct gk20a *g, u32 gpc_index)
{
	(void)g;
	(void)gpc_index;
}
#endif

static u32 gr_gv100_get_active_fbpa_mask(struct gk20a *g)
{
	u32 active_fbpa_mask;
	u32 num_fbpas;

	num_fbpas = g->ops.top.get_max_fbpas_count(g);

	/*
	 * Read active fbpa mask from fuse
	 * Note that 0:enable and 1:disable in value read from fuse so we've to
	 * flip the bits.
	 * Also set unused bits to zero
	 */
	active_fbpa_mask = g->ops.fuse.fuse_status_opt_fbio(g);
	active_fbpa_mask = ~active_fbpa_mask;
	active_fbpa_mask = active_fbpa_mask & (BIT32(num_fbpas) - 1U);

	return active_fbpa_mask;
}

void gr_gv100_split_fbpa_broadcast_addr(struct gk20a *g, u32 addr,
				      u32 num_fbpas,
				      u32 *priv_addr_table, u32 *t)
{
	u32 active_fbpa_mask;
	u32 fbpa_id;

	active_fbpa_mask = gr_gv100_get_active_fbpa_mask(g);

	for (fbpa_id = 0; fbpa_id < num_fbpas; fbpa_id++) {
		if ((active_fbpa_mask & BIT32(fbpa_id)) != 0U) {
			priv_addr_table[(*t)++] = pri_fbpa_addr(g,
					pri_fbpa_addr_mask(g, addr), fbpa_id);
		}
	}
}
