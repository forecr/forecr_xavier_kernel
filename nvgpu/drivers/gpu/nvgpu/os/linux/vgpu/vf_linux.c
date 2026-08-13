// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/nvgpu_init.h>

#include <linux/of_address.h>
#include <linux/pci.h>

#include "os/linux/module.h"
#include "os/linux/os_linux.h"
#include "os/linux/platform_gk20a.h"
#include "os/linux/vgpu/vgpu_common.h"
#include "vf_linux.h"

static irqreturn_t vf_isr(int irq, void *dev_id)
{
	struct gk20a *g = dev_id;
	u32 ret_nonstall = nvgpu_cic_mon_intr_nonstall_isr(g);

	if (ret_nonstall == NVGPU_CIC_INTR_HANDLE) {
		return IRQ_WAKE_THREAD;
	}

	return IRQ_NONE;
}

static irqreturn_t vf_intr_thread(int irq, void *dev_id)
{
	struct gk20a *g = dev_id;

	nvgpu_cic_mon_intr_nonstall_handle(g);
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

int vf_probe(struct pci_dev *pdev, struct gk20a_platform *platform)
{
	struct nvgpu_os_linux *l;
	struct gk20a *g;
	struct device_node *np;
	int err;

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

	np = nvgpu_get_node(g);
	if (of_dma_is_coherent(np)) {
		nvgpu_set_enabled(g, NVGPU_USE_COHERENT_SYSMEM, true);
		nvgpu_set_enabled(g, NVGPU_SUPPORT_IO_COHERENCE, true);
	}

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
	/* TODO: separate stall/non-stall interrupts once support multi-vector */
	l->interrupts.nonstall_size = err;
	g->msi_enabled = true;

	/* TODO: support multiple nonstall irqs */
	l->interrupts.nonstall_line = pci_irq_vector(pdev, 0);

	err = devm_request_threaded_irq(&pdev->dev,
			l->interrupts.nonstall_line,
			vf_isr,
			vf_intr_thread,
			0, "nvgpu", g);
	if (err) {
		nvgpu_err(g,
			"failed to request irq @ %d err=%d",
			l->interrupts.nonstall_line, err);
		return err;
	}
	nvgpu_disable_irqs(g);

	err = vf_pci_init_support(pdev);
	if (err)
		return err;

	err = vgpu_probe_common(l);
	if (err) {
		nvgpu_err(g, "common probe failed, err=%d", err);
		return err;
	}

	g->probe_done = true;

	return 0;
}
