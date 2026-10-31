/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_VOLT_DEV_H
#define NVGPU_VOLT_DEV_H

#include <nvgpu/boardobjgrp.h>
#include <common/pmu/boardobj/boardobj.h>

#include "ucode_volt_inf.h"

#define VOLTAGE_TABLE_MAX_ENTRIES_ONE	1U
#define VOLTAGE_TABLE_MAX_ENTRIES	256U
#define VOLTAGE_DESCRIPTOR_TABLE_ENTRY_INVALID	0xFFU
#define VOLT_DEV_PWM_VOLTAGE_STEPS_INVALID	0U
#define VOLT_DEV_PWM_VOLTAGE_STEPS_DEFAULT	1U

#define VBIOS_VOLTAGE_DEVICE_1X_ENTRY_SIZE_18 	0X00000018
#define VBIOS_VOLTAGE_DEVICE_1X_ENTRY_SIZE_20 	0X00000020

#define GPIO_FUNC_SKIPENTRY 			0xFF

#define CTRL_VOLT_DEVICE_PSV_GATE_VOLTAGE_UV_INVALID 0xFFFFFFFF

struct voltage_device {
	struct pmu_board_obj super;
	u8 volt_domain;
	u8 i2c_dev_idx;
	u32 switch_delay_us;
	u32 num_entries;
	struct voltage_device_entry *pentry[VOLTAGE_TABLE_MAX_ENTRIES];
	struct voltage_device_entry *pcurr_entry;
	u8 deep_idle_vdt_idx;
	u8 gcx_state_retention_vdt_idx;
	u8 operation_type;
	u32 voltage_min_uv;
	u32 voltage_max_uv;
	u32 volt_step_uv;
	u16 power_up_settle_time_us;
	u16 power_down_settle_time_us;
};

struct voltage_device_entry {
	u32  voltage_uv;
};

/*!
 * Extends VOLTAGE_DEVICE providing attributes specific to PWM controllers.
 */
struct voltage_device_pwm {
	struct voltage_device super;
	s32 voltage_base_uv;
	s32 voltage_offset_scale_uv;
	enum nv_pmu_pmgr_pwm_source source;
	u32 raw_period;
	u8 volt_en_gpio_pin;
	u32 gate_voltage_uv;
	enum nv_pmu_pmgr_pwm_source clvc_offset_source;
	u8 b_clvc_hw_offset_support;
	enum nv_pmu_pmgr_pwm_source accumulator_pwm_sources[CTRL_VOLT_PWM_ACCUMULATOR_TYPE_COUNT];
};

struct voltage_device_pwm_entry {
	struct voltage_device_entry  super;
	u32 duty_cycle;
};
/* PWM end */

int volt_dev_sw_setup(struct gk20a *g);
int volt_dev_pmu_setup(struct gk20a *g);
void nvgpu_pmu_volt_rpc_handler(struct gk20a *g, struct nv_pmu_rpc_header *rpc);

#endif /* NVGPU_VOLT_DEV_H */
