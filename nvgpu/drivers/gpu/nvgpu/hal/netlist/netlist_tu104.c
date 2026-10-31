// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>

#include "netlist_tu104.h"

int tu104_netlist_get_name(struct gk20a *g, int index, char *name)
{
	u32 ver = g->params.gpu_arch + g->params.gpu_impl;
	int valid = 0;

	switch (ver) {
	case NVGPU_GPUID_TU104:
		(void) strcpy(name, "tu104/");
		(void) strcat(name, TU104_NETLIST_IMAGE_FW_NAME);
		break;
	default:
		nvgpu_err(g, "no support for GPUID %x", ver);
		valid = -1;
		break;
	}

	return valid;
}

bool tu104_netlist_is_firmware_defined(void)
{
#ifdef TU104_NETLIST_IMAGE_FW_NAME
	return true;
#else
	return false;
#endif
}
