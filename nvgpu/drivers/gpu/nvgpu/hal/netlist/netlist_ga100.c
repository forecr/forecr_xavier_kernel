// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>

#include "netlist_ga100.h"

int ga100_netlist_get_name(struct gk20a *g, int index, char *name)
{
	u32 ver = g->params.gpu_arch + g->params.gpu_impl;
	int valid = 0;

	switch (ver) {
	case NVGPU_GPUID_GA100:
		(void) strcpy(name, "ga100/");
		(void) strcat(name, GA100_NETLIST_IMAGE_FW_NAME);
		break;
	default:
		nvgpu_err(g, "no support for GPUID %x", ver);
		valid = -1;
		break;
	}

	return valid;
}

bool ga100_netlist_is_firmware_defined(void)
{
#ifdef GA100_NETLIST_IMAGE_FW_NAME
	return true;
#else
	return false;
#endif
}
