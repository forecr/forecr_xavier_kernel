// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

/*
 * GB20C Tegra PCI Interface
 */

/* GB10B platform data has been used as a baseline for GB20C */

#include <linux/pci.h>
#include <linux/of_platform.h>
#include <linux/debugfs.h>
#include <linux/dma-buf.h>
#include <linux/reset.h>
#include <linux/iommu.h>
#include <linux/hashtable.h>
#include <linux/pm_runtime.h>
#include <linux/fuse.h>
#include <linux/version.h>

#include <nvgpu/gk20a.h>
#include <nvgpu/nvhost.h>
#include <nvgpu/soc.h>
#include <nvgpu/fuse.h>

#include <uapi/linux/nvgpu.h>

#include "os/linux/platform_gk20a.h"
#include "os/linux/scale.h"
#include "os/linux/module.h"

#include "os/linux/platform_gp10b.h"

#include "os/linux/os_linux.h"
#include "os/linux/platform_gk20a_tegra.h"


static int gb20c_tegra_probe(struct device *dev)
{
	struct gk20a_platform *platform = dev_get_drvdata(dev);
	bool joint_xpu_rail = false;
	struct gk20a *g = platform->g;
#ifdef CONFIG_OF
	struct device_node *of_chosen;
#endif

	platform->disable_bigpage = !iommu_get_domain_for_dev(dev) &&
		(NVGPU_CPU_PAGE_SIZE < SZ_64K);

#ifdef CONFIG_OF
	of_chosen = of_find_node_by_path("/chosen");
	if (!of_chosen)
		return -ENODEV;

	/* TODO: Update this for GB20C: JIRA NVGPU-11051 */
	joint_xpu_rail = of_property_read_bool(of_chosen,
				"nvidia,tegra-joint_xpu_rail");
#endif

	if (joint_xpu_rail) {
		nvgpu_log_info(g, "XPU rails are joint\n");
		platform->can_railgate_init = false;
		nvgpu_set_enabled(g, NVGPU_CAN_RAILGATE, false);
	}

	return 0;
}

static int gb20c_tegra_late_probe(struct device *dev)
{
	struct gk20a_platform *platform = gk20a_get_platform(dev);
	int err;

	err = gk20a_tegra_init_secure_alloc(platform);
	if (err)
		return err;

	return 0;
}

static int gb20c_tegra_remove(struct device *dev)
{
	struct gk20a_platform *platform = gk20a_get_platform(dev);

#ifdef CONFIG_TEGRA_GK20A_NVHOST
	nvgpu_free_nvhost_dev(get_gk20a(dev));
#endif

	nvgpu_mutex_destroy(&platform->clk_get_freq_lock);
	nvgpu_mutex_destroy(&platform->clks_lock);

	return 0;
}

struct gk20a_platform gb20c_pci_tegra_platform = {
#ifdef CONFIG_TEGRA_GK20A_NVHOST
	.has_syncpoints = true,
#endif

	/* ptimer src frequency in hz*/
	.ptimer_src_freq = 31250000,

	.ch_wdt_init_limit_ms = 5000,

	.probe = gb20c_tegra_probe,
	.late_probe = gb20c_tegra_late_probe,
	.remove = gb20c_tegra_remove,
	.railgate_delay_init    = 500,
	/* Disable railgate to support GPU_BRINGUP */
	/* Enabling tracked in JIRA NVGPU-9261 */
	.can_railgate_init      = false,

	/* power management features */
	/* Enable power management features: JIRA NVGPU-9261 */
	/* Disable power features to support GPU_BRINGUP */
	.can_slcg               = false,
	.enable_slcg            = false,
	.can_blcg               = false,
	.enable_blcg            = false,
	.can_elcg               = false,
	.enable_elcg            = false,
	.can_flcg               = false,
	.enable_flcg            = false,
	.enable_gpcclk_flcg     = false,
	.enable_perfmon         = false,

	.enable_elpg            = false,
	.enable_elpg_ms         = false,
	.can_elpg_init          = true,
	.enable_aelpg           = false,

	/* TODO: power management callbacks */
	.suspend = NULL,
	.railgate = NULL,
	.unrailgate = NULL,
	.is_railgated = NULL,

	.busy = gk20a_tegra_busy,
	.idle = gk20a_tegra_idle,

	.clk_round_rate = NULL,
	.get_clk_freqs = NULL,

	/* frequency scaling configuration */
	.initscale = NULL,
	.prescale = NULL,
	.postscale = NULL,
	/* Enable frequency scaling: JIRA NVGPU-9261 */
	/* Disable frequency scaling to support GPU_BRINGUP */
	.devfreq_governor = NULL,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	.qos_min_notify = NULL,
	.qos_max_notify = NULL,
#else
	.qos_notify = NULL,
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0) */

	.dump_platform_dependencies = NULL,

	.platform_chip_id = TEGRA_256,
	.soc_name = "tegra256",

	.honors_aperture = true,
	.unified_memory = true,

	/*
	 * This specifies the maximum contiguous size of a DMA mapping to Linux
	 * kernel's DMA framework.
	 * The IOMMU is capable of mapping all of physical memory and hence
	 * dma_mask is set to memory size (512GB in this case).
	 * For iGPU, nvgpu executes own dma allocs (e.g. alloc_page()) and
	 * sg_table construction. No IOMMU mapping is required and so dma_mask
	 * value is not important.
	 * However, for dGPU connected over PCIe through an IOMMU, dma_mask is
	 * significant. In this case, IOMMU bit in GPU physical address is not
	 * relevant.
	 */
	.dma_mask = DMA_BIT_MASK(41),

	.reset_assert = gp10b_tegra_reset_assert,
	.reset_deassert = gp10b_tegra_reset_deassert,

	/*
	 * Size includes total size of ctxsw VPR buffers.
	 * The size can vary for different chips as attribute ctx buffer
	 * size depends on max number of tpcs supported on the chip.
	 */
	.secure_buffer_size = 0x8e1000, /* ~8.9 MiB */
	.is_pci_igpu = true,

#ifdef CONFIG_NVGPU_SIM
	/* Fake rpc details might change with SIM NET releases */
	.fake_rpc_base = 0x03b50000,
	.fake_rpc_size = 0x00010000,
#endif
};
