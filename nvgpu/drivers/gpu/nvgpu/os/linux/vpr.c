// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/vpr.h>

#include <linux/init.h>

#if NVGPU_VPR_RESIZE_SUPPORTED
#include <linux/platform/tegra/common.h>
#endif

bool nvgpu_is_vpr_resize_enabled(void)
{
#if NVGPU_VPR_RESIZE_SUPPORTED
	return tegra_is_vpr_resize_enabled();
#else
	return false;
#endif
}
