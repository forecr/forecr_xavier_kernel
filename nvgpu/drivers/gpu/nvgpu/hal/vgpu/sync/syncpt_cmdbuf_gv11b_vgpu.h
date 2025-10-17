/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_SYNCPT_CMDBUF_GV11B_VGPU_H
#define NVGPU_SYNCPT_CMDBUF_GV11B_VGPU_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_channel;
struct nvgpu_mem;
struct vm_gk20a;

int vgpu_gv11b_syncpt_alloc_buf(struct nvgpu_channel *c,
			u32 syncpt_id, struct nvgpu_mem *syncpt_buf);
void vgpu_gv11b_syncpt_free_buf(struct nvgpu_channel *c,
					struct nvgpu_mem *syncpt_buf);
int vgpu_gv11b_syncpt_get_sync_ro_map(struct vm_gk20a *vm,
	u64 *base_gpuva, u32 *sync_size, u32 *num_syncpoints);
#endif /* NVGPU_SYNCPT_CMDBUF_GV11B_VGPU_H */
