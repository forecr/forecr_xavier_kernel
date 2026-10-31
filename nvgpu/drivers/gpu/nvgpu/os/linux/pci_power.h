/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PCI_POWER_H
#define NVGPU_PCI_POWER_H

#include <linux/version.h>
#include <linux/pci.h>

#define NVGPU_POWER_OFF		0
#define NVGPU_POWER_ON		1

int nvgpu_pci_set_powerstate(char *dev_name, int powerstate);

int nvgpu_pci_add_pci_power(struct pci_dev *pdev);
int nvgpu_pci_clear_pci_power(const char *dev_name);

void *tegra_pcie_detach_controller(struct pci_dev *pdev);
int tegra_pcie_attach_controller(void *cookie);

int __init nvgpu_pci_power_init(struct pci_driver *nvgpu_pci_driver);
void __exit nvgpu_pci_power_exit(struct pci_driver *nvgpu_pci_driver);
void __exit nvgpu_pci_power_cleanup(void);

#endif
