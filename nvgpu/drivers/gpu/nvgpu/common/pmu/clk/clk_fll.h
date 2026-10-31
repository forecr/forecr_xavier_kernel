/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_CLK_FLL_H
#define NVGPU_CLK_FLL_H

/*
 * Valid NAFLL ID values for VBIOS NAFLL table version 2X and HAL ID 3.
 * Used for NAFLL device type 35.
 */
#define NV_VBIOS_HAL_V3_NAFLL_DEVICE_ID_SYS			(0x00)
#define NV_VBIOS_HAL_V3_NAFLL_DEVICE_ID_GPC0		(0x01)
#define NV_VBIOS_HAL_V3_NAFLL_DEVICE_ID_GPC1		(0x02)
#define NV_VBIOS_HAL_V3_NAFLL_DEVICE_ID_GPC2		(0x03)
#define NV_VBIOS_HAL_V3_NAFLL_DEVICE_ID_GPC3		(0x04)
#define NV_VBIOS_HAL_V3_NAFLL_DEVICE_ID_GPCS		(0x05)
#define NV_VBIOS_HAL_V3_NAFLL_DEVICE_ID_NVD			(0x06)
#define NV_VBIOS_HAL_V3_NAFLL_DEVICE_ID_UPROC		(0x07)

/* Maximum NAFLL devices we support */
#define NV_VBIOS_NAFLL_MAX_DEVICES					(0x08)

/*!
 * Valid global NAFLL ID values.
 */
#define NV2080_CTRL_CLK_NAFLL_ID_SYS				(0x00000000U)
#define NV2080_CTRL_CLK_NAFLL_ID_LTC				(0x00000001U)
#define NV2080_CTRL_CLK_NAFLL_ID_XBAR				(0x00000002U)
#define NV2080_CTRL_CLK_NAFLL_ID_GPC0				(0x00000003U)
#define NV2080_CTRL_CLK_NAFLL_ID_GPC1				(0x00000004U)
#define NV2080_CTRL_CLK_NAFLL_ID_GPC2				(0x00000005U)
#define NV2080_CTRL_CLK_NAFLL_ID_GPC3				(0x00000006U)
#define NV2080_CTRL_CLK_NAFLL_ID_GPCS				(0x00000009U)
#define NV2080_CTRL_CLK_NAFLL_ID_NVD				(0x0000000AU)
#define NV2080_CTRL_CLK_NAFLL_ID_UPROC				(0x0000001AU)
#define NV2080_CTRL_CLK_NAFLL_ID_MAX				(0x0000001BU)  // MUST BE LAST

#define NV2080_CTRL_CLK_NAFLL_ID_UNDEFINED			(0x000000FFU)
#define NV2080_CTRL_CLK_NAFLL_MASK_UNDEFINED		(0x00000000U)

struct gk20a;
struct fll_device;

struct clk_avfs_fll_objs {
	struct boardobjgrp_e32 super;
	struct boardobjgrpmask_e32 lut_prog_master_mask;
	u32 lut_step_size_uv;
	u32 lut_min_voltage_uv;
	u8 lut_num_entries;
	u16 max_min_freq_mhz;
	u8 freq_margin_vfe_idx;
};

typedef int fll_lut_broadcast_slave_register(struct gk20a *g,
	struct clk_avfs_fll_objs *pfllobjs,
	struct fll_device *pfll,
	struct fll_device *pfll_slave);

struct fll_device {
	struct pmu_board_obj super;
	u8 id;
	u8 mdiv;
	u16 input_freq_mhz;
	u32 clk_domain;
	u8 vin_idx_logic;
	u8 vin_idx_sram;
	u8 rail_idx_for_lut;
	struct nv_pmu_clk_lut_device_desc lut_device;
	struct nv_pmu_clk_regime_desc regime_desc;
	u8 min_freq_vfe_idx;
	u8 freq_ctrl_idx;
	u8 target_regime_id_override;
	bool b_skip_pldiv_below_dvco_min;
	bool b_dvco_1x;
	struct boardobjgrpmask_e32 lut_prog_broadcast_slave_mask;
	fll_lut_broadcast_slave_register *lut_broadcast_slave_register;
};

/*
 * V2.0 VBIOS Structure containing the global properties and boardobjgrp for NAFLL devices
 */
struct clk_avfs_fll_objs_v20 {
	struct boardobjgrp_e32 super;
	struct boardobjgrpmask_e32 lut_prog_master_mask;
	u32 lut_step_size_uv;
	u32 lut_min_voltage_uv;
	u8 lut_num_entries;
	u16 max_min_freq_mhz;
	struct boardobjgrpmask_e32 unicast_nafll_dev_mask;
	struct boardobjgrpmask_e32 broadcast_nafll_dev_mask;
	bool b_die1_all_gpc_floorswept;
};

/*
 * NAFLL device for V2.0 VBIOS derived data
 */
struct nafll_device {
	struct pmu_board_obj super;
	u8 id;
	u8 mdiv;
	u16 input_ref_freq_mhz;
	u16 input_ref_div_val;
	u32 clk_domain;
	u8 adc_idx_logic;
	u8 adc_idx_sram;
	u8 rail_idx_for_lut;
	struct nv_pmu_clk_lut_device_desc lut_device;
	struct nv_pmu_clk_regime_desc regime_desc;
	u16 dvco_min_freq_vfe_idx;
	bool b_dvco_1x;
	u8 freq_ctrl_idx;
	bool b_skip_pldiv_below_dvco_min;
	bool b_multistep_pldiv_switch_en;
	struct boardobjgrpmask_e32 lut_prog_bcast_secndary_mask;
};

/*
 * Structure describing an individual NAFLL Device
 */
struct nafll_device_model_10 {
	struct nafll_device super;
	struct boardobjgrp model10;
};

/*
 * Extends nafll_device providing attributes specific to V3X (common to V30 and V35).
 */
struct naffl_device_v3x {
	struct nafll_device_model_10 super;
};

/*
 * Extends nafll_device providing attributes specific to V35 NAFLL Device.
 */
struct nafll_device_v35 {
	struct naffl_device_v3x super;
	bool b_hw_pldiv_en;
};

s32 clk_fll_init_pmupstate(struct gk20a *g);
void clk_fll_free_pmupstate(struct gk20a *g);
s32 clk_fll_sw_setup(struct gk20a *g);
s32 clk_fll_pmu_setup(struct gk20a *g);
#endif /* NVGPU_CLK_FLL_H */
