// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/string.h>
#include <nvgpu/soc.h>

#include "netlist_gb10b.h"

static int gb10b_netlist_silicon_get_name(struct gk20a *g, int index, char *name)
{
	int ret = 0;

	(void)g;

	switch (index) {
#if defined(GB10B_NETLIST_DBG_IMAGE_FW_NAME) || defined(GB10B_NETLIST_PROD_IMAGE_FW_NAME) || \
    defined(GB10B_PROD_TEST_NSR_NETLIST_DBG_IMAGE_FW_NAME) || defined(GB10B_PROD_TEST_NSR_NETLIST_PROD_IMAGE_FW_NAME)
	case NETLIST_FINAL:
#ifndef CONFIG_NVGPU_NON_FUSA
		if (g->ops.pmu.is_debug_mode_enabled(g)) {
			(void) strcpy(name, GB10B_NETLIST_DBG_IMAGE_FW_NAME);
		} else {
			(void) strcpy(name, GB10B_NETLIST_PROD_IMAGE_FW_NAME);
		}
#else
		if (g->ops.pmu.is_debug_mode_enabled(g)) {
			if (nvgpu_is_hypervisor_mode(g) && (g->support_gpu_tools == 0)) {
				(void) strcpy(name, GB10B_PROD_TEST_NSR_NETLIST_DBG_IMAGE_FW_NAME);
			} else {
				(void) strcpy(name, GB10B_NETLIST_DBG_IMAGE_FW_NAME);
			}
		} else {
			if (nvgpu_is_hypervisor_mode(g) && (g->support_gpu_tools == 0)) {
				(void) strcpy(name, GB10B_PROD_TEST_NSR_NETLIST_PROD_IMAGE_FW_NAME);
			} else {
				(void) strcpy(name, GB10B_NETLIST_PROD_IMAGE_FW_NAME);
			}
		}
#endif
		break;
#else
#ifdef CONFIG_NVGPU_NON_FUSA
#if defined(NVGPU_NETLIST_DBG_IMAGE_A) || defined(NVGPU_NETLIST_PROD_IMAGE_A) || \
    defined(NVGPU_PROD_TEST_NSR_NETLIST_DBG_IMAGE_A) || defined(NVGPU_PROD_TEST_NSR_NETLIST_PROD_IMAGE_A)
	case NETLIST_SLOT_A:
		if (g->ops.pmu.is_debug_mode_enabled(g)) {
			if (nvgpu_is_hypervisor_mode(g) && (g->support_gpu_tools == 0)) {
				(void) strcpy(name, NVGPU_PROD_TEST_NSR_NETLIST_DBG_IMAGE_A);
			} else {
				(void) strcpy(name, NVGPU_NETLIST_DBG_IMAGE_A);
			}
		} else {
			if (nvgpu_is_hypervisor_mode(g) && (g->support_gpu_tools == 0)) {
				(void) strcpy(name, NVGPU_PROD_TEST_NSR_NETLIST_PROD_IMAGE_A);
			} else {
				(void) strcpy(name, NVGPU_NETLIST_PROD_IMAGE_A);
			}
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

static int gb10b_netlist_sim_get_name(struct gk20a *g, int index, char *name)
{
	int ret = 0;

	(void)g;
	switch (index) {
#ifdef GB10B_NETLIST_IMAGE_FW_NAME
	case NETLIST_FINAL:
		(void) strcpy(name, GB10B_NETLIST_IMAGE_FW_NAME);
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

int gb10b_netlist_get_name(struct gk20a *g, int index, char *name)
{
	int ret = 0;

	if (nvgpu_platform_is_simulation(g)) {
		ret = gb10b_netlist_sim_get_name(g, index, name);
	} else {
		ret = gb10b_netlist_silicon_get_name(g, index, name);
	}

	return ret;
}

bool gb10b_netlist_is_firmware_defined(void)
{
#if defined(GB10B_NETLIST_DBG_IMAGE_FW_NAME) || defined(GB10B_NETLIST_PROD_IMAGE_FW_NAME) || defined(GB10B_NETLIST_IMAGE_FW_NAME)
	return true;
#else
	return false;
#endif
}

void gb10b_fecs_pkc_get_name(struct gk20a *g, char *name)
{
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	if (nvgpu_is_hypervisor_mode(g) && (g->support_gpu_tools == 0)) {
		(void) strcpy(name, GB10B_PROD_TEST_NSR_FECS_RISCV_UCODE_ENCRYPT_PKC_SIG);
	} else {
		(void) strcpy(name, GB10B_FECS_RISCV_UCODE_ENCRYPT_PKC_SIG);
	}
#else
	(void) strcpy(name, GB10B_FECS_RISCV_UCODE_ENCRYPT_PKC_SIG);
#endif
}

void gb10b_gpccs_pkc_get_name(struct gk20a *g, char *name)
{
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	if (nvgpu_is_hypervisor_mode(g) && (g->support_gpu_tools == 0)) {
		(void) strcpy(name, GB10B_PROD_TEST_NSR_GPCCS_RISCV_UCODE_ENCRYPT_PKC_SIG);
	} else {
		(void) strcpy(name, GB10B_GPCCS_RISCV_UCODE_ENCRYPT_PKC_SIG);
	}
#else
	(void) strcpy(name, GB10B_GPCCS_RISCV_UCODE_ENCRYPT_PKC_SIG);
#endif
}

void gb10b_fecs_desc_get_name(struct gk20a *g, char *name)
{
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	if (nvgpu_is_hypervisor_mode(g) && (g->support_gpu_tools == 0)) {
		(void) strcpy(name, GB10B_PROD_TEST_NSR_FECS_RISCV_UCODE_DESC);
	} else {
		(void) strcpy(name, GB10B_FECS_RISCV_UCODE_DESC);
	}
#else
	(void) strcpy(name, GB10B_FECS_RISCV_UCODE_DESC);
#endif
}

void gb10b_gpccs_desc_get_name(struct gk20a *g, char *name)
{
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	if (nvgpu_is_hypervisor_mode(g) && (g->support_gpu_tools == 0)) {
		(void) strcpy(name, GB10B_PROD_TEST_NSR_GPCCS_RISCV_UCODE_DESC);
	} else {
		(void) strcpy(name, GB10B_GPCCS_RISCV_UCODE_DESC);
	}
#else
	(void) strcpy(name, GB10B_GPCCS_RISCV_UCODE_DESC);
#endif
}
