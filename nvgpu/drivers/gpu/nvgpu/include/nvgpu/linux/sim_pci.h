/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __SIM_PCI_LINUX_H__
#define __SIM_PCI_LINUX_H__

int nvgpu_init_sim_support_linux_pci(struct gk20a *g);
void nvgpu_remove_sim_support_linux_pci(struct gk20a *g);

#endif
