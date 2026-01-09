// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/device.h>
#include <nvgpu/types.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include "top_gm20b.h"

#include <nvgpu/hw/gm20b/hw_top_gm20b.h>

int gm20b_device_info_parse_data(struct gk20a *g, u32 table_entry, u32 *inst_id,
						u32 *pri_base, u32 *fault_id)
{
	if (top_device_info_entry_v(table_entry) !=
					top_device_info_entry_data_v()) {
		nvgpu_err(g, "Invalid device_info_data %u",
				top_device_info_entry_v(table_entry));
		return -EINVAL;
	}

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

	/* In Maxwell days, instance id was not relevant as each instance of
	 * an engine would be assigned new engine_type.
	 */
	*inst_id = 0;
	nvgpu_log_info(g, "Inst_id: %u", *inst_id);

	return 0;
}
