// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "channel_gv100.h"

#include <nvgpu/hw/gv100/hw_ccsr_gv100.h>

u32 gv100_channel_count(struct gk20a *g)
{
	return ccsr_channel__size_1_v();
}
