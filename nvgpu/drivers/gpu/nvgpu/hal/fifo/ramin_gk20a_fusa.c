// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/channel.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>

#include "hal/fifo/ramin_gk20a.h"

#include <nvgpu/hw/gk20a/hw_ram_gk20a.h>

u32 gk20a_ramin_base_shift(void)
{
	return ram_in_base_shift_v();
}

u32 gk20a_ramin_alloc_size(void)
{
	return ram_in_alloc_size_v();
}
