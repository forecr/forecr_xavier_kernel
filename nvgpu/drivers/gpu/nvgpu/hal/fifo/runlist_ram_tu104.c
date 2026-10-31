// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "runlist_ram_tu104.h"

#include <nvgpu/hw/tu104/hw_ram_tu104.h>

u32 tu104_runlist_entry_size(struct gk20a *g)
{
	return ram_rl_entry_size_v();
}


