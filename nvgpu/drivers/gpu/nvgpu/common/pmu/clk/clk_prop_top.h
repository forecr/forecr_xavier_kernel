/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLK_PROP_TOP_H
#define NVGPU_CLK_PROP_TOP_H

#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <common/pmu/boardobj/boardobj.h>

#define NV2080_CTRL_CLK_CLK_DOMAIN_CLIENT_MAX_DOMAINS          16U

struct ctrl_clk_clk_prop_top_info_frequency_search_space {
	u16 minRatioClkPropTopIdx;
	u16 maxRatioClkPropTopIdx;
	u8 searchSpaceHal;
	bool bUseHigherIsoSecFmaxVminMhz;
};

/*
 * Array of source to destination clock propagation topology path for a given
 * programmable clock domain. Each path stores the clock propagation relationship
 * index which represents the next relationship index on the path from source
 * clock domain to desitnation clock domain.
 */
struct ctrl_clk_clk_prop_top_clk_domain_dst_path {
	u16 dstPath[NV2080_CTRL_CLK_CLK_DOMAIN_CLIENT_MAX_DOMAINS];
};

/*
 * Array of source to destination clock propagation topology path for all
 * programmable clock domains.
 */
struct ctrl_clk_clk_prop_top_clk_domains_dst_path {
	struct ctrl_clk_clk_prop_top_clk_domain_dst_path
		domainDstPath[NV2080_CTRL_CLK_CLK_DOMAIN_CLIENT_MAX_DOMAINS];
};

struct clk_prop_top {
	struct pmu_board_obj super;
	u8 topId;
	struct boardobjgrp_e255 clkPropTopRelMask;
};

struct nvgpu_clk_prop_tops {
	struct boardobjgrp_e32 super;
	u8 topHal;
	u8 activeTopId;
	u8 activeTopIdForced;
	u16 topIdToIdxMap[NV2080_CTRL_CLK_CLK_PROP_TOP_ID_MAX];
	struct ctrl_clk_clk_prop_top_info_frequency_search_space
		freqSearchSpace;
};

#define VBIOS_CLOCK_PROP_TOP_TABLE_HAL_TOP_ID_MAX                          0x32U
#define VBIOS_CLOCK_PROP_TOP_TABLE_NUM_HALS                                0x06U
#define VBIOS_CLIENT_CLOCK_PROP_TOP_POL_TABLE_ENTRY_MAX                    0x01U
#define VBIOS_CLIENT_CLOCK_PROP_TOP_POL_TABLE_ENTRY_1X_SLIDER_POINT_MAX    0x10U
#define NV2080_CTRL_CLK_CLIENT_CLK_PROP_TOP_POL_ID_GRAPHICS_MEMORY         0x00U
#define NV2080_CTRL_CLK_CLIENT_CLK_PROP_TOP_POL_TYPE_1X_SLIDER             0x03U

#define NV2080_CTRL_CLK_CLIENT_CLK_PROP_TOP_POL_SLIDER_POINT_NAME_BASE           0x00U
#define NV2080_CTRL_CLK_CLIENT_CLK_PROP_TOP_POL_SLIDER_POINT_NAME_EXTREME        0x01U
#define NV2080_CTRL_CLK_CLIENT_CLK_PROP_TOP_POL_SLIDER_POINT_NAME_INTERMEDIATE_1 0x02U
#define NV2080_CTRL_CLK_CLIENT_CLK_PROP_TOP_POL_SLIDER_POINT_NAME_INTERMEDIATE_2 0x03U
#define NV2080_CTRL_CLK_CLIENT_CLK_PROP_TOP_POL_SLIDER_POINT_NAME_INTERMEDIATE_3 0x04U
#define NV2080_CTRL_CLK_CLIENT_CLK_PROP_TOP_POL_SLIDER_POINT_NAME_INVALID        U8_MAX
/*
 * Struct defining the POR confirmation information for the points in 1.X version
 * of slider based topology policy.
 */
struct vbios_client_clock_prop_top_pol_table_entry_1x_slider_point {
	/*
	 * Name associated with this point.
	 */
	u8    name;
	/*
	 * Internal clk propagation topology id mapped to this point.
	 */
	u8    topId;
};

/*!
 * Struct defining the POR confirmation information for the 1.X version
 * of slider based topology policy.
 */
struct vbios_client_clock_prop_top_pol_table_entry_1x_slider {
	/*
	 * Default POR point index on this slider.
	 */
	u8    defaultPoint;
	/*
	 * Total number of discrete points supported for this policy slider.
	 */
	u8    numPoints;
	/*
	 * Array of 1.X version slider point POR information captured in HAL.
	 */
	struct vbios_client_clock_prop_top_pol_table_entry_1x_slider_point
		points[VBIOS_CLIENT_CLOCK_PROP_TOP_POL_TABLE_ENTRY_1X_SLIDER_POINT_MAX];
};

/*!
 * Type specific data union.
 */
union vbios_client_clock_prop_top_pol_table_entry_data {
	struct vbios_client_clock_prop_top_pol_table_entry_1x_slider v1xSlider;
};

struct vbios_client_clock_prop_top_pol_table_entry {
	/*
	 * Client Clock Propagation Topology Policy Id.
	 */
	u8 topPolId;
	/*
	 * Class type.
	 */
	u8 type;
	/*
	 * Type specific data
	 */
	union vbios_client_clock_prop_top_pol_table_entry_data data;
};

struct vbios_clock_prop_top_pol_table_hal {
	struct vbios_client_clock_prop_top_pol_table_entry
		topPolEntries[VBIOS_CLIENT_CLOCK_PROP_TOP_POL_TABLE_ENTRY_MAX];
};

struct vbios_clock_prop_top_table_hal {
	u8 topHal;
	u8 topId[VBIOS_CLOCK_PROP_TOP_TABLE_HAL_TOP_ID_MAX];
	struct vbios_clock_prop_top_pol_table_hal topPolHal;
};

s32 clk_prop_top_init_pmupstate(struct gk20a *g);
void clk_prop_top_free_pmupstate(struct gk20a *g);
s32 clk_prop_top_sw_setup(struct gk20a *g);
s32 clk_prop_top_pmu_setup(struct gk20a *g);

#endif /* NVGPU_CLK_PROP_TOP_H */
