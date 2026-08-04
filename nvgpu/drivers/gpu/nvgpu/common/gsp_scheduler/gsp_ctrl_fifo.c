// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/gsp.h>
#include <nvgpu/gsp_sched.h>

#include "gsp_runlist.h"
#include "gsp_scheduler.h"
#include "gsp_ctrl_fifo.h"
#include "common/gsp/ipc/gsp_cmd.h"

#ifdef CONFIG_NVS_PRESENT
/**
 * @brief Retrieves information about a GSP control FIFO queue and populates the
 *        control FIFO structure with this information.
 *
 * The steps performed by the function are as follows:
 * -# Determine the queue type based on the queue direction parameter. If the
 *    direction is from client to scheduler, set the queue type to #CONTROL_QUEUE.
 *    If the direction is from scheduler to client, set it to #RESPONSE_QUEUE.
 *    If the direction is neither, log an error, set the error code to -EINVAL,
 *    and go to the exit label.
 * -# Check if the queue pointer is NULL. If it is, log an error, set the error
 *    code to -EFAULT, and go to the exit label.
 * -# Populate the control FIFO structure with the lower 32 bits of the queue's
 *    GPU virtual address.
 * -# Populate the control FIFO structure with the higher 32 bits of the queue's
 *    GPU virtual address.
 * -# Set the queue size in the control FIFO structure to the predefined constant
 *    #GSP_CTRL_FIFO_QUEUE_SIZE.
 * -# Set the number of queue entries in the control FIFO structure to the
 *    predefined constant #GSP_CTRL_FIFO_QUEUE_ENTRIES.
 * -# Set the queue type in the control FIFO structure to the determined queue
 *    type.
 * -# Return the error code, which will be 0 if no errors occurred.
 *
 * @param [in] g                  The GPU structure.
 * @param [out] ctrl_fifo         The control FIFO information structure to be populated.
 * @param [in] queue              The NVS control queue for which information is being retrieved.
 * @param [in] queue_direction    The direction of the queue, indicating whether it is for
 *                                client to scheduler or scheduler to client communication.
 *
 * @return 0 if the control FIFO information is successfully retrieved.
 * @return -EINVAL if the queue direction is not supported.
 * @return -EFAULT if the queue pointer is NULL.
 */
static int gsp_ctrl_fifo_get_queue_info(struct gk20a *g,
	struct nvgpu_gsp_ctrl_fifo_info *ctrl_fifo, struct nvgpu_nvs_ctrl_queue *queue,
	enum nvgpu_nvs_ctrl_queue_direction queue_direction)
{
	int err = 0;
	enum queue_type qtype;

	if (queue_direction == NVGPU_NVS_DIR_CLIENT_TO_SCHEDULER) {
		qtype = CONTROL_QUEUE;
	} else if (queue_direction == NVGPU_NVS_DIR_SCHEDULER_TO_CLIENT) {
		qtype = RESPONSE_QUEUE;
	} else {
		nvgpu_err(g, "GSP queue type not supported");
		err = -EINVAL;
		goto exit;
	}

	if (queue == NULL) {
		nvgpu_err(g, "GSP ctrlfifo queue is null");
		err = -EFAULT;
		goto exit;
	}
	ctrl_fifo->fifo_addr_lo = u64_lo32(queue->mem.gpu_va);
	ctrl_fifo->fifo_addr_hi = u64_hi32(queue->mem.gpu_va);
	ctrl_fifo->queue_size = GSP_CTRL_FIFO_QUEUE_SIZE;
	ctrl_fifo->queue_entries = GSP_CTRL_FIFO_QUEUE_ENTRIES;
	ctrl_fifo->qtype  = (u32)qtype;

exit:
	return err;

}

int nvgpu_gsp_sched_send_queue_info(struct gk20a *g, struct nvgpu_nvs_ctrl_queue *queue,
	enum nvgpu_nvs_ctrl_queue_direction queue_direction)
{
	int err = 0;
	struct nv_flcn_cmd_gsp cmd = { };
	struct nvgpu_gsp_ctrl_fifo_info ctrl_fifo = {};

	/* getting the queue info */
	err = gsp_ctrl_fifo_get_queue_info(g, &ctrl_fifo, queue, queue_direction);
	if (err != 0) {
		nvgpu_err(g, "getting fifo queue info failed");
		goto exit;
	}

	/* updating the command with control fifo info */
	cmd.cmd.ctrl_fifo.fifo_addr_lo = ctrl_fifo.fifo_addr_lo;
	cmd.cmd.ctrl_fifo.fifo_addr_hi = ctrl_fifo.fifo_addr_hi;
	cmd.cmd.ctrl_fifo.queue_size = ctrl_fifo.queue_size;
	cmd.cmd.ctrl_fifo.qtype = ctrl_fifo.qtype;
	cmd.cmd.ctrl_fifo.queue_entries = ctrl_fifo.queue_entries;

	/* sending control fifo info to GSP */
	err = gsp_send_cmd_and_wait_for_ack(g, &cmd, NV_GSP_UNIT_CONTROL_INFO_SEND,
	nvgpu_safe_cast_u64_to_u32(sizeof(struct nvgpu_gsp_ctrl_fifo_info)));
	if (err != 0) {
		nvgpu_err(g, "sending control fifo queue to GSP failed");
	}

exit:
	return err;
}

void nvgpu_gsp_sched_erase_ctrl_fifo(struct gk20a *g,
	enum nvgpu_nvs_ctrl_queue_direction queue_direction, bool is_abrupt_close)
{
	int err = 0;
	struct nv_flcn_cmd_gsp cmd = { };
	enum queue_type qtype;

	/*  updating the abrupt flag */
	cmd.cmd.fifo_close.is_abrupt_close = is_abrupt_close;

	/* populating command with only queue direction */
	if (queue_direction == NVGPU_NVS_DIR_CLIENT_TO_SCHEDULER) {
		qtype = CONTROL_QUEUE;
	} else if (queue_direction == NVGPU_NVS_DIR_SCHEDULER_TO_CLIENT) {
		qtype = RESPONSE_QUEUE;
	} else {
		nvgpu_err(g, "Erase queue failed queue type not supported");
		return;
	}

	cmd.cmd.fifo_close.qtype = (u32)qtype;

	err = gsp_send_cmd_and_wait_for_ack(g, &cmd, NV_GSP_UNIT_CONTROL_FIFO_ERASE,
			nvgpu_safe_cast_u64_to_u32(sizeof(struct nvgpu_gsp_ctrl_fifo_info)));
	if (err != 0) {
		nvgpu_err(g, "GSP ctrl fifo erase cmd failed");
	}

};
#endif /* CONFIG_NVS_PRESENT*/
