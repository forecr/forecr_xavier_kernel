/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_XVE_H
#define NVGPU_GOPS_XVE_H

#ifdef CONFIG_NVGPU_DGPU
struct gops_xve {
	int (*get_speed)(struct gk20a *g, u32 *xve_link_speed);
	int (*set_speed)(struct gk20a *g, u32 xve_link_speed);
	void (*available_speeds)(struct gk20a *g, u32 *speed_mask);
	u32 (*xve_readl)(struct gk20a *g, u32 reg);
	void (*xve_writel)(struct gk20a *g, u32 reg, u32 val);
	void (*disable_aspm)(struct gk20a *g);
	void (*reset_gpu)(struct gk20a *g);
#if defined(CONFIG_PCI_MSI)
	void (*rearm_msi)(struct gk20a *g);
#endif
	void (*enable_shadow_rom)(struct gk20a *g);
	void (*disable_shadow_rom)(struct gk20a *g);
	u32 (*get_link_control_status)(struct gk20a *g);
	void (*devinit_deferred_settings)(struct gk20a *g);
};
#endif

#endif /* NVGPU_GOPS_XVE_H */
