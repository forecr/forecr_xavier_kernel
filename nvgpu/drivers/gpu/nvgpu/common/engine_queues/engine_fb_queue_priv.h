/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ENGINE_FB_QUEUE_PRIV_H
#define NVGPU_ENGINE_FB_QUEUE_PRIV_H

#include <nvgpu/lock.h>

struct nvgpu_engine_fb_queue {
	struct gk20a *g;
	u32 flcn_id;

	/* used by nvgpu, for command LPQ/HPQ */
	struct nvgpu_mutex mutex;

	/* current write position */
	u32 position;
	/* logical queue identifier */
	u32 id;
	/* physical queue index */
	u32 index;
	/* in bytes */
	u32 size;
	/* open-flag */
	u32 oflag;

	/* members unique to the FB version of the falcon queues */
	struct {
		/* Holds super surface base address */
		struct nvgpu_mem *super_surface_mem;

		/*
		 * Holds the offset of queue data (0th element).
		 * This is used for FB Queues to hold a offset of
		 * Super Surface for this queue.
		 */
		 u32 fb_offset;

		/*
		 * Define the size of a single queue element.
		 * queues_size above is used for the number of
		 * queue elements.
		 */
		u32 element_size;

		/* To keep track of elements in use */
		u64 element_in_use;

		/*
		 * Define a pointer to a local (SYSMEM) allocated
		 * buffer to hold a single queue element
		 * it is being assembled.
		 */
		 u8 *work_buffer;
		 struct nvgpu_mutex work_buffer_mutex;

		/*
		 * Tracks how much of the current FB Queue MSG queue
		 * entry have been read. This is needed as functions read
		 * the MSG queue as a byte stream, rather
		 * than reading a whole MSG at a time.
		 */
		u32 read_position;

		/*
		 * Tail as tracked on the nvgpu "side".  Because the queue
		 * elements and its associated payload (which is also moved
		 * PMU->nvgpu through the FB CMD Queue) can't be free-ed until
		 * the command is complete, response is received and any "out"
		 * payload delivered to the client, it is necessary for the
		 * nvgpu to track it's own version of "tail".  This one is
		 * incremented as commands and completed entries are found
		 * following tail.
		 */
		u32 tail;
	} fbq;

	/* engine and queue specific ops */
	int (*tail)(struct nvgpu_engine_fb_queue *queue, u32 *tail, bool set);
	int (*head)(struct nvgpu_engine_fb_queue *queue, u32 *head, bool set);

	/* engine specific ops */
	int (*queue_head)(struct gk20a *g, u32 queue_id, u32 queue_index,
		u32 *head, bool set);
	int (*queue_tail)(struct gk20a *g, u32 queue_id, u32 queue_index,
		u32 *tail, bool set);
};

#endif /* NVGPU_ENGINE_FB_QUEUE_PRIV_H */
