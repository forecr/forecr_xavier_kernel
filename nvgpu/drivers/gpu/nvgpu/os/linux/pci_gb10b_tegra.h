/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef GB10B_PCI_TEGRA_H
#define GB10B_PCI_TEGRA_H

extern struct gk20a_platform gb10b_pci_tegra_platform;

#define GB10B_NEXT_PCI_DEVICE_ID	(0x3400)

#define gb10b_pci_id {						\
		PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, 0x2B00),	\
		.class = PCI_BASE_CLASS_DISPLAY << 16,		\
		.class_mask = 0xff << 16,			\
		.driver_data = 5,				\
	},							\
	{							\
		PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, GB10B_NEXT_PCI_DEVICE_ID),	\
		.class = PCI_BASE_CLASS_DISPLAY << 16,				\
		.class_mask = 0xff << 16,					\
		.driver_data = 5,						\
	}

#endif
