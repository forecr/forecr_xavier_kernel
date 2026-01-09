// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/string.h>
#include <nvgpu/soc.h>
#include <nvgpu/acr.h>

#include "netlist_gb20c.h"

static int gb20c_netlist_silicon_get_name(struct gk20a *g, int index, char *name)
{
	int ret = 0;

	(void)g;

	switch (index) {
#if defined(GB20C_NETLIST_DBG_IMAGE_FW_NAME) || defined(GB20C_NETLIST_PROD_IMAGE_FW_NAME)
	case NETLIST_FINAL:
		if (g->ops.pmu.is_debug_mode_enabled(g)) {
			(void) strcpy(name, GB20C_NETLIST_DBG_IMAGE_FW_NAME);
		} else {
			(void) strcpy(name, GB20C_NETLIST_PROD_IMAGE_FW_NAME);
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

static int gb20c_netlist_sim_get_name(struct gk20a *g, int index, char *name)
{
	int ret = 0;

	(void)g;
	switch (index) {
#ifdef GB20C_NETLIST_IMAGE_FW_NAME
	case NETLIST_FINAL:
		(void) strcpy(name, GB20C_NETLIST_IMAGE_FW_NAME);
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

int gb20c_netlist_get_name(struct gk20a *g, int index, char *name)
{
	int ret = 0;

	if (nvgpu_platform_is_simulation(g) ||
			!nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
		ret = gb20c_netlist_sim_get_name(g, index, name);
	} else {
		ret = gb20c_netlist_silicon_get_name(g, index, name);
	}

	return ret;
}

bool gb20c_netlist_is_firmware_defined(void)
{
#if defined(GB20C_NETLIST_DBG_IMAGE_FW_NAME) || defined(GB20C_NETLIST_PROD_IMAGE_FW_NAME) || defined(GB20C_NETLIST_IMAGE_FW_NAME)
	return true;
#else
	return false;
#endif
}

void gb20c_fecs_pkc_get_name(struct gk20a *g, char *name)
{
	if (nvgpu_platform_is_simulation(g)) {
		(void) strcpy(name, GB20C_SIM_FECS_PKC_FW_NAME);
	} else if (nvgpu_acr_is_lsb_v3_supported(g)) {
		(void) strcpy(name, GB20C_FECS_RISCV_UCODE_ENCRYPT_LSB_PKC_SIG);
	} else {
		(void) strcpy(name, GB20C_FECS_RISCV_UCODE_ENCRYPT_PKC_SIG);
	}
}

void gb20c_gpccs_pkc_get_name(struct gk20a *g, char *name)
{
	if (nvgpu_platform_is_simulation(g)) {
		(void) strcpy(name, GB20C_SIM_GPCCS_PKC_FW_NAME);
	} else if (nvgpu_acr_is_lsb_v3_supported(g)) {
		(void) strcpy(name, GB20C_GPCCS_RISCV_UCODE_ENCRYPT_LSB_PKC_SIG);
	} else {
		(void) strcpy(name, GB20C_GPCCS_RISCV_UCODE_ENCRYPT_PKC_SIG);
	}
}

void gb20c_fecs_desc_get_name(struct gk20a *g, char *name)
{
	(void) g;
	(void) strcpy(name, GB20C_FECS_RISCV_UCODE_DESC);
}

void gb20c_gpccs_desc_get_name(struct gk20a *g, char *name)
{
	(void) g;
	(void) strcpy(name, GB20C_GPCCS_RISCV_UCODE_DESC);
}
