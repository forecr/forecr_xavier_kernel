// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/nvhost.h>
#include <nvgpu/dma.h>
#include <nvgpu/hw/gb10b/hw_func_gb10b.h>

#include "common/vgpu/clk_vgpu.h"
#include "os/linux/vgpu/vgpu_linux.h"
#include "os/linux/vgpu/platform_vgpu_tegra.h"
#include "os/linux/platform_gk20a_tegra.h"

#include "platform_gb10b_vf.h"

int gb10b_vf_probe(struct device *dev)
{
	struct gk20a_platform *platform = dev_get_drvdata(dev);
	struct gk20a *g = platform->g;
#ifdef CONFIG_TEGRA_GK20A_NVHOST
	int ret;
#endif

	g->usermode_regs = g->func_regs + func_cfg0_r();
	g->usermode_regs_bus_addr = g->regs_bus_addr + func_cfg0_r();

#ifdef CONFIG_NVGPU_VPR
	ret = gk20a_tegra_init_secure_alloc(platform);
	if (ret) {
		return ret;
	}
#endif

#ifdef CONFIG_TEGRA_GK20A_NVHOST
	if (platform->has_syncpoints) {
		ret = nvgpu_get_nvhost_dev(g);
		if (ret) {
			g->usermode_regs = 0U;
			return ret;
		}

		ret = nvgpu_nvhost_get_syncpt_aperture(g->nvhost,
							&g->syncpt_unit_base[0],
							&g->syncpt_unit_size, false);
		if (ret) {
			nvgpu_err(g, "Failed to get syncpt interface");
			return -ENOSYS;
		}
		if (nvgpu_iommuable(g)) {
			struct scatterlist *sg;
			struct nvgpu_mem *mem = &g->syncpt_mem[0];
			int err;

			mem->priv.sgt = nvgpu_kzalloc(g, sizeof(struct sg_table));
			if (!mem->priv.sgt) {
				err = -ENOMEM;
				return err;
			}

			err = sg_alloc_table(mem->priv.sgt, 1, GFP_KERNEL);
			if (err) {
				err = -ENOMEM;
				return err;
			}
			sg = mem->priv.sgt->sgl;
			sg_set_page(sg, phys_to_page(g->syncpt_unit_base[0]),
					g->syncpt_unit_size, 0);
			err = dma_map_sg_attrs(dev, sg, 1,
					DMA_BIDIRECTIONAL, DMA_ATTR_SKIP_CPU_SYNC);
			/* dma_map_sg_attrs returns 0 on errors */
			if (err == 0) {
				nvgpu_err(g, "iova creation for syncpoint failed");
				err = -ENOMEM;
				return err;
			}
			g->syncpt_unit_base_spa[0] = g->syncpt_unit_base[0];
			g->syncpt_unit_base[0] = sg_dma_address(sg);
		}
			g->syncpt_size =
				nvgpu_nvhost_syncpt_unit_interface_get_byte_offset(g, 1);
			nvgpu_info(g, "syncpt_unit_base %llx syncpt_unit_size %zx size %x\n",
				g->syncpt_unit_base[0], g->syncpt_unit_size, g->syncpt_size);
	}
#endif
	vgpu_init_clk_support(platform->g);

	return 0;
}

struct gk20a_platform gb10b_vf_tegra_platform = {
#ifdef CONFIG_TEGRA_GK20A_NVHOST
	.has_syncpoints = true,
#endif

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

	.platform_chip_id = TEGRA_194_VGPU,

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

	.platform_chip_id = TEGRA_264_VF,
	.soc_name = "tegra264",

	/*
	 * Size includes total size of ctxsw VPR buffers.
	 * The size can vary for different chips as attribute ctx buffer
	 * size depends on max number of tpcs supported on the chip.
	 */
	.secure_buffer_size = 0x400000, /* 4 MB */
	.is_pci_igpu = true,
};
