/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_VF_LINUX_H
#define NVGPU_VF_LINUX_H

struct pci_dev;
struct gk20a_platform;

#ifdef CONFIG_NVGPU_GR_VIRTUALIZATION
int vf_probe(struct pci_dev *pdev, struct gk20a_platform *platform);
#else
static inline int vf_probe(struct pci_dev *pdev,
		struct gk20a_platform *platform)
{
	return -ENOSYS;
}
#endif

#endif