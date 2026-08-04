// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>

#include "netlist_gp10b.h"

int gp10b_netlist_get_name(struct gk20a *g, int index, char *name)
{
	int ret  = 0;

	(void)g;

	switch (index) {
#ifdef GP10B_NETLIST_IMAGE_FW_NAME
	case NETLIST_FINAL:
		(void) strcpy(name, GP10B_NETLIST_IMAGE_FW_NAME);
		break;
#endif
#ifdef NVGPU_NETLIST_IMAGE_A
	case NETLIST_SLOT_A:
		(void) strcpy(name, NVGPU_NETLIST_IMAGE_A);
		break;
#endif
#ifdef NVGPU_NETLIST_IMAGE_B
	case NETLIST_SLOT_B:
		(void) strcpy(name, NVGPU_NETLIST_IMAGE_B);
		break;
#endif
#ifdef NVGPU_NETLIST_IMAGE_C
	case NETLIST_SLOT_C:
		(void) strcpy(name, NVGPU_NETLIST_IMAGE_C);
		break;
#endif
#ifdef NVGPU_NETLIST_IMAGE_D
	case NETLIST_SLOT_D:
		(void) strcpy(name, NVGPU_NETLIST_IMAGE_D);
		break;
#endif
	default:
		ret = -1;
		break;
	}

	return ret;
}

bool gp10b_netlist_is_firmware_defined(void)
{
#ifdef GP10B_NETLIST_IMAGE_FW_NAME
	return true;
#else
	return false;
#endif
}
