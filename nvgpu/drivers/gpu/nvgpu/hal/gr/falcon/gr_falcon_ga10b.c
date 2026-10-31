// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>

#include "gr_falcon_ga10b.h"

#include <nvgpu/hw/ga10b/hw_gr_ga10b.h>

#ifdef CONFIG_NVGPU_GR_FALCON_NON_SECURE_BOOT
void ga10b_gr_falcon_gpccs_dmemc_write(struct gk20a *g, u32 port, u32 offs,
	u32 blk, u32 ainc)
{
	nvgpu_writel(g, gr_gpccs_dmemc_r(port),
			gr_gpccs_dmemc_offs_f(offs) |
			gr_gpccs_dmemc_blk_f(blk) |
			gr_gpccs_dmemc_aincw_f(ainc));
}

void ga10b_gr_falcon_gpccs_imemc_write(struct gk20a *g, u32 port, u32 offs,
	u32 blk, u32 ainc)
{
	nvgpu_writel(g, gr_gpccs_imemc_r(port),
			gr_gpccs_imemc_offs_f(offs) |
			gr_gpccs_imemc_blk_f(blk) |
			gr_gpccs_imemc_aincw_f(ainc));
}

void ga10b_gr_falcon_fecs_imemc_write(struct gk20a *g, u32 port, u32 offs,
	u32 blk, u32 ainc)
{
	nvgpu_writel(g, gr_fecs_imemc_r(port),
			gr_fecs_imemc_offs_f(offs) |
			gr_fecs_imemc_blk_f(blk) |
			gr_fecs_imemc_aincw_f(ainc));
}

#endif
