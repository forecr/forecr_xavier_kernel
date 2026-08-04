// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "runlist_fifo_gv100.h"

#include <nvgpu/hw/gv100/hw_fifo_gv100.h>
struct gk20a;

u32 gv100_runlist_count_max(struct gk20a *g)
{
	return fifo_eng_runlist_base__size_1_v();
}
