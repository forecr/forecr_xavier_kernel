/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMUIF_VOLT_H
#define NVGPU_PMUIF_VOLT_H

#include <nvgpu/flcnif_cmn.h>

struct nv_pmu_volt_volt_rail_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
};

struct nv_pmu_volt_volt_rail_3x_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
	u8 volt_domain_hal;
	struct ctrl_boardobjgrp_mask_e32 volt_rails_physical_rail_mask;
	struct ctrl_boardobjgrp_mask_e32 volt_rails_logical_rail_mask;
};

struct nv_pmu_volt_volt_rail_boardobj_set {

	struct nv_pmu_boardobj super;
	u8 rel_limit_vfe_equ_idx;
	u8 alt_rel_limit_vfe_equ_idx;
	u8 ov_limit_vfe_equ_idx;
	u8 vmin_limit_vfe_equ_idx;
	u8 volt_margin_limit_vfe_equ_idx;
	u8 pwr_equ_idx;
	u8 volt_dev_idx_default;
	u8 volt_dev_idx_ipc_vmin;
	u8 volt_scale_exp_pwr_equ_idx;
	struct ctrl_boardobjgrp_mask_e32 vin_dev_mask;
	struct ctrl_boardobjgrp_mask_e32 volt_dev_mask;
	s32 volt_delta_uv[CTRL_VOLT_RAIL_VOLT_DELTA_MAX_ENTRIES];
};

union nv_pmu_volt_volt_rail_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_volt_volt_rail_boardobj_set super;
};

/*
 * For the below structures, followed the naming format from RM
 * so that it will be easy to compare the code with RM.
 * For PMU board objects, this helps in maintaining the same alignment as RM
 */
struct nv_pmu_boardobj_interface {
	u8 rsvd;
};

struct nv_pmu_boardobj_iface_model {
	struct nv_pmu_boardobj_interface super;
};

struct nv_pmu_boardobj_iface_model_10 {
	struct nv_pmu_boardobj_iface_model super;
};

struct nv_pmu_volt_volt_rail_boardobj_set_2x {
	struct nv_pmu_boardobj super;
	struct nv_pmu_boardobj_iface_model_10 model10;
	u16 rel_limit_vfe_equ_idx;
	u16 alt_rel_limit_vfe_equ_idx;
	u16 ov_limit_vfe_equ_idx;
	u16 vmin_limit_vfe_equ_idx;
	u16 volt_margin_limit_vfe_equ_idx;
	u8 leakage_pwr_equ_idx;
	u8 volt_dev_idx_default;
	u8 volt_dev_idx_ipc_vmin;
	u8 dynamic_pwr_equ_idx;
	struct ctrl_boardobjgrp_mask_e32 adc_dev_mask;
	struct ctrl_boardobjgrp_mask_e32 volt_dev_mask;
	struct ctrl_boardobjgrp_mask_e32 clk_domains_prog_mask;
	s32 volt_delta_uv[CTRL_VOLT_RAIL_VOLT_DELTA_MAX_ENTRIES];
	u32 vbios_boot_voltage_uv;
	u8 ba_scaling_pwr_eqn_idx;
	u8 domain;
	bool b_hw_clvc_en;
};

struct nv_pmu_volt_volt_rail_model_10_boardobj_set {
	struct nv_pmu_volt_volt_rail_boardobj_set_2x super;
	struct nv_pmu_boardobj_iface_model_10 model10;
};

struct nv_pmu_volt_volt_rail_2x_boardobj_set {
	struct nv_pmu_volt_volt_rail_model_10_boardobj_set super;
};

struct nv_pmu_volt_volt_rail_2x_physical_boardobj_set {
	struct nv_pmu_volt_volt_rail_2x_boardobj_set super;
	struct ctrl_boardobjgrp_mask_e32 volt_device_mask;
	struct ctrl_boardobjgrp_mask_e32 volt_threshold_controller_mask;
	u16 logical_rail_idx;
};

union nv_pmu_volt_volt_rail_3x_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_volt_volt_rail_2x_physical_boardobj_set super;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E32(volt, volt_rail);
NV_PMU_BOARDOBJ_GRP_SET_MAKE_E32(volt, volt_rail_3x);

/* ------------ VOLT_DEVICE's GRP_SET defines and structures ------------ */

struct nv_pmu_volt_volt_device_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
};

struct nv_pmu_volt_volt_device_boardobj_set {
	struct nv_pmu_boardobj super;
	struct nv_pmu_boardobj_iface_model_10 model10;
	u32 switch_delay_us;
	u32 voltage_min_uv;
	u32 voltage_max_uv;
	u32 volt_step_uv;
	u16 power_up_settle_time_us;
	u16 power_down_settle_time_us;
};

struct nv_pmu_volt_volt_device_vid_boardobj_set {
	struct nv_pmu_volt_volt_device_boardobj_set super;
	s32 voltage_base_uv;
	s32 voltage_offset_scale_uv;
	u8 gpio_pin[CTRL_VOLT_VOLT_DEV_VID_VSEL_MAX_ENTRIES];
	u8 vsel_mask;
};

struct nv_pmu_volt_volt_device_pwm_boardobj_set {
	struct nv_pmu_volt_volt_device_boardobj_set super;
	u32 raw_period;
	s32 voltage_base_uv;
	s32 voltage_offset_scale_uv;
	enum nv_pmu_pmgr_pwm_source pwm_source;
	u8 volt_en_gpio_pin;
	u32 gate_voltage_uv;
	enum nv_pmu_pmgr_pwm_source clvc_offset_source;
	u8 b_clvc_hw_offset_support;
	enum nv_pmu_pmgr_pwm_source accumulator_pwm_sources[CTRL_VOLT_PWM_ACCUMULATOR_TYPE_COUNT];
};

union nv_pmu_volt_volt_device_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_volt_volt_device_boardobj_set super;
	struct nv_pmu_volt_volt_device_vid_boardobj_set vid;
	struct nv_pmu_volt_volt_device_pwm_boardobj_set pwm;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E32(volt, volt_device);

/* ------------ VOLT_POLICY's GRP_SET defines and structures ------------ */
struct nv_pmu_volt_volt_policy_boardobjgrp_set_header {
	struct nv_pmu_boardobjgrp_e32 super;
	u8 perf_core_vf_seq_policy_idx;
};

struct nv_pmu_volt_volt_policy_boardobj_set {
	struct nv_pmu_boardobj super;
	struct nv_pmu_boardobj_iface_model_10 model10;
};
struct nv_pmu_volt_volt_policy_sr_boardobj_set {
	struct nv_pmu_volt_volt_policy_boardobj_set super;
	u8 rail_idx;
};

struct nv_pmu_volt_volt_policy_multi_rail_boardobj_set {
	struct nv_pmu_volt_volt_policy_boardobj_set super;
	struct ctrl_boardobjgrp_mask_e32 volt_rail_mask;
};

struct nv_pmu_volt_volt_policy_sr_multi_step_boardobj_set {
	struct nv_pmu_volt_volt_policy_sr_boardobj_set super;
	u16 inter_switch_delay_us;
	u32 ramp_up_step_size_uv;
	u32 ramp_down_step_size_uv;
};

struct nv_pmu_volt_volt_policy_splt_r_boardobj_set {
	struct nv_pmu_volt_volt_policy_boardobj_set super;
	u8 rail_idx_master;
	u8 rail_idx_slave;
	u8 delta_min_vfe_equ_idx;
	u8 delta_max_vfe_equ_idx;
	s32 offset_delta_min_uv;
	s32 offset_delta_max_uv;
};

struct nv_pmu_volt_volt_policy_srms_boardobj_set {
	struct nv_pmu_volt_volt_policy_splt_r_boardobj_set super;
	u16 inter_switch_delayus;
};

/* sr - > single_rail */
struct nv_pmu_volt_volt_policy_srss_boardobj_set {
	struct nv_pmu_volt_volt_policy_splt_r_boardobj_set super;
};

union nv_pmu_volt_volt_policy_boardobj_set_union {
	struct nv_pmu_boardobj obj;
	struct nv_pmu_volt_volt_policy_boardobj_set super;
	struct nv_pmu_volt_volt_policy_sr_boardobj_set single_rail;
	struct nv_pmu_volt_volt_policy_sr_multi_step_boardobj_set
		single_rail_ms;
	struct nv_pmu_volt_volt_policy_splt_r_boardobj_set split_rail;
	struct nv_pmu_volt_volt_policy_srms_boardobj_set
			split_rail_m_s;
	struct nv_pmu_volt_volt_policy_srss_boardobj_set
			split_rail_s_s;
	struct nv_pmu_volt_volt_policy_multi_rail_boardobj_set
		multi_rail;
};

NV_PMU_BOARDOBJ_GRP_SET_MAKE_E32(volt, volt_policy);

/* ----------- VOLT_RAIL's GRP_GET_STATUS defines and structures ----------- */
struct nv_pmu_volt_volt_rail_boardobjgrp_get_status_header {
	struct nv_pmu_boardobjgrp_e32 super;
};

struct nv_pmu_volt_volt_rail_3x_boardobjgrp_get_status_header {
	struct nv_pmu_boardobjgrp_e32 super;
};

struct nv_pmu_volt_volt_rail_boardobj_get_status {
	struct nv_pmu_boardobj_query super;
	u32 curr_volt_defaultu_v;
	u32 rel_limitu_v;
	u32 alt_rel_limitu_v;
	u32 ov_limitu_v;
	u32 max_limitu_v;
	u32 vmin_limitu_v;
	s32 volt_margin_limitu_v;
	u32 rsvd;
};

struct nv_pmu_volt_volt_rail_boardobj_get_status_3x {
	struct nv_pmu_boardobj_query super;
	u32 curr_volt_defaultu_v;
	u32 sw_last_target_voltu_v;
	u32 rel_limitu_v;
	u32 alt_rel_limitu_v;
	u32 ov_limitu_v;
	u32 max_limitu_v;
	u32 vmin_limitu_v;
	s32 volt_margin_limitu_v;
	u32 volt_min_noise_unawareu_v;
	u32 curr_volt_sensedu_v;
	u8 rail_action;
	s32 clvc_offsetu_v;
	u32 hw_target_voltageu_v[CTRL_VOLT_PWM_ACCUMULATOR_TYPE_COUNT];
};

union nv_pmu_volt_volt_rail_boardobj_get_status_union {
	struct nv_pmu_boardobj_query obj;
	struct nv_pmu_volt_volt_rail_boardobj_get_status super;
};

union nv_pmu_volt_volt_rail_3x_boardobj_get_status_union {
	struct nv_pmu_boardobj_query obj;
	struct nv_pmu_volt_volt_rail_boardobj_get_status_3x super;
};

NV_PMU_BOARDOBJ_GRP_GET_STATUS_MAKE_E32(volt, volt_rail);
NV_PMU_BOARDOBJ_GRP_GET_STATUS_MAKE_E32(volt, volt_rail_3x);

#define NV_PMU_VOLT_CMD_ID_BOARDOBJ_GRP_SET			(0x00000000U)
#define NV_PMU_VOLT_CMD_ID_RPC					(0x00000001U)
#define NV_PMU_VOLT_CMD_ID_BOARDOBJ_GRP_GET_STATUS		(0x00000002U)

/*
 * VOLT MSG ID definitions
 */
#define NV_PMU_VOLT_MSG_ID_BOARDOBJ_GRP_SET			(0x00000000U)
#define NV_PMU_VOLT_MSG_ID_RPC					(0x00000001U)
#define NV_PMU_VOLT_MSG_ID_BOARDOBJ_GRP_GET_STATUS		(0x00000002U)

/* VOLT RPC */
#define NV_PMU_RPC_ID_VOLT_BOARD_OBJ_GRP_CMD			0x00U
#define NV_PMU_RPC_ID_VOLT_VOLT_SET_VOLTAGE			0x01U
#define NV_PMU_RPC_ID_VOLT_LOAD					0x02U
#define NV_PMU_RPC_ID_VOLT_VOLT_RAIL_GET_VOLTAGE		0x03U
#define NV_PMU_RPC_ID_VOLT_VOLT_POLICY_SANITY_CHECK		0x04U
#define NV_PMU_RPC_ID_VOLT_TEST_EXECUTE				0x05U
#define NV_PMU_RPC_ID_VOLT__COUNT				0x06U

/*
 * Defines the structure that holds data
 * used to execute LOAD RPC.
 */
struct nv_pmu_rpc_struct_volt_load {
	/*[IN/OUT] Must be first field in RPC structure */
	struct nv_pmu_rpc_header hdr;
	u32  scratch[1];
};

#endif /* NVGPU_PMUIF_VOLT_H */
