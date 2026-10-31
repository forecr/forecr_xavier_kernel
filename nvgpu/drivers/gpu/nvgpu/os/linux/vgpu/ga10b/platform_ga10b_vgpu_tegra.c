// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <linux/platform_device.h>

#include <linux/dma-mapping.h>

#include <nvgpu/nvhost.h>
#include <nvgpu/gk20a.h>

#include "common/vgpu/clk_vgpu.h"
#include "os/linux/platform_gk20a.h"
#include "os/linux/os_linux.h"
#include "os/linux/vgpu/vgpu_linux.h"
#include "os/linux/vgpu/platform_vgpu_tegra.h"
#include "os/linux/vgpu/gv11b/platform_gv11b_vgpu_tegra.h"

struct gk20a_platform ga10b_vgpu_tegra_platform = {
#ifdef CONFIG_TEGRA_GK20A_NVHOST
	.has_syncpoints = true,
#endif

	/* ptimer src frequency in hz*/
	.ptimer_src_freq = 31250000,

	/* power management configuration */
	.can_railgate_init      = false,
	.can_elpg_init          = false,
	.enable_slcg            = false,
	.enable_blcg            = false,
	.enable_elcg            = false,
	.enable_elpg            = false,
	.enable_elpg_ms         = false,
	.enable_aelpg           = false,
	.can_slcg               = false,
	.can_blcg               = false,
	.can_elcg               = false,

	.ch_wdt_init_limit_ms = 5000,

	.probe = gv11b_vgpu_probe,

	.clk_round_rate = vgpu_plat_clk_round_rate,
	.get_clk_freqs = vgpu_plat_clk_get_freqs,

	.platform_chip_id = TEGRA_194_VGPU,

	/* frequency scaling configuration */
	.devfreq_governor = "userspace",

	.virtual_dev = true,

	/* power management callbacks */
	.suspend = vgpu_tegra_suspend,
	.resume = vgpu_tegra_resume,

	.unified_memory = true,
	.dma_mask = DMA_BIT_MASK(39),
};
