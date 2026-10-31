// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>

#include "netlist_gv11b.h"

#ifdef CONFIG_NVGPU_NON_FUSA
int gv11b_netlist_get_name(struct gk20a *g, int index, char *name)
{
	int ret = 0;

	(void)g;

	switch (index) {
#ifdef GV11B_NETLIST_IMAGE_FW_NAME
	case NETLIST_FINAL:
		(void) strcpy(name, GV11B_NETLIST_IMAGE_FW_NAME);
		break;
#endif
#ifdef CONFIG_NVGPU_NON_FUSA
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
#endif

bool gv11b_netlist_is_firmware_defined(void)
{
#ifdef GV11B_NETLIST_IMAGE_FW_NAME
	return true;
#else
	return false;
#endif
}
