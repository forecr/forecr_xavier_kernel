/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLK_DOMAIN_H
#define NVGPU_CLK_DOMAIN_H

#include <nvgpu/pmu/pmuif/nvgpu_cmdif.h>
#include <common/pmu/boardobj/boardobj.h>

#define CLK_DOMAIN_BOARDOBJGRP_VERSION 0x30U
#define CLK_DOMAIN_BOARDOBJGRP_VERSION_35 0x35U
#define CLK_DOMAIN_BOARDOBJGRP_VERSION_50 0x50U

#define CLK_TABLE_HAL_ENTRY_GP 0x02U
#define CLK_TABLE_HAL_ENTRY_GV 0x03U
#define CLK_TABLE_HAL_ENTRY_GB20Y 0x09U

#define CLK_CLKMON_VFE_INDEX_INVALID 0xFFU

/*
 * Macro defining max allowed voltage rail VF items per clk domain.
 */
#define NV_CTRL_CLK_CLK_DOMAIN_PROG_RAIL_VF_ITEM_MAX 0x6U
#define NV_CTRL_CLK_CLK_DOMAIN_50_PROG_PHYSICAL_FBVDD_VF_MAPPING_TABLE_MAX_MAPPINGS 16U

typedef int nvgpu_clkproglink(struct gk20a *g, struct nvgpu_clk_pmupstate *pclk,
	struct nvgpu_clk_domain *pdomain);

typedef int nvgpu_clkvfsearch(struct gk20a *g, struct nvgpu_clk_pmupstate *pclk,
	struct nvgpu_clk_domain *pdomain, u16 *clkmhz,
	u32 *voltuv, u8 rail);

typedef int nvgpu_clkgetfpoints(struct gk20a *g,
	struct nvgpu_clk_pmupstate *pclk, struct nvgpu_clk_domain *pdomain,
	u32 *pfpointscount, u16 *pfreqpointsinmhz, u8 rail);

struct nvgpu_clk_domain {
	struct pmu_board_obj super;
	u32 api_domain;
	u32 part_mask;
	u32 domain;
	u8 perf_domain_index;
	u8 perf_domain_grp_idx;
	u8 ratio_domain;
	u8 usage;
	nvgpu_clkproglink *clkdomainclkproglink;
	nvgpu_clkvfsearch *clkdomainclkvfsearch;
	nvgpu_clkgetfpoints *clkdomainclkgetfpoints;
};

struct nvgpu_clk_domains {
	struct boardobjgrp_e32 super;
	u8 n_num_entries;
	u8 version;
	bool b_enforce_vf_monotonicity;
	bool b_enforce_vf_smoothening;
	bool b_override_o_v_o_c;
	bool b_debug_mode;
	u32 vbios_domains;
	u16 cntr_sampling_periodms;
	u16 clkmon_refwin_usec;
	struct boardobjgrpmask_e32 prog_domains_mask;
	union {
		struct boardobjgrpmask_e32 master_domains_mask;
		struct boardobjgrpmask_e32 physical_domains_mask;
		struct boardobjgrpmask_e32 logical_domains_mask;
	};
	struct boardobjgrpmask_e32 clkmon_domains_mask;
	struct ctrl_clk_clk_delta  deltas;

	struct nvgpu_clk_domain
		*ordered_noise_aware_list[CTRL_BOARDOBJ_MAX_BOARD_OBJECTS];

	struct nvgpu_clk_domain
		*ordered_noise_unaware_list[CTRL_BOARDOBJ_MAX_BOARD_OBJECTS];
};

typedef int clkgetslaveclk(struct gk20a *g, struct nvgpu_clk_pmupstate *pclk,
			struct nvgpu_clk_domain *pdomain, u16 *clkmhz,
			u16 masterclkmhz);

struct clk_domain_3x {
	struct nvgpu_clk_domain super;
	bool b_noise_aware_capable;
};

struct clk_domain_3x_fixed {
	struct clk_domain_3x super;
	u16  freq_mhz;
};

struct clk_domain_3x_prog {
	struct clk_domain_3x super;
	u8  clk_prog_idx_first;
	u8  clk_prog_idx_last;
	bool b_force_noise_unaware_ordering;
	struct ctrl_clk_freq_delta factory_delta;
	short freq_delta_min_mhz;
	short freq_delta_max_mhz;
	struct ctrl_clk_clk_delta deltas;
	u8 noise_unaware_ordering_index;
	u8 noise_aware_ordering_index;
};

struct clk_domain_35_prog {
	struct clk_domain_3x_prog super;
	u8 pre_volt_ordering_index;
	u8 post_volt_ordering_index;
	u8 clk_pos;
	u8 clk_vf_curve_count;
	struct ctrl_clk_domain_info_35_prog_clk_mon clkmon_info;
	struct ctrl_clk_domain_control_35_prog_clk_mon clkmon_ctrl;
	u32 por_volt_delta_uv[CTRL_VOLT_VOLT_RAIL_CLIENT_MAX_RAILS];
};

struct clk_domain_3x_master {
	struct clk_domain_3x_prog super;
	u32 slave_idxs_mask;
};

struct clk_domain_35_master {
	struct clk_domain_35_prog super;
	struct clk_domain_3x_master master;
	struct boardobjgrpmask_e32 master_slave_domains_grp_mask;
};

struct clk_domain_3x_slave {
	struct clk_domain_3x_prog super;
	u8 master_idx;
	clkgetslaveclk *clkdomainclkgetslaveclk;
};

struct clk_domain_30_slave {
	u8 rsvd;
	u8 master_idx;
	clkgetslaveclk *clkdomainclkgetslaveclk;
};

struct clk_domain_35_slave {
	struct clk_domain_35_prog super;
	struct clk_domain_30_slave slave;
};

struct ctrl_clk_domain_control_50_prog_physical_clk_mon {
	u32 flags;
	u32 low_threshold_override;
	u32 high_threshold_override;
};

struct ctrl_clk_domain_info_50_prog_physical_clk_mon {
	u16 low_threshold_vfe_idx;
	u16 high_threshold_vfe_idx;
};

/*
 * Data that is used to adjust power measurements made on the FBVDD rail.
 */
struct ctrl_clk_domain_50_prog_physical_fbvdd_pwr_adj {
	u32 slope;
	u32 interceptmW;
};

/*
 * Defines a mapping between max freq at required voltage.
 */
struct ctrl_clk_domain_50_prog_physical_fbvdd_vf_mapping {
	u32 max_freq_khz;
	u32 volt_uv;
};

/*
 * A lookup table from FBVDD frequencies to voltage.
 */
struct ctrl_clk_domain_50_prog_physical_fbvdd_vf_mapping_tbl {
	u8 number_mappings;
	struct ctrl_clk_domain_50_prog_physical_fbvdd_vf_mapping
		mappings[NV_CTRL_CLK_CLK_DOMAIN_50_PROG_PHYSICAL_FBVDD_VF_MAPPING_TABLE_MAX_MAPPINGS];
};

/*
 * FBVDD data that may be associated with a given clock domain.
 */
struct crtl_clk_domain_50_prog_physical_fbvdd_data {
	bool valid;
	struct ctrl_clk_domain_50_prog_physical_fbvdd_pwr_adj pwr_adjustment;
	struct ctrl_clk_domain_50_prog_physical_fbvdd_vf_mapping_tbl vf_mapping_tbl;
};

struct ctrl_clk_domain_info_50_physical_rail_vf_primary {
	u8 clk_vf_rel_idx_first;
	u8 clk_vf_rel_idex_last;
	struct boardobjgrp_e32 secondary_domain_mask;
	struct boardobjgrp_e32 primary_secondary_domain_mask;
};

struct ctrl_clk_domain_info_50_physical_rail_vf_secondary {
	u8 primary_idx;
};

struct ctrl_clk_domain_info_50_prog_physical_rail_vf_item {
	u8 type;
	u8 clk_position;
	union {
		struct ctrl_clk_domain_info_50_physical_rail_vf_primary primary;
		struct ctrl_clk_domain_info_50_physical_rail_vf_secondary secondary;
	} data;
};

struct clk_domain_5x {
	struct nvgpu_clk_domain super;
	bool b_noise_aware_capable;
};

struct clk_domain_5x_fixed {
	struct clk_domain_5x super;
	u16 freq_mhz;
};

struct clk_domain_5x_prog {
	struct clk_domain_5x super;
	u8 clk_prog_idx_first;
	u8 clk_prog_idx_last;
	struct ctrl_clk_freq_delta factory_delta;
	short freq_delta_min_mhz;
	short freq_delta_max_mhz;
	struct ctrl_clk_clk_delta deltas;
};

struct clk_domain_50_prog {
	struct clk_domain_5x_prog super;
	u8 pre_volt_ordering_index;
	u8 post_volt_ordering_index;
	u8 clk_vf_curve_count;
	u8 clk_enum_idx_first;
	u8 clk_enum_idx_last;
	u8 clk_logical_domain_idx;
	s16 freq_delta_min_mhz;
	s16 freq_delta_max_mhz;
	struct ctrl_clk_freq_delta factory_delta;
	struct ctrl_clk_freq_delta grd_delta;
	u32 rail_mask;
	struct ctrl_clk_domain_info_50_prog_physical_rail_vf_item
		rail_vf_item[NV_CTRL_CLK_CLK_DOMAIN_PROG_RAIL_VF_ITEM_MAX];
	struct ctrl_clk_domain_info_50_prog_physical_clk_mon clkmon_info;
	struct crtl_clk_domain_50_prog_physical_fbvdd_data fbvdd_data;
	u8 cpmu_clk_id;
};

struct clk_domain_50_prog_logical {
	struct clk_domain_50_prog super;
	struct boardobjgrpmask_e32 logical_domains_grp_mask;
};

struct clk_domain_50_prog_physical {
	struct clk_domain_50_prog super;
	struct ctrl_clk_clk_delta deltas;
	struct ctrl_clk_domain_control_35_prog_clk_mon clkmon_ctrl;	// v5.0 same as 3.5.
	struct boardobjgrpmask_e32 physical_domains_grp_mask;
};

s32 clk_domain_init_pmupstate(struct gk20a *g);
void clk_domain_free_pmupstate(struct gk20a *g);
s32 clk_pmu_clk_domains_load(struct gk20a *g);
s32 clk_domain_sw_setup(struct gk20a *g);
s32 clk_domain_pmu_setup(struct gk20a *g);

#endif /* NVGPU_CLK_DOMAIN_H */
