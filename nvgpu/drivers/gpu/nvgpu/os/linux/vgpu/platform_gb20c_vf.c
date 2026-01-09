/*
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <nvgpu/nvhost.h>
#include <nvgpu/dma.h>
 #include <nvgpu/hw/gb20c/hw_func_gb20c.h>

#include "common/vgpu/clk_vgpu.h"
#include "os/linux/vgpu/vgpu_linux.h"
#include "os/linux/vgpu/platform_vgpu_tegra.h"

#include "platform_gb20c_vf.h"
#include "platform_gb10b_vf.h"

struct gk20a_platform gb20c_vf_tegra_platform = {
	.has_syncpoints = true,

	/* ptimer src frequency in hz*/
	.ptimer_src_freq = 31250000,

	.ch_wdt_init_limit_ms = 5000,

	.probe = gb10b_vf_probe,

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

	.clk_round_rate = vgpu_plat_clk_round_rate,
	.get_clk_freqs = vgpu_plat_clk_get_freqs,

	/* frequency scaling configuration */
	.devfreq_governor = "userspace",

	.virtual_dev = true,

	/* power management callbacks */
	.suspend = vgpu_tegra_suspend,
	.resume = vgpu_tegra_resume,

	.unified_memory = true,
	.honors_aperture = true,
	.dma_mask = DMA_BIT_MASK(41),

	.busy = gk20a_tegra_busy,
	.idle = gk20a_tegra_idle,

	.dump_platform_dependencies = gk20a_tegra_debug_dump,

	.platform_chip_id = TEGRA_256_VF,
	.soc_name = "tegra256",

	/*
	 * Size includes total size of ctxsw VPR buffers.
	 * The size can vary for different chips as attribute ctx buffer
	 * size depends on max number of tpcs supported on the chip.
	 */
	.secure_buffer_size = 0x8e1000, /* ~8.9 MiB */
	.is_pci_igpu = true,
};
