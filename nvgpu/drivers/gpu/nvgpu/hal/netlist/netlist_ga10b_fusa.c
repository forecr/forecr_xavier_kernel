/*
 * Copyright (c) 2020-2022, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <nvgpu/gk20a.h>
#include <nvgpu/string.h>
#include <nvgpu/soc.h>

#include "netlist_ga10b.h"

static int ga10b_netlist_silicon_get_name(struct gk20a *g, int index, char *name)
{
	int ret = 0;

	(void)g;

	switch (index) {
#if defined(GA10B_NETLIST_DBG_IMAGE_FW_NAME) || defined(GA10B_NETLIST_PROD_IMAGE_FW_NAME)
	case NETLIST_FINAL:
		if (g->ops.pmu.is_debug_mode_enabled(g)) {
			(void) strcpy(name, GA10B_NETLIST_DBG_IMAGE_FW_NAME);
		} else {
			(void) strcpy(name, GA10B_NETLIST_PROD_IMAGE_FW_NAME);
		}
		break;
#else
#ifdef CONFIG_NVGPU_NON_FUSA
#if defined(NVGPU_NETLIST_DBG_IMAGE_A) || defined(NVGPU_NETLIST_PROD_IMAGE_A)
	case NETLIST_SLOT_A:
		if (g->ops.pmu.is_debug_mode_enabled(g)) {
			(void) strcpy(name, NVGPU_NETLIST_DBG_IMAGE_A);
		} else {
			(void) strcpy(name, NVGPU_NETLIST_PROD_IMAGE_A);
		}
		break;
#endif
#if defined(NVGPU_NETLIST_DBG_IMAGE_B) || defined(NVGPU_NETLIST_PROD_IMAGE_B)
	case NETLIST_SLOT_B:
		if (g->ops.pmu.is_debug_mode_enabled(g)) {
			(void) strcpy(name, NVGPU_NETLIST_DBG_IMAGE_B);
		} else {
			(void) strcpy(name, NVGPU_NETLIST_PROD_IMAGE_B);
		}
		break;
#endif
#if defined(NVGPU_NETLIST_DBG_IMAGE_C) || defined(NVGPU_NETLIST_PROD_IMAGE_C)
	case NETLIST_SLOT_C:
		if (g->ops.pmu.is_debug_mode_enabled(g)) {
			(void) strcpy(name, NVGPU_NETLIST_DBG_IMAGE_C);
		} else {
			(void) strcpy(name, NVGPU_NETLIST_PROD_IMAGE_C);
		}
		break;
#endif
#if defined(NVGPU_NETLIST_DBG_IMAGE_D) || defined(NVGPU_NETLIST_PROD_IMAGE_D)
	case NETLIST_SLOT_D:
		if (g->ops.pmu.is_debug_mode_enabled(g)) {
			(void) strcpy(name, NVGPU_NETLIST_DBG_IMAGE_D);
		} else {
			(void) strcpy(name, NVGPU_NETLIST_PROD_IMAGE_D);
		}
		break;
#endif
#endif
#endif
	default:
		ret = -ENOENT;
		break;

	}

	return ret;
}

static int ga10b_netlist_sim_get_name(struct gk20a *g, int index, char *name)
{
	int ret = 0;

	(void)g;

	switch (index) {
#ifdef GA10B_NETLIST_IMAGE_FW_NAME
	case NETLIST_FINAL:
		(void) strcpy(name, GA10B_NETLIST_IMAGE_FW_NAME);
		break;
#else
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
#ifdef NVGPU_NETLIST_IMAGE_D
	case NETLIST_SLOT_D:
		(void) strcpy(name, NVGPU_NETLIST_IMAGE_D);
		break;
#endif
#endif
#endif
	default:
		ret = -ENOENT;
		break;

	}

	return ret;
}

int ga10b_netlist_get_name(struct gk20a *g, int index, char *name)
{
	int ret = 0;

	if (nvgpu_platform_is_simulation(g)) {
		ret = ga10b_netlist_sim_get_name(g, index, name);
	} else {
		ret = ga10b_netlist_silicon_get_name(g, index, name);
	}

	return ret;
}

bool ga10b_netlist_is_firmware_defined(void)
{
#if defined(GA10B_NETLIST_DBG_IMAGE_FW_NAME) || defined(GA10B_NETLIST_PROD_IMAGE_FW_NAME) || defined(GA10B_NETLIST_IMAGE_FW_NAME)
	return true;
#else
	return false;
#endif
}
