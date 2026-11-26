// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/pmu.h>
#include <nvgpu/log.h>
#include <nvgpu/gsp.h>

#include "gsp_seq.h"
#include "gsp_queue.h"
#include "gsp_cmd.h"

bool gsp_unit_id_is_valid(u8 id)
{
	return (id < NV_GSP_UNIT_END);
}

/**
 * @brief Validates a GSP command by checking its queue ID, size, and unit ID.
 *
 * The steps performed by the function are as follows:
 * -# Check if the queue ID of the command matches the expected #GSP_NV_CMDQ_LOG_ID.
 *    If it does not, proceed to the invalid command handling.
 * -# Check if the command header size is less than the minimum #PMU_CMD_HDR_SIZE.
 *    If it is, proceed to the invalid command handling.
 * -# Retrieve the size of the queue specified by the queue ID using nvgpu_gsp_queue_get_size().
 * -# Check if the command header size is greater than half of the queue size.
 *    If it is, proceed to the invalid command handling.
 * -# Check if the unit ID in the command header is valid using gsp_unit_id_is_valid().
 *    If it is not, proceed to the invalid command handling.
 * -# If all validations pass, return true indicating the command is valid.
 * -# If the command is determined to be invalid at any validation step, log an error
 *    message with the queue ID, command size, and unit ID, then return false.
 *
 * @param [in] gsp        The GSP structure.
 * @param [in] cmd        The GSP command to validate.
 * @param [in] queue_id   The ID of the queue to which the command is to be submitted.
 *
 * @return true if the command is valid.
 * @return false if the command is invalid, after logging an error message.
 */
static bool gsp_validate_cmd(struct nvgpu_gsp *gsp,
	struct nv_flcn_cmd_gsp *cmd, u32 queue_id)
{
	struct gk20a *g = gsp->g;
	u32 queue_size;

	if (queue_id != GSP_NV_CMDQ_LOG_ID) {
		goto invalid_cmd;
	}

	if (cmd->hdr.size < PMU_CMD_HDR_SIZE) {
		goto invalid_cmd;
	}

	queue_size = nvgpu_gsp_queue_get_size(gsp->gsp_ipc->queues, queue_id);

	if (cmd->hdr.size > (queue_size >> 1)) {
		goto invalid_cmd;
	}

	if (gsp_unit_id_is_valid(cmd->hdr.unit_id) == false) {
		goto invalid_cmd;
	}

	return true;

invalid_cmd:
	nvgpu_err(g, "invalid gsp cmd :");
	nvgpu_err(g, "queue_id=%d, cmd_size=%d, cmd_unit_id=%d\n",
		queue_id, cmd->hdr.size, cmd->hdr.unit_id);

	return false;
}

/**
 * @brief Writes a command to the specified GSP queue with a timeout mechanism.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a timeout structure with the specified timeout in milliseconds
 *    using nvgpu_timeout_init_cpu_timer().
 * -# Enter a loop to attempt to push the command onto the GSP queue.
 *    -# Attempt to push the command onto the queue using nvgpu_gsp_queue_push().
 *    -# Check if the timeout has expired using nvgpu_timeout_expired().
 *    -# If the push operation returns -EAGAIN (queue full) and the timeout has not
 *       expired, sleep for a short duration using nvgpu_usleep_range() and then retry.
 *    -# If the push operation is successful or a timeout occurs, exit the loop.
 * -# If an error occurs during the push operation, log an error message indicating
 *    the failure to write the command to the queue.
 * -# Return the error code, which will be 0 if the command is successfully written
 *    to the queue or a non-zero error code if an error occurs.
 *
 * @param [in] gsp        The GSP structure.
 * @param [in] cmd        The GSP command to be written to the queue.
 * @param [in] queue_id   The ID of the queue to which the command is to be written.
 * @param [in] timeout_ms The timeout in milliseconds for the write operation.
 *
 * @return 0 if the command is successfully written to the queue.
 * @return Non-zero error code if the command could not be written to the queue.
 */
static int gsp_write_cmd(struct nvgpu_gsp *gsp,
	struct nv_flcn_cmd_gsp *cmd, u32 queue_id,
	u32 timeout_ms)
{
	struct nvgpu_timeout timeout;
	struct gk20a *g = gsp->g;
	int err = 0;
	s32 is_timedout = 0;

	nvgpu_gsp_dbg(g, " ");

	nvgpu_timeout_init_cpu_timer(g, &timeout, timeout_ms);

	do {
		err = nvgpu_gsp_queue_push(gsp->gsp_ipc->queues, queue_id, gsp->gsp_flcn,
						cmd, cmd->hdr.size);
		is_timedout = nvgpu_timeout_expired(&timeout);

		if ((err == -EAGAIN) && (is_timedout == 0)) {
				nvgpu_usleep_range(1000U, 2000U);
		} else {
			break;
		}
	} while (true);

	if (err != 0) {
		nvgpu_err(g, "fail to write cmd to queue %d", queue_id);
	}

	return err;
}

int nvgpu_gsp_cmd_post(struct gk20a *g, struct nv_flcn_cmd_gsp *cmd,
	u32 queue_id, gsp_callback callback,
	void *cb_param, u32 timeout)
{
	struct nvgpu_gsp *gsp = g->gsp;
	struct nvgpu_gsp_ipc *gsp_ipc = g->gsp->gsp_ipc;
	struct gsp_sequence *seq = NULL;
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	if (cmd == NULL) {
		nvgpu_err(g, "gsp cmd buffer is empty");
		err = -EINVAL;
		goto exit;
	}
	/* Sanity check the command input. */
	if (!gsp_validate_cmd(gsp, cmd, queue_id)) {
		err = -EINVAL;
		goto exit;
	}

	/* Attempt to reserve a sequence for this command. */
	err = nvgpu_gsp_seq_acquire(g, gsp_ipc->sequences, &seq,
			callback, cb_param);
	if (err != 0) {
		goto exit;
	}

	/* Set the sequence number in the command header. */
	cmd->hdr.seq_id = nvgpu_gsp_seq_get_id(seq);

	cmd->hdr.ctrl_flags = PMU_CMD_FLAGS_STATUS;

	nvgpu_gsp_seq_set_state(seq, GSP_SEQ_STATE_USED);
	nvgpu_gsp_dbg(g, "GSP cmd->hdr.unit_id:0x%x", cmd->hdr.unit_id);
	nvgpu_gsp_dbg(g, "GSP cmd->hdr.size:0x%x", cmd->hdr.size);
	nvgpu_gsp_dbg(g, "GSP cmd->hdr.ctrl_flags:0x%x", cmd->hdr.ctrl_flags);
	nvgpu_gsp_dbg(g, "GSP cmd->hdr.seq_id:0x%x", cmd->hdr.seq_id);
	nvgpu_gsp_dbg(g, "GSP cmd->cmd.acr_cmd.cmd_type:0x%x", cmd->cmd.acr_cmd.cmd_type);

	err = gsp_write_cmd(gsp, cmd, queue_id, timeout);
	if (err != 0) {
		gsp_seq_release(gsp_ipc->sequences, seq);
	}

exit:
	return err;
}
