// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/gk20a.h>
#include <nvgpu/netlist.h>

#include "netlist_gv100.h"

int gv100_netlist_get_name(struct gk20a *g, int index, char *name)
{
	u32 ver = g->params.gpu_arch + g->params.gpu_impl;
	int valid = 0;

	switch (ver) {
	case NVGPU_GPUID_GV100:
		(void) snprintf(name, MAX_NETLIST_NAME, "%s", "gv100/");
		(void) snprintf(name + strlen(name),
					nvgpu_safe_sub_u64(MAX_NETLIST_NAME, strlen(name)),
					"%s", GV100_NETLIST_IMAGE_FW_NAME);
		break;
	default:
		nvgpu_err(g, "no support for GPUID %x", ver);
		valid = -1;
		break;
	}

	return valid;
}

bool gv100_netlist_is_firmware_defined(void)
{
	return true;
}
