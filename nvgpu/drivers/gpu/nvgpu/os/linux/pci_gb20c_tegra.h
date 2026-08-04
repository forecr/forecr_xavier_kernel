/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef GB20C_PCI_TEGRA_H
#define GB20C_PCI_TEGRA_H

extern struct gk20a_platform gb20c_pci_tegra_platform;

#define gb20c_pci_id {						\
		PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, 0x2E80),	\
		.class = PCI_BASE_CLASS_DISPLAY << 16,		\
		.class_mask = 0xff << 16,			\
		.driver_data = 7,				\
	}

#define gb20c_next_pci_id {					\
		PCI_DEVICE(PCI_VENDOR_ID_NVIDIA, 0x3100),	\
		.class = PCI_BASE_CLASS_DISPLAY << 16,		\
		.class_mask = 0xff << 16,			\
		.driver_data = 7,				\
	}

#endif
