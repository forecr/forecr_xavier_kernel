/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PCI_H
#define NVGPU_PCI_H

#ifdef CONFIG_NVGPU_DGPU
int nvgpu_pci_init(void);
void nvgpu_pci_exit(void);
#else
static inline int nvgpu_pci_init(void) { return 0; }
static inline void nvgpu_pci_exit(void) {}
#endif
void nvgpu_pci_flr(struct gk20a *g);
void nvgpu_pci_pm_deinit(struct pci_dev *pdev);
#endif
