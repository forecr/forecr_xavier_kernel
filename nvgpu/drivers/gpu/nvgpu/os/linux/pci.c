// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2016-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/pm_runtime.h>
#include <linux/reset.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/reboot.h>
#include <linux/notifier.h>

#include <nvgpu/nvhost.h>
#include <nvgpu/nvgpu_common.h>
#include <nvgpu/kmem.h>
#include <nvgpu/mc.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/cic_rm.h>
#include <nvgpu/enabled.h>
#include <nvgpu/errata.h>
#include <nvgpu/nvlink_probe.h>
#include <nvgpu/soc.h>
#include <nvgpu/sim.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>
#include <nvgpu/gr/gr.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/linux/sim.h>

#include "nvlink.h"
#include "module.h"
#include "sysfs.h"
#include "os_linux.h"
#include "platform_gk20a.h"

#include "pci.h"
#include "pci_power.h"

#include "driver_common.h"
#include "dmabuf_priv.h"

#include "vgpu/vf_linux.h"
#include "vgpu/vgpu_common.h"

#if defined(CONFIG_NVGPU_HAL_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
#include <nvgpu_next_chips.h>
#endif

#define BOOT_GPC2CLK_MHZ	2581U

static int nvgpu_pci_tegra_probe(struct device *dev)
{
	return 0;
}

static int nvgpu_pci_tegra_remove(struct device *dev)
{
	return 0;
}

static bool nvgpu_pci_tegra_is_railgated(struct device *pdev)
{
	return false;
}

static long nvgpu_pci_clk_round_rate(struct device *dev, unsigned long rate)
{
	long ret = (long)rate;

	if (rate == UINT_MAX)
		ret = BOOT_GPC2CLK_MHZ * 1000000UL;

	return ret;
}

static struct gk20a_platform nvgpu_pci_device[] = {
	/* SKU 0x1ebf */
	{
		/* ptimer src frequency in hz */
		.ptimer_src_freq	= 31250000,

		.probe = nvgpu_pci_tegra_probe,
		.remove = nvgpu_pci_tegra_remove,

		/* power management configuration */
		.railgate_delay_init	= 500,
		.can_railgate_init	= false,
		.can_elpg_init = false,
		.enable_elpg = false,
		.enable_elcg = false,
		.enable_slcg = false,
		.enable_blcg = false,
		.enable_mscg = false,
		.can_slcg    = false,
		.can_blcg    = false,
		.can_elcg    = false,

		.disable_aspm = true,
		.disable_nvlink = false,
		.pstate = false,

		/* power management callbacks */
		.is_railgated = nvgpu_pci_tegra_is_railgated,
		.clk_round_rate = nvgpu_pci_clk_round_rate,

		/*
		 * Fix: PCIE X1 is very slow, set to very high value till
		 * nvlink is up
		 */
		.ch_wdt_init_limit_ms = 30000,

		.honors_aperture = true,
		.dma_mask = DMA_BIT_MASK(40),
		.hardcode_sw_threshold = false,
		.unified_memory = false,
		.is_pci_igpu = false,
	},

	/* 0x1eba, 0x1efa, 0x1ebb, 0x1efb */
	/* 0x1eae, 0x1eaf (internal chip SKUs) */
	{
		/* ptimer src frequency in hz */
		.ptimer_src_freq	= 31250000,

		.probe = nvgpu_pci_tegra_probe,
		.remove = nvgpu_pci_tegra_remove,

		/* power management configuration */
		.railgate_delay_init	= 500,
		.can_railgate_init	= false,
		.can_pci_gc_off		= false,
		.can_elpg_init = false,
		.enable_elpg = false,
		.enable_elcg = false,
		.enable_slcg = true,
		.enable_blcg = true,
		.enable_mscg = false,
		.can_slcg    = true,
		.can_blcg    = true,
		.can_elcg    = false,

		.disable_aspm = true,
		.disable_nvlink = false,
		.pstate = false,

		/* power management callbacks */
		.is_railgated = nvgpu_pci_tegra_is_railgated,
		.clk_round_rate = nvgpu_pci_clk_round_rate,

		.ch_wdt_init_limit_ms = 7000,

		.honors_aperture = true,
		.dma_mask = DMA_BIT_MASK(40),
		.hardcode_sw_threshold = false,
#ifdef CONFIG_TEGRA_GK20A_NVHOST
		.has_syncpoints = true,
#endif
		.is_pci_igpu = false,
	},

	/* 0x1eb0 (RTX 5000 : TU104 based) */
	{
		/* ptimer src frequency in hz */
		.ptimer_src_freq	= 31250000,

		.probe = nvgpu_pci_tegra_probe,
		.remove = nvgpu_pci_tegra_remove,

		/* power management configuration */
		.railgate_delay_init	= 500,
		.can_railgate_init	= false,
		.can_pci_gc_off		= false,
		.can_elpg_init = false,
		.enable_elpg = false,
		.enable_elcg = false,
		.enable_slcg = true,
		.enable_blcg = true,
		.enable_mscg = false,
		.can_slcg    = true,
		.can_blcg    = true,
		.can_elcg    = false,

		.disable_aspm = true,
		.disable_nvlink = false,

		/* power management callbacks */
		.is_railgated = nvgpu_pci_tegra_is_railgated,
		.clk_round_rate = nvgpu_pci_clk_round_rate,

		.ch_wdt_init_limit_ms = 7000,

		.honors_aperture = true,
		.dma_mask = DMA_BIT_MASK(40),
		.hardcode_sw_threshold = false,
#ifdef CONFIG_TEGRA_GK20A_NVHOST
		.has_syncpoints = true,
#endif
		.is_pci_igpu = false,
	},
	/* PG209 */
	{
		/* ptimer src frequency in hz */
		.ptimer_src_freq	= 31250000,

		.probe = nvgpu_pci_tegra_probe,
		.remove = nvgpu_pci_tegra_remove,

		/* power management configuration */
		.railgate_delay_init	= 500,
		.can_railgate_init	= false,
		.can_pci_gc_off		= false,
		.can_elpg_init = false,
		.enable_elpg = false,
		.enable_elcg = false,
		.enable_slcg = true,
		.enable_blcg = true,
		.enable_mscg = false,
		.can_slcg    = true,
		.can_blcg    = true,
		.can_elcg    = false,

		.disable_aspm = true,
		.disable_nvlink = true,
		.pstate = false,

		/* power management callbacks */
		.is_railgated = nvgpu_pci_tegra_is_railgated,
		.clk_round_rate = nvgpu_pci_clk_round_rate,

		.ch_wdt_init_limit_ms = 7000,

		.honors_aperture = true,
		.dma_mask = DMA_BIT_MASK(40),
		.hardcode_sw_threshold = false,
#ifdef CONFIG_TEGRA_GK20A_NVHOST
		.has_syncpoints = true,
#endif
		.is_pci_igpu = false,
	},
};

static int nvgpu_pci_get_platform_data(struct gk20a_platform *platform, int index)
{
	struct gk20a_platform nvgpu_next_pci_device[] = {
#if defined(CONFIG_NVGPU_NEXT) && defined(CONFIG_NVGPU_NON_FUSA)
		NVGPU_NEXT_PCI_DEVICES
#endif
	};
	int device_cnt = ARRAY_SIZE(nvgpu_pci_device);
	int next_device_cnt = ARRAY_SIZE(nvgpu_next_pci_device);

	if (index < device_cnt) {
		nvgpu_memcpy((u8 *)platform,
			(u8 *)&nvgpu_pci_device[index],
			sizeof(struct gk20a_platform));
	} else if (index < device_cnt + next_device_cnt) {
		nvgpu_memcpy((u8 *)platform,
			(u8 *)&nvgpu_next_pci_device[index - device_cnt],
			sizeof(struct gk20a_platform));
	} else {
		return -EINVAL;
	}

	return 0;
}

#define PCI_DEVICE_INDEX(driver_data) ((driver_data) & 0x0000FFFFU)
#define PCI_DEVICE_FLAGS(driver_data) ((driver_data) & 0xFFFF0000U)

#define PCI_DEVICE_F_INTERNAL_CHIP_SKU	BIT(31)
#define PCI_DEVICE_F_FUSA_CHIP_SKU	BIT(30)

static struct pci_device_id nvgpu_pci_table[] = {
	{
		PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, 0x1ebf),
		.class = PCI_BASE_CLASS_DISPLAY << 16,
		.class_mask = 0xff << 16,
		.driver_data = 0,
	},
	{
		PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, 0x1eba),
		.class = PCI_BASE_CLASS_DISPLAY << 16,
		.class_mask = 0xff << 16,
		.driver_data = 1,
	},
	{
		PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, 0x1eb0),
		.class = PCI_BASE_CLASS_DISPLAY << 16,
		.class_mask = 0xff << 16,
		.driver_data = 2,
	},
	{
		PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, 0x1efa),
		.class = PCI_BASE_CLASS_DISPLAY << 16,
		.class_mask = 0xff << 16,
		.driver_data = 1,
	},
	{
		PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, 0x1ebb),
		.class = PCI_BASE_CLASS_DISPLAY << 16,
		.class_mask = 0xff << 16,
		.driver_data = 1,
	},
	{
		PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, 0x1efb),
		.class = PCI_BASE_CLASS_DISPLAY << 16,
		.class_mask = 0xff << 16,
		.driver_data = 1,
	},
	{
		PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, 0x1eae),
		.class = PCI_BASE_CLASS_DISPLAY << 16,
		.class_mask = 0xff << 16,
		.driver_data = 1 | PCI_DEVICE_F_INTERNAL_CHIP_SKU,
	},
	{
		PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, 0x1eaf),
		.class = PCI_BASE_CLASS_DISPLAY << 16,
		.class_mask = 0xff << 16,
		.driver_data = 1 | PCI_DEVICE_F_INTERNAL_CHIP_SKU,
	},
	{
		/* TU104-QS SKU*/
		PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, 0x1ebc),
		.class = PCI_BASE_CLASS_DISPLAY << 16,
		.class_mask = 0xff << 16,
		.driver_data = 1 | PCI_DEVICE_F_FUSA_CHIP_SKU,
	},
	{
		/* TU104-QS SKU*/
		PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, 0x1efc),
		.class = PCI_BASE_CLASS_DISPLAY << 16,
		.class_mask = 0xff << 16,
		.driver_data = 1 | PCI_DEVICE_F_FUSA_CHIP_SKU,
	},
	{
		/* PG209 SKU*/
		PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, 0x20b0),
		.class = PCI_BASE_CLASS_DISPLAY << 16,
		.class_mask = 0xff << 16,
		.driver_data = 3,
	},
	{
		/* PG199 SKU*/
		PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, 0x20bb),
		.class = PCI_BASE_CLASS_DISPLAY << 16,
		.class_mask = 0xff << 16,
		.driver_data = 3,
	},
#ifdef CONFIG_NVGPU_NEXT
#if defined(CONFIG_NVGPU_HAL_NON_FUSA) && defined(CONFIG_NVGPU_NON_FUSA)
	NVGPU_NEXT_PCI_IDS
#endif
#endif
	{}
};

static irqreturn_t nvgpu_pci_isr(int irq, void *dev_id)
{
	struct gk20a *g = dev_id;
	u32 ret_stall = nvgpu_cic_mon_intr_stall_isr(g);
	u32 ret_nonstall = nvgpu_cic_mon_intr_nonstall_isr(g);

#if defined(CONFIG_PCI_MSI)
	/* Send MSI EOI */
	if (g->ops.xve.rearm_msi && g->msi_enabled)
		g->ops.xve.rearm_msi(g);
#endif

	if ((ret_stall == NVGPU_CIC_INTR_HANDLE) ||
		(ret_nonstall == NVGPU_CIC_INTR_HANDLE)) {
		return IRQ_WAKE_THREAD;
	}

	return IRQ_NONE;
}

static irqreturn_t nvgpu_pci_intr_thread(int irq, void *dev_id)
{
	struct gk20a *g = dev_id;

	nvgpu_cic_mon_intr_stall_handle(g);
	nvgpu_cic_mon_intr_nonstall_handle(g);

	return IRQ_HANDLED;
}

static int nvgpu_pci_init_support(struct pci_dev *pdev, bool is_pci_igpu)
{
	struct gk20a_platform *platform = pci_get_drvdata(pdev);
	int err = 0;
	struct gk20a *g = get_gk20a(&pdev->dev);
	struct device *dev = &pdev->dev;
	void __iomem *addr;

	addr = nvgpu_devm_ioremap(dev, pci_resource_start(pdev, 0),
				     pci_resource_len(pdev, 0));
	if (IS_ERR(addr)) {
		nvgpu_err(g, "failed to remap gk20a registers");
		err = PTR_ERR(addr);
		goto fail;
	}
	g->regs = (uintptr_t)addr;
	g->regs_size = pci_resource_len(pdev, 0);

	g->regs_bus_addr = pci_resource_start(pdev, 0);
	if (!g->regs_bus_addr) {
		nvgpu_err(g, "failed to read register bus offset");
		err = -ENODEV;
		goto fail;
	}

	addr = nvgpu_devm_ioremap(dev, pci_resource_start(pdev, 1),
				     pci_resource_len(pdev, 1));
	if (IS_ERR(addr)) {
		nvgpu_err(g, "failed to remap gk20a bar1");
		err = PTR_ERR(addr);
		goto fail;
	}
	g->bar1 = (uintptr_t)addr;

	if (is_pci_igpu) {
		err = nvgpu_init_sim_support_linux_igpu_pci(g, platform);
		if (err)
			goto fail;
		err = nvgpu_init_sim_support(g);
		if (err)
			goto fail_sim;
	} else {
		err = nvgpu_init_sim_support_linux_pci(g);
		if (err)
			goto fail;
		err = nvgpu_init_sim_support_pci(g);
		if (err)
			goto fail_sim;
	}

	return 0;

 fail_sim:
	if (is_pci_igpu)
		nvgpu_remove_sim_support_linux(g);
	else
		nvgpu_remove_sim_support_linux_pci(g);
 fail:
	if (g->regs)
		g->regs = 0U;

	if (g->bar1)
		g->bar1 = 0U;

	return err;
}

#ifdef CONFIG_PM
static int nvgpu_pci_pm_runtime_resume(struct device *dev)
{
	if (gk20a_gpu_is_virtual(dev))
		return vgpu_pm_finalize_poweron(dev);
	else
		return gk20a_pm_finalize_poweron(dev);
}

static int nvgpu_pci_pm_runtime_suspend(struct device *dev)
{
	return 0;
}

static int nvgpu_pci_pm_resume(struct device *dev)
{
	return gk20a_pm_finalize_poweron(dev);
}

static int nvgpu_pci_pm_suspend(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops nvgpu_pci_pm_ops = {
	.runtime_resume = nvgpu_pci_pm_runtime_resume,
	.runtime_suspend = nvgpu_pci_pm_runtime_suspend,
	.resume = nvgpu_pci_pm_resume,
	.suspend = nvgpu_pci_pm_suspend,
};
#endif

static int nvgpu_pci_pm_init(struct device *dev)
{
#ifdef CONFIG_PM
	struct gk20a *g = get_gk20a(dev);

	if (!nvgpu_is_enabled(g, NVGPU_CAN_RAILGATE)) {
		pm_runtime_disable(dev);
	} else {
		if (g->railgate_delay)
			pm_runtime_set_autosuspend_delay(dev,
				g->railgate_delay);

		/*
		 * set gpu dev's use_autosuspend flag to allow
		 * runtime power management of GPU
		 */
		pm_runtime_use_autosuspend(dev);

		/*
		 * runtime PM for PCI devices is forbidden
		 * by default, so unblock RTPM of GPU
		 */
		pm_runtime_put_noidle(dev);
		pm_runtime_allow(dev);
	}
#endif
	return 0;
}

static int nvgpu_pci_pm_deinit(struct device *dev)
{
#ifdef CONFIG_PM
	struct gk20a *g = get_gk20a(dev);

	if (!nvgpu_is_enabled(g, NVGPU_CAN_RAILGATE))
		pm_runtime_enable(dev);
	else
		pm_runtime_forbid(dev);
#endif
	return 0;
}

static int nvgpu_get_dt_clock_limit(struct gk20a *g, u16 *gpuclk_clkmhz)
{
	struct device_node *np;
	u32 gpuclk_dt_cap = 0U;

	np = of_find_node_by_name(NULL, "nvgpu");
	if (!np) {
		return -ENOENT;
	}

	if (of_property_read_u32(np, "dgpuclk-max-mhz", &gpuclk_dt_cap)) {
		nvgpu_info(g, "dgpuclk-max-mhz not defined,"
				"P-state will be used");
	}

	*gpuclk_clkmhz = (u16)gpuclk_dt_cap;

	return 0;
}

static int nvgpu_pci_probe(struct pci_dev *pdev,
			   const struct pci_device_id *pent)
{
	struct gk20a_platform *platform = NULL;
	struct nvgpu_os_linux *l;
	struct gk20a *g;
	int err;
	struct device_node *np;
	u32 device_index = PCI_DEVICE_INDEX(pent->driver_data);
	u32 device_flags = PCI_DEVICE_FLAGS(pent->driver_data);

	/* Allocate memory to hold platform data*/
	platform = (struct gk20a_platform *)kzalloc(
			sizeof(struct gk20a_platform), GFP_KERNEL);
	if (!platform) {
		dev_err(&pdev->dev, "couldn't allocate platform data");
		return -ENOMEM;
	}

	/* copy detected device data to allocated platform space*/
	err = nvgpu_pci_get_platform_data(platform, device_index);
	if (err)
		goto err_free_platform;

	if (platform->virtual_dev) {
		err = vf_probe(pdev, platform);
		if (err)
			goto err_free_platform;
	}

	l = kzalloc(sizeof(*l), GFP_KERNEL);
	if (!l) {
		dev_err(&pdev->dev, "couldn't allocate gk20a support");
		err = -ENOMEM;
		goto err_free_platform;
	}

	g = &l->g;

	g->log_mask = NVGPU_DEFAULT_DBG_MASK;

	nvgpu_init_gk20a(g);

	nvgpu_kmem_init(g);

	g->is_pci_igpu = platform->is_pci_igpu;
	nvgpu_info(g, "is_pci_igpu: %s", g->is_pci_igpu ? "true" : "false");
	pci_set_drvdata(pdev, platform);

	err = nvgpu_init_errata_flags(g);
	if (err)
		goto err_free_l;

	err = nvgpu_init_enabled_flags(g);
	if (err) {
		goto err_free_errata;
	}

	platform->g = g;
	l->dev = &pdev->dev;

	np = nvgpu_get_node(g);
	if (of_dma_is_coherent(np)) {
		nvgpu_set_enabled(g, NVGPU_USE_COHERENT_SYSMEM, true);
		nvgpu_set_enabled(g, NVGPU_SUPPORT_IO_COHERENCE, true);
	}

	if (!g->is_pci_igpu) {
		err = pci_enable_device(pdev);
		if (err)
			goto err_free_platform;
	} else {
		if (nvgpu_platform_is_simulation(g))
			nvgpu_set_enabled(g, NVGPU_IS_FMODEL, true);
	}
	pci_set_master(pdev);

	g->pci_vendor_id = pdev->vendor;
	g->pci_device_id = pdev->device;
	g->pci_subsystem_vendor_id = pdev->subsystem_vendor;
	g->pci_subsystem_device_id = pdev->subsystem_device;
	g->pci_class = (pdev->class >> 8) & 0xFFFFU; // we only want base/sub
	g->pci_revision = pdev->revision;

	if ((device_flags & PCI_DEVICE_F_INTERNAL_CHIP_SKU) != 0U) {
		nvgpu_err(g, "internal chip SKU %08x detected",
				 g->pci_device_id);
		nvgpu_err(g, "replace board, or use at your own risks");
	}

	if ((device_flags & PCI_DEVICE_F_FUSA_CHIP_SKU) != 0U) {
		g->is_fusa_sku = true;
	}

	g->ina3221_dcb_index = platform->ina3221_dcb_index;
	g->ina3221_i2c_address = platform->ina3221_i2c_address;
	g->ina3221_i2c_port = platform->ina3221_i2c_port;
	g->hardcode_sw_threshold = platform->hardcode_sw_threshold;

#if defined(CONFIG_PCI_MSI)
	err = pci_enable_msi(pdev);
	if (err) {
		nvgpu_err(g,
			"MSI could not be enabled, falling back to legacy");
		g->msi_enabled = false;
	} else
		g->msi_enabled = true;
#endif

	err = nvgpu_cic_mon_setup(g);
	if (err != 0) {
		nvgpu_err(g, "CIC-MON setup failed");
		goto err_disable_msi;
	}

	/* Number of stall interrupt line = 1 (for dgpu <= tu10x) */
	l->interrupts.stall_size = 1U;
	l->interrupts.nonstall_size = 0U;

	l->interrupts.stall_lines[0] = pdev->irq;
	l->interrupts.nonstall_line = pdev->irq;

	if ((int)l->interrupts.stall_lines[0] < 0) {
		err = -ENXIO;
		goto err_deinit_cic_mon;
	}

	err = devm_request_threaded_irq(&pdev->dev,
			l->interrupts.stall_lines[0],
			nvgpu_pci_isr,
			nvgpu_pci_intr_thread,
#if defined(CONFIG_PCI_MSI)
			g->msi_enabled ? 0 :
#endif
			IRQF_SHARED, "nvgpu", g);
	if (err) {
		nvgpu_err(g,
			"failed to request irq @ %d", l->interrupts.stall_lines[0]);
		goto err_deinit_cic_mon;
	}
	nvgpu_disable_irqs(g);

	err = nvgpu_pci_init_support(pdev, g->is_pci_igpu);
	if (err)
		goto err_free_irq;

	if (g->is_pci_igpu) {
		err = nvgpu_read_fuse_overrides(g);
#ifdef CONFIG_RESET_CONTROLLER
		platform->reset_control = devm_reset_control_get(&pdev->dev, NULL);
		if (IS_ERR(platform->reset_control))
			platform->reset_control = NULL;
#endif
	}

	if (strchr(dev_name(&pdev->dev), '%')) {
		nvgpu_err(g, "illegal character in device name");
		err = -EINVAL;
		goto err_free_irq;
	}

	err = nvgpu_probe(g, "gpu_pci");
	if (err)
		goto err_free_irq;

	err = nvgpu_pci_pm_init(&pdev->dev);
	if (err) {
		nvgpu_err(g, "pm init failed");
		goto err_free_irq;
	}

	if (!platform->disable_nvlink) {
		err = nvgpu_nvlink_probe(g);
	} else {
		err = -ENODEV;
	}

	/*
	 * ENODEV is a legal error which means there is no NVLINK
	 * any other error is fatal
	 */
	if (err) {
		if (err != -ENODEV) {
			nvgpu_err(g, "fatal error probing nvlink, bailing out");
			goto err_free_irq;
		}
		nvgpu_set_enabled(g, NVGPU_SUPPORT_NVLINK, false);
	}

#ifdef CONFIG_TEGRA_GK20A_NVHOST
	err = nvgpu_nvhost_syncpt_init(g);
	if (err) {
		if (err != -ENOSYS) {
			nvgpu_err(g, "syncpt init failed");
			goto err_free_irq;
		}
	}
#endif

	if (g->is_pci_igpu) {
		l->nvgpu_reboot_nb.notifier_call =
			nvgpu_kernel_shutdown_notification;
		err = register_reboot_notifier(&l->nvgpu_reboot_nb);
		if (err)
			goto err_free_irq;
	} else {
		err = nvgpu_get_dt_clock_limit(g, &g->dgpu_max_clk);
		if (err != 0)
			nvgpu_info(g, "Missing nvgpu node");
	}

	err = nvgpu_pci_add_pci_power(pdev);
	if (err) {
		nvgpu_err(g, "add pci power failed (%d).", err);
		goto err_free_irq;
	}

	nvgpu_mutex_init(&l->dmabuf_priv_list_lock);
	nvgpu_init_list_node(&l->dmabuf_priv_list);

	g->probe_done = true;

	return 0;

err_free_irq:
	nvgpu_free_irq(g);
err_deinit_cic_mon:
	nvgpu_cic_mon_remove(g);
err_disable_msi:
#if defined(CONFIG_PCI_MSI)
	if (g->msi_enabled)
		pci_disable_msi(pdev);
#endif
	nvgpu_free_enabled_flags(g);
err_free_errata:
	nvgpu_free_errata_flags(g);
err_free_l:

	if (g->is_pci_igpu)
		nvgpu_kmem_fini(g, NVGPU_KMEM_FINI_FORCE_CLEANUP);
	kfree(l);
err_free_platform:
	kfree(platform);
	return err;
}

static void nvgpu_thermal_deinit(struct gk20a *g)
{
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct device *dev = dev_from_gk20a(g);

	devm_free_irq(dev, l->thermal_alert.therm_alert_irq, g);
	if (l->thermal_alert.workqueue != NULL) {
		cancel_work_sync(&l->thermal_alert.work);
		destroy_workqueue(l->thermal_alert.workqueue);
		l->thermal_alert.workqueue = NULL;
	}
}

static void nvgpu_pci_remove(struct pci_dev *pdev)
{
	struct gk20a *g = get_gk20a(&pdev->dev);
	struct device *dev = dev_from_gk20a(g);
	int err;
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);

	/* no support yet for unbind if DGPU is in VGPU mode */
	if (gk20a_gpu_is_virtual(dev))
		return;

#ifdef CONFIG_NVGPU_COMPRESSION
	gk20a_dma_buf_priv_list_clear(l);
#endif
	nvgpu_mutex_destroy(&l->dmabuf_priv_list_lock);

	if (!g->is_pci_igpu) {
		err = nvgpu_pci_clear_pci_power(dev_name(dev));
		WARN(err, "gpu failed to clear pci power");
	}

	err = nvgpu_nvlink_deinit(g);
	/* ENODEV is a legal error if there is no NVLINK */
	if (err != -ENODEV) {
		WARN(err, "gpu failed to remove nvlink");
	}

	gk20a_driver_start_unload(g);

	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_DGPU_THERMAL_ALERT) &&
			nvgpu_platform_is_silicon(g)) {
		nvgpu_thermal_deinit(g);
	}

	err = nvgpu_quiesce(g);
	/* TODO: handle failure to idle */
	WARN(err, "gpu failed to idle during driver removal");

	nvgpu_free_irq(g);

	nvgpu_remove(dev);

	unregister_reboot_notifier(&l->nvgpu_reboot_nb);

#if defined(CONFIG_PCI_MSI)
	if (g->msi_enabled)
		pci_disable_msi(pdev);
	else {
		/* IRQ does not need to be enabled in MSI as the line is not
		 * shared. nonstall_size = 0, so only stall intr are enabled.
		 */
		nvgpu_enable_irqs(g);
	}
#endif

	if (!g->is_pci_igpu)
		nvgpu_pci_pm_deinit(&pdev->dev);
	else
		gk20a_pm_deinit(dev);

	/* free allocated platform data space */
	gk20a_get_platform(&pdev->dev)->g = NULL;
	nvgpu_kfree(g, gk20a_get_platform(&pdev->dev));

	nvgpu_put(g);
}

static void nvgpu_pci_shutdown(struct pci_dev *pdev)
{
	struct gk20a *g = get_gk20a(&pdev->dev);
	struct device *dev = dev_from_gk20a(g);
	int err;

	nvgpu_info(g, "shutting down");

	/* no support yet if DGPU is in VGPU mode */
	if (gk20a_gpu_is_virtual(dev))
		return;

	if (!nvgpu_is_powered_on(g)) {
		return;
	}

	gk20a_driver_start_unload(g);

	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_DGPU_THERMAL_ALERT) &&
			nvgpu_platform_is_silicon(g)) {
		nvgpu_thermal_deinit(g);
	}

	if (is_nvgpu_gpu_state_valid(g)) {
		err = nvgpu_nvlink_deinit(g);
		/* ENODEV is a legal error if there is no NVLINK */
		if (err != -ENODEV) {
			WARN(err, "gpu failed to remove nvlink");
		}
		err = nvgpu_quiesce(g);
		WARN(err, "gpu failed to idle during shutdown");
	} else {
		nvgpu_err(g, "skipped nvlink deinit and HW quiesce");
	}

	nvgpu_info(g, "shut down complete");
}

static struct pci_driver nvgpu_pci_driver = {
	.name = "nvgpu",
	.id_table = nvgpu_pci_table,
	.probe = nvgpu_pci_probe,
	.remove = nvgpu_pci_remove,
	.shutdown = nvgpu_pci_shutdown,
#ifdef CONFIG_PM
	.driver.pm = &nvgpu_pci_pm_ops,
#endif
};

int __init nvgpu_pci_init(void)
{
	int ret;

	ret = pci_register_driver(&nvgpu_pci_driver);
	if (ret)
		return ret;

	ret = nvgpu_pci_power_init(&nvgpu_pci_driver);
	if (ret)
		goto power_init_fail;

	return 0;

power_init_fail:
	pci_unregister_driver(&nvgpu_pci_driver);
	return ret;
}

void __exit nvgpu_pci_exit(void)
{
	nvgpu_pci_power_exit(&nvgpu_pci_driver);
	pci_unregister_driver(&nvgpu_pci_driver);
	nvgpu_pci_power_cleanup();
}
