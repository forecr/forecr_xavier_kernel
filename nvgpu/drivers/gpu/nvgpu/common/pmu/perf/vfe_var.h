/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PERF_VFE_VAR_H
#define NVGPU_PERF_VFE_VAR_H

#define CTRL_PERF_VFE_VAR_SINGLE_SENSED_FUSE_VALUE_INIT(pvar, value, b_fuse_val_signed) \
    do {                                                                        \
        if (b_fuse_val_signed)                                                     \
        {                                                                       \
            ((pvar)->data.signed_value) = ((s32)(value));                      \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            ((pvar)->data.unsigned_value) = ((u32)(value));                    \
        }                                                                       \
        ((pvar)->b_signed) = (b_fuse_val_signed);                                   \
    } while (0)


struct vfe_vars {
	struct boardobjgrp_e255 super;
};

struct vfe_var {
	struct pmu_board_obj super;
	u32 out_range_min;
	u32 out_range_max;
	u8 recursion_depth;
	struct boardobjgrpmask_e255 mask_depending_vars;
	struct boardobjgrpmask_e255 mask_dependent_vars;
	struct boardobjgrpmask_e2048 mask_dependent_equs;
	int (*mask_depending_build)(struct gk20a *g,
			struct boardobjgrp *pboardobjgrp,
			struct vfe_var *pvfe_var);
	int (*vfe_var_state_init)(struct gk20a *g,
			struct boardobjgrp *pboardobjgrp,
			struct vfe_var *pvfe_var);
	int (*vfe_var_sanity_check)(struct gk20a *g,
			struct boardobjgrp *pboardobjgrp,
			struct vfe_var *pvfe_var);

	bool b_is_dynamic_valid;
	bool b_is_dynamic;
};

struct vfe_var_derived {
	struct vfe_var super;
};

struct vfe_var_derived_product {
	struct vfe_var_derived super;
	u8 var_idx0;
	u8 var_idx1;
};

struct vfe_var_derived_sum {
	struct vfe_var_derived super;
	u8 var_idx0;
	u8 var_idx1;
};

struct vfe_var_single {
	struct vfe_var super;
	u8 override_type;
	bool b_override_die_aware;
	u32 override_value[2];
};

struct vfe_var_single_frequency {
	struct vfe_var_single  super;
	u8 clk_domain_idx;
};

struct vfe_var_single_voltage {
	struct vfe_var_single super;
};

struct vfe_var_single_caller_specified {
	struct vfe_var_single  super;
	u8 uid;
};

struct vfe_var_single_sensed {
	struct vfe_var_single super;
};

struct vfe_var_single_sensed_fuse_base {
	struct vfe_var_single_sensed super;
	struct ctrl_perf_vfe_var_single_sensed_fuse_override_info       override_info;
	bool   b_fuse_value_signed;
	struct ctrl_perf_vfe_var_single_sensed_fuse_value fuse_val_integer;
	struct ctrl_perf_vfe_var_single_sensed_fuse_value fuse_val_hw_integer;
	struct ctrl_perf_vfe_var_single_sensed_fuse_value fuse_val_default;
	u8 fuse_version;
	bool b_version_check_done;
	struct ctrl_perf_vfe_var_single_sensed_fuse_ver_info fuse_ver_info;
	u32 hw_correction_scale;
	s32 hw_correction_offset;
};

struct vfe_var_single_sensed_fuse {
	struct vfe_var_single_sensed super;
	struct ctrl_perf_vfe_var_single_sensed_fuse_override_info	override_info;
	struct ctrl_perf_vfe_var_single_sensed_fuse_vfield_info vfield_info;
	struct ctrl_perf_vfe_var_single_sensed_fuse_ver_vfield_info vfield_ver_info;
	struct ctrl_perf_vfe_var_single_sensed_fuse_value fuse_val_default;
	bool b_fuse_value_signed;
	u32 fuse_value_integer;
	u32 fuse_value_hw_integer;
	u8 fuse_version;
	bool b_version_check_done;
};

struct vfe_var_single_sensed_fuse_20 {
	struct vfe_var_single_sensed_fuse_base super;
	u16 fuse_id;
	u16 fuse_id_ver;
};

struct vfe_var_single_sensed_temp {
	struct vfe_var_single_sensed super;
	u8 therm_channel_index;
	int temp_hysteresis_positive;
	int temp_hysteresis_negative;
	bool b_use_snapped_value;
	int temp_default;
};

int perf_vfe_var_sw_setup(struct gk20a *g);
int perf_vfe_var_pmu_setup(struct gk20a *g);

#endif /* NVGPU_PERF_VFE_VAR_H */
