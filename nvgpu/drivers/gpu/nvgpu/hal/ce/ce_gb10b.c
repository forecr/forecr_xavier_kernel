// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/log.h>
#include <nvgpu/device.h>
#include <nvgpu/engines.h>

#include "ce_gb10b.h"

#include <nvgpu/hw/gb10b/hw_ce_gb10b.h>

u32 gb10b_grce_config_size(void)
{
	return ce_grce_config__size_1_v();
}

u32 gb10b_ce_pce_map_value(struct gk20a *g)
{
	(void)g;

	/*
	 * Hardcode the expected PCE map here. This bitmask indicates which
	 * physical copy engines are present (and not floorswept). Only PCEs
	 * with a corresponding bit set here may be assigned to an LCE.
	 */
	return (BIT32(1) | BIT32(0));
}

static void gb10b_ce_print_config(struct gk20a *g)
{
	u32 i = 0U;
	u32 grce_config, pce_map, grce_config_size;

	grce_config_size = g->ops.ce.grce_config_size();
	pce_map = nvgpu_readl(g, ce_pce_map_r());
	nvgpu_log(g, gpu_dbg_ce, "PCE_MAP = 0x%x", pce_map);
	if (pce_map != g->ops.ce.pce_map_value(g)) {
		nvgpu_warn(g, "PCE map mismatch (%d != %d)",
				pce_map, g->ops.ce.pce_map_value(g));
	}
	for (i = 0U; i < ce_pce2lce_config__size_1_v(); i++) {
		nvgpu_log(g, gpu_dbg_ce, "PCE2LCE_CONFIG(%d) = 0x%x", i,
				nvgpu_readl(g, ce_pce2lce_config_r(i)));
	}
	for (i = 0U; i < grce_config_size; i++) {
		grce_config = nvgpu_readl(g, ce_grce_config_r(i));
		nvgpu_log(g, gpu_dbg_ce, "GRCE_CONFIG(%d) = 0x%x", i,
				grce_config);
		nvgpu_log(g, gpu_dbg_ce, "Shared: %s",
				(grce_config & ce_grce_config_shared_m()) ?
				"true" : "false");
		nvgpu_log(g, gpu_dbg_ce, "Sharing with LCE: %d",
				ce_grce_config_shared_lce_f(grce_config));
	}
}

void gb10b_ce_init_pce2lce_configs(struct gk20a *g, u32 *pce2lce_configs)
{
	(void)g;

	/* PCE0 is assigned to LCE2. */
	pce2lce_configs[0] = ce_pce2lce_config_pce_assigned_lce_config_0_init_f();
	/* PCE1 is assigned to LCE3. */
	pce2lce_configs[1] = ce_pce2lce_config_pce_assigned_lce_config_1_init_f();
}

void gb10b_ce_init_grce_configs(struct gk20a *g, u32 *grce_configs)
{
	(void)g;

	/* GRCE0 shares PCE1 with LCE3. */
	grce_configs[0] = (ce_grce_config_shared_config_0_init_f() |
			ce_grce_config_shared_lce_config_0_init_f());
	/* No LCEs mapped to GRCE1. */
	grce_configs[1] = (ce_grce_config_shared_config_1_init_f() |
			ce_grce_config_shared_lce_config_1_init_f());
}

void gb10b_ce_set_pce2lce_mapping(struct gk20a *g)
{
	u32 pce_map = nvgpu_readl(g, ce_pce_map_r());
	u32 i, grce_config_size;
	struct nvgpu_device *dev;
	const struct nvgpu_device *gr_dev;
	/* Store the PCE2LCE configs for PCE[0,1] */
	u32 pce2lce_configs[ce_pce2lce_config__size_1_v()];
	/*
	 * Store GRCE configs for GRCE[0,1].
	 * Each GRCE can have a PCE assigned to it based on the PCE2LCE config
	 * or it can share PCE with another LCE.
	 * NOTE: GRCE config size is 2 for GB10B and 1 for GB20C.
	 * When this HAL is used by GB20C, we are allocating an extra entry
	 * in arrays grce_configs and pce_bitmask_per_lce but those will not be used.
	 */
	u32 grce_configs[ce_grce_config__size_1_v()];
	/* Store the PCE assigned to each LCE */
	u32 pce_bitmask_per_lce[ce_pce2lce_config__size_1_v() +
		ce_grce_config__size_1_v()] = {0};

	gr_dev = nvgpu_device_get(g, NVGPU_DEVTYPE_GRAPHICS, 0U);
	nvgpu_assert(gr_dev != NULL);

	g->ops.ce.init_pce2lce_configs(g, pce2lce_configs);

	g->ops.ce.init_grce_configs(g, grce_configs);

	grce_config_size = g->ops.ce.grce_config_size();

	/* Print default CE configuration */
	nvgpu_log(g, gpu_dbg_ce, "CE default config:");
	gb10b_ce_print_config(g);

	if (pce_map != g->ops.ce.pce_map_value(g)) {
		nvgpu_warn(g, "Overriding pce_map to 0x%x",
				g->ops.ce.pce_map_value(g));
		pce_map = g->ops.ce.pce_map_value(g);
	}

	/*
	 * Ensure that PCE2LCE and GRCE_CONFIG registers are configured
	 * correctly. This is done by reconfiguring them with the config
	 * values from HW manual.
	 */
	for (i = 0U; i < ce_pce2lce_config__size_1_v(); i++) {
		u32 reg_val = nvgpu_readl(g, ce_pce2lce_config_r(i));
		u32 assigned_lce = ce_pce2lce_config_pce_assigned_lce_v(
				pce2lce_configs[i]);

		/*
		 * Check if PCE is FSed, if so, set PCE2LCE config to NONE.
		 */
		if (!(pce_map & BIT(i))) {
			nvgpu_log(g, gpu_dbg_ce,
				"PCE(%d) is FSed, assigning to none", i);
			reg_val = set_field(reg_val,
				ce_pce2lce_config_pce_assigned_lce_m(),
				ce_pce2lce_config_pce_assigned_lce_none_f());
			nvgpu_writel(g, ce_pce2lce_config_r(i), reg_val);
		} else {
			/*
			 * For non FSed PCE, configure it using the
			 * corresponding pce2lce_configs[] entry.
			 */

			/* Peform sanity check on the LCE assignment */
			nvgpu_assert(assigned_lce <
					(ce_pce2lce_config__size_1_v() +
						grce_config_size));

			nvgpu_log(g, gpu_dbg_ce, "assigning PCE(%d) to LCE(%d)",
				i, pce2lce_configs[i]);
			reg_val = set_field(reg_val,
				ce_pce2lce_config_pce_assigned_lce_m(),
				pce2lce_configs[i]);
			nvgpu_writel(g, ce_pce2lce_config_r(i), reg_val);
			pce_bitmask_per_lce[pce2lce_configs[i]] = BIT(i);
		}
	}
	/*
	 * Configure GRCEs based on the values in grce_configs[].
	 */
	for (i = 0U; i < grce_config_size; i++) {
		u32 reg_val = nvgpu_readl(g, ce_grce_config_r(i));

		reg_val = set_field(reg_val, ce_grce_config_shared_m() |
				ce_grce_config_shared_lce_m(),
				grce_configs[i]);
		nvgpu_writel(g, ce_grce_config_r(i), reg_val);
		/*
		 * Check if GRCE is sharing PCE with ASYCE CE, if so, get
		 * the PCE bit mask of that ASYNC CE.
		 */
		if (grce_configs[i] & ce_grce_config_shared_m()) {
			pce_bitmask_per_lce[i] =
				pce_bitmask_per_lce[ce_grce_config_shared_lce_v(
					grce_configs[i])];
		}
	}
	/* Print updated CE config */
	nvgpu_log(g, gpu_dbg_ce, "CE updated config:");
	gb10b_ce_print_config(g);

	/*
	 * LCEs which do not have a PCE mapped to it cannot be used, hence
	 * remove corresponding nvgpu_device entry from fifo->host_engines and
	 * fifo->active_engines list.
	 */
	nvgpu_device_for_each(g, dev, NVGPU_DEVTYPE_LCE) {
		nvgpu_log(g, gpu_dbg_ce, "LCE(%u) pce_bitmask: 0x%x",
			dev->inst_id, pce_bitmask_per_lce[dev->inst_id]);
		if (pce_bitmask_per_lce[dev->inst_id] == 0U) {
			dev->is_invalid_lce = true;
		} else {
			if (dev->runlist_id == gr_dev->runlist_id) {
				dev->is_grce = true;
			}
			dev->pce_bitmask = pce_bitmask_per_lce[dev->inst_id];
		}
	}
}
