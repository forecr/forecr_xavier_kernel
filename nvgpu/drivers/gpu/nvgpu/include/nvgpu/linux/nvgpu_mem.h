/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __NVGPU_LINUX_NVGPU_MEM_H__
#define __NVGPU_LINUX_NVGPU_MEM_H__

struct page;
struct sg_table;
struct scatterlist;
struct nvgpu_sgt;

struct gk20a;
struct nvgpu_mem;
struct nvgpu_gmmu_attrs;

struct nvgpu_mem_priv {
	struct page **pages;
	struct sg_table *sgt;
	unsigned long flags;
};

u64 nvgpu_mem_get_addr_sgl(struct gk20a *g, struct scatterlist *sgl);
struct nvgpu_sgt *nvgpu_mem_linux_sgt_create(struct gk20a *g,
					   struct sg_table *sgt);
void nvgpu_mem_linux_sgt_free(struct gk20a *g, struct nvgpu_sgt *sgt);
struct nvgpu_sgt *nvgpu_linux_sgt_create(struct gk20a *g,
					   struct sg_table *sgt);

#endif
