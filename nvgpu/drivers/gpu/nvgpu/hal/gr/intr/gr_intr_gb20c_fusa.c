// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>

#include "gr_intr_gb20c.h"

#include <nvgpu/hw/gb20c/hw_gr_gb20c.h>

u32 gb20c_gpc0_rop0_crop_hww_esr_offset(void)
{
	return gr_gpc0_rop0_crop_hww_esr_r();
}

u32 gb20c_gpc0_rop0_rrh_status_offset(void)
{
	return gr_gpc0_rop0_rrh_status_r();
}

void gb20c_gr_intr_enable_gpc_crop_hww(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");
	nvgpu_writel(g, gr_gpcs_rops_crop_hww_esr_r(),
			gr_gpcs_rops_crop_hww_esr_reset_active_f() |
			gr_gpcs_rops_crop_hww_esr_en_enable_f());
}

int gb20c_gr_intr_handle_sw_method(struct gk20a *g, u32 addr,
			u32 class_num, u32 offset, u32 data)
{
	int ret = -EINVAL;

	(void)addr;
	(void)class_num;
	(void)offset;
	(void)data;

	nvgpu_log_fn(g, " ");

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	if (class_num == gr_compute_class_v()) {
		ret = g->ops.gr.intr.handle_compute_sw_method(g, addr, class_num,
				offset, data);
	}
#endif

#if defined(CONFIG_NVGPU_DEBUGGER) && defined(CONFIG_NVGPU_GRAPHICS)
	if (class_num == gr_graphics_class_v()) {
		ret = g->ops.gr.intr.handle_gfx_sw_method(g, addr, class_num,
				offset, data);
	}
#endif

	return ret;
}