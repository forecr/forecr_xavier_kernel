/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMGR_PWRDEV_H
#define NVGPU_PMGR_PWRDEV_H

#include <nvgpu/boardobjgrp_e32.h>
#include <nvgpu/pmu/pmuif/nvgpu_cmdif.h>
#include <nvgpu/pmu/pmuif/ctrlpmgr.h>
#include <common/pmu/boardobj/boardobj.h>

#define  PWRDEV_I2CDEV_DEVICE_INDEX_NONE  (0xFF)

#define  PWR_DEVICE_PROV_NUM_DEFAULT                                           1

struct pwr_device {
	struct pmu_board_obj super;
	u8 power_rail;
	u8 i2c_dev_idx;
	bool bIs_inforom_config;
	u32 power_corr_factor;
};

struct pwr_devices {
	struct boardobjgrp_e32 super;
};

struct pwr_device_ina3221 {
	struct pwr_device super;
	struct ctrl_pmgr_pwr_device_info_rshunt
		r_shuntm_ohm[NV_PMU_PMGR_PWR_DEVICE_INA3221_CH_NUM];
	u16 configuration;
	u16 mask_enable;
	u8 gpio_function;
	u16 curr_correct_m;
	s16 curr_correct_b;
} ;

int pmgr_device_sw_setup(struct gk20a *g);

#endif /* NVGPU_PMGR_PWRDEV_H */
