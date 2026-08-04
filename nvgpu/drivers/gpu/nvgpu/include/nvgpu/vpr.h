/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_VPR_H
#define NVGPU_VPR_H

#include <nvgpu/types.h>

#ifdef __KERNEL__
#include <linux/version.h>

/*
 * VPR resize is enabled only on 4.9 kernel because kernel core mm changes to
 * support it are intrusive and they can't be upstreamed easily. Upstream
 * kernel will have support for static VPR. Note that static VPR is
 * supported on all kernels.
 */
#define NVGPU_VPR_RESIZE_SUPPORTED (LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0))
#endif /* __KERNEL__ */

#ifdef CONFIG_NVGPU_VPR
bool nvgpu_is_vpr_resize_enabled(void);
#else
static inline bool nvgpu_is_vpr_resize_enabled(void)
{
	return false;
}
#endif

#endif /* NVGPU_VPR_H */
