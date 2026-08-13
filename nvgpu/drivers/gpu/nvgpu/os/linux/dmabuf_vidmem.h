/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_DMABUF_VIDMEM_H
#define NVGPU_DMABUF_VIDMEM_H

#ifdef CONFIG_NVGPU_DGPU

#include <nvgpu/types.h>

struct dma_buf;

struct gk20a;
struct scatterlist;

struct gk20a *nvgpu_vidmem_buf_owner(struct dma_buf *dmabuf);
int nvgpu_vidmem_export_linux(struct gk20a *g, size_t bytes);

void nvgpu_vidmem_set_page_alloc(struct scatterlist *sgl, u64 addr);
struct nvgpu_page_alloc *nvgpu_vidmem_get_page_alloc(struct scatterlist *sgl);

int nvgpu_vidmem_buf_access_memory(struct gk20a *g, struct dma_buf *dmabuf,
		void *buffer, u64 offset, u64 size, u32 cmd);

struct nvgpu_vidmem_linux {
	struct dma_buf	*dmabuf;
	void		*dmabuf_priv;
	void		(*dmabuf_priv_delete)(void *);
};

#endif
#endif
