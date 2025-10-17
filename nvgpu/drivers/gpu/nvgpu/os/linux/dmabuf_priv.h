/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_DMABUF_PRIV_H
#define NVGPU_DMABUF_PRIV_H

#include <linux/dma-buf.h>

#include <nvgpu/comptags.h>
#include <nvgpu/list.h>
#include <nvgpu/lock.h>
#include <nvgpu/gmmu.h>
#include <nvgpu/user_fence.h>

struct sg_table;
struct dma_buf;
struct dma_buf_attachment;
struct device;
struct nvgpu_os_linux;

struct gk20a;
struct gk20a_buffer_state {
	struct nvgpu_list_node list;

	/* The valid compbits and the fence must be changed atomically. */
	struct nvgpu_mutex lock;

	/*
	 * Offset of the surface within the dma-buf whose state is
	 * described by this struct (one dma-buf can contain multiple
	 * surfaces with different states).
	 */
	size_t offset;

	/* A bitmask of valid sets of compbits (0 = uncompressed). */
	u32 valid_compbits;

	/* The ZBC color used on this buffer. */
	u32 zbc_color;

	/*
	 * This struct reflects the compression state of the associated buffer
	 * when this fence signals. The fence is provided to userspace via
	 * struct nvgpu_gpu_prepare_compressible_read_args.
	 */
	struct nvgpu_user_fence fence;
};

static inline struct gk20a_buffer_state *
gk20a_buffer_state_from_list(struct nvgpu_list_node *node)
{
	return (struct gk20a_buffer_state *)
		((uintptr_t)node - offsetof(struct gk20a_buffer_state, list));
};

#ifdef CONFIG_NVGPU_COMPRESSION
struct gk20a_dmabuf_priv {
	struct nvgpu_mutex lock;

	struct gk20a *g;

	struct gk20a_comptag_allocator *comptag_allocator;
	struct gk20a_comptags comptags;

	u8 *metadata_blob;
	u32 metadata_blob_size;

	bool registered;
	bool mutable_metadata;

	struct nvgpu_list_node states;

	/* Used for retrieving the associated dmabuf from the priv */
	struct dma_buf *dmabuf;
	struct sg_table *st;
	/* 'dma_buf->ops' which is a pointer to a constant struct is
	 * altered to point to the local copy for the entire lifetime
	 * of this existing dma-buf until the driver is shutdown or
	 * the last reference to this dma_buf instance is put. This local
	 * copy replaces the 'release' callback with nvgpu's custom
	 * release function handler. This custom function handler frees the
	 * priv structure and replaces back the original pointer associated
	 * with the 'producer' of the dma_buf.
	 */
	struct dma_buf_ops local_ops;
	/* Store a copy of the original ops for later restoration */
	const struct dma_buf_ops *previous_ops;

	/* list node for tracking the dmabuf_priv instances per gpu */
	struct nvgpu_list_node list;
};
#endif

struct sg_table *nvgpu_mm_pin(struct device *dev,
			struct dma_buf *dmabuf,
			struct dma_buf_attachment **attachment,
			enum dma_data_direction direction);

void nvgpu_mm_unpin(struct device *dev,
		struct dma_buf *dmabuf,
		struct dma_buf_attachment *attachment,
		struct sg_table *sgt);

#ifdef CONFIG_NVGPU_COMPRESSION
void gk20a_mm_delete_priv(struct gk20a_dmabuf_priv *priv);

int gk20a_dmabuf_alloc_or_get_drvdata(struct dma_buf *dmabuf, struct device *dev,
				      struct gk20a_dmabuf_priv **priv_ptr);

int gk20a_dmabuf_get_state(struct dma_buf *dmabuf, struct gk20a *g,
			   u64 offset, struct gk20a_buffer_state **state);

void gk20a_dma_buf_priv_list_clear(struct nvgpu_os_linux *l);
struct gk20a_dmabuf_priv *gk20a_dma_buf_get_drvdata(
		struct dma_buf *dmabuf, struct device *device);
int gk20a_dmabuf_map_private(struct dma_buf *dmabuf, struct gk20a *g,
			      struct sg_table *pages);

#endif

void *gk20a_dmabuf_vmap(struct dma_buf *dmabuf);
void gk20a_dmabuf_vunmap(struct dma_buf *dmabuf, void *addr);

#endif
