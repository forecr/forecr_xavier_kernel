/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_VOLT_RAIL_H
#define NVGPU_VOLT_RAIL_H

#include <nvgpu/boardobjgrp.h>
#include <common/pmu/boardobj/boardobj.h>

#define CTRL_PMGR_PWR_EQUATION_INDEX_INVALID	0xFFU

#define CTRL_VOLT_RAIL_VERSION_10		0x10U
#define CTRL_VOLT_RAIL_VERSION_30		0x30U

struct voltage_rail {
	struct pmu_board_obj super;
	u32 boot_voltage_uv;
	u8 rel_limit_vfe_equ_idx;
	u8 alt_rel_limit_vfe_equ_idx;
	u8 ov_limit_vfe_equ_idx;
	u8 pwr_equ_idx;
	u8 volt_scale_exp_pwr_equ_idx;
	u8 volt_dev_idx_default;
	u8 volt_dev_idx_ipc_vmin;
	u8 boot_volt_vfe_equ_idx;
	u8 vmin_limit_vfe_equ_idx;
	u8 volt_margin_limit_vfe_equ_idx;
	u32 volt_margin_limit_vfe_equ_mon_handle;
	u32 rel_limit_vfe_equ_mon_handle;
	u32 alt_rel_limit_vfe_equ_mon_handle;
	u32 ov_limit_vfe_equ_mon_handle;
	struct boardobjgrpmask_e32 volt_dev_mask;
	s32  volt_delta_uv[CTRL_VOLT_RAIL_VOLT_DELTA_MAX_ENTRIES];
	u32 vmin_limitu_v;
	u32 max_limitu_v;
	u32 current_volt_uv;
};

/*!
 * Struct representing nvgpu driver side voltage rail board object
 * for parsing VBIOS volt rail table version 3x
 */
struct voltage_rail_3x {
	struct pmu_board_obj super;
	u32 boot_voltage_uv;
	u16 rel_limit_vfe_equ_idx;
	u16 alt_rel_limit_vfe_equ_idx;
	u16 ov_limit_vfe_equ_idx;
	u16 leakage_pwr_equ_idx;
	u16 dynamic_pwr_equ_idx;
	u8 volt_dev_idx_default;
	u8 volt_dev_idx_ipc_vmin;
	u16 boot_volt_vfe_equ_idx;
	u8 boot_volt_vfield_id;
	u16 vmin_limit_vfe_equ_idx;
	u16 volt_margin_limit_vfe_equ_idx;
	u32 volt_margin_limit_vfe_equ_mon_handle;
	u32 rel_limit_vfe_equ_mon_handle;
	u32 alt_rel_limit_vfe_equ_mon_handle;
	u32 ov_limit_vfe_equ_mon_handle;
	struct boardobjgrpmask_e32 adc_dev_mask;
	struct boardobjgrpmask_e32 volt_dev_mask;
	struct boardobjgrpmask_e32 clk_domains_prog_mask;
	s32 volt_delta_uv[CTRL_VOLT_RAIL_VOLT_DELTA_MAX_ENTRIES];
	u32 vbios_boot_voltage_uv;
	u16 ba_scaling_pwr_eqn_idx;
	u8 domain;
	struct boardobjgrpmask_e32 volt_device_mask;
	struct boardobjgrpmask_e32 volt_threshold_controller_mask;
	u16 logical_rail_idx;
	u8 boot_volt_fuse_id;
	bool b_hw_clvc_en;
	u32 vmin_limitu_v;
	u32 max_limitu_v;
	u32 current_volt_uv;
};

int volt_rail_volt_dev_register(struct gk20a *g, void *pvolt_rail,
	u8 volt_dev_idx, u8 operation_type);
int volt_rail_sw_setup(struct gk20a *g);
int volt_rail_pmu_setup(struct gk20a *g);

int volt_rail_state_init_3x(struct gk20a *g,
		struct voltage_rail_3x *pvolt_rail);
int volt_get_volt_rail_table_3x(struct gk20a *g,
		struct voltage_rail_metadata *pvolt_rail_metadata);
int volt_rail_obj_update_3x(struct gk20a *g,
		struct pmu_board_obj *obj,
		struct nv_pmu_boardobj *pmu_obj);
int volt_rail_volt_dev_register_3x(struct gk20a *g, struct voltage_rail_3x
	*pvolt_rail, u8 volt_dev_idx, u8 operation_type);
int volt_rail_devgrp_pmudata_instget_3x(struct gk20a *g,
	struct nv_pmu_boardobjgrp *pmuboardobjgrp, struct nv_pmu_boardobj
	**pmu_obj, u8 idx);
int volt_rail_devgrp_pmu_hdr_data_init_3x(struct gk20a *g,
	struct boardobjgrp *pboardobjgrp,
	struct nv_pmu_boardobjgrp_super *pboardobjgrppmu,
	struct boardobjgrpmask *mask);

#endif /* NVGPU_VOLT_RAIL_H */
