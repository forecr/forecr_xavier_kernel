// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/dt.h>
#include <linux/of.h>

#include "os_linux.h"

int nvgpu_dt_read_u32_index(struct gk20a *g, const char *name,
				u32 index, u32 *value)
{
	struct device *dev = dev_from_gk20a(g);
	struct device_node *np = dev->of_node;

	return of_property_read_u32_index(np, name, index, value);
}
