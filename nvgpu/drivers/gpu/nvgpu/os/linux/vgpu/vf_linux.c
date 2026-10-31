// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/nvgpu_init.h>
#include <nvgpu/tsg.h>

#include <linux/pci.h>
#include <linux/pm_runtime.h>

#include "os/linux/module.h"
#include "os/linux/os_linux.h"
#include "os/linux/platform_gk20a.h"
#include "os/linux/vgpu/vgpu_common.h"
#include "vf_linux.h"

static irqreturn_t vf_isr(int irq, void *dev_id)
{
	struct nvgpu_msi_cookie *cookie = dev_id;
	struct gk20a *g = cookie->g;
	u32 ret = nvgpu_cic_mon_intr_msi_isr(g, cookie->id);

	if (ret == NVGPU_CIC_INTR_HANDLE) {
		return IRQ_WAKE_THREAD;
	}

	return IRQ_NONE;
}

static irqreturn_t vf_intr_thread(int irq, void *dev_id)
{
	struct nvgpu_msi_cookie *cookie = dev_id;
	struct gk20a *g = cookie->g;

	nvgpu_cic_mon_intr_msi_handle(g, cookie->id);

	return IRQ_HANDLED;
}

static int vf_pci_init_support(struct pci_dev *pdev)
{
	struct device *dev = &pdev->dev;
	struct gk20a *g = get_gk20a(dev);
	void __iomem *addr;
	int err;

	addr = devm_ioremap(dev, pci_resource_start(pdev, 0),
				pci_resource_len(pdev, 0));
	if (IS_ERR(addr)) {
		nvgpu_err(g, "failed to remap gk20a registers");
		err = PTR_ERR(addr);
		return err;
	}
	g->func_regs = (uintptr_t)addr;

	g->regs_bus_addr = pci_resource_start(pdev, 0);
	if (!g->regs_bus_addr) {
		nvgpu_err(g, "failed to read register bus offset");
		err = -ENODEV;
		return err;
	}

	addr = devm_ioremap(dev, pci_resource_start(pdev, 1),
				     pci_resource_len(pdev, 1));
	if (IS_ERR(addr)) {
		nvgpu_err(g, "failed to remap gk20a bar1");
		err = PTR_ERR(addr);
		return err;
	}
	g->bar1 = (uintptr_t)addr;

	return 0;
}

static void vf_init_timeslice(struct gk20a *g)
{
	g->runlist_interleave = true;

	g->tsg_timeslice_low_priority_us =
			NVGPU_TSG_TIMESLICE_LOW_PRIORITY_US;
	g->tsg_timeslice_medium_priority_us =
			NVGPU_TSG_TIMESLICE_MEDIUM_PRIORITY_US;
	g->tsg_timeslice_high_priority_us =
			NVGPU_TSG_TIMESLICE_HIGH_PRIORITY_US;

	g->tsg_timeslice_min_us = NVGPU_TSG_TIMESLICE_MIN_US;
	g->tsg_timeslice_max_us = NVGPU_TSG_TIMESLICE_MAX_US;
	g->tsg_dbg_timeslice_max_us = NVGPU_TSG_DBG_TIMESLICE_MAX_US_DEFAULT;
}

int vf_probe(struct pci_dev *pdev, struct gk20a_platform *platform)
{
	struct nvgpu_os_linux *l;
	struct gk20a *g;
	int err;
	u32 i = 0U;

	l = devm_kzalloc(&pdev->dev, sizeof(*l), GFP_KERNEL);
	if (l == NULL) {
		dev_err(&pdev->dev, "couldn't allocate gk20a support");
		return -ENOMEM;
	}

	g = &l->g;
	platform->g = g;
	l->dev = &pdev->dev;

	g->log_mask = NVGPU_DEFAULT_DBG_MASK;
	g->is_pci_igpu = platform->is_pci_igpu;

	pci_set_drvdata(pdev, platform);

	err = pci_enable_device(pdev);
	if (err) {
		nvgpu_err(g, "enable device failed err=%d", err);
		return err;
	}
	pci_set_master(pdev);

	g->pci_vendor_id = pdev->vendor;
	g->pci_device_id = pdev->device;
	g->pci_subsystem_vendor_id = pdev->subsystem_vendor;
	g->pci_subsystem_device_id = pdev->subsystem_device;
	g->pci_class = (pdev->class >> 8) & 0xFFFFU; // we only want base/sub
	g->pci_revision = pdev->revision;

	err = pci_alloc_irq_vectors(pdev, 1, 64, PCI_IRQ_MSIX);
	if (err < 0) {
		nvgpu_err(g,
			"MSI-X alloc failed, err=%d", err);
		return err;
	}

	l->interrupts.msi_size = nvgpu_safe_cast_s32_to_u32(err);
	g->msi_enabled = true;

	for (i = 0U; i < l->interrupts.msi_size; i++) {
		l->interrupts.msi_lines[i] =
			nvgpu_safe_cast_s32_to_u32(pci_irq_vector(pdev, i));
		l->interrupts.msi_cookies[i].g = g;
		l->interrupts.msi_cookies[i].id = i;

		err = devm_request_threaded_irq(&pdev->dev,
				l->interrupts.msi_lines[i],
				vf_isr,
				vf_intr_thread,
				0, "nvgpu", &l->interrupts.msi_cookies[i]);
		if (err) {
			nvgpu_err(g,
					"failed to request irq @ %d err=%d",
					l->interrupts.msi_lines[i], err);
			return err;
		}
	}

	nvgpu_disable_irqs(g);

	err = vf_pci_init_support(pdev);
	if (err)
		return err;

	vf_init_timeslice(g);
	err = vgpu_probe_common(l);
	if (err) {
		nvgpu_err(g, "common probe failed, err=%d", err);
		return err;
	}

	nvgpu_set_enabled(g, NVGPU_IS_VIRTUAL_FUNCTION, true);
	pm_runtime_disable(&pdev->dev);

	g->probe_done = true;

	gk20a_busy(g);

	return 0;
}
