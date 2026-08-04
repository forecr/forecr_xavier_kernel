// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/vgpu/tegra_vgpu.h>
#include <nvgpu/nvgpu_ivm.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/gr/fecs_trace.h>

#include <linux/mm.h>

#include "common/vgpu/gr/fecs_trace_vgpu.h"

void vgpu_fecs_trace_data_update(struct gk20a *g)
{
	nvgpu_gr_fecs_trace_wake_up(g, 0);
}

int vgpu_alloc_user_buffer(struct gk20a *g, void **buf, size_t *size)
{
	struct vgpu_fecs_trace *vcst = (struct vgpu_fecs_trace *)g->fecs_trace;

	*buf = vcst->buf;
	*size = nvgpu_ivm_get_size(vcst->cookie);
	return 0;
}

void vgpu_get_mmap_user_buffer_info(struct gk20a *g,
				void **mmapaddr, size_t *mmapsize)
{
	struct vgpu_fecs_trace *vcst = (struct vgpu_fecs_trace *)g->fecs_trace;

	*mmapsize = nvgpu_ivm_get_size(vcst->cookie);
	*mmapaddr = (void *) (nvgpu_ivm_get_ipa(vcst->cookie) >> PAGE_SHIFT);
}
