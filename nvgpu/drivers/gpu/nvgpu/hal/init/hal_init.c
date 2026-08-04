// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/hal_init.h>
#include <nvgpu/mc.h>
#include <nvgpu/soc.h>
#include <nvgpu/static_analysis.h>

#include "hal_gv11b.h"
#include "hal_ga10b.h"
#ifdef CONFIG_NVGPU_DGPU
#include "hal_tu104.h"
#include "hal_ga100.h"
#endif
#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
#include "hal_gb10b.h"
#endif

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_GB20C)
#include "hal/init/hal_gb20c.h"
#endif

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
#include <nvgpu_next_hal_init.h>
#endif

#include "hal/mc/mc_gm20b.h"

int nvgpu_init_hal(struct gk20a *g)
{
	int err = 0;
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch, g->params.gpu_impl);

	switch (ver) {
	case NVGPU_GPUID_GA10B:
		if (ga10b_init_hal(g) != 0) {
			err = -ENODEV;
		}
		break;
#if defined(CONFIG_NVGPU_SUPPORT_GV11B) && defined(CONFIG_NVGPU_HAL_NON_FUSA)
	case NVGPU_GPUID_GV11B:
		if (gv11b_init_hal(g) != 0) {
			err = -ENODEV;
		}
		break;
#endif

#if defined(CONFIG_NVGPU_DGPU) && defined(CONFIG_NVGPU_HAL_NON_FUSA)
	case NVGPU_GPUID_TU104:
		if (tu104_init_hal(g) != 0) {
			err = -ENODEV;
		}
		break;
#if defined(CONFIG_NVGPU_HAL_NON_FUSA) && defined(CONFIG_NVGPU_DGPU)
	case NVGPU_GPUID_GA100:
		if (ga100_init_hal(g) != 0) {
			err = -ENODEV;
		}
		break;
#endif

#endif

#if defined(CONFIG_NVGPU_HAL_NON_FUSA) && defined(CONFIG_NVGPU_GB10B)
	case NVGPU_GPUID_GB10B:
	case NVGPU_GPUID_GB10B_NEXT:
		nvgpu_log_info(g, "gb10b detected");
		if (gb10b_init_hal(g) != 0) {
			nvgpu_err(g, "hal init failed");
			return -ENODEV;
		}
		break;
#endif
#if defined(CONFIG_NVGPU_HAL_NON_FUSA) && defined(CONFIG_NVGPU_GB20C)
	case NVGPU_GPUID_GB20B:
	case NVGPU_GPUID_GB20C:
	case NVGPU_GPUID_GB20C_NEXT:
		nvgpu_log_info(g, "gb20c detected");
		if (gb20c_init_hal(g) != 0) {
			return -ENODEV;
		}
		break;
#endif

	default:
#if defined(CONFIG_NVGPU_HAL_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
		if (nvgpu_next_init_hal(g) != 0)
#endif /* CONFIG_NVGPU_NEXT */
		{
			nvgpu_err(g, "no support for %x", ver);
			err = -ENODEV;
		}
		break;
	}

	return err;
}


int nvgpu_detect_chip(struct gk20a *g)
{
	struct nvgpu_gpu_params *p = &g->params;
	u32 boot_0;
	int err;

	if (p->gpu_arch != 0U) {
		return 0;
	}

	boot_0 = gm20b_get_chip_details(g, &p->gpu_arch,
					&p->gpu_impl, &p->gpu_rev);
	if (boot_0 == U32_MAX) {
		nvgpu_err(g, "get_chip_details failure!");
		return -ENODEV;
	}

#ifdef CONFIG_NVGPU_SUPPORT_GV11B
	if (nvgpu_safe_add_u32(p->gpu_arch, p->gpu_impl) ==
						(u32)NVGPU_GPUID_GV11B) {
		/* overwrite gpu revison for A02  */
		if (!nvgpu_is_soc_t194_a01(g)) {
			p->gpu_rev = 0xa2;
		}
	}
#endif
	nvgpu_log_info(g, "arch: %x, impl: %x, rev: %x\n",
			g->params.gpu_arch,
			g->params.gpu_impl,
			g->params.gpu_rev);

	err = nvgpu_init_hal(g);
	if (err != 0) {
		nvgpu_err(g, "nvgpu_init_hal failure!");
		return err;
	}

	if (g->func_regs == 0U &&
		g->ops.func.get_full_phys_offset != NULL) {
		g->func_regs = nvgpu_safe_add_u64(g->regs,
					g->ops.func.get_full_phys_offset(g));
		g->func_regs_saved = g->func_regs;
	}

	return 0;
}
