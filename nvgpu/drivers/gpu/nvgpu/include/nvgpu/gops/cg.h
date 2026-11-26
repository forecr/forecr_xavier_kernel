/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_CG_H
#define NVGPU_GOPS_CG_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * CG HAL interface.
 */
struct gk20a;

/**
 * CG HAL operations.
 *
 * @see gpu_ops.
 */
struct gops_cg {
	/** @cond DOXYGEN_SHOULD_SKIP_THIS */
	void (*slcg_bus_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_ce2_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_chiplet_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_fb_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_fifo_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_gr_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_ltc_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_perf_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_priring_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_fbhub_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_pmu_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_therm_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_xbar_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_hshub_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_acb_load_gating_prod)(struct gk20a *g, bool prod);
	void (*blcg_bus_load_gating_prod)(struct gk20a *g, bool prod);
	void (*blcg_ce_load_gating_prod)(struct gk20a *g, bool prod);
	void (*blcg_fb_load_gating_prod)(struct gk20a *g, bool prod);
	void (*blcg_fifo_load_gating_prod)(struct gk20a *g, bool prod);
	void (*blcg_gr_load_gating_prod)(struct gk20a *g, bool prod);
	void (*blcg_ltc_load_gating_prod)(struct gk20a *g, bool prod);
	void (*blcg_pmu_load_gating_prod)(struct gk20a *g, bool prod);
	void (*blcg_xbar_load_gating_prod)(struct gk20a *g, bool prod);
	void (*blcg_fbhub_load_gating_prod)(struct gk20a *g, bool prod);
	void (*blcg_hshub_load_gating_prod)(struct gk20a *g, bool prod);
	void (*blcg_ctrl_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_runlist_load_gating_prod)(struct gk20a *g, bool prod);
	void (*blcg_runlist_load_gating_prod)(struct gk20a *g, bool prod);
#ifdef CONFIG_NVGPU_NON_FUSA
	void (*flcg_perf_load_gating_prod)(struct gk20a *g, bool prod);
#endif
	/* Ring station slcg prod gops */
	void (*slcg_rs_ctrl_fbp_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_rs_ctrl_gpc_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_rs_ctrl_sys_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_rs_fbp_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_rs_gpc_load_gating_prod)(struct gk20a *g, bool prod);
	void (*slcg_rs_sys_load_gating_prod)(struct gk20a *g, bool prod);

	void (*slcg_timer_load_gating_prod)(struct gk20a *g, bool prod);

	void (*slcg_ctrl_load_gating_prod)(struct gk20a *g, bool prod);

	void (*slcg_gsp_load_gating_prod)(struct gk20a *g, bool prod);

	/* SLCG and BLCG prod values for Multimedia Engines */
	void (*slcg_nvenc_load_gating_prod)(struct gk20a *g, u32 inst_id,
			bool prod);
	void (*slcg_ofa_load_gating_prod)(struct gk20a *g, u32 inst_id,
			bool prod);
	void (*slcg_nvdec_load_gating_prod)(struct gk20a *g, u32 inst_id,
			bool prod);
	void (*slcg_nvjpg_load_gating_prod)(struct gk20a *g, u32 inst_id,
			bool prod);
	void (*blcg_nvenc_load_gating_prod)(struct gk20a *g, u32 inst_id,
			bool prod);
	void (*blcg_ofa_load_gating_prod)(struct gk20a *g, u32 inst_id,
			bool prod);
	void (*blcg_nvdec_load_gating_prod)(struct gk20a *g, u32 inst_id,
			bool prod);
	void (*blcg_nvjpg_load_gating_prod)(struct gk20a *g, u32 inst_id,
			bool prod);

	void (*slcg_xal_ep_load_gating_prod)(struct gk20a *g, bool prod);
	void (*blcg_xal_ep_load_gating_prod)(struct gk20a *g, bool prod);

	void (*elcg_ce_load_gating_prod)(struct gk20a *g, bool prod);
	/** @endcond DOXYGEN_SHOULD_SKIP_THIS */
};

#endif
