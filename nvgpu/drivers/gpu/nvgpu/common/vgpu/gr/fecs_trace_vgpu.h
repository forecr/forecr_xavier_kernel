/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FECS_TRACE_VGPU_H
#define NVGPU_FECS_TRACE_VGPU_H

#include <nvgpu/types.h>

struct gk20a;
struct vm_area_struct;
struct nvgpu_gpu_ctxsw_trace_filter;
struct tegra_hv_ivm_cookie;
struct nvgpu_gpu_ctxsw_trace_entry;
struct nvgpu_ctxsw_ring_header_internal;

struct vgpu_fecs_trace {
	struct tegra_hv_ivm_cookie *cookie;
	struct nvgpu_ctxsw_ring_header_internal *header;
	struct nvgpu_gpu_ctxsw_trace_entry *entries;
	int num_entries;
	bool enabled;
	void *buf;
};

void vgpu_fecs_trace_data_update(struct gk20a *g);
int vgpu_fecs_trace_init(struct gk20a *g);
int vgpu_fecs_trace_deinit(struct gk20a *g);
int vgpu_fecs_trace_enable(struct gk20a *g);
int vgpu_fecs_trace_disable(struct gk20a *g);
bool vgpu_fecs_trace_is_enabled(struct gk20a *g);
int vgpu_fecs_trace_poll(struct gk20a *g);
int vgpu_alloc_user_buffer(struct gk20a *g, void **buf, size_t *size);
int vgpu_free_user_buffer(struct gk20a *g);
void vgpu_get_mmap_user_buffer_info(struct gk20a *g,
				void **mmapaddr, size_t *mmapsize);
int vgpu_fecs_trace_max_entries(struct gk20a *g,
			struct nvgpu_gpu_ctxsw_trace_filter *filter);
int vgpu_fecs_trace_set_filter(struct gk20a *g,
			struct nvgpu_gpu_ctxsw_trace_filter *filter);
struct tegra_hv_ivm_cookie *vgpu_fecs_trace_get_ivm(struct gk20a *g);

#endif /* NVGPU_FECS_TRACE_VGPU_H */
