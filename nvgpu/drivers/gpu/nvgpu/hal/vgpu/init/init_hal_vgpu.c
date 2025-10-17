// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/gk20a.h>

#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/vgpu/os_init_hal_vgpu.h>

#include "init_hal_vgpu.h"
#include "vgpu_hal_gv11b.h"
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
#include "vgpu_hal_ga10b.h"
#if defined(CONFIG_NVGPU_GB10B)
#include "vf_hal_gb10b.h"
#endif
#if defined(CONFIG_NVGPU_GB20C)
#include "vf_hal_gb20c.h"
#endif
#endif

#if defined(CONFIG_NVGPU_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
#include <nvgpu_next_hal_init.h>
#endif

int vgpu_init_hal(struct gk20a *g)
{
	u32 ver = g->params.gpu_arch + g->params.gpu_impl;
	int err;

#if defined(CONFIG_NVGPU_HAL_NON_FUSA) && defined(CONFIG_NVGPU_NEXT)
	err = vgpu_next_init_hal(g);
	if (err != -ENODEV) {
		return err;
	}
#endif

	switch (ver) {
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	case NVGPU_GPUID_GV11B:
		err = vgpu_gv11b_init_hal(g);
		break;
	case NVGPU_GPUID_GA10B:
		err = vgpu_ga10b_init_hal(g);
		break;
#if defined(CONFIG_NVGPU_GR_VIRTUALIZATION)
#if defined(CONFIG_NVGPU_GB10B)
	case NVGPU_GPUID_GB10B:
	case NVGPU_GPUID_GB10B_NEXT:
		err = vf_gb10b_init_hal(g);
		if (err != 0) {
			return err;
		}
		break;
#endif
#ifdef CONFIG_NVGPU_GB20C
	case NVGPU_GPUID_GB20C:
	case NVGPU_GPUID_GB20C_NEXT:
		err = vf_gb20c_init_hal(g);
		if (err != 0) {
			return err;
		}
		break;
#endif
#endif
#endif
	default:
		nvgpu_err(g, "no support for %x", ver);
		err = -ENODEV;
		break;
	}

	if (err == 0) {
		err = vgpu_init_hal_os(g);
	}

	return err;
}

void vgpu_detect_chip(struct gk20a *g)
{
	struct nvgpu_gpu_params *p = &g->params;
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);

	p->gpu_arch = priv->constants.arch;
	p->gpu_impl = priv->constants.impl;
	p->gpu_rev = priv->constants.rev;

	nvgpu_log_info(g, "arch: %x, impl: %x, rev: %x\n",
			p->gpu_arch,
			p->gpu_impl,
			p->gpu_rev);
}
