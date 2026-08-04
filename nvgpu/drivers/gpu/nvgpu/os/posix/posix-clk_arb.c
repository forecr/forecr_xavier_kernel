// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/list.h>
#include <nvgpu/clk_arb.h>
#include <nvgpu/bug.h>

/**
 * So for POSIX post event is stubbed.
 */

void nvgpu_clk_arb_event_post_event(struct nvgpu_clk_dev *dev)
{
	(void)dev;
	BUG();
}
