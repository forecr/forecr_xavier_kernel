// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/io.h>
#include <nvgpu/utils.h>
#include <nvgpu/fifo.h>
#include <nvgpu/gk20a.h>
#ifdef CONFIG_NVGPU_GSP_SCHEDULER_VM
#include <nvgpu/gsp_vm_scheduler.h>
#endif

#include "fifo_gb10b.h"
#include "fifo_intr_gb10b.h"
#include "hal/fifo/fifo_ga10b.h"

int gb10b_init_fifo_setup_hw(struct gk20a *g)
{
	struct nvgpu_fifo *f = &g->fifo;

	nvgpu_log_fn(g, " ");

	/*
	 * Current Flow:
	 * Nvgpu Init sequence:
	 * g->ops.fifo.reset_enable_hw
	 * ....
	 * g->ops.fifo.fifo_init_support
	 *
	 * Fifo Init Sequence called from g->ops.fifo.fifo_init_support:
	 * fifo.reset_enable_hw   -> enables interrupts
	 * fifo.fifo_init_support -> fifo.setup_sw (Sets up runlist info)
	 * fifo.fifo_init_support -> fifo.init_fifo_setup_hw
	 *
	 * Runlist info is required for getting vector id and enabling
	 * interrupts at top level.
	 * Get vector ids before enabling interrupts at top level to make sure
	 * vectorids are initialized in nvgpu_mc struct before intr_top_enable
	 * is called.
	 */
	gb10b_fifo_runlist_intr_vectorid_init(g);

	f->max_subctx_count = g->ops.gr.init.get_max_subctx_count();

	g->ops.usermode.setup_hw(g);

	ga10b_fifo_enable_intr(g);

	return 0;
}

