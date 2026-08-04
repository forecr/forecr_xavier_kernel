/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ENGINE_FB_QUEUE_H
#define NVGPU_ENGINE_FB_QUEUE_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_engine_fb_queue;

struct nvgpu_engine_fb_queue_params {
	struct gk20a *g;
	u32 flcn_id;

	/* logical queue identifier */
	u32 id;
	/* physical queue index */
	u32 index;
	/* in bytes */
	u32 size;
	/* open-flag */
	u32 oflag;

	/* fb queue params*/
	/* Holds the offset of queue data (0th element) */
	u32 fbq_offset;

	/* fb queue element size*/
	u32 fbq_element_size;

	/* Holds super surface base address */
	struct nvgpu_mem *super_surface_mem;

	/* engine specific ops */
	int (*queue_head)(struct gk20a *g, u32 queue_id, u32 queue_index,
		u32 *head, bool set);
	int (*queue_tail)(struct gk20a *g, u32 queue_id, u32 queue_index,
		u32 *tail, bool set);
};

/* queue public functions */
int nvgpu_engine_fb_queue_init(struct nvgpu_engine_fb_queue **queue_p,
	struct nvgpu_engine_fb_queue_params params);
bool nvgpu_engine_fb_queue_is_empty(struct nvgpu_engine_fb_queue *queue);
int nvgpu_engine_fb_queue_pop(struct nvgpu_engine_fb_queue *queue,
	void *data, u32 size, u32 *bytes_read);
int nvgpu_engine_fb_queue_push(struct nvgpu_engine_fb_queue *queue,
	void *data, u32 size);
void nvgpu_engine_fb_queue_free(struct nvgpu_engine_fb_queue **queue_p);
u32 nvgpu_engine_fb_queue_get_position(struct nvgpu_engine_fb_queue *queue);
u32 nvgpu_engine_fb_queue_get_element_size(struct nvgpu_engine_fb_queue *queue);
u32 nvgpu_engine_fb_queue_get_offset(struct nvgpu_engine_fb_queue *queue);
u8 *nvgpu_engine_fb_queue_get_work_buffer(struct nvgpu_engine_fb_queue *queue);
int nvgpu_engine_fb_queue_free_element(struct nvgpu_engine_fb_queue *queue,
		u32 queue_pos);
void nvgpu_engine_fb_queue_lock_work_buffer(
	struct nvgpu_engine_fb_queue *queue);
void nvgpu_engine_fb_queue_unlock_work_buffer(
	struct nvgpu_engine_fb_queue *queue);

#endif /* NVGPU_ENGINE_FB_QUEUE_H */
