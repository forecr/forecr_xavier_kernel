/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_SEC2_QUEUE_H
#define NVGPU_SEC2_QUEUE_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_falcon;
struct nv_flcn_cmd_sec2;
struct nvgpu_engine_mem_queue;
struct sec2_init_msg_sec2_init;

int nvgpu_sec2_queues_init(struct gk20a *g,
			   struct nvgpu_engine_mem_queue **queues,
			   struct sec2_init_msg_sec2_init *init);
void nvgpu_sec2_queues_free(struct gk20a *g,
			    struct nvgpu_engine_mem_queue **queues);
u32 nvgpu_sec2_queue_get_size(struct nvgpu_engine_mem_queue **queues,
			      u32 queue_id);
int nvgpu_sec2_queue_push(struct nvgpu_engine_mem_queue **queues,
			  u32 queue_id, struct nvgpu_falcon *flcn,
			  struct nv_flcn_cmd_sec2 *cmd, u32 size);
bool nvgpu_sec2_queue_is_empty(struct nvgpu_engine_mem_queue **queues,
			       u32 queue_id);
bool nvgpu_sec2_queue_read(struct gk20a *g,
			   struct nvgpu_engine_mem_queue **queues,
			   u32 queue_id, struct nvgpu_falcon *flcn, void *data,
			   u32 bytes_to_read, int *status);
int nvgpu_sec2_queue_rewind(struct nvgpu_falcon *flcn,
			    struct nvgpu_engine_mem_queue **queues,
			    u32 queue_id);

#endif /* NVGPU_SEC2_QUEUE_H */
