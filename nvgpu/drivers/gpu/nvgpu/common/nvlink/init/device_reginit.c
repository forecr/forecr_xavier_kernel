// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/nvlink_device_reginit.h>

#ifdef CONFIG_NVGPU_NVLINK
int nvgpu_nvlink_reg_init(struct gk20a *g)
{
	int err;

	err = g->ops.nvlink.reg_init(g);

	return err;
}
#endif
