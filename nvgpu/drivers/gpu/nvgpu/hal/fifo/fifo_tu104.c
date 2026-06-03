// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/gk20a.h>

#include "hal/fifo/fifo_gv11b.h"
#include "hal/fifo/fifo_tu104.h"

int tu104_init_fifo_setup_hw(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	g->ops.usermode.setup_hw(g);

	return gv11b_init_fifo_setup_hw(g);
}
