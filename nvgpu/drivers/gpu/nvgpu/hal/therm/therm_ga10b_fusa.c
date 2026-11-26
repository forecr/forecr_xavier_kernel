// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/soc.h>
#include <nvgpu/io.h>
#include <nvgpu/utils.h>
#include <nvgpu/enabled.h>
#include <nvgpu/fifo.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/device.h>

#include <nvgpu/power_features/cg.h>

#include "therm_ga10b.h"

#include <nvgpu/hw/ga10b/hw_therm_ga10b.h>

u32 ga10b_therm_grad_stepping_pdiv_duration(void)
{
	/* minimum duration between steps 15usec * UTILSCLK@102 MHz */
	return 0x5FA;
}

u32 ga10b_therm_max_fpdiv_factor(void)
{
	return therm_grad_stepping_table_slowdown_factor0_fpdiv_by31_f();
}

int ga10b_elcg_init_idle_filters(struct gk20a *g)
{
	u32 gate_ctrl, idle_filter;
	u32 i;
	const struct nvgpu_device *dev;
	struct nvgpu_fifo *f = &g->fifo;

#ifdef CONFIG_NVGPU_NON_FUSA
	if (nvgpu_platform_is_simulation(g)) {
		return 0;
	}
#endif

	nvgpu_log_info(g, "init clock/power gate reg");

	for (i = 0U; i < f->num_engines; i++) {
		dev = f->active_engines[i];

		gate_ctrl = nvgpu_readl(g, therm_gate_ctrl_r(dev->engine_id));
		gate_ctrl = set_field(gate_ctrl,
			therm_gate_ctrl_eng_idle_filt_exp_m(),
			therm_gate_ctrl_eng_idle_filt_exp__prod_f());
		gate_ctrl = set_field(gate_ctrl,
			therm_gate_ctrl_eng_idle_filt_mant_m(),
			therm_gate_ctrl_eng_idle_filt_mant__prod_f());
		gate_ctrl = set_field(gate_ctrl,
			therm_gate_ctrl_eng_delay_before_m(),
			therm_gate_ctrl_eng_delay_before__prod_f());
		gate_ctrl = set_field(gate_ctrl,
				therm_gate_ctrl_eng_delay_after_m(),
				therm_gate_ctrl_eng_delay_after__prod_f());
		nvgpu_writel(g, therm_gate_ctrl_r(dev->engine_id), gate_ctrl);
	}

	idle_filter = nvgpu_readl(g, therm_fecs_idle_filter_r());
	idle_filter = set_field(idle_filter,
			therm_fecs_idle_filter_value_m(),
			therm_fecs_idle_filter_value__prod_f());
	nvgpu_writel(g, therm_fecs_idle_filter_r(), idle_filter);

	idle_filter = nvgpu_readl(g, therm_hubmmu_idle_filter_r());
	idle_filter = set_field(idle_filter,
			therm_hubmmu_idle_filter_value_m(),
			therm_hubmmu_idle_filter_value__prod_f());
	nvgpu_writel(g, therm_hubmmu_idle_filter_r(), idle_filter);

	return 0;
}
