// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/gr/config.h>

#include "gr_config_gm20b.h"

#include <nvgpu/hw/gm20b/hw_gr_gm20b.h>

int gm20b_gr_config_init_sm_id_table(struct gk20a *g,
		struct nvgpu_gr_config *gr_config)
{
	u32 gpc, tpc;
	u32 sm_id = 0;
	u32 num_sm;

	num_sm = nvgpu_safe_mult_u32(nvgpu_gr_config_get_tpc_count(gr_config),
				     nvgpu_gr_config_get_sm_count_per_tpc(gr_config));
	nvgpu_gr_config_set_no_of_sm(gr_config, num_sm);

	(void)g;

	for (tpc = 0;
	     tpc < nvgpu_gr_config_get_max_tpc_per_gpc_count(gr_config);
	     tpc++) {
		for (gpc = 0; gpc < nvgpu_gr_config_get_gpc_count(gr_config); gpc++) {

			if (tpc < nvgpu_gr_config_get_gpc_tpc_count(gr_config, gpc)) {
				struct nvgpu_sm_info *sm_info =
					nvgpu_gr_config_get_sm_info(gr_config, sm_id);
				nvgpu_gr_config_set_sm_info_tpc_index(sm_info, tpc);
				nvgpu_gr_config_set_sm_info_gpc_index(sm_info, gpc);
				nvgpu_gr_config_set_sm_info_sm_index(sm_info, 0);
				nvgpu_gr_config_set_sm_info_global_tpc_index(sm_info, sm_id);
				sm_id = nvgpu_safe_add_u32(sm_id, 1U);
			}
		}
	}
	nvgpu_assert(num_sm == sm_id);
	return 0;
}
