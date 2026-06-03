/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_VOLT_POLICY_H
#define NVGPU_VOLT_POLICY_H

#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrpmask.h>
#include <common/pmu/boardobj/boardobj.h>

#define NV_VBIOS_VOLTAGE_POLICY_1X_VERSION		0x10U
#define NV_VBIOS_VOLTAGE_POLICY_2X_VERSION		0x20U

#define VOLT_POLICY_INDEX_IS_VALID(pvolt, policy_idx)	\
		(boardobjgrp_idxisvalid(	\
		&((pvolt)->volt_policy_metadata.volt_policies.super), \
		(policy_idx)))

/*!
 * extends boardobj providing attributes common to all voltage_policies.
 */
struct voltage_policy {
	struct pmu_board_obj super;
};

struct voltage_policy_multi_rail {
	struct voltage_policy super;
	struct boardobjgrpmask_e32 volt_rail_mask;
};

struct voltage_policy_single_rail {
	struct voltage_policy  super;
	u8 rail_idx;
};

struct voltage_policy_single_rail_multi_step {
	struct voltage_policy_single_rail super;
	u16 inter_switch_delay_us;
	u32 ramp_up_step_size_uv;
	u32 ramp_down_step_size_uv;
};

int volt_policy_sw_setup(struct gk20a *g);
int volt_policy_pmu_setup(struct gk20a *g);

#endif /* NVGPU_VOLT_POLICY_H */
