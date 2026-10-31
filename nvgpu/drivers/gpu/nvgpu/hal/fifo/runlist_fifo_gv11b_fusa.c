// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/runlist.h>

#include "runlist_fifo_gv11b.h"

#include <nvgpu/hw/gv11b/hw_fifo_gv11b.h>
struct gk20a;

u32 gv11b_runlist_count_max(struct gk20a *g)
{
	(void)g;
	return fifo_eng_runlist_base__size_1_v();
}
