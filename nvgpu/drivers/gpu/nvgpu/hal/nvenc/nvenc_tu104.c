// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/multimedia.h>
#include "nvenc_tu104.h"
#include <nvgpu/hw/tu104/hw_pnvenc_tu104.h>

u32 tu104_nvenc_base_addr(u32 inst_id)
{
	nvgpu_assert(inst_id == 0U);
	return pnvenc_falcon_irqsset_r(0);
}

void tu104_nvenc_setup_boot_config(struct gk20a *g, u32 inst_id)
{
	u32 data = 0;

	nvgpu_assert(inst_id == 0U);

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

void tu104_nvenc_halt_engine(struct gk20a *g, u32 inst_id)
{
	u32 data;

	nvgpu_assert(inst_id == 0U);

	data = nvgpu_readl(g, pnvenc_falcon_engctl_r());
	data |= pnvenc_falcon_engctl_stallreq_true_f();
	nvgpu_writel(g, pnvenc_falcon_engctl_r(), data);

	data = nvgpu_readl(g, pnvenc_falcon_engctl_r());
	if (pnvenc_falcon_engctl_stallack_v(data) == 0U) {
		nvgpu_err(g, "NVENC engine is not idle while reset");
	}
}

void tu104_nvenc_interface_enable(struct gk20a *g, u32 inst_id)
{
	u32 itfen;

	nvgpu_assert(inst_id == 0U);

	itfen = nvgpu_readl(g, pnvenc_falcon_itfen_r()) |
		pnvenc_falcon_itfen_ctxen_enable_f() |
		pnvenc_falcon_itfen_mthden_enable_f();
	nvgpu_writel(g, pnvenc_falcon_itfen_r(), itfen);
}
