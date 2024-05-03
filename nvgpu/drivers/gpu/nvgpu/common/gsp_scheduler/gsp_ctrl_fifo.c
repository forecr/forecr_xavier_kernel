/*
 * Copyright (c) 2022-2023, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/gsp.h>
#include <nvgpu/gsp_sched.h>

#include "gsp_runlist.h"
#include "gsp_scheduler.h"
#include "gsp_ctrl_fifo.h"
#include "ipc/gsp_cmd.h"

#ifdef CONFIG_NVS_PRESENT
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
	ctrl_fifo->qtype  = qtype;

exit:
	return err;

}

/* get and send the control fifo info to gsp */
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
	sizeof(struct nvgpu_gsp_ctrl_fifo_info));
	if (err != 0) {
		nvgpu_err(g, "sending control fifo queue to GSP failed");
	}

exit:
	return err;
}

int nvgpu_gsp_sched_erase_ctrl_fifo(struct gk20a *g,
	enum nvgpu_nvs_ctrl_queue_direction queue_direction)
{
	int err = 0;
	struct nv_flcn_cmd_gsp cmd = { };
	enum queue_type qtype;

	/* populating command with only queue direction */
	cmd.cmd.ctrl_fifo.fifo_addr_lo = 0U;
	cmd.cmd.ctrl_fifo.fifo_addr_hi = 0U;
	cmd.cmd.ctrl_fifo.queue_entries = 0U;
	cmd.cmd.ctrl_fifo.queue_size = 0U;

	if (queue_direction == NVGPU_NVS_DIR_CLIENT_TO_SCHEDULER) {
		qtype = CONTROL_QUEUE;
	} else if (queue_direction == NVGPU_NVS_DIR_SCHEDULER_TO_CLIENT) {
		qtype = RESPONSE_QUEUE;
	} else {
		nvgpu_err(g, "Erase queue failed queue type not supported");
		err = -EINVAL;
		goto exit;
	}

	cmd.cmd.ctrl_fifo.qtype = (u32)qtype;

	err = gsp_send_cmd_and_wait_for_ack(g, &cmd, NV_GSP_UNIT_CONTROL_FIFO_ERASE,
			sizeof(struct nvgpu_gsp_ctrl_fifo_info));
	if (err != 0) {
		nvgpu_err(g, "GSP ctrl fifo erase cmd failed");
	}

exit:
	return err;
};
#endif /* CONFIG_NVS_PRESENT*/