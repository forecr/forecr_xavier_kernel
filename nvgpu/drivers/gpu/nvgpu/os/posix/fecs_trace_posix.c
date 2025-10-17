// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/gr/fecs_trace.h>

int vgpu_alloc_user_buffer(struct gk20a *g, void **buf, size_t *size);
void vgpu_fecs_trace_data_update(struct gk20a *g);
void vgpu_get_mmap_user_buffer_info(struct gk20a *g,
				void **mmapaddr, size_t *mmapsize);

int nvgpu_gr_fecs_trace_ring_alloc(struct gk20a *g,
		void **buf, size_t *size)
{
	return -EINVAL;
}

int nvgpu_gr_fecs_trace_ring_free(struct gk20a *g)
{
	return -EINVAL;
}

void nvgpu_gr_fecs_trace_get_mmap_buffer_info(struct gk20a *g,
				void **mmapaddr, size_t *mmapsize)
{
}

int nvgpu_gr_fecs_trace_write_entry(struct gk20a *g,
		struct nvgpu_gpu_ctxsw_trace_entry *entry)
{
	return -EINVAL;
}

void nvgpu_gr_fecs_trace_wake_up(struct gk20a *g, int vmid)
{
}

void nvgpu_gr_fecs_trace_add_tsg_reset(struct gk20a *g, struct nvgpu_tsg *tsg)
{
}

u8 nvgpu_gpu_ctxsw_tags_to_common_tags(u8 tags)
{
	return 0;
}

int vgpu_alloc_user_buffer(struct gk20a *g, void **buf, size_t *size)
{
	return -EINVAL;
}

void vgpu_fecs_trace_data_update(struct gk20a *g)
{
}

void vgpu_get_mmap_user_buffer_info(struct gk20a *g,
				void **mmapaddr, size_t *mmapsize)
{
}
