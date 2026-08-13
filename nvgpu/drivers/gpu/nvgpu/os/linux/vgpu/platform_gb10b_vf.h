/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef PLATFORM_GB10B_VF_H
#define PLATFORM_GB10B_VF_H

#include <linux/pci.h>

#include "os/linux/platform_gk20a.h"

extern struct gk20a_platform gb10b_vf_tegra_platform;

int gb10b_vf_probe(struct device *dev);

/* VF uses subdev different from PF */
#define gb10b_vf_pci_id {					\
		PCI_DEVICE_SUB(PCI_VENDOR_ID_NVIDIA, 0x2B00,	\
				PCI_VENDOR_ID_NVIDIA, 0x1),	\
		.class = PCI_BASE_CLASS_DISPLAY << 16,		\
		.class_mask = 0xff << 16,			\
		.driver_data = 4,				\
	}

#endif
