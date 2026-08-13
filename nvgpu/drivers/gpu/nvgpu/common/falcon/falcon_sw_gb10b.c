// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/gk20a.h>
#include <nvgpu/falcon.h>
#include <nvgpu/soc.h>

#include "common/falcon/falcon_sw_gk20a.h"
#include "common/falcon/falcon_sw_ga10b.h"
#include "falcon_sw_gb10b.h"

static void gb10b_falcon_engine_dependency_ops(struct nvgpu_falcon *flcn)
{
	struct gk20a *g = flcn->g;
	struct nvgpu_falcon_engine_dependency_ops *flcn_eng_dep_ops =
			&flcn->flcn_engine_dep_ops;

	switch (flcn->flcn_id) {
	case FALCON_ID_NVENC_RISCV_EB:
		flcn_eng_dep_ops->riscv_bootstrap_config =
			g->ops.nvenc.setup_boot_config;
		flcn_eng_dep_ops->enable_irq =
			g->ops.nvenc.enable_irq;
		flcn_eng_dep_ops->reset_eng =
			g->ops.nvenc.reset_eng;
		flcn_eng_dep_ops->slcg_load_enable =
			g->ops.cg.slcg_nvenc_load_gating_prod;
		flcn_eng_dep_ops->blcg_load_enable =
			g->ops.cg.blcg_nvenc_load_gating_prod;
		break;
	case FALCON_ID_OFA_RISCV_EB:
		flcn_eng_dep_ops->riscv_bootstrap_config =
			g->ops.ofa.setup_boot_config;
		flcn_eng_dep_ops->enable_irq =
			g->ops.ofa.enable_irq;
		flcn_eng_dep_ops->reset_eng =
			g->ops.ofa.reset_eng;
		flcn_eng_dep_ops->slcg_load_enable =
			g->ops.cg.slcg_ofa_load_gating_prod;
		flcn_eng_dep_ops->blcg_load_enable =
			g->ops.cg.blcg_ofa_load_gating_prod;
		break;
	case FALCON_ID_NVDEC_RISCV:
		flcn_eng_dep_ops->riscv_bootstrap_config =
			g->ops.nvdec.setup_boot_config;
		flcn_eng_dep_ops->enable_irq =
			g->ops.nvdec.enable_irq;
		flcn_eng_dep_ops->reset_eng =
			g->ops.nvdec.reset_eng;
		flcn_eng_dep_ops->slcg_load_enable =
			g->ops.cg.slcg_nvdec_load_gating_prod;
		flcn_eng_dep_ops->blcg_load_enable =
			g->ops.cg.blcg_nvdec_load_gating_prod;
		break;
	case FALCON_ID_NVJPG_RISCV_EB:
		flcn_eng_dep_ops->riscv_bootstrap_config =
			g->ops.nvjpg.setup_boot_config;
		flcn_eng_dep_ops->enable_irq =
			g->ops.nvjpg.enable_irq;
		flcn_eng_dep_ops->reset_eng =
			g->ops.nvjpg.reset_eng;
		flcn_eng_dep_ops->slcg_load_enable =
			g->ops.cg.slcg_nvjpg_load_gating_prod;
		flcn_eng_dep_ops->blcg_load_enable =
			g->ops.cg.blcg_nvjpg_load_gating_prod;
		break;
	case FALCON_ID_PMU:
		gk20a_falcon_engine_dependency_ops(flcn);
		break;
	case FALCON_ID_GSPLITE:
		flcn_eng_dep_ops->reset_eng = g->ops.gsp.gsp_reset;
		flcn_eng_dep_ops->setup_bootstrap_config =
				g->ops.gsp.falcon_setup_boot_config;
		flcn_eng_dep_ops->copy_to_emem = g->ops.gsp.gsp_copy_to_emem;
		flcn_eng_dep_ops->copy_from_emem =
						g->ops.gsp.gsp_copy_from_emem;
		break;
	default:
		/*
		 * NULL assignment make sure
		 * CPU hard reset in gk20a_falcon_reset() gets executed
		 * if falcon doesn't need specific reset implementation
		 */
		flcn_eng_dep_ops->reset_eng = NULL;
		break;
	}
}

void gb10b_falcon_sw_init(struct nvgpu_falcon *flcn)
{
	struct gk20a *g = flcn->g;

	switch (flcn->flcn_id) {
	case FALCON_ID_NVENC_RISCV_EB:
		flcn->flcn_base = g->ops.nvenc.base_addr(flcn->inst_id);
		flcn->flcn2_base = g->ops.nvenc.falcon2_base_addr(flcn->inst_id);
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = true;
		flcn->is_falcon2_enabled = true;
		break;
	case FALCON_ID_OFA_RISCV_EB:
		flcn->flcn_base = g->ops.ofa.base_addr(flcn->inst_id);
		flcn->flcn2_base = g->ops.ofa.falcon2_base_addr(flcn->inst_id);
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = true;
		flcn->is_falcon2_enabled = true;
		break;
	case FALCON_ID_NVDEC_RISCV:
		flcn->flcn_base = g->ops.nvdec.base_addr(flcn->inst_id);
		flcn->flcn2_base = g->ops.nvdec.falcon2_base_addr(flcn->inst_id);
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = true;
		flcn->is_brom_boot = true;
		flcn->is_falcon2_enabled = true;
		break;
	case FALCON_ID_NVJPG_RISCV_EB:
		flcn->flcn_base = g->ops.nvjpg.base_addr(flcn->inst_id);
		flcn->flcn2_base = g->ops.nvjpg.falcon2_base_addr(flcn->inst_id);
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = true;
		flcn->is_falcon2_enabled = true;
		break;
	case FALCON_ID_PMU:
		flcn->flcn_base = g->ops.pmu.falcon_base_addr();
		flcn->flcn2_base = g->ops.pmu.falcon2_base_addr();
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = true;
		flcn->is_falcon2_enabled = true;
		break;
	case FALCON_ID_FECS:
	case FALCON_ID_FECS_RISCV_EB:
		flcn->inst_id = 1;
		flcn->flcn_base = g->ops.gr.falcon.fecs_base_addr();
		flcn->flcn2_base = g->ops.gr.falcon.fecs2_base_addr();
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = false;
		if (nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
			/*
			 * For simulation boot falcon due to bug in fmodel
			 * regarding ctxsw eb recovery
			 * TODO: Enable CTXSW EB RISCV in SCSIM
			 */
			if (nvgpu_platform_is_simulation(g)) {
				flcn->is_falcon2_enabled = false;
			} else {
				flcn->is_falcon2_enabled = true;
			}
		} else {
			flcn->is_falcon2_enabled = false;
		}
		break;
	case FALCON_ID_GPCCS:
	case FALCON_ID_GPCCS_RISCV_EB:
		flcn->inst_id = 1;
		flcn->flcn_base = g->ops.gr.falcon.gpccs_base_addr();
		flcn->flcn2_base = g->ops.gr.falcon.gpccs2_base_addr();
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = false;
		if (nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
			/*
			 * For simulation boot falcon due to bug in fmodel
			 * regarding ctxsw eb recovery
			 * TODO: Enable CTXSW EB RISCV in SCSIM
			 */
			if (nvgpu_platform_is_simulation(g)) {
				flcn->is_falcon2_enabled = false;
			} else {
				flcn->is_falcon2_enabled = true;
			}
		} else {
			flcn->is_falcon2_enabled = false;
		}
		break;
	case FALCON_ID_GSPLITE:
		flcn->flcn_base = g->ops.gsp.falcon_base_addr();
		flcn->flcn2_base = g->ops.gsp.falcon2_base_addr();
		flcn->is_falcon_supported = true;
		flcn->is_interrupt_enabled = true;
		flcn->emem_supported = true;
		flcn->is_falcon2_enabled = true;
		break;
	default:
		/*
		 * Set false to inherit falcon support
		 * from previous chips HAL
		 */
		flcn->is_falcon_supported = false;
		break;
	}

	if (flcn->is_falcon_supported) {
		gb10b_falcon_engine_dependency_ops(flcn);
	} else {
		/*
		 * Forward call to previous chip's SW init
		 * to fetch info for requested
		 * falcon as no changes between
		 * current & previous chips.
		 */
		ga10b_falcon_sw_init(flcn);
	}

}
