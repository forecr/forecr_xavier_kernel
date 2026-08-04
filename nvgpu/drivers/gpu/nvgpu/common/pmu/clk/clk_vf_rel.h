/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CLK_VF_REL_H
#define NVGPU_CLK_VF_REL_H

#include <nvgpu/boardobjgrp.h>
#include <nvgpu/boardobjgrp_e32.h>
#include <common/pmu/boardobj/boardobj.h>

/*!
 * Clock Vf Relationship group info.
 * Specifies all Clock Vf Relationships
 * that we will control per the VBIOS specification.
 */
struct nvgpu_clk_vf_rels {
	struct boardobjgrp_e255 super;
	struct boardobjgrp_iface_model_10 model10;
	/*!
	 * Number of Slave entries per VF Rel entry.
	 */
	u8 slave_entry_count;
	/*!
	 * Count of secondary VF entries.
	 */
	u8 vf_entry_count_sec;
};
struct clk_vf_rel;
struct client_clk_domain_prog;
typedef s32 ClkVfRelVfFlatten(struct gk20a *g, struct clk_vf_rel *pVfRel, u8 clkDomainIdx);

struct clk_vf_rel {
	struct pmu_board_obj super;
	u8 rail_idx;
	bool b_ocov_enabled;
	u16 freq_max_mhz;
	s32 volt_delta_uv;
	struct ctrl_clk_freq_delta freq_delta;
	struct nv_pmu_clk_clk_vf_rel_vf_entry_pri vf_entry_pri;
	struct mv_pmu_clk_clk_vf_rel_vf_entry_sec
		vf_entries_sec[NV2080_CTRL_CLK_CLK_VF_REL_VF_ENTRY_SEC_MAX];
	/*!
	 * Virtual Function Pointers
	 */
	ClkVfRelVfFlatten *vfFlatten;
};

struct clk_vf_rel_50 {
	struct clk_vf_rel super;
	bool b_base_vf_update_req;
	bool b_offset_update_req;
	u16 safe_vf_curve_tuple_idx;
	u16 vf_tuple_idx_first;
	u16 vf_tuple_idx_last;
	u16 picked_vf_tuple_idx;
	u16 current_vf_curve_cache_idx;
};

struct clk_vf_rel_4x_table {
	s16 *offset;	// Todo: TBD update to interface_vertual_table struct? or keep flattened?
	struct nv_pmu_clk_clk_vf_rel_table_secondary_entry
		slaveEntries[NV2080_CTRL_CLK_CLK_VF_REL_TABLE_SECONDARY_ENTRIES_MAX];

};

struct clk_vf_rel_4x_ratio {
	s16 *offset;
	struct nv_pmu_clk_clk_vf_rel_ratio_secondary_entry
		slaveEntries[NV2080_CTRL_CLK_CLK_VF_REL_RATIO_SECONDARY_ENTRIES_MAX];
};

struct clk_vf_rel_50_ratio {
	struct clk_vf_rel_50 super;
	struct clk_vf_rel_4x_ratio ratio;
};


struct clk_vf_rel_4x_ratio_volt {
	s16 *offset;
	struct nv_pmu_clk_clk_vf_rel_ratio_volt_vf_smooth_data_grp
		vfSmoothDataGrp;
};

struct clk_vf_rel_50_ratio_volt {
	struct clk_vf_rel_50_ratio super;
	struct clk_vf_rel_4x_ratio_volt ratioVolt;
	u8 vfGenMethod;
	u16 devinitVfeIdx;
	u16 favDeratedSubCurveVFEEqnIdx;
};

struct clk_vf_rel_50_ratio_freq {
	struct clk_vf_rel_50_ratio super;
};

struct clk_vf_rel_50_table {
	struct clk_vf_rel_50 super;
	struct clk_vf_rel_4x_table table;
};

struct clk_vf_rel_50_table_freq {
	struct clk_vf_rel_50_table super;
};


s32 clk_vf_rel_init_pmupstate(struct gk20a *g);
void clk_vf_rel_free_pmupstate(struct gk20a *g);
s32 clk_vf_rel_sw_setup(struct gk20a *g);
s32 clk_vf_rel_pmu_setup(struct gk20a *g);

#endif
