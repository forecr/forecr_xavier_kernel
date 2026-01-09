/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLK_VIN_H
#define NVGPU_CLK_VIN_H

#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <common/pmu/boardobj/boardobj.h>

#define EXT_SHIFT(name)			(name##_SHIFT)
#define EXT_LEFT_SHIFT(name)	(31U - (name##_MSB))
/* sign-extend the extracted 2's complement value */
#define EXT_VAL_SIGNED(typ, v, bits) ((typ)(((s32)((v) << EXT_LEFT_SHIFT(bits))) >> \
				(EXT_LEFT_SHIFT(bits) + EXT_SHIFT(bits))))

struct clk_adc_device;

typedef u32 vin_device_state_load(struct gk20a *g,
		struct nvgpu_clk_pmupstate *clk, struct clk_vin_device *pdev);

typedef u32 adc_device_state_load(struct gk20a *g,
		struct nvgpu_clk_pmupstate *clk, struct clk_adc_device *pdev);

typedef u32 adc_device_cal_fuse_rev_check(struct gk20a *g,
		struct nvgpu_clk_pmupstate *clk, struct clk_adc_device *pdev,
		bool *update_adc_cal_settings, bool *update_adc_code_errors);

struct clk_vin_device {
	struct pmu_board_obj super;
	u8 id;
	u8 volt_domain;
	u8 volt_domain_vbios;
	u8 por_override_mode;
	u8 override_mode;
	u32 flls_shared_mask;
	vin_device_state_load  *state_load;
};

struct vin_device_v20 {
	struct clk_vin_device super;
	struct ctrl_clk_vin_device_info_data_v20 data;
};

struct nvgpu_avfsvinobjs {
	struct boardobjgrp_e32 super;
	u8 calibration_rev_vbios;
	u8 calibration_rev_fused;
	u8 version;
	bool vin_is_disable_allowed;
	struct boardobjgrpmask_e32 naffl_adc_devices_mask;
};

struct clk_adc_device {
	struct pmu_board_obj super;
	u8 id;
	u8 volt_domain;
	u8 volt_domain_vbios;
	u8 por_override_mode;
	u8 override_mode;
	u32 naflls_shared_mask;
	bool dynamic_cal;
	u8 logical_api_id;
	bool devinit_enabled;
	// Interfaces
	adc_device_state_load  *state_load;
	adc_device_cal_fuse_rev_check *cal_fuse_rev_check;
};

struct adc_device_model_10 {
	struct clk_adc_device super;
	struct boardobjgrp model10;
};

struct avfs_adcobjs_grp_model_10 {
	struct nvgpu_avfsvinobjs super;
	struct boardobjgrp model10;
};

struct avfs_adcobjs_model_10 {
	struct avfs_adcobjs_grp_model_10 super;
	struct avfs_adcobjs_grp_model_10 default_objects;
};

struct nvgpu_avfs_adcobjs_v20 {
	struct avfs_adcobjs_model_10 super;
	struct nv_ctrl_clk_adc_devices_info_data_v20 data;
};

struct ctrl_boardobj_interface {
	u8 rsvd;
};

struct ctrl_boardobj_info_interface_model {
	struct ctrl_boardobj_interface super;
};

struct ctrl_boardobj_info_iface_model_10 {
	struct ctrl_boardobj_info_interface_model super;
};

struct ctrl_clk_adc_device_info_data_model_10 {
	struct ctrl_boardobj_info_iface_model_10 model10;
};

struct ctrl_clk_adc_device_info_data_v30 {
	struct ctrl_clk_adc_device_info_data_model_10 super;
	s8 offset;
	s8 gain;
	s8 course_offset;
	s8 course_gain;
	s8 low_temp_low_volt_err;	/* read from fuse */
	s8 low_temp_hi_volt_err;	/* read from fuse */
	s8 hi_temp_low_volt_err;	/* read from fuse */
	s8 hi_temp_hi_volt_err;		/* read from fuse */
	s8 adc_code_correction_offset;
};

struct ctrl_clk_adc_device_v30 {
	struct adc_device_model_10 super;
	struct ctrl_clk_adc_device_info_data_v30 data;
};

s32 clk_vin_init_pmupstate(struct gk20a *g);
void clk_vin_free_pmupstate(struct gk20a *g);
s32 clk_pmu_vin_load(struct gk20a *g);
s32 clk_vin_sw_setup(struct gk20a *g);
s32 clk_vin_pmu_setup(struct gk20a *g);
#endif /* NVGPU_CLK_VIN_H */
