// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#if defined(CONFIG_NVIDIA_CONFTEST)
#include <nvidia/conftest.h>
#endif

#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/iommu.h>
#include <linux/pm_runtime.h>
#include <linux/pm_domain.h>
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
#include <nvgpu/dma.h>
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
#include <nvgpu/log.h>

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

#if defined(CONFIG_NVGPU_HAL_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
#include "pci_gb10b_tegra.h"
#include "vgpu/platform_gb10b_vf.h"
#endif

#if defined(CONFIG_NVGPU_HAL_NON_FUSA) && defined(CONFIG_NVGPU_GB20C)
#include "pci_gb20c_tegra.h"
#include "vgpu/platform_gb20c_vf.h"
#endif

#if defined(CONFIG_NVGPU_HAL_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
#include <nvgpu_next_chips.h>
#endif

#define BOOT_GPC2CLK_MHZ	2581U

void nvgpu_pci_flr(struct gk20a *g)
{
	struct device *dev = dev_from_gk20a(g);
	struct pci_dev *pdev = to_pci_dev(dev);
	int rc;

	nvgpu_log(g, gpu_dbg_pm, " ");

	rc = pci_save_state(pdev);
	if (rc) {
		nvgpu_err(g, "Failed to save pci state");
		return;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
	pcie_reset_flr(pdev, PCI_RESET_DO_RESET);
#else
	nvgpu_err(g, "PCIE FLR not supported");
#endif

	pci_restore_state(pdev);

}

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
		.enable_gpcclk_flcg = false,
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
		.enable_gpcclk_flcg = false,
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
		.enable_gpcclk_flcg = false,
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
		.enable_gpcclk_flcg = false,
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
	struct gk20a_platform *nvgpu_gb10b_pci_device[] = {
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
	&gb10b_vf_tegra_platform,
	&gb10b_pci_tegra_platform,
#endif
	};
	struct gk20a_platform *nvgpu_gb20c_pci_device[] = {
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB20C)
	&gb20c_vf_tegra_platform,
	&gb20c_pci_tegra_platform,
#endif
	};
	struct gk20a_platform *nvgpu_next_pci_device[] = {
#if defined(CONFIG_NVGPU_NEXT) && defined(CONFIG_NVGPU_NON_FUSA)
		NVGPU_NEXT_PCI_DEVICES
#endif
	};
	int device_cnt = ARRAY_SIZE(nvgpu_pci_device);
	int gb10b_device_cnt = ARRAY_SIZE(nvgpu_gb10b_pci_device);
	int gb20c_device_cnt = ARRAY_SIZE(nvgpu_gb20c_pci_device);
	int next_device_cnt = ARRAY_SIZE(nvgpu_next_pci_device);

	if (index < device_cnt) {
		nvgpu_memcpy((u8 *)platform,
			(u8 *)&nvgpu_pci_device[index],
			sizeof(struct gk20a_platform));
	} else if (index < device_cnt + gb10b_device_cnt) {
		nvgpu_memcpy((u8 *)platform,
			(u8 *)nvgpu_gb10b_pci_device[index - device_cnt],
			sizeof(struct gk20a_platform));
	} else if (index < device_cnt + gb10b_device_cnt + gb20c_device_cnt) {
		nvgpu_memcpy((u8 *)platform,
			(u8 *)nvgpu_gb20c_pci_device[index - (device_cnt + gb10b_device_cnt)],
			sizeof(struct gk20a_platform));
	} else if (index < device_cnt + gb10b_device_cnt + gb20c_device_cnt + next_device_cnt) {
		nvgpu_memcpy((u8 *)platform,
			(u8 *)nvgpu_next_pci_device[index -
			(device_cnt + gb10b_device_cnt + gb20c_device_cnt)],
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
#if defined(CONFIG_NVGPU_HAL_NON_FUSA) && defined(CONFIG_NVGPU_NON_FUSA)
#if defined(CONFIG_NVGPU_GB10B)
	gb10b_vf_pci_id,
	gb10b_pci_id,
#endif
#if defined(CONFIG_NVGPU_GB20C)
	gb20c_vf_pci_id,
	gb20c_pci_id,
	gb20c_next_pci_id,
#endif
#ifdef CONFIG_NVGPU_NEXT
	NVGPU_NEXT_PCI_IDS
#endif
#endif
	{}
};

#if defined(CONFIG_PCI_MSI)
static irqreturn_t nvgpu_pci_msi_isr(int irq, void *dev_id)
{
	struct nvgpu_msi_cookie *cookie = dev_id;
	struct gk20a *g = cookie->g;
	u32 ret = nvgpu_cic_mon_intr_msi_isr(g, cookie->id);

	if (ret == NVGPU_CIC_INTR_HANDLE) {
		return IRQ_WAKE_THREAD;
	}

	return IRQ_NONE;
}

static irqreturn_t nvgpu_pci_msi_intr_thread(int irq, void *dev_id)
{
	struct nvgpu_msi_cookie *cookie = dev_id;
	struct gk20a *g = cookie->g;

	nvgpu_cic_mon_intr_msi_handle(g, cookie->id);

	return IRQ_HANDLED;
}
#endif

static irqreturn_t nvgpu_pci_isr(int irq, void *dev_id)
{
	struct gk20a *g = dev_id;
	u32 ret_stall = nvgpu_cic_mon_intr_stall_isr(g);
	u32 ret_nonstall = nvgpu_cic_mon_intr_nonstall_isr(g);

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

static int nvgpu_pci_pm_runtime_resume(struct device *dev)
{
	struct gk20a *g = get_gk20a(dev);
	int err = 0;

	nvgpu_log(g, gpu_dbg_pm, " ");

	err = gk20a_pm_runtime_resume(dev);
	if (err)
		dev_warn(dev, "fail to run gk20a_pm_runtime_resume:%d\n", err);

	return err;
}

static int nvgpu_pci_pm_runtime_suspend(struct device *dev)
{
	struct gk20a *g = get_gk20a(dev);
	int err = 0;

	nvgpu_log(g, gpu_dbg_pm, " ");

	err = gk20a_pm_runtime_suspend(dev);
	if (err)
		dev_warn(dev, "fail to run gk20a_pm_runtime_suspend:%d\n", err);

	return err;
}

static int nvgpu_pci_pm_resume(struct device *dev)
{
	struct gk20a_platform *platform = dev_get_drvdata(dev);
	struct gk20a *g = get_gk20a(dev);
	struct pci_dev *pdev = to_pci_dev(dev);
	struct pci_bus *bus = pdev->bus;
	struct pci_host_bridge *bridge;
	struct device *ctrl;
	int err = 0;

	nvgpu_log(g, gpu_dbg_pm, " ");

	err = gk20a_pm_resume(dev);
	if (err)
		dev_warn(dev, "fail to run gk20a_pm_resume:%d\n", err);

	if (platform->can_railgate_init && \
		!nvgpu_platform_is_simulation(platform->g)) {

		/* PCI iGPU: Get GPU pm_domain from controller */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
		bridge = pci_find_host_bridge(bus);
#else
		while (bus->parent)
			bus = bus->parent;

		bridge = to_pci_host_bridge(bus->bridge);
#endif
		ctrl = bridge->dev.parent;


		/*
		 * Attach GPU power domain back, this driver cannot directly use
		 * dev_pm_domain_set to recover the pm_domain because kernel warning
		 * will be triggered if the caller driver is already bounded.
		 */
		dev->pm_domain = ctrl->pm_domain;

		/* wake-up system */
		err = gk20a_busy(g);
		if (err) {
			dev_warn(dev, "fail to wakeup system:%d\n", err);
			return err;
		}
		gk20a_idle(g);

		if (platform->runtime_auto_before_suspend == true)
			pm_runtime_allow(dev);
	}

	return err;

}

static int nvgpu_pci_pm_suspend(struct device *dev)
{
	struct gk20a_platform *platform = dev_get_drvdata(dev);
	struct gk20a *g = get_gk20a(dev);
	int err = 0;

	nvgpu_log(g, gpu_dbg_pm, " ");

	if (platform->can_railgate_init && \
		!nvgpu_platform_is_simulation(platform->g)) {

		platform->runtime_auto_before_suspend = dev->power.runtime_auto;

		/* Turn on the GPU power before saving PCI configuration */
		pm_runtime_forbid(dev);

		/*
		 * If a PCI device is attached to a GenPD power domain,
		 * resume_early callback in PCI framework will not be
		 * executed during static resume. That leads to the PCI
		 * configuration couldn't be properly restored.
		 *
		 * Clear the power domain of PCI GPU before static suspend
		 * to make sure its PCI configuration could be properly
		 * restored during static resume.
		 */
		dev_pm_domain_set(dev, NULL);
	}

	err = gk20a_pm_suspend(dev);
	if (err)
		dev_warn(dev, "fail to run gk20a_pm_suspend:%d\n", err);

	return err;
}

static const struct dev_pm_ops nvgpu_pci_pm_ops = {
	.runtime_resume = nvgpu_pci_pm_runtime_resume,
	.runtime_suspend = nvgpu_pci_pm_runtime_suspend,
	.resume = nvgpu_pci_pm_resume,
	.suspend = nvgpu_pci_pm_suspend,
};

static void nvgpu_pci_pm_init(struct pci_dev *pdev)
{
	struct gk20a_platform *platform = dev_get_drvdata(&pdev->dev);
	struct pci_bus *bus = pdev->bus;
	struct device *dev = &pdev->dev;
	struct gk20a *g = get_gk20a(dev);
	struct pci_host_bridge *bridge;
	struct device *ctrl;


	if (!platform->can_railgate_init || nvgpu_platform_is_simulation(platform->g)) {
		pm_runtime_disable(dev);
		return;
	}

	nvgpu_log(g, gpu_dbg_pm, " ");

	/*
	 * Enable runtime PM for the generic platform pci host controller.
	 * The upstream pci host controller driver does not enable the runtime
	 * PM by default. Therefore, we need to enable it here explicitly
	 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
	bridge = pci_find_host_bridge(bus);
#else
	while (bus->parent)
		bus = bus->parent;

	bridge = to_pci_host_bridge(bus->bridge);
#endif
	ctrl = bridge->dev.parent;
	pm_runtime_enable(ctrl);

	/*
	 * Enable runtime autosuspend for pci device
	 * set gpu dev's use_autosuspend flag to allow
	 * runtime power management of GPU
	 */
	pm_runtime_set_autosuspend_delay(dev, platform->railgate_delay_init);
	pm_runtime_use_autosuspend(dev);
	pm_runtime_put_autosuspend(dev);

	/*
	 * Runtime PM for PCI devices is forbidden
	 * by default, so unblock RTPM of GPU
	 */
	pm_runtime_allow(dev);
}

void nvgpu_pci_pm_deinit(struct pci_dev *pdev)
{
	struct gk20a_platform *platform = dev_get_drvdata(&pdev->dev);
	struct pci_bus *bus = pdev->bus;
	struct device *dev = &pdev->dev;
	struct gk20a *g = get_gk20a(dev);
	struct pci_host_bridge *bridge;
	struct device *ctrl;

	if (!platform->can_railgate_init || nvgpu_platform_is_simulation(platform->g)) {
		/*
		 * PM deinit was already called when MIG was configured.
		 * Avoid duplicate operation here.
		 */
		if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_MIG))
			pm_runtime_enable(dev);
		return;
	}

	nvgpu_log(g, gpu_dbg_pm, " ");

	/* Disable runtime PM for pci igpu */
	pm_runtime_forbid(dev);
	pm_runtime_get_noresume(dev);
	pm_runtime_dont_use_autosuspend(dev);

	/* Disable runtime PM for platform pci host controller */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
	bridge = pci_find_host_bridge(bus);
#else
	while (bus->parent)
		bus = bus->parent;

	bridge = to_pci_host_bridge(bus->bridge);
#endif
	ctrl = bridge->dev.parent;
	pm_runtime_disable(ctrl);
}

static int nvgpu_pci_power_domain_register(struct pci_dev *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	struct gk20a_platform *platform = pci_get_drvdata(pdev);

	if (!platform->can_railgate_init || \
		nvgpu_platform_is_simulation(platform->g))
		return 0;

	nvgpu_log(platform->g, gpu_dbg_pm, " ");

	if (!node) {
		dev_warn(&pdev->dev,
			 "no dt node associated with this device\n");
		return -ENODEV;
	}

	if (!of_find_property(node, "power-domains", NULL)) {
		dev_warn(&pdev->dev,
			 "no power-domains is defined in the dt node\n");
		return -ENODEV;
	}

	nvgpu_log(platform->g, gpu_dbg_pm, "attach GPU pm_domain");
	return dev_pm_domain_attach(&pdev->dev, true);
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
	int nvec = -1;
	struct device_node *np;
	u32 device_index = PCI_DEVICE_INDEX(pent->driver_data);
	u32 device_flags = PCI_DEVICE_FLAGS(pent->driver_data);
	u32 i = 0U;


	/* Avoid PCI hotplug for GPU */
	pci_ignore_hotplug(pdev);

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
		return err;
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

	/*
	 * This is used to enable PCI_COMMAND_IO, PCI_COMMAND_MEMORY in
	 * PCIe config space.
	 */
	err = pci_enable_device(pdev);
	if (err)
		goto err_free_platform;

	if (nvgpu_platform_is_simulation(g))
		nvgpu_set_enabled(g, NVGPU_IS_FMODEL, true);

	pci_set_master(pdev);

	g->pci_vendor_id = pdev->vendor;
	g->pci_device_id = pdev->device;
	g->pci_subsystem_vendor_id = pdev->subsystem_vendor;
	g->pci_subsystem_device_id = pdev->subsystem_device;
	g->pci_class = (pdev->class >> 8) & 0xFFFFU; // we only want base/sub
	g->pci_revision = pdev->revision;

	if (g->pci_device_id == GB10B_NEXT_PCI_DEVICE_ID) {
		g->is_mig_supported = false;
	} else {
		g->is_mig_supported = true;
	}

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
	nvec = pci_alloc_irq_vectors(pdev, 1, 64, PCI_IRQ_MSI | PCI_IRQ_MSIX);
	if (nvec < 0) {
		nvgpu_err(g, "MSI could not be enabled, falling back to legacy");
#endif
#if defined(NV_PCI_IRQ_INTX)
		nvec = pci_alloc_irq_vectors(pdev, 1, 1, PCI_IRQ_INTX);
#else
		nvec = pci_alloc_irq_vectors(pdev, 1, 1, PCI_IRQ_LEGACY);
#endif
		nvgpu_assert(nvec == 1);

		l->interrupts.stall_size = 1U;
		l->interrupts.nonstall_size = 0U;

		l->interrupts.stall_lines[0U] =
			nvgpu_safe_cast_s32_to_u32(pci_irq_vector(pdev, 0U));
		l->interrupts.nonstall_line = 0U;
#if defined(CONFIG_PCI_MSI)
	} else {
		g->msi_enabled = true;

		l->interrupts.msi_size = nvgpu_safe_cast_s32_to_u32(nvec);
		for (i = 0U; i < l->interrupts.msi_size; i++) {
			l->interrupts.msi_lines[i] =
				nvgpu_safe_cast_s32_to_u32(pci_irq_vector(pdev, i));
			l->interrupts.msi_cookies[i].g = g;
			l->interrupts.msi_cookies[i].id = i;
		}
	}
#endif

	err = nvgpu_cic_mon_setup(g);
	if (err != 0) {
		nvgpu_err(g, "CIC-MON setup failed");
		goto err_disable_msi;
	}

#if defined(CONFIG_PCI_MSI)
	if (g->msi_enabled) {
		for (i = 0U; i < l->interrupts.msi_size; i++) {
			err = devm_request_threaded_irq(&pdev->dev,
					l->interrupts.msi_lines[i],
					nvgpu_pci_msi_isr,
					nvgpu_pci_msi_intr_thread,
					0, "nvgpu", &l->interrupts.msi_cookies[i]);
			if (err) {
				nvgpu_err(g, "failed to request irq @ %d",
						l->interrupts.msi_lines[i]);
				goto err_deinit_cic_mon;
			}
		}
	} else {
#endif
		err = devm_request_threaded_irq(&pdev->dev,
				l->interrupts.stall_lines[0],
				nvgpu_pci_isr,
				nvgpu_pci_intr_thread,
				IRQF_SHARED, "nvgpu", g);
		if (err) {
			nvgpu_err(g, "failed to request irq @ %d",
					l->interrupts.stall_lines[0]);
			goto err_deinit_cic_mon;
		}
#if defined(CONFIG_PCI_MSI)
	}
#endif
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

	if (!platform->disable_nvlink) {
		err = nvgpu_nvlink_probe(g);
	} else {
		err = -ENODEV;
	}

	/*
	 * Register pci device to the power domain defined in the dt node.
	 * If the device cannot be register under the gpu power domain, users
	 * should explicitly disable gpu powergate feature explicitly in BPMP
	 * dtb file. Otherwise, device probe could fail.
	 *
	 * It is expected that at this point GPU rail will be turned on if the
	 * device is registered to the GPU power domain successfully.
	 */
	err = nvgpu_pci_power_domain_register(pdev);
	if (err) {
		dev_warn(&pdev->dev,
			 "fail to register to gpu power domain:%d\n",
			 err);
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

	/*
	 * Enable/Disable PM runtime autosuspend for dynamic GPU railgate
	 * feature based on the platform configuration.
	 */
	nvgpu_pci_pm_init(pdev);

	/*
	 * Device-specific suspend/resume callbacks should do things only
	 * when probe_done becomes true
	 */
	g->probe_done = true;

	return 0;

err_free_irq:
	nvgpu_free_irq(g);
err_deinit_cic_mon:
	nvgpu_cic_mon_remove(g);
err_disable_msi:
#if defined(CONFIG_PCI_MSI)
	if (g->msi_enabled)
		pci_free_irq_vectors(pdev);
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
	struct gk20a_platform *platform = pci_get_drvdata(pdev);

	nvgpu_log(g, gpu_dbg_pm, " ");

	/* no support yet for unbind if DGPU is in VGPU mode */
	if (gk20a_gpu_is_virtual(dev))
		return;

	nvgpu_pci_pm_deinit(pdev);

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
		pci_free_irq_vectors(pdev);
	else {
		/* IRQ does not need to be enabled in MSI as the line is not
		 * shared. nonstall_size = 0, so only stall intr are enabled.
		 */
		nvgpu_enable_irqs(g);
	}
#endif

	if (platform->can_railgate_init && \
		!nvgpu_platform_is_simulation(platform->g)) {
		nvgpu_log(g, gpu_dbg_pm, "detach GPU pm_domain");
		dev_pm_domain_detach(&pdev->dev, true);
	}

	nvgpu_log(g, gpu_dbg_pm, "done");

	/* free allocated platform data space */
	gk20a_get_platform(&pdev->dev)->g = NULL;
	nvgpu_kfree_impl(g, gk20a_get_platform(&pdev->dev));

	nvgpu_put(g);
}

static void nvgpu_pci_shutdown(struct pci_dev *pdev)
{
	struct gk20a *g = get_gk20a(&pdev->dev);
	struct device *dev = dev_from_gk20a(g);
	struct gk20a_platform *platform = dev_get_drvdata(dev);
	int err;

	nvgpu_info(g, "shutting down");

	/* no support yet if DGPU is in VGPU mode */
	if (gk20a_gpu_is_virtual(dev))
		return;

	if (!nvgpu_is_powered_on(g)) {
		return;
	}

	gk20a_driver_start_unload(g);

	/*
	 * If GPU is already railgated,
	 * just prevent more requests, and return.
	 */
	if (platform->is_railgated && platform->is_railgated(&pdev->dev)) {
		__pm_runtime_disable(&pdev->dev, false);
		nvgpu_info(g, "already railgated, shut down complete");
		return;
	}

	/* Prevent more requests by disabling Runtime PM */
	__pm_runtime_disable(&pdev->dev, false);

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

		err = gk20a_pm_railgate(&pdev->dev);
		if (err)
			nvgpu_err(g, "failed to railgate, err=%d", err);
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
	.driver.pm = &nvgpu_pci_pm_ops,
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
