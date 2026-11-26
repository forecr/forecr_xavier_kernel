/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_MC_GM20B_H
#define NVGPU_MC_GM20B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_device;

u32 gm20b_get_chip_details(struct gk20a *g, u32 *arch, u32 *impl, u32 *rev);

#ifdef CONFIG_NVGPU_NON_FUSA
u32  gm20b_mc_isr_nonstall(struct gk20a *g);
int gm20b_mc_enable_units(struct gk20a *g, u32 units, bool enable);
int gm20b_mc_enable_dev(struct gk20a *g, const struct nvgpu_device *dev,
			bool enable);
int gm20b_mc_enable_devtype(struct gk20a *g, u32 devtype, bool enable);
#endif

#ifdef CONFIG_NVGPU_LS_PMU
bool gm20b_mc_is_enabled(struct gk20a *g, u32 unit);
#endif

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gm20b_mc_intr_mask(struct gk20a *g);
void gm20b_mc_intr_enable(struct gk20a *g);
void gm20b_mc_intr_stall_unit_config(struct gk20a *g, u32 unit,
				     bool enable);
void gm20b_mc_intr_nonstall_unit_config(struct gk20a *g, u32 unit,
					bool enable);
void gm20b_mc_isr_stall(struct gk20a *g);
u32  gm20b_mc_intr_stall(struct gk20a *g);
void gm20b_mc_intr_stall_pause(struct gk20a *g);
void gm20b_mc_intr_stall_resume(struct gk20a *g);
u32  gm20b_mc_intr_nonstall(struct gk20a *g);
void gm20b_mc_intr_nonstall_pause(struct gk20a *g);
void gm20b_mc_intr_nonstall_resume(struct gk20a *g);
bool gm20b_mc_is_intr1_pending(struct gk20a *g, u32 unit, u32 mc_intr_1);
void gm20b_mc_log_pending_intrs(struct gk20a *g);
void gm20b_mc_fb_reset(struct gk20a *g);
void gm20b_mc_ltc_isr(struct gk20a *g);

bool gm20b_mc_is_mmu_fault_pending(struct gk20a *g);
#endif

#endif /* NVGPU_MC_GM20B_H */
