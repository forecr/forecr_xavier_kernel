/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_NVGPU_MEM_H
#define NVGPU_POSIX_NVGPU_MEM_H

struct nvgpu_mem_priv {
	struct nvgpu_sgt *sgt;
};

struct nvgpu_mem;
struct nvgpu_mem_sgl;

struct nvgpu_mem_sgl *nvgpu_mem_sgl_posix_create_from_list(struct gk20a *g,
				struct nvgpu_mem_sgl *sgl_list, u32 nr_sgls,
				u64 *total_size);
void nvgpu_mem_sgl_free(struct gk20a *g, struct nvgpu_mem_sgl *sgl);
struct nvgpu_sgt *nvgpu_mem_sgt_posix_create_from_list(struct gk20a *g,
				struct nvgpu_mem_sgl *sgl_list, u32 nr_sgls,
				u64 *total_size);
int nvgpu_mem_posix_create_from_list(struct gk20a *g, struct nvgpu_mem *mem,
				struct nvgpu_mem_sgl *sgl_list, u32 nr_sgls);
int __nvgpu_mem_create_from_phys(struct gk20a *g, struct nvgpu_mem *dest,
				  u64 src_phys, int nr_pages);

#endif /* NVGPU_POSIX_NVGPU_MEM_H */
