// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>

#include "top_gv11b.h"

#include <nvgpu/hw/gv11b/hw_top_gv11b.h>

u32 gv11b_top_get_num_lce(struct gk20a *g)
{
	u32 reg_val, num_lce;

	reg_val = nvgpu_readl(g, top_num_ces_r());
	num_lce = top_num_ces_value_v(reg_val);
	nvgpu_log_info(g, "num LCE: %d", num_lce);

	return num_lce;
}

u32 gv11b_top_get_max_pes_per_gpc(struct gk20a *g)
{
	u32 tmp;

	tmp = nvgpu_readl(g, top_num_pes_per_gpc_r());
	return top_num_pes_per_gpc_value_v(tmp);
}
int gv11b_device_info_parse_data(struct gk20a *g, u32 table_entry, u32 *inst_id,
		u32 *pri_base, u32 *fault_id)
{
	if (top_device_info_data_type_v(table_entry) !=
					top_device_info_data_type_enum2_v()) {
		nvgpu_err(g, "Unknown device_info_data_type %u",
				top_device_info_data_type_v(table_entry));
		return -EINVAL;
	}

	nvgpu_log_info(g, "Entry_data to be parsed 0x%x", table_entry);

	*pri_base = (top_device_info_data_pri_base_v(table_entry) <<
				top_device_info_data_pri_base_align_v());
	nvgpu_log_info(g, "Pri Base addr: 0x%x", *pri_base);

	if (top_device_info_data_fault_id_v(table_entry) ==
				top_device_info_data_fault_id_valid_v()) {
		*fault_id = top_device_info_data_fault_id_enum_v(table_entry);
	} else {
		*fault_id = U32_MAX;
	}
	nvgpu_log_info(g, "Fault_id: %u", *fault_id);

	*inst_id = top_device_info_data_inst_id_v(table_entry);
	nvgpu_log_info(g, "Inst_id: %u", *inst_id);

	return 0;
}
