// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/channel.h>
#include <nvgpu/channel_sync.h>

u32 nvgpu_channel_get_max_subctx_count(struct nvgpu_channel *ch)
{
	(void)ch;
	return 64;
}

u32 nvgpu_channel_get_synpoints(struct gk20a *g)
{
	(void)g;
	return 256;
}
