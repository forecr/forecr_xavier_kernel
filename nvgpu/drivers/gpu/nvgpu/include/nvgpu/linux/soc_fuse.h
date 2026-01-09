/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LINUX_SOC_FUSE_H
#define NVGPU_LINUX_SOC_FUSE_H

/*
 * Note: Following define should be ideally in tegra fuse driver. It is
 * defined here since nvgpu uses the tegra_fuse_readl API directly to
 * read that fuse. See Bug 200633045.
 */

#ifndef FUSE_OPT_GPC_DISABLE_0
#define FUSE_OPT_GPC_DISABLE_0		0x188
#endif

#ifndef FUSE_OPT_EMC_DISABLE_0
#define FUSE_OPT_EMC_DISABLE_0		0x8c0
#endif

#ifndef CONFIG_NVGPU_NVMEM_FUSE

#ifndef FUSE_GCPLEX_CONFIG_FUSE_0
#define FUSE_GCPLEX_CONFIG_FUSE_0       0x1c8
#endif

#ifndef FUSE_RESERVED_CALIB0_0
#define FUSE_RESERVED_CALIB0_0          0x204
#endif

/* T186+ */
#if !defined(FUSE_PDI0) && !defined(FUSE_PDI1)
#define FUSE_PDI0			0x300
#define FUSE_PDI1			0x304
#endif

#endif /* !CONFIG_NVGPU_NVMEM_FUSE */

#endif /* NVGPU_LINUX_SOC_FUSE_H */
