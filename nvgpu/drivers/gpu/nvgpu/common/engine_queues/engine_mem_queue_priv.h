/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ENGINE_MEM_QUEUE_PRIV_H
#define NVGPU_ENGINE_MEM_QUEUE_PRIV_H

#include <nvgpu/lock.h>
#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_falcon;

struct nvgpu_engine_mem_queue {
	struct gk20a *g;
	u32 flcn_id;

	/* Queue Type (engine_queue_type) */
	u8 engine_queue_type;

	/* used by nvgpu, for command LPQ/HPQ */
	struct nvgpu_mutex mutex;

	/* current write position */
	u32 position;
	/* physical dmem offset where this queue begins */
	u32 offset;
	/* logical queue identifier */
	u32 id;
	/* physical queue index */
	u32 index;
	/* in bytes */
	u32 size;
	/* open-flag */
	u32 oflag;

	/* queue type(DMEM-Q/EMEM-Q) specific ops */
	int (*push)(struct nvgpu_falcon *flcn,
		    struct nvgpu_engine_mem_queue *queue,
		    u32 dst, void *data, u32 size);
	int (*pop)(struct nvgpu_falcon *flcn,
		   struct nvgpu_engine_mem_queue *queue,
		   u32 src, void *data, u32 size);

	/* engine specific ops */
	int (*head)(struct gk20a *g, u32 queue_id, u32 queue_index,
		u32 *head, bool set);
	int (*tail)(struct gk20a *g, u32 queue_id, u32 queue_index,
		u32 *tail, bool set);
};

#endif /* NVGPU_ENGINE_MEM_QUEUE_PRIV_H */
