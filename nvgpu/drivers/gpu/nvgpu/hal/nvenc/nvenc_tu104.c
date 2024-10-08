/*
 * Copyright (c) 2023, NVIDIA CORPORATION.  All rights reserved.
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
#include <nvgpu/io.h>
#include <nvgpu/multimedia.h>
#include "nvenc_tu104.h"
#include <nvgpu/hw/tu104/hw_pnvenc_tu104.h>

u32 tu104_nvenc_base_addr(void)
{
	return pnvenc_falcon_irqsset_r(0);
}

void tu104_nvenc_setup_boot_config(struct gk20a *g)
{
	u32 data = 0;

	data = nvgpu_readl(g, pnvenc_fbif_ctl_r());
	data |= pnvenc_fbif_ctl_allow_phys_no_ctx_allow_f();
	nvgpu_writel(g, pnvenc_fbif_ctl_r(), data);

	/* Setup aperture (physical) for ucode loading */
	data = nvgpu_readl(g, pnvenc_fbif_transcfg_r(UCODE_DMA_ID));
	data |= pnvenc_fbif_transcfg_mem_type_physical_f() |
			pnvenc_fbif_transcfg_target_noncoherent_sysmem_f();
	nvgpu_writel(g, pnvenc_fbif_transcfg_r(UCODE_DMA_ID), data);

	/* Enable the context interface */
	nvgpu_writel(g, pnvenc_falcon_itfen_r(),
		nvgpu_readl(g, pnvenc_falcon_itfen_r()) |
		pnvenc_falcon_itfen_ctxen_enable_f());
}

void tu104_nvenc_halt_engine(struct gk20a *g)
{
	u32 data;

	data = nvgpu_readl(g, pnvenc_falcon_engctl_r());
	data |= pnvenc_falcon_engctl_stallreq_true_f();
	nvgpu_writel(g, pnvenc_falcon_engctl_r(), data);

	data = nvgpu_readl(g, pnvenc_falcon_engctl_r());
	if (pnvenc_falcon_engctl_stallack_v(data) == 0U) {
		nvgpu_err(g, "NVENC engine is not idle while reset");
	}
}

void tu104_nvenc_set_irq_regs(struct gk20a *g, struct nvgpu_falcon *flcn)
{
	u32 intr_mask;
	u32 intr_dest;

	/* dest 0=falcon, 1=host; level 0=irq0, 1=irq1 */
	intr_dest = pnvenc_falcon_irqdest_host_gptmr_f(0)        |
			pnvenc_falcon_irqdest_host_wdtmr_f(1)    |
			pnvenc_falcon_irqdest_host_mthd_f(0)     |
			pnvenc_falcon_irqdest_host_ctxsw_f(0)    |
			pnvenc_falcon_irqdest_host_halt_f(1)     |
			pnvenc_falcon_irqdest_host_exterr_f(1)   |
			pnvenc_falcon_irqdest_host_swgen0_f(1)   |
			pnvenc_falcon_irqdest_host_swgen1_f(1)   |
			pnvenc_falcon_irqdest_host_ext_f(0xff)   |
			pnvenc_falcon_irqdest_target_gptmr_f(0)  |
			pnvenc_falcon_irqdest_target_wdtmr_f(0)  |
			pnvenc_falcon_irqdest_target_mthd_f(0)   |
			pnvenc_falcon_irqdest_target_ctxsw_f(0)  |
			pnvenc_falcon_irqdest_target_halt_f(0)   |
			pnvenc_falcon_irqdest_target_exterr_f(0) |
			pnvenc_falcon_irqdest_target_swgen0_f(0) |
			pnvenc_falcon_irqdest_target_swgen1_f(0) |
			pnvenc_falcon_irqdest_target_ext_f(0);

	/* 0=disable, 1=enable */
	intr_mask = pnvenc_falcon_irqmset_gptmr_f(0)      |
			pnvenc_falcon_irqmset_wdtmr_f(1)  |
			pnvenc_falcon_irqmset_mthd_f(0)   |
			pnvenc_falcon_irqmset_ctxsw_f(0)  |
			pnvenc_falcon_irqmset_halt_f(1)   |
			pnvenc_falcon_irqmset_exterr_f(0) |
			pnvenc_falcon_irqmset_swgen0_f(1) |
			pnvenc_falcon_irqmset_swgen1_f(1) |
			pnvenc_falcon_irqmset_ext_f(0xff);

	g->ops.falcon.set_irq(flcn, true, intr_mask, intr_dest);
}

void tu104_nvenc_interface_enable(struct gk20a *g)
{
	u32 itfen;

	itfen = nvgpu_readl(g, pnvenc_falcon_itfen_r()) |
		pnvenc_falcon_itfen_ctxen_enable_f() |
		pnvenc_falcon_itfen_mthden_enable_f();
	nvgpu_writel(g, pnvenc_falcon_itfen_r(), itfen);
}
