// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/gk20a.h>

bool nvgpu_log_mask_enabled(struct gk20a *g, u64 log_mask)
{
	return (g->log_mask & log_mask) != 0ULL;
}