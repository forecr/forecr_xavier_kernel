/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

// general clock structures & definitions

#ifndef NVGPU_PMU_CLK_H
#define NVGPU_PMU_CLK_H

#include <nvgpu/types.h>
#include <nvgpu/boardobjgrpmask.h>
#include <nvgpu/boardobjgrp_e32.h>

/* Following include will be removed in further CL */
#include "../../../../common/pmu/boardobj/ucode_boardobj_inf.h"

/*!
 * Valid global VIN ID values
 */
#define	CTRL_CLK_VIN_ID_SYS			0x00000000U
#define	CTRL_CLK_VIN_ID_LTC			0x00000001U
#define	CTRL_CLK_VIN_ID_XBAR			0x00000002U
#define	CTRL_CLK_VIN_ID_GPC0			0x00000003U
#define	CTRL_CLK_VIN_ID_GPC1			0x00000004U
#define	CTRL_CLK_VIN_ID_GPC2			0x00000005U
#define	CTRL_CLK_VIN_ID_GPC3			0x00000006U
#define	CTRL_CLK_VIN_ID_GPC4			0x00000007U
#define	CTRL_CLK_VIN_ID_GPC5			0x00000008U
#define	CTRL_CLK_VIN_ID_GPCS			0x00000009U
#define	CTRL_CLK_VIN_ID_SRAM			0x0000000AU
#define	CTRL_CLK_VIN_ID_UNDEFINED		0x000000FFU
#define	CTRL_CLK_VIN_TYPE_DISABLED		0x00000000U
#define CTRL_CLK_VIN_TYPE_V20			0x00000002U

/* valid clock domain values */
#define CTRL_CLK_DOMAIN_MCLK			0x00000010U
#define CTRL_CLK_DOMAIN_HOSTCLK			0x00000020U
#define CTRL_CLK_DOMAIN_DISPCLK			0x00000040U
#define CTRL_CLK_DOMAIN_GPC2CLK			0x00010000U
#define CTRL_CLK_DOMAIN_XBAR2CLK		0x00040000U
#define CTRL_CLK_DOMAIN_SYS2CLK			0x00800000U
#define CTRL_CLK_DOMAIN_HUB2CLK			0x01000000U
#define CTRL_CLK_DOMAIN_UTILSCLK		0x00040000U
#define CTRL_CLK_DOMAIN_PWRCLK			0x00080000U
#define CTRL_CLK_DOMAIN_UPROCCLK		0x00080000U
#define CTRL_CLK_DOMAIN_NVDCLK			0x00100000U
#define CTRL_CLK_DOMAIN_PCIEGENCLK		0x00200000U
#define CTRL_CLK_DOMAIN_XCLK			0x04000000U
#define CTRL_CLK_DOMAIN_NVL_COMMON		0x08000000U
#define CTRL_CLK_DOMAIN_PEX_REFCLK		0x10000000U
#define CTRL_CLK_DOMAIN_GPCCLK			0x00000001U
#define CTRL_CLK_DOMAIN_XBARCLK			0x00000002U
#define CTRL_CLK_DOMAIN_SYSCLK			0x00000004U
#define CTRL_CLK_DOMAIN_HUBCLK			0x00000008U

#define CTRL_CLK_CLK_DOMAIN_CLIENT_MAX_DOMAINS	16

/*!
 * Enumeration of Clock Domains, shared across NVGPU and PMU.
 */
typedef u32 NV_PMU_CLK_CLKWHICH;
/*
 *  Try to get gpc2clk, mclk, sys2clk, xbar2clk work for Pascal
 *
 *  mclk is same for both
 *  gpc2clk is 17 for Pascal and 13 for Volta, making it 17
 *    as volta uses gpcclk
 *  sys2clk is 20 in Pascal and 15 in Volta.
 *    Changing for Pascal would break nvdclk of Volta
 *  xbar2clk is 19 in Pascal and 14 in Volta
 *    Changing for Pascal would break pwrclk of Volta
 */
#define CLKWHICH_DEFAULT			0
#define CLKWHICH_GPCCLK				1U
#define CLKWHICH_XBARCLK			2U
#define CLKWHICH_SYSCLK				3U
#define CLKWHICH_HUBCLK				4U
#define CLKWHICH_MCLK				5U
#define CLKWHICH_HOSTCLK			6U
#define CLKWHICH_DISPCLK			7U
#define CLKWHICH_XCLK				12U
#define CLKWHICH_XBAR2CLK			14U
#define CLKWHICH_SYS2CLK			15U
#define CLKWHICH_HUB2CLK			16U
#define CLKWHICH_GPC2CLK			17U
#define CLKWHICH_PWRCLK				19U
#define CLKWHICH_UPROCCLK			CLKWHICH_PWRCLK
#define CLKWHICH_NVDCLK				20U
#define CLKWHICH_PCIEGENCLK			26U

/* legacy clock domain values */
#define NV2080_CTRL_CLK_DOMAIN_LEGACY                   (31:0)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_UNDEFINED         (0x00000000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_GPCCLK            (0x00000001U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_XBARCLK           (0x00000002U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_SYSCLK            (0x00000004U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_HUBCLK            (0x00000008U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_MCLK              (0x00000010U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_HOSTCLK           (0x00000020U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_DISPCLK           (0x00000040U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_PCLK0             (0x00000080U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_PCLK1             (0x00000100U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_PCLK2             (0x00000200U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_PCLK3             (0x00000400U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_PCLK(i) \
		((1U << (i)) * NV2080_CTRL_CLK_DOMAIN_LEGACY_PCLK0)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_PCLK__SIZE_1      (0x00000004U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_GPCCLK_0          (0x00000800U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_GPC2CLK           (0x00001000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_LTC2CLK           (0x00002000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_XBAR2CLK          (0x00004000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_SYSCLK_N          NV2080_CTRL_CLK_DOMAIN_LEGACY_XBAR2CLK
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_SYS2CLK           (0x00008000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_HUB2CLK           (0x00010000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_SYSCLK_S          NV2080_CTRL_CLK_DOMAIN_LEGACY_HUB2CLK
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_GPCCLK_1          (0x00020000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_UTILSCLK          (0x00040000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_PWRCLK            (0x00080000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_UPROCCLK          NV2080_CTRL_CLK_DOMAIN_LEGACY_PWRCLK
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_NVDCLK            (0x00100000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_MSDCLK            NV2080_CTRL_CLK_DOMAIN_LEGACY_NVDCLK
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_PCIEGENCLK        (0x00200000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_XBARCLK_0         (0x04000000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_XBARCLK_1         (0x08000000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_SYSCLK_0          (0x10000000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_SYSCLK_1          (0x20000000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_NVDCLK_0          (0x40000000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_NVDCLK_1          (0x80000000U)

#define NV2080_CTRL_CLK_DOMAIN_LEGACY_VCLK0             (0x00400000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_VCLK1             (0x00800000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_VCLK2             (0x01000000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_VCLK3             (0x02000000U)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_VCLK(i) \
		((1U << (i)) * NV2080_CTRL_CLK_DOMAIN_LEGACY_VCLK0)
#define NV2080_CTRL_CLK_DOMAIN_LEGACY_VCLK__SIZE_1      (0x00000004U)

typedef u32 NV2080_CTRL_CLK_PUBLIC_DOMAIN;

/*!
 * Enumeration of public clock domains. Of type
 *  NV2080_CTRL_CLK_PUBLIC_DOMAIN.
 */
#define NV2080_CTRL_CLK_PUBLIC_DOMAIN_GRAPHICS  (0x00000001U)
#define NV2080_CTRL_CLK_PUBLIC_DOMAIN_PROCESSOR (0x00000002U)
#define NV2080_CTRL_CLK_PUBLIC_DOMAIN_MEMORY    (0x00000004U)
#define NV2080_CTRL_CLK_PUBLIC_DOMAIN_VIDEO     (0x00000008U)

//
// Special value, not to be used in NV2080_CTRL_CMD_CLK_GET_PUBLIC_DOMAINS,
// NV2080_CTRL_CMD_CLK_GET_PUBLIC_DOMAIN_INFO, NV2080_CTRL_CMD_CLK_GET_PUBLIC_DOMAIN_INFO_V2
//
#define NV2080_CTRL_CLK_PUBLIC_DOMAIN_INVALID   (0xFFFFFFFFU)

/*!
 * CLK_DOMAIN CPMU_CLOCK_ID Type Definition
 */
typedef u8 NV2080_CTRL_CLK_CLK_DOMAIN_CPMU_CLOCK_ID;
#define NV2080_CTRL_CLK_CLK_DOMAIN_CPMU_CLOCK_ID_INVALID 0x0ffU

#define NV2080_CTRL_CLK_CLK_DOMAIN_CLIENT_MAX_DOMAINS          16U

/*
 * Mask of all GPC VIN IDs supported by NVGPU
 */
#define CTRL_CLK_LUT_NUM_ENTRIES_MAX		128U
#define CTRL_CLK_LUT_NUM_ENTRIES_GV10x		128U
#define CTRL_CLK_LUT_NUM_ENTRIES_GP10x		100U

/*
 * The Minimum resolution of frequency which is supported
 */
#define FREQ_STEP_SIZE_MHZ			15U

struct gk20a;
struct clk_avfs_fll_objs;
struct nvgpu_clk_domains;
struct nvgpu_clk_progs;
struct nvgpu_clk_vf_points;
struct nvgpu_clk_mclk_state;
struct nvgpu_clk_slave_freq;
struct nvgpu_pmu_perf_change_input_clk_info;
struct clk_vin_device;
struct nvgpu_clk_domain;
struct nvgpu_clk_arb;
struct nvgpu_clk_pmupstate;

struct ctrl_clk_clk_domain_list_item_v1 {
	u32  clk_domain;
	u32  clk_freq_khz;
	u8   regime_id;
	u8   source;
};

struct ctrl_clk_clk_domain_list {
	u8 num_domains;
	struct ctrl_clk_clk_domain_list_item_v1
		clk_domains[CTRL_BOARDOBJ_MAX_BOARD_OBJECTS];
};

struct nvgpu_clk_slave_freq{
	u16 gpc_mhz;
	u16 sys_mhz;
	u16 xbar_mhz;
	u16 host_mhz;
	u16 nvd_mhz;
};

struct nvgpu_clk_vf_point_5x {
	u16 gpc_mhz;
	u16 sys_mhz;
	u16 nvd_mhz;
	u16 uproc_mhz;
	u32 gpc_volt_uv;
	u32 gpm_volt_uv;
	u32 clk_domain_mask;
};

struct nvgpu_clk_vf_table_entry {
	u16 freq_mhz;
	u32 voltage_uv;
};

int clk_get_fll_clks_per_clk_domain(struct gk20a *g,
		struct nvgpu_clk_slave_freq *setfllclk);
s32 nvgpu_pmu_clk_resume_clks(struct gk20a *g);
int nvgpu_pmu_clk_domain_freq_to_volt(struct gk20a *g, u16 clkdomain_idx,
	u32 *pclkmhz, u32 *pvoltuv);
int nvgpu_pmu_clk_domain_get_from_index(struct gk20a *g, u32 *domain, u32 index);
s32 nvgpu_pmu_clk_domain_get_index_by_domain(struct gk20a *g, u32 domain, u32 *idx);
int nvgpu_pmu_clk_pmu_setup(struct gk20a *g);
int nvgpu_pmu_clk_sw_setup(struct gk20a *g);
int nvgpu_pmu_clk_init(struct gk20a *g);
void nvgpu_pmu_clk_deinit(struct gk20a *g);
u8 nvgpu_pmu_clk_fll_get_fmargin_idx(struct gk20a *g);
int nvgpu_clk_arb_find_slave_points(struct nvgpu_clk_arb *arb,
	struct nvgpu_clk_slave_freq *vf_point);
int nvgpu_clk_vf_point_cache(struct gk20a *g);
int nvgpu_clk_domain_volt_to_freq(struct gk20a *g, u16 clkdomain_idx,
	u32 *pclkmhz, u32 *pvoltuv);
u16 nvgpu_pmu_clk_fll_get_min_max_freq(struct gk20a *g);
s32 nvgpu_pmu_clk_domains_fll_set_regime_50(struct gk20a *g,
	u8 regime, u32 api_clk_domain);
u32 nvgpu_pmu_clk_fll_get_lut_step_size(struct nvgpu_clk_pmupstate *pclk);
int nvgpu_pmu_clk_domain_get_f_points(struct gk20a *g,
	u32 clkapidomain,
	u32 *pfpointscount,
	u16 *pfreqpointsinmhz);
u8 nvgpu_pmu_clk_domain_update_clk_info(struct gk20a *g,
		struct ctrl_clk_clk_domain_list *clk_list);
void clk_set_p0_clk_per_domain(struct gk20a *g, u8 *gpcclk_domain,
		u32 *gpcclk_clkmhz,
		struct nvgpu_clk_slave_freq *vf_point,
		struct nvgpu_pmu_perf_change_input_clk_info *change_input);
int nvgpu_clk_find_vf_point_for_gpc_rate(struct gk20a *g, u32 gpc_freq_mhz,
	struct nvgpu_clk_vf_point_5x *vf_point);
int nvgpu_pmu_clk_set_boot_clk(struct gk20a *g);
int nvgpu_pmu_clk_restore_saved_state(struct gk20a *g);
unsigned long nvgpu_pmu_clk_mon_init_domains(struct gk20a *g);
u32 nvgpu_pmu_clk_mon_init_domains_5x(struct gk20a *g);
int set_clock_rate_via_pmu_rpc(struct gk20a *g,
	u32 requested_rate_mhz, u32 clk_domain, u32 mode);

#define NVGPU_CLK_VF_POINTS_MAX_OBJECTS 2048

/* NVGPU boardobj super class equivalent */
struct nvgpu_clk_boardobj {
	u8 type;
	u16 index;
	u8 reserved;
};

/* NVGPU boardobjgrp mask equivalent */
struct nvgpu_clk_boardobjgrp_mask {
	u32 objCount;
	u32 data[NVGPU_CLK_VF_POINTS_MAX_OBJECTS / 32];
};

/* NVGPU boardobjgrp E2048 super class equivalent */
struct nvgpu_clk_boardobjgrp_e2048 {
	struct nvgpu_clk_boardobj super;
	struct nvgpu_clk_boardobjgrp_mask objMask;
	u32 objCount;
};

/* VF Point Status Type-specific structures */
struct nvgpu_clk_vf_point_status_30 {
	struct nvgpu_clk_vf_pair {
		u16 freq_mhz;
		u32 voltage_uv;
	} pair;
};

struct nvgpu_clk_vf_point_base_vf_tuple {
	u16 freq_mhz[4];  /* Max 4 frequency tuples */
	u32 voltage_uv;
};

struct nvgpu_clk_vf_point_vf_tuple {
	u16 freq_mhz;
	u32 voltage_uv;
};

struct nvgpu_clk_vf_point_status_35 {
	struct nvgpu_clk_vf_point_base_vf_tuple base_vf_tuple;
	struct nvgpu_clk_vf_point_vf_tuple offseted_vf_tuple[4];
};

struct nvgpu_clk_vf_point_status_35_freq {
	struct nvgpu_clk_vf_point_base_vf_tuple base_vf_tuple;
	struct nvgpu_clk_vf_point_vf_tuple offseted_vf_tuple[4];
};

struct nvgpu_clk_vf_point_status_35_volt_pri {
	struct nvgpu_clk_vf_point_base_vf_tuple base_vf_tuple;
	struct nvgpu_clk_vf_point_vf_tuple offseted_vf_tuple[4];
};

struct nvgpu_clk_vf_point_status_35_volt_sec {
	struct nvgpu_clk_vf_point_base_vf_tuple base_vf_tuple;
	struct nvgpu_clk_vf_point_vf_tuple offseted_vf_tuple[4];
};

struct nvgpu_clk_vf_point_status_50_freq {
	struct nvgpu_clk_vf_point_base_vf_tuple active_base_vf_tuple;
	struct nvgpu_clk_vf_point_base_vf_tuple picked_base_vf_tuple;
	struct nvgpu_clk_vf_point_vf_tuple active_offseted_vf_tuple[4];
	struct nvgpu_clk_vf_point_vf_tuple active_offset_vf_tuple[4];
};

struct nvgpu_clk_vf_point_status_50_volt_pri {
	struct nvgpu_clk_vf_point_base_vf_tuple active_base_vf_tuple;
	struct nvgpu_clk_vf_point_base_vf_tuple picked_base_vf_tuple;
	struct nvgpu_clk_vf_point_vf_tuple active_offseted_vf_tuple[4];
	struct nvgpu_clk_vf_point_vf_tuple active_offset_vf_tuple[4];
};

struct nvgpu_clk_vf_point_status_50_volt_sec {
	struct nvgpu_clk_vf_point_base_vf_tuple active_base_vf_tuple;
	struct nvgpu_clk_vf_point_base_vf_tuple picked_base_vf_tuple;
	struct nvgpu_clk_vf_point_vf_tuple active_offseted_vf_tuple[4];
	struct nvgpu_clk_vf_point_vf_tuple active_offset_vf_tuple[4];
};

/* Individual VF Point Status Structure - equivalent to NV2080_CTRL_CLK_CLK_VF_POINT_STATUS */
struct nvgpu_clk_vf_point_status {
	struct nvgpu_clk_boardobj super;
	u8 type;
	u32 voltage_uv;  /* Voltage (uV) for this VF point */
	u16 freq_mhz;    /* Frequency (MHz) for this VF point */

	/* Type-specific data union */
	union {
		struct nvgpu_clk_vf_point_status_30          v30;
		struct nvgpu_clk_vf_point_status_35          v35;
		struct nvgpu_clk_vf_point_status_35_freq     v35_freq;
		struct nvgpu_clk_vf_point_status_35_volt_pri v35_volt_pri;
		struct nvgpu_clk_vf_point_status_35_volt_sec v35_volt_sec;
		struct nvgpu_clk_vf_point_status_50_freq     v50_freq;
		struct nvgpu_clk_vf_point_status_50_volt_pri v50_volt_pri;
		struct nvgpu_clk_vf_point_status_50_volt_sec v50_volt_sec;
	} data;
};

/* Secondary VF Points Status Structure */
struct nvgpu_clk_vf_points_sec_status {
	struct nvgpu_clk_boardobjgrp_e2048 super;
	struct nvgpu_clk_vf_point_status vf_points[NVGPU_CLK_VF_POINTS_MAX_OBJECTS];
};

/* Main VF Points Status Structure - equivalent to NV2080_CTRL_CLK_CLK_VF_POINTS_STATUS */
struct nvgpu_clk_vf_points_status {
	struct nvgpu_clk_boardobjgrp_e2048 super;
	struct nvgpu_clk_vf_point_status vf_points[NVGPU_CLK_VF_POINTS_MAX_OBJECTS];
	struct nvgpu_clk_vf_points_sec_status sec;
};

/* VF points status functions - using NVGPU-specific structures */
int nvgpu_clk_vf_points_get_status(struct gk20a *g,
	struct nvgpu_clk_vf_points_status *vf_points_status);
int nvgpu_clk_vf_points_get_count(struct gk20a *g, u32 *num_vf_points);
int nvgpu_clk_vf_points_get_vf_point_data(struct gk20a *g, u32 vf_point_index,
	struct nvgpu_clk_vf_point_status *vf_point_status);

#endif /* NVGPU_PMU_CLK_H */
