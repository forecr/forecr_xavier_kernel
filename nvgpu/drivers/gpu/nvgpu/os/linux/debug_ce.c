// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "debug_ce.h"
#include "os_linux.h"

#include <nvgpu/ce_app.h>

#include <common/ce/ce_priv.h>

#include <linux/debugfs.h>

void nvgpu_ce_debugfs_init(struct gk20a *g)
{
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);

	debugfs_create_u32("ce_app_ctx_count", S_IWUSR | S_IRUGO,
			   l->debugfs, &g->ce_app->ctx_count);
	debugfs_create_u32("ce_app_state", S_IWUSR | S_IRUGO,
			   l->debugfs, &g->ce_app->app_state);
	debugfs_create_u32("ce_app_next_ctx_id", S_IWUSR | S_IRUGO,
			   l->debugfs, &g->ce_app->next_ctx_id);
}
