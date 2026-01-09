// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/ltc.h>
#include <nvgpu/cbc.h>
#include <nvgpu/comptags.h>
#include <nvgpu/io.h>
#include <nvgpu/timers.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/soc.h>
#include <nvgpu/trace.h>
#include <nvgpu/hw/gb10b/hw_ltc_gb10b.h>

#include "cbc_gb10b.h"

void gb10b_cbc_remove_support(struct gk20a *g)
{
	struct nvgpu_cbc *cbc = g->cbc;

	nvgpu_log_fn(g, " ");

	if (cbc == NULL) {
		return;
	}
	nvgpu_kfree(g, cbc);
	g->cbc = NULL;
}

int gb10b_cbc_init_support(struct gk20a *g)
{
	int err = 0;
	struct nvgpu_cbc *cbc = g->cbc;

	nvgpu_log_fn(g, " ");

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_COMPRESSION)) {
		return 0;
	}

	if (cbc == NULL) {
		cbc = nvgpu_kzalloc(g, sizeof(*cbc));
		if (cbc == NULL) {
			return -ENOMEM;
		}
		g->cbc = cbc;
	}
	if (g->ops.fb.cbc_configure != NULL)
		g->ops.fb.cbc_configure(g, cbc);

	return err;
}
