/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLK_DOMAIN_H
#define NVGPU_CLK_DOMAIN_H

#include <nvgpu/pmu/pmuif/nvgpu_cmdif.h>
#include <common/pmu/boardobj/boardobj.h>
#include <nvgpu/pmu/clk/clk.h>

#define CLK_DOMAIN_BOARDOBJGRP_VERSION 0x30U
#define CLK_DOMAIN_BOARDOBJGRP_VERSION_35 0x35U
#define CLK_DOMAIN_BOARDOBJGRP_VERSION_50 0x50U

#define CLK_TABLE_HAL_ENTRY_GP 0x02U
#define CLK_TABLE_HAL_ENTRY_GV 0x03U
#define CLK_TABLE_HAL_ENTRY_GB20Y 0x09U

#define CLK_CLKMON_VFE_INDEX_INVALID 0xFFU

#define NV_VBIOS_CLK_MAX_DEVICES	10

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
	u32 public_domain;
	u32 part_mask;
	u32 domain;
	bool b_noise_aware_capable;
	u8 perf_domain_index;
	u8 perf_domain_grp_idx;
	u8 ratio_domain;
	u8 usage;
	/*
	 * Virtual Function Pointers
	 */
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

struct client_clk_domain {
	struct pmu_board_obj super;
	u8 domainIdx;
};

struct client_clk_domains {
	struct boardobjgrp_e32 super;
};

struct nvgpu_clk_domains_5x {
	struct boardobjgrp_e32 super;
	u8 nNumEntries;
	u8 version;
	u8 clocksHAL;
	u8 vbiosClocksHAL;
	u8 overClockingBin;
	bool bEnforceVfMonotonicity;
	bool bEnforceVfSmoothening;
	bool bGrdFreqOCEnabled;
	bool bOverrideOVOC;
	bool bDebugMode;
	bool bInsAwareVFDisable;
	u16 cntrSamplingPeriodms;
	bool bClkMonEnabled;
	bool bSkipMclkSwitch;
	u16 clkMonRefWinUsec;
	u16 xbarBoostVfeIdx;
	u64 vbiosDomains;
	u64 logicalDomains;
	struct boardobjgrpmask_e32 progDomainsMask;
	struct boardobjgrpmask_e32 primaryDomainsMask;
	struct boardobjgrpmask_e32 clientDomainsMask;
	struct boardobjgrpmask_e32 clkMonDomainsMask;
	struct ctrl_clk_clk_delta deltas;
	struct client_clk_domains clientDomains;
	struct nvgpu_clk_domain *orderedNoiseAwareList[CTRL_BOARDOBJ_MAX_BOARD_OBJECTS];
	struct nvgpu_clk_domain *orderedNoiseUnawareList[CTRL_BOARDOBJ_MAX_BOARD_OBJECTS];
};

struct nvgpu_clk_domains_50 {
	struct nvgpu_clk_domains_5x super;
	struct boardobjgrpmask_e32 progClientVisibleDomainsMask;
	struct boardobjgrpmask_e32 progPhysicalDomainsMask;
	struct boardobjgrpmask_e32 progPhysicalCpmuDomainsMask;
	struct nvgpu_clk_vf_point_5x currentClocksVfInfo;
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

struct clk_domain_50 {
	struct nvgpu_clk_domain super;
	u8 clientDomainIdx;
};

struct clk_domain_50_fixed {
	struct clk_domain_50 super;
	u16 freq_mhz;
};

struct clk_domain_prog {
	s16 offset;
};

struct clk_domain_50_prog {
	struct clk_domain_50 super;
	struct clk_domain_prog prog;
	bool b_fav_point_available;
};

struct clk_domain_50_physical_rail_vf_primary {
	u8 clk_vf_rel_idx_first;
	u8 clk_vf_rel_idx_last;
	struct boardobjgrpmask_e32 secondary_domain_mask;
	struct boardobjgrpmask_e32 primary_secondary_domain_mask;
};

struct clk_domain_50_physical_rail_vf_secondary {
	u8 primary_idx;
};

struct clk_domain_50_prog_physical_rail_vf_item {
	u8 type;
	u8 clk_position;
	union {
		struct clk_domain_50_physical_rail_vf_primary primary;
		struct clk_domain_50_physical_rail_vf_secondary secondary;
	} data;
};


struct clk_domain_50_prog_physical {
	struct clk_domain_50_prog super;
	u8 clk_enum_idx_first;
	u8 clk_enum_idx_last;
	u8 pre_volt_ordering_index;
	u8 post_volt_ordering_index;
	u8 clk_vf_curve_count;
	u8 logical_domain_idx;
	s16 freq_delta_min_mhz;
	s16 freq_delta_max_mhz;
	struct ctrl_clk_freq_delta factory_delta;
	struct ctrl_clk_freq_delta grd_freq_delta;
	struct ctrl_clk_clk_delta deltas;
	u32 rail_mask;
	struct clk_domain_50_prog_physical_rail_vf_item
		rail_vf_item[NV_CTRL_CLK_CLK_DOMAIN_PROG_RAIL_VF_ITEM_MAX];
	struct ctrl_clk_domain_info_50_prog_physical_clk_mon clkmon_info;
	struct ctrl_clk_domain_control_50_prog_physical_clk_mon clkmon_ctrl;
	struct crtl_clk_domain_50_prog_physical_fbvdd_data fbvdd_data;
	u8 cpmu_clk_id;
};

/*!
 * Structure representing the HAL enumeration of a CLK_DOMAIN from the Clocks
 * Table Domains
 */
struct vbios_clocks_table_1x_hal_clock_entry {
	u32 domain;
	bool b_noise_aware_capable;
	u8 clk_vf_curve_count;
	u32 public_domain;
	NV2080_CTRL_CLK_CLK_DOMAIN_CPMU_CLOCK_ID cpmuClkId;
};

struct vbios_clocks_table
{
	u8 nvClocksHAL;    // NVGPU Clock HAL ID
	struct vbios_clocks_table_1x_hal_clock_entry domains[NV_VBIOS_CLK_MAX_DEVICES];
};

u32 clktranslatehalmumsettoapinumset(u32 clkhaldomains);

struct nvgpu_clk_domain *clkDomainsFindByApiDomain(struct gk20a *g,
	u32 apiDomain);

struct nvgpu_clk_domain *construct_clk_domain(struct gk20a *g,
		void *pargs);
s32 clk_domain_construct_super(struct gk20a *g,
	struct pmu_board_obj **obj,
	size_t size, void *pargs);

s32 clkdomainclkproglink_fixed(struct gk20a *g,
	struct nvgpu_clk_pmupstate *pclk,
	struct nvgpu_clk_domain *pdomain);

s32 clkdomainvfsearch(struct gk20a *g,
	struct nvgpu_clk_pmupstate *pclk,
	struct nvgpu_clk_domain *pdomain,
	u16 *pclkmhz,
	u32 *pvoltuv,
	u8 rail);

s32 clkdomaingetfpoints(struct gk20a *g,
	struct nvgpu_clk_pmupstate *pclk,
	struct nvgpu_clk_domain *pdomain,
	u32 *pfpointscount,
	u16 *pfreqpointsinmhz,
	u8 rail);

s32 clk_domains_set_offset_50(struct gk20a *g,
	s32 offset_khz, u32 api_domain);

s32 clk_domains_pmudata_instget(struct gk20a *g,
	struct nv_pmu_boardobjgrp *pmuboardobjgrp,
	struct nv_pmu_boardobj **pmu_obj, u16 idx);

s32 clk_domain_pmudatainit_super(struct gk20a *g, struct pmu_board_obj
	*obj,	struct nv_pmu_boardobj *pmu_obj);

s32 devinitClocksTableHALTranslate_1X(struct gk20a *g,
		u8 vbiosClocksHAL,
		struct vbios_clocks_table **pClocksHAL);
s32 clk_domain_pmudatainit_50_prog(struct gk20a *g,
			struct pmu_board_obj *pBoardObj,
			struct nv_pmu_boardobj *pPmuData);
s32 clk_domain_sw_setup_50(struct gk20a *g);
s32 clk_domain_construct_50_fixed(struct gk20a *g,
					struct pmu_board_obj **obj,
					size_t size, void *pargs);
s32 clk_domain_construct_50_physical(struct gk20a *g,
					struct pmu_board_obj **obj,
					size_t size, void *pargs);
u8 clkDomainProgVoltRailIdxGet_50_PROG_PHYSICAL(struct gk20a *g,
					void*     pDomainProg);

s32 clkClkDomainGetApiDomainByIndex(struct gk20a *g, u32 *api_domain, u32 index);

s32 clk_domain_get_gpc_drive_mode_freq(struct gk20a *g, u32 *freq);
s32 clk_domain_get_gpc_sentry_mode_freq(struct gk20a *g, u32 *freq);

s32 clk_domain_get_vf_tables(struct gk20a *g);
void clk_domain_free_vf_tables(struct gk20a *g);

s32 clk_domain_init_pmupstate(struct gk20a *g);
void clk_domain_free_pmupstate(struct gk20a *g);
s32 clk_pmu_clk_domains_load(struct gk20a *g);
s32 clk_domain_sw_setup(struct gk20a *g);
s32 clk_domain_pmu_setup(struct gk20a *g);

#endif /* NVGPU_CLK_DOMAIN_H */
