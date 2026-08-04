/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */


#ifndef NVGPU_CLK_H
#define NVGPU_CLK_H

#include <nvgpu/boardobjgrp_e255.h>
#include "ucode_clk_inf.h"

#define CTRL_CLK_FLL_REGIME_ID_INVALID			((u8)0x00000000)
#define CTRL_CLK_FLL_REGIME_ID_FFR			((u8)0x00000001)
#define CTRL_CLK_FLL_REGIME_ID_FR			((u8)0x00000002)

#define CTRL_CLK_FLL_LUT_VSELECT_LOGIC			(0x00000000U)
#define CTRL_CLK_FLL_LUT_VSELECT_MIN			(0x00000001U)
#define CTRL_CLK_FLL_LUT_VSELECT_SRAM			(0x00000002U)

#define CTRL_CLK_VIN_SW_OVERRIDE_VIN_USE_HW_REQ		(0x00000000U)
#define CTRL_CLK_VIN_SW_OVERRIDE_VIN_USE_MIN		(0x00000001U)
#define CTRL_CLK_VIN_SW_OVERRIDE_VIN_USE_SW_REQ		(0x00000003U)

#define CTRL_CLK_VIN_STEP_SIZE_UV			(6250U)
#define CTRL_CLK_LUT_MIN_VOLTAGE_UV			(450000U)
#define CTRL_CLK_FLL_TYPE_DISABLED			(0U)

#define NV2080_CTRL_CLK_CLK_PROP_REGIME_TYPE_BASE            0x00U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_TYPE_1X              0x01U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_TYPE_MODEL_10        0x02U
/*
 * Enumerations of the CLK_PROP_REGIME Id HALs.
 */
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_HAL_GA10X			0x00U

#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_INVALID			0U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_STRICT			1U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_LOOSE			2U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_DRAM_STRICT		3U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_DRAM_LOCK		4U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_GPC_STRICT		5U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_DISP_STRICT		6U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_PCIE_STRICT		7U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_XBAR_STRICT		8U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_XBAR_LOCK		9U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_PERF_STRICT		10U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_STRICT_NO_DISP	\
	NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_PERF_STRICT
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_IMP				11U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_IMP_CLIENT_STRICT 12U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_GPC_POWER_STRICT	13U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_XBAR_POWER_STRICT	14U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_NVVDD_STRICT		15U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_MSVDD_STRICT		16U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_MSVDD_PERF_STRICT 17U
/*
 * Reserved prop regimes for bring up.
 */
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_RSVD_1			18U
#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_RSVD_2			19U

#define NV2080_CTRL_CLK_CLK_PROP_REGIME_ID_MAX				20U

#define NV2080_CTRL_CLK_CLK_DOMAIN_HAL_GB20Y				0x09U

/*!
 *
 * MACROs of VPSTATE indexes/names.
 * These indexes are used to abstract away implementation details from
 * client interfaces - e.g. a client can request the "BOOST" vpstate without
 * having to know which vpstate index.
 */
#define NV2080_CTRL_PERF_VPSTATES_IDX_RATEDTDP                    0x10
#define NV2080_CTRL_PERF_VPSTATES_IDX_TURBOBOOST                  0x12

struct nvgpu_clk_enums;
struct nvgpu_clk_vf_rels;
struct nvgpu_clk_prop_regimes;
struct nvgpu_clk_prop_tops;
struct nvgpu_clk_prop_top_rels;
/*
 * These are board object group objects
 * for the various types of board objects.
 */
struct nvgpu_clk_pmupstate {
	struct nvgpu_avfsvinobjs *avfs_vinobjs;
	struct clk_avfs_fll_objs *avfs_fllobjs;
	struct nvgpu_clk_domains *clk_domainobjs;
	struct nvgpu_clk_progs *clk_progobjs;
	struct nvgpu_clk_vf_points *clk_vf_pointobjs;
	struct nvgpu_clk_enums *clk_enumobjs;
	struct nvgpu_clk_vf_rels *clk_vfrelobjs;
	struct nvgpu_clk_prop_regimes *clk_regimeobjs;
	struct nvgpu_clk_prop_tops *clk_topobjs;
	struct nvgpu_clk_prop_top_rels *clk_top_relobjs;
};

struct clk_vf_point {
	struct pmu_board_obj super;
	u8  vfe_equ_idx;
	u8  volt_rail_idx;
	struct ctrl_clk_vf_pair pair;
};

struct clk_vf_point_volt {
	struct clk_vf_point super;
	u32 source_voltage_uv;
	struct ctrl_clk_freq_delta freq_delta;
};

struct clk_vf_point_freq {
	struct clk_vf_point super;
	int volt_delta_uv;
};

struct nvgpu_clk_vf_points {
	struct boardobjgrp_e255 super;
};

struct clk_vf_point *nvgpu_construct_clk_vf_point(struct gk20a *g,
	void *pargs);

u32 nvgpu_pmu_clk_fll_get_lut_min_volt(struct nvgpu_clk_pmupstate *pclk);
u8 clk_get_fll_lut_vf_num_entries(struct nvgpu_clk_pmupstate *pclk);
struct clk_vin_device *clk_get_vin_from_index(
		struct nvgpu_avfsvinobjs *pvinobjs, u8 idx);
int clk_domain_clk_prog_link(struct gk20a *g,
		struct nvgpu_clk_pmupstate *pclk);
#endif /* NVGPU_CLK_VIN_H */
