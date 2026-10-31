// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/engine_mem_queue.h>
#include <nvgpu/engine_queue.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/gsp.h>

#include "gsp_queue.h"
#include "gsp_msg.h"

/**
 * @brief Initializes a GSP queue based on the provided ID and initialization parameters.
 *
 * The steps performed by the function are as follows:
 * -# Initialize the queue parameters structure with default values.
 * -# Determine the operation flag (oflag) based on the queue ID. If the ID corresponds
 *    to the command queue #GSP_NV_CMDQ_LOG_ID, set the flag to #OFLAG_WRITE, indicating
 *    that the nvgpu will push and the falcon ucode will pop from the queue. Otherwise,
 *    set the flag to #OFLAG_READ for the message queue, indicating that the falcon ucode
 *    will push and the nvgpu will pop from the queue.
 * -# Retrieve the logical queue ID from the initialization parameters.
 * -# Populate the queue parameters structure with values from the initialization parameters
 *    and the determined operation flag.
 * -# Call nvgpu_engine_mem_queue_init() to initialize the memory queue with the populated
 *    parameters.
 * -# If the initialization fails, log an error message and return the error code.
 *
 * @param [in] g        The GPU structure.
 * @param [out] queues  The array of engine memory queues to be initialized.
 * @param [in] id       The ID of the queue to initialize.
 * @param [in] init     The initialization parameters for the GSP queues.
 *
 * @return 0 if the queue is successfully initialized.
 * @return Non-zero error code if the initialization fails.
 */
static int gsp_queue_init(struct gk20a *g,
			struct nvgpu_engine_mem_queue **queues, u32 id,
			struct gsp_init_msg_gsp_init *init)
{
	struct nvgpu_engine_mem_queue_params params = {0};
	u32 queue_log_id = 0;
	u32 oflag = 0;
	int err = 0;

	if (id == GSP_NV_CMDQ_LOG_ID) {
		/*
		 * set OFLAG_WRITE for command queue
		 * i.e, push from nvgpu &
		 * pop form falcon ucode
		 */
		oflag = OFLAG_WRITE;
	} else {
		/*
		 * set OFLAG_READ for message queue
		 * i.e, push from falcon ucode &
		 * pop form nvgpu
		 */
		oflag = OFLAG_READ;
	}

	/* init queue parameters */
	queue_log_id = init->q_info[id].queue_log_id;

	params.g = g;
	params.flcn_id = FALCON_ID_GSPLITE;
	params.id = queue_log_id;
	params.index = init->q_info[id].queue_phy_id;
	params.offset = init->q_info[id].queue_offset;
	params.position = init->q_info[id].queue_offset;
	params.size = init->q_info[id].queue_size;
	params.oflag = oflag;
	params.queue_head = g->ops.gsp.gsp_queue_head;
	params.queue_tail = g->ops.gsp.gsp_queue_tail;
	params.engine_queue_type = QUEUE_TYPE_EMEM;
	if (g->ops.gsp.gsp_queue_type != NULL) {
		params.engine_queue_type = QUEUE_TYPE_DMEM;
	}

	err = nvgpu_engine_mem_queue_init(&queues[queue_log_id],
				      params);
	if (err != 0) {
		nvgpu_err(g, "queue-%d init failed", queue_log_id);
	}

	return err;
}

/**
 * @brief Frees a GSP queue based on the provided queue ID.
 *
 * The steps performed by the function are as follows:
 * -# Check if the queue at the given ID is NULL. If it is, log an error and return early.
 * -# Call nvgpu_engine_mem_queue_free() to free the memory queue at the given ID.
 *
 * @param [in] g       The GPU structure.
 * @param [in] queues  The array of engine memory queues from which the queue is to be freed.
 * @param [in] id      The ID of the queue to be freed.
 */
static void gsp_queue_free(struct gk20a *g,
			struct nvgpu_engine_mem_queue **queues, u32 id)
{

	if (queues[id] == NULL) {
		nvgpu_err(g, " queues[id] is NULL");
		return;
	}

	nvgpu_engine_mem_queue_free(&queues[id]);
}

int nvgpu_gsp_queues_init(struct gk20a *g,
			struct nvgpu_engine_mem_queue **queues,
			struct gsp_init_msg_gsp_init *init)
{
	u32 i, j;
	int err;

	for (i = 0; i < GSP_QUEUE_NUM; i++) {
		err = gsp_queue_init(g, queues, i, init);
		if (err != 0) {
			for (j = 0; j < i; j++) {
				gsp_queue_free(g, queues, j);
			}
			nvgpu_err(g, "GSP queue init failed");
			return err;
		}
	}

	return 0;
}

void nvgpu_gsp_queues_free(struct gk20a *g,
			struct nvgpu_engine_mem_queue **queues)
{
	u32 i;

	for (i = 0; i < GSP_QUEUE_NUM; i++) {
		gsp_queue_free(g, queues, i);
	}
}

u32 nvgpu_gsp_queue_get_size(struct nvgpu_engine_mem_queue **queues,
			u32 queue_id)
{
	return nvgpu_engine_mem_queue_get_size(queues[queue_id]);
}

int nvgpu_gsp_queue_push(struct nvgpu_engine_mem_queue **queues,
			u32 queue_id, struct nvgpu_falcon *flcn,
			struct nv_flcn_cmd_gsp *cmd, u32 size)
{
	s32 err;
	struct nvgpu_engine_mem_queue *queue;

	queue = queues[queue_id];
	err = nvgpu_engine_mem_queue_push(flcn, queue, cmd, size);
	if (err != 0) {
		nvgpu_err(flcn->g, "fail to push cmd to queue %d", queue_id);
	}

	return err;
}

bool nvgpu_gsp_queue_is_empty(struct nvgpu_engine_mem_queue **queues,
			u32 queue_id)
{
	struct nvgpu_engine_mem_queue *queue = queues[queue_id];

	return nvgpu_engine_mem_queue_is_empty(queue);
}

bool nvgpu_gsp_queue_read(struct gk20a *g,
			struct nvgpu_engine_mem_queue **queues,
			u32 queue_id, struct nvgpu_falcon *flcn, void *data,
			u32 bytes_to_read, int *status)
{
	struct nvgpu_engine_mem_queue *queue = queues[queue_id];
	u32 bytes_read;
	int err;

	err = nvgpu_engine_mem_queue_pop(flcn, queue, data,
			bytes_to_read, &bytes_read);
	if (err != 0) {
		nvgpu_err(g, "fail to read msg: err %d", err);
		*status = err;
		return false;
	}
	if (bytes_read != bytes_to_read) {
		nvgpu_err(g, "fail to read requested bytes: 0x%x != 0x%x",
			bytes_to_read, bytes_read);
		*status = -EINVAL;
		return false;
	}

	return true;
}

int nvgpu_gsp_queue_rewind(struct nvgpu_falcon *flcn,
			struct nvgpu_engine_mem_queue **queues,
			u32 queue_id)
{
	s32 err;
	struct nvgpu_engine_mem_queue *queue = queues[queue_id];

	err = nvgpu_engine_mem_queue_rewind(flcn, queue);
	if (err != 0) {
		nvgpu_err(flcn->g, "fail to rewind queue %d", queue_id);
	}

	return err;
}
