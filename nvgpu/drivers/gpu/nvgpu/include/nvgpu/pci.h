/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PCI_H
#define NVGPU_PCI_H

#ifdef __KERNEL__
#include <linux/pci_ids.h>
#elif defined(__NVGPU_POSIX__)
#include <nvgpu/posix/pci.h>
#elif defined(__QNX__)
#include <nvgpu/qnx/pci.h>
#else
/*
 * In case someone tries to use this without implementing support!
 */
#error "Build bug: need PCI headers!"
#endif

#endif /* NVGPU_PCI_H */
