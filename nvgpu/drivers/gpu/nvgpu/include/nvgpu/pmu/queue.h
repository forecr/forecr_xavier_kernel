/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_QUEUE_H
#define NVGPU_PMU_QUEUE_H

#include <nvgpu/pmu/pmuif/cmn.h>
#include <nvgpu/types.h>

union pmu_init_msg_pmu;
struct nvgpu_falcon;
struct nvgpu_mem;
struct pmu_cmd;
struct gk20a;

struct pmu_queues {
	struct nvgpu_engine_fb_queue *fb_queue[PMU_QUEUE_COUNT];
	struct nvgpu_engine_mem_queue *queue[PMU_QUEUE_COUNT];
	u32 engine_queue_type;
};

int nvgpu_pmu_queues_init(struct gk20a *g,
			  union pmu_init_msg_pmu *init,
			  struct pmu_queues *queues,
			  struct nvgpu_mem *super_surface_buf);

void nvgpu_pmu_queues_free(struct gk20a *g, struct pmu_queues *queues);

bool nvgpu_pmu_queue_is_empty(struct pmu_queues *queues, u32 queue_id);
u32 nvgpu_pmu_queue_get_size(struct pmu_queues *queues, u32 queue_id);
int nvgpu_pmu_queue_push(struct pmu_queues *queues, struct nvgpu_falcon *flcn,
			 u32 queue_id, struct pmu_cmd *cmd);
int nvgpu_pmu_queue_pop(struct pmu_queues *queues, struct nvgpu_falcon *flcn,
			u32 queue_id, void *data, u32 bytes_to_read,
			u32 *bytes_read);

bool nvgpu_pmu_fb_queue_enabled(struct pmu_queues *queues);
struct nvgpu_engine_fb_queue *nvgpu_pmu_fb_queue(struct pmu_queues *queues,
						 u32 queue_id);
int nvgpu_pmu_queue_rewind(struct pmu_queues *queues, u32 queue_id,
			   struct nvgpu_falcon *flcn);

#endif /* NVGPU_PMU_QUEUE_H */
