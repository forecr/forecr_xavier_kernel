/*
 * Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

#include <nvgpu/types.h>
#include <nvgpu/timers.h>
#include <nvgpu/dma.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/falcon.h>
#include <nvgpu/nvenc.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/soc.h>
#include <nvgpu/io.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/string.h>
#include <nvgpu/multimedia.h>
#include <nvgpu/nvgpu_err.h>
#include "multimedia_priv.h"

/* NVENC init */
int nvgpu_nvenc_sw_init(struct gk20a *g)
{
	u32 ver = nvgpu_safe_add_u32(g->params.gpu_arch,
					g->params.gpu_impl);
	int err = 0;

	if (g->nvenc != NULL) {
		/* Initialized already, so reuse the same to perform boot faster */
		return err;
	}

	g->nvenc = (struct nvgpu_nvenc *)nvgpu_kzalloc(g, sizeof(struct nvgpu_nvenc));
	if (g->nvenc == NULL) {
		err = -ENOMEM;
		goto done;
	}

	switch (ver) {
#ifdef CONFIG_NVGPU_DGPU
	case NVGPU_GPUID_TU104:
		g->nvenc->fw_name = TU104_NVENC_UCODE_FW;
		g->nvenc->nvenc_flcn = &g->nvenc_flcn;
		err = nvgpu_multimedia_copy_fw(g, g->nvenc->fw_name,
						g->nvenc->ucode_header,
						&g->nvenc->nvenc_mem_desc);
		break;
#endif
	default:
		nvgpu_kfree(g, g->nvenc);
		err = -ENODEV;
		nvgpu_err(g, "no support for GPUID %x", ver);
		break;
	}

done:
	return err;
}

/* NVENC deinit */
int nvgpu_nvenc_sw_deinit(struct gk20a *g)
{
	struct nvgpu_nvenc *nvenc = g->nvenc;

	nvgpu_dma_free(g, &nvenc->nvenc_mem_desc);
	nvgpu_kfree(g, nvenc);
	return 0;
}

/* NVENC falcon boot */
int nvgpu_nvenc_falcon_boot(struct gk20a *g)
{
	int err = 0;
	int completion = 0;

	struct nvgpu_nvenc *nvenc = g->nvenc;
	struct nvgpu_falcon *flcn = nvenc->nvenc_flcn;
	struct nvgpu_mem *nvenc_mem_desc = &nvenc->nvenc_mem_desc;
	u32 *ucode_header = nvenc->ucode_header;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_mme, " ");

	/* Reset nvenc HW unit */
	err = nvgpu_mc_reset_units(g, NVGPU_UNIT_NVENC);
	if (err != 0) {
		nvgpu_err(g, "Failed to reset NVENC unit");
		goto done;
	}

	/* Falcon reset */
	err = nvgpu_falcon_reset(flcn);
	if (err != 0) {
		nvgpu_err(g, "nvenc falcon_reset failed, err=%d", err);
		goto done;
	}

	/* Set falcon interrupt mask and routing registers */
	if (g->ops.nvenc.set_irq_regs != NULL)
		g->ops.nvenc.set_irq_regs(g, flcn);

	/* Setup falcon apertures, boot-config */
	if (flcn->flcn_engine_dep_ops.setup_bootstrap_config != NULL) {
		flcn->flcn_engine_dep_ops.setup_bootstrap_config(flcn->g);
	}

	/* Load nvenc ucode */
	err = nvgpu_falcon_load_ucode(flcn, nvenc_mem_desc, ucode_header);
	if (err != 0) {
		nvgpu_err(g, "nvenc ucode loading failed, err=%d", err);
		goto done;
	}

	/* Enable falcon interfaces */
	if (g->ops.nvenc.interface_enable != NULL)
		g->ops.nvenc.interface_enable(g);

	/* Kick-start falcon cpu */
	g->ops.falcon.bootstrap(flcn, 0U);

	/* Wait for falcon idle */
	completion = nvgpu_falcon_wait_idle(flcn);
	if (completion != 0) {
		nvgpu_err(g, "nvenc flcn: ucode boot timed out");
		err = -ETIMEDOUT;
	}

	nvgpu_log(g, gpu_dbg_info | gpu_dbg_mme,
		"NVENC NS boot %s!", err ? "SUCCESS" : "FAILED");

done:
	return err;
}

int nvgpu_nvenc_reset(struct gk20a *g)
{
	int err = 0;

	nvgpu_log(g, gpu_dbg_info | gpu_dbg_mme, "Resetting nvenc");

	if (g->ops.nvenc.halt_engine != NULL) {
		g->ops.nvenc.halt_engine(g);
	}

	/* Reset nvenc HW unit and load ucode */
	err = g->ops.nvenc.bootstrap(g);

	return err;
}
