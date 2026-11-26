// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/engines.h>
#include <nvgpu/device.h>
#include <nvgpu/runlist.h>
#include <nvgpu/ce.h>
#include <nvgpu/power_features/cg.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/mc.h>
#include <nvgpu/rc.h>
#include <nvgpu/nvgpu_init.h>

int nvgpu_ce_early_init_support(struct gk20a *g)
{
	if (g->ops.ce.set_pce2lce_mapping != NULL) {
		g->ops.ce.set_pce2lce_mapping(g);
	}
	return 0;
}

int nvgpu_ce_init_support(struct gk20a *g)
{
	int err = 0;
#ifdef CONFIG_NVGPU_MIG
	struct nvgpu_runlist *runlist = NULL;
	struct nvgpu_fifo *f = &g->fifo;
#endif
	const struct nvgpu_device *dev = NULL;

	/**
	 * Remove the invalid LCE which does not have valid PCE
	 * mask.
	 */
	nvgpu_device_for_each(g, dev, NVGPU_DEVTYPE_LCE) {
		if (dev->is_invalid_lce) {
			nvgpu_engine_remove_one_dev(&g->fifo, dev);
		}
	}

	/*
	 * Bug 1895019
	 * Each time PCE2LCE config is updated and if it happens to
	 * map a LCE which was previously unmapped, then ELCG would have turned
	 * off the clock to the unmapped LCE and when the LCE config is updated,
	 * a race occurs between the config update and ELCG turning on the clock
	 * to that LCE, this might result in LCE dropping the config update.
	 * To avoid such a race, each time PCE2LCE config is updated toggle
	 * resets for all LCEs.
	 */
	if (g->ops.ce.set_engine_reset != NULL) {
		nvgpu_device_for_each(g, dev, NVGPU_DEVTYPE_LCE) {
			g->ops.ce.set_engine_reset(g, dev, true);
			g->ops.ce.set_engine_reset(g, dev, false);
		}
	} else {
		err = nvgpu_mc_reset_devtype(g, NVGPU_DEVTYPE_LCE);
		if (err != 0) {
			nvgpu_err(g, "NVGPU_DEVTYPE_LCE reset failed");
			return err;
		}
	}

	nvgpu_cg_slcg_ce2_load_enable(g);

	nvgpu_cg_blcg_ce_load_enable(g);

	nvgpu_cg_elcg_ce_load_enable(g);

	if (g->ops.ce.init_prod_values != NULL) {
		g->ops.ce.init_prod_values(g);
	}

#ifdef CONFIG_NVGPU_NONSTALL_INTR
	if (g->ops.ce.init_hw != NULL) {
		g->ops.ce.init_hw(g);
	}
#endif

	if (g->ops.ce.intr_enable != NULL) {
		g->ops.ce.intr_enable(g, true);
	}

	/** Enable interrupts at MC level */
	if (nvgpu_gin_is_present(g)) {
		nvgpu_device_for_each(g, dev, NVGPU_DEVTYPE_LCE) {
			nvgpu_gin_stall_unit_config(g, NVGPU_GIN_INTR_UNIT_CE,
					dev->inst_id, NVGPU_GIN_INTR_ENABLE);
#ifdef CONFIG_NVGPU_NONSTALL_INTR
			nvgpu_gin_nonstall_unit_config(g, NVGPU_GIN_INTR_UNIT_CE,
					dev->inst_id, NVGPU_GIN_INTR_ENABLE);
#endif
		}
	} else {
		nvgpu_cic_mon_intr_stall_unit_config(g, NVGPU_CIC_INTR_UNIT_CE, NVGPU_CIC_INTR_ENABLE);
#ifdef CONFIG_NVGPU_NONSTALL_INTR
		nvgpu_cic_mon_intr_nonstall_unit_config(g, NVGPU_CIC_INTR_UNIT_CE, NVGPU_CIC_INTR_ENABLE);
#endif
	}
#ifdef CONFIG_NVGPU_MIG
	/**
	 * Add GRCE to runlist for gfx capable MIG instance.
	 * Currently MIG0 with syspipe 0 is always gfx supported.
	 * So add grce there.
	 */
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_MIG)) {
		runlist = &f->active_runlists[0];
		if (runlist != NULL) {
			g->ops.grmgr.add_grce_syspipe_gfx(g, 0, runlist, true);
		}
	}
#endif
	return 0;
}

void nvgpu_ce_stall_isr(struct gk20a *g, u32 inst_id, u32 pri_base)
{
	bool needs_rc = false;
	bool needs_quiesce = false;

	if (g->ops.ce.isr_stall != NULL) {
		g->ops.ce.isr_stall(g, inst_id, pri_base, &needs_rc,
					&needs_quiesce);
	}

	if (needs_quiesce) {
		nvgpu_sw_quiesce(g);
	}

	if (needs_rc) {
		nvgpu_log(g, gpu_dbg_intr,
			"Recovery needed to handle CE interrupt.");
		nvgpu_rc_ce_fault(g, inst_id);
	}
}

s32 nvgpu_ce_reset(struct gk20a *g, const struct nvgpu_device *dev)
{
	s32 ret = 0;

	if (g->ops.ce.halt_engine != NULL) {
		g->ops.ce.halt_engine(g, dev);
	}

	if (g->ops.ce.set_engine_reset != NULL) {
		g->ops.ce.set_engine_reset(g, dev, true);
		g->ops.ce.set_engine_reset(g, dev, false);
	} else {
		ret = nvgpu_mc_reset_dev(g, dev);
	}

	if (g->ops.ce.request_idle != NULL) {
		/*
		 * Read CE register for CE to switch
		 * from reset to idle state.
		 */
		g->ops.ce.request_idle(g);
	}

	return ret;
}
