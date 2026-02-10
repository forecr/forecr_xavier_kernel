// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2017-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>

#include "os_linux.h"
#include "module_usermode.h"

/*
 * Locks out the driver from accessing GPU registers. This prevents access to
 * thse registers after the GPU has been clock or power gated. This should help
 * find annoying bugs where register reads and writes are silently dropped
 * after the GPU has been turned off. On older chips these reads and writes can
 * also lock the entire CPU up.
 */
void nvgpu_lockout_usermode_registers(struct gk20a *g)
{
	g->usermode_regs = 0U;
}

/*
 * Undoes t19x_lockout_registers().
 */
void nvgpu_restore_usermode_registers(struct gk20a *g)
{
	g->usermode_regs = g->usermode_regs_saved;
}

void nvgpu_remove_usermode_support(struct gk20a *g)
{
	if (g->usermode_regs) {
		g->usermode_regs = 0U;
	}
}

void nvgpu_init_usermode_support(struct gk20a *g)
{
	if (g->ops.usermode.base == NULL) {
		return;
	}

	if (g->usermode_regs == 0U) {
		g->usermode_regs = g->regs + g->ops.usermode.bus_base(g);
		g->usermode_regs_saved = g->usermode_regs;
	}

	g->usermode_regs_bus_addr = g->regs_bus_addr +
					g->ops.usermode.bus_base(g);
}
