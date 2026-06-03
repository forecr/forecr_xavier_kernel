/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef PLATFORM_GB20C_VF_H
#define PLATFORM_GB20C_VF_H

#include <linux/pci.h>

#include "os/linux/platform_gk20a.h"

extern struct gk20a_platform gb20c_vf_tegra_platform;

/* VF uses subdev different from PF */
#define gb20c_vf_pci_id {					\
		PCI_DEVICE_SUB(PCI_VENDOR_ID_NVIDIA, 0x2E80,	\
				PCI_VENDOR_ID_NVIDIA, 0x1),	\
		.class = PCI_BASE_CLASS_DISPLAY << 16,		\
		.class_mask = 0xff << 16,			\
		.driver_data = 6,				\
	}

#endif
