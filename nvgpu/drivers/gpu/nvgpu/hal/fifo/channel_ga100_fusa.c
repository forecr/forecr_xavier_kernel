// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.


#include <nvgpu/log.h>
#include <nvgpu/gk20a.h>

#include "channel_ga100.h"

#include <nvgpu/hw/ga100/hw_runlist_ga100.h>

u32 ga100_channel_count(struct gk20a *g)
{
	u32 num_channels = 0;

	num_channels = ((0x1U) << runlist_channel_config_num_channels_log2_2k_v());
	nvgpu_log(g, gpu_dbg_info, "Number of channels supported by hw = %u",
		num_channels);

	return num_channels;
}
