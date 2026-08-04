// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/engine_queue.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/gsp.h>

#include "gsp_queue.h"
#include "gsp_msg.h"
#include "gsp_seq.h"

/**
 * @brief Handles the response from the GSP based on the message received.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the GPU structure from the GSP ipc structure.
 * -# Call nvgpu_gsp_seq_response_handle() to handle the response for the
 *    sequence associated with the message, using the sequence ID from the message header.
 * -# Return the status from the nvgpu_gsp_seq_response_handle() call, which indicates
 *    the success or failure of the response handling.
 *
 * @param [in] gsp        The GSP structure.
 * @param [in] msg        The message received from the GSP for which the response is to be handled.
 *
 * @return 0 if the response is successfully handled.
 * @return Non-zero error code if there is an error in handling the response.
 */
static int gsp_response_handle(struct nvgpu_gsp *gsp,
	struct nv_flcn_msg_gsp *msg)
{
	struct gk20a *g = gsp->g;

	nvgpu_gsp_dbg(g, " ");

	return nvgpu_gsp_seq_response_handle(g, gsp->gsp_ipc->sequences,
				msg, msg->hdr.seq_id);
}

/**
 * @brief Reads a message from the specified GSP queue and validates its contents.
 *
 * The steps performed by the function are as follows:
 * -# Initialize the status to 0.
 * -# Check if the GSP queue is empty using nvgpu_gsp_queue_is_empty(). If it is,
 *    return false indicating no message to read.
 * -# Attempt to read the message header from the queue using nvgpu_gsp_queue_read().
 *    If the read fails, log an error, update the status, and proceed to clean up.
 * -# If the message header indicates a rewind is necessary, perform the rewind using
 *    nvgpu_gsp_queue_rewind(). If the rewind fails, log an error, update the status,
 *    and proceed to clean up.
 * -# If a rewind was performed, attempt to read the message header again from the queue
 *    using nvgpu_gsp_queue_read(). If the read fails, log an error, update the status,
 *    and proceed to clean up.
 * -# Validate the unit ID in the message header using gsp_unit_id_is_valid(). If it is
 *    invalid, log an error, update the status, and proceed to clean up.
 * -# If the message size is greater than the header size, calculate the size to read and
 *    attempt to read the rest of the message from the queue using nvgpu_gsp_queue_read().
 *    If the read fails, log an error, update the status, and proceed to clean up.
 * -# Return true if the message is successfully read and validated, or false if any step
 *    fails during the process.
 *
 * @param [in] gsp            The GSP structure.
 * @param [in] queue_id       The ID of the queue from which the message is to be read.
 * @param [out] msg           The structure where the read message will be stored.
 * @param [out] status        The status of the read operation.
 *
 * @return true if the message is successfully read and validated.
 * @return false if the queue is empty, the read operation fails, or the message is invalid.
 */
static bool gsp_read_message(struct nvgpu_gsp *gsp,
	u32 queue_id, struct nv_flcn_msg_gsp *msg, int *status)
{
	struct gk20a *g = gsp->g;
	struct nvgpu_gsp_ipc *gsp_ipc = gsp->gsp_ipc;
	u32 read_size;
	int err;

	nvgpu_gsp_dbg(g, " ");

	*status = 0;

	if (nvgpu_gsp_queue_is_empty(gsp_ipc->queues, queue_id)) {
		return false;
	}

	if (!nvgpu_gsp_queue_read(g, gsp_ipc->queues, queue_id,
			gsp->gsp_flcn,  &msg->hdr,
				GSP_MSG_HDR_SIZE, status)) {
		nvgpu_err(g, "fail to read msg from queue %d", queue_id);
		goto clean_up;
	}

	if (msg->hdr.unit_id == NV_GSP_UNIT_REWIND) {
		err = nvgpu_gsp_queue_rewind(gsp->gsp_flcn,
				gsp_ipc->queues, queue_id);
		if (err != 0) {
			nvgpu_err(g, "fail to rewind queue %d", queue_id);
			*status = err;
			goto clean_up;
		}

		/* read again after rewind */
		if (!nvgpu_gsp_queue_read(g, gsp_ipc->queues, queue_id,
				gsp->gsp_flcn, &msg->hdr,
				GSP_MSG_HDR_SIZE, status)) {
			nvgpu_err(g, "fail to read msg from queue %d",
				queue_id);
			goto clean_up;
		}
	}

	if (gsp_unit_id_is_valid(msg->hdr.unit_id) == false) {
		nvgpu_err(g, "read invalid unit_id %d from queue %d",
			msg->hdr.unit_id, queue_id);
			*status = -EINVAL;
			goto clean_up;
	}

	if (msg->hdr.size > GSP_MSG_HDR_SIZE) {
		read_size = msg->hdr.size - GSP_MSG_HDR_SIZE;
		if (!nvgpu_gsp_queue_read(g, gsp_ipc->queues, queue_id,
				gsp->gsp_flcn, &msg->msg,
				read_size, status)) {
			nvgpu_err(g, "fail to read msg from queue %d",
				queue_id);
			goto clean_up;
		}
	}

	return true;

clean_up:
	return false;
}

/**
 * @brief Processes the initialization message for the GSP ipc.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the current message queue tail position using gops_gsp.msgq_tail().
 * -# Copy the message header from EMEM to the local message structure using
 *    nvgpu_falcon_copy_from_emem().
 * -# If the unit ID in the message header is not #NV_GSP_UNIT_INIT, log an error, set the error
 *    code to -EINVAL, and go to the exit label.
 * -# Check for potential unsigned integer wraparound when calculating the tail position plus
 *    the message header size. If a wraparound may occur, log an error, set the error code to
 *    -ERANGE, and go to the exit label.
 * -# Copy the rest of the message from EMEM to the local message structure using
 *    nvgpu_falcon_copy_from_emem().
 * -# If the message type is not #NV_GSP_INIT_MSG_ID_GSP_INIT, log an error, set the error code
 *    to -EINVAL, and go to the exit label.
 * -# Calculate the aligned size of the message based on the #GSP_DMEM_ALIGNMENT using #NVGPU_ALIGN.
 * -# Check for potential unsigned integer wraparound when calculating the tail position plus
 *    the aligned size. If a wraparound may occur, log an error, set the error code to -ERANGE,
 *    and go to the exit label.
 * -# Update the message queue tail position with the new aligned size using gops_gsp.msgq_tail().
 * -# Initialize the GSP queues with the information from the initialization message using
 *    nvgpu_gsp_queues_init().
 * -# If the GSP queues initialization fails, return the error code.
 * -# Set the GSP ipc to ready.
 * -# Return the error code, which will be 0 if the initialization message is processed successfully.
 *
 * @param [in] gsp        The GSP structure.
 * @param [in] msg        The initialization message to be processed.
 *
 * @return 0 if the initialization message is processed successfully.
 * @return -EINVAL if the message is not an initialization message or the message type is incorrect.
 * @return -ERANGE if there is a potential unsigned integer wraparound.
 */
static int gsp_process_init_msg(struct nvgpu_gsp *gsp,
	struct nv_flcn_msg_gsp *msg)
{
	struct gk20a *g = gsp->g;
	struct nvgpu_gsp_ipc *gsp_ipc = gsp->gsp_ipc;
	struct gsp_init_msg_gsp_init *gsp_init;
	u32 tail = 0U, aligned_size = 0U;
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	g->ops.gsp.msgq_tail(g, gsp, &tail, QUEUE_GET);

	if (g->ops.gsp.gsp_queue_type == NULL) {
		err = nvgpu_falcon_copy_from_emem(gsp->gsp_flcn, tail,
		    (u8 *)&msg->hdr, GSP_MSG_HDR_SIZE, 0U);
	} else {
#if defined(CONFIG_NVGPU_FALCON_DEBUG) || defined(CONFIG_NVGPU_FALCON_NON_FUSA)
		err = nvgpu_falcon_copy_from_dmem(gsp->gsp_flcn, tail,
		    (u8 *)&msg->hdr, GSP_MSG_HDR_SIZE, 0U);
#else
		err = -EINVAL;
#endif
	}
	if (err != 0) {
		goto exit;
	}

	if (msg->hdr.unit_id != NV_GSP_UNIT_INIT) {
		nvgpu_err(g, "expecting init msg");
		err = -EINVAL;
		goto exit;
	}

	if (UINT_MAX - tail < GSP_MSG_HDR_SIZE) {
		nvgpu_err(g, "Unsigned integer operation may wrap.");
		err = -ERANGE;
		goto exit;
	}

	if (g->ops.gsp.gsp_queue_type == NULL) {
		err = nvgpu_falcon_copy_from_emem(gsp->gsp_flcn, tail + GSP_MSG_HDR_SIZE,
			(u8 *)&msg->msg, msg->hdr.size - GSP_MSG_HDR_SIZE, 0U);
	} else {
#if defined(CONFIG_NVGPU_FALCON_DEBUG) || defined(CONFIG_NVGPU_FALCON_NON_FUSA)
		err = nvgpu_falcon_copy_from_dmem(gsp->gsp_flcn, tail + GSP_MSG_HDR_SIZE,
			(u8 *)&msg->msg, msg->hdr.size - GSP_MSG_HDR_SIZE, 0U);
#else
		err = -EINVAL;
#endif
	}
	if (err != 0) {
		goto exit;
	}

	if (msg->msg.init.msg_type != NV_GSP_INIT_MSG_ID_GSP_INIT) {
		nvgpu_err(g, "expecting init msg");
		err = -EINVAL;
		goto exit;
	}

	aligned_size = NVGPU_ALIGN((u32)msg->hdr.size, GSP_DMEM_ALIGNMENT);

	if (UINT_MAX - tail < aligned_size) {
		nvgpu_err(g, "Unsigned integer operation may wrap.");
		err = -ERANGE;
		goto exit;
	}

	tail += aligned_size;
	g->ops.gsp.msgq_tail(g, gsp, &tail, QUEUE_SET);

	gsp_init = &msg->msg.init.gsp_init;

	err = nvgpu_gsp_queues_init(g, gsp_ipc->queues, gsp_init);
	if (err != 0) {
		return err;
	}

	gsp_ipc->gsp_ready = true;

exit:
	return err;
}

int nvgpu_gsp_process_ipc_message(struct gk20a *g)
{
	struct nv_flcn_msg_gsp msg;
	bool read_msg;
	struct nvgpu_gsp *gsp = g->gsp;
	struct nvgpu_gsp_ipc *gsp_ipc = g->gsp->gsp_ipc;
	int status = 0;

	nvgpu_gsp_dbg(g, " ");

	if (unlikely(!gsp_ipc->gsp_ready)) {
		status = gsp_process_init_msg(gsp, &msg);
		if (status != 0) {
			nvgpu_gsp_dbg(g, "init msg failed");
		}
		goto exit;
	}

	do {
		read_msg = gsp_read_message(gsp,
				GSP_NV_MSGQ_LOG_ID, &msg, &status);
		if (read_msg == false) {
			break;
		}

		nvgpu_gsp_dbg(g, "read msg hdr: ");
		nvgpu_gsp_dbg(g, "unit_id = 0x%08x, size = 0x%08x",
			msg.hdr.unit_id, msg.hdr.size);
		nvgpu_gsp_dbg(g, "ctrl_flags = 0x%08x, seq_id = 0x%08x",
			msg.hdr.ctrl_flags, msg.hdr.seq_id);

		status = gsp_response_handle(gsp, &msg);
		if (status != 0) {
			return status;
		}

		if (!nvgpu_gsp_queue_is_empty(gsp_ipc->queues,
				GSP_NV_MSGQ_LOG_ID)) {
			g->ops.gsp.set_msg_intr(g);
		}
	NVGPU_COV_WHITELIST(deviate, NVGPU_MISRA(Rule, 14_3), "SWE-NVGPU-052-SWSADR.docx")
	} while (read_msg);

exit:
	return status;
}

int nvgpu_gsp_wait_message_cond(struct gk20a *g, u32 timeout_ms,
		void *var, u8 val)
{
	struct nvgpu_timeout timeout;
	u32 delay = POLL_DELAY_MIN_US;

	nvgpu_timeout_init_cpu_timer(g, &timeout, timeout_ms);

	do {
		if (*(u8 *)var == val) {
			return 0;
		}

		nvgpu_usleep_range(delay, delay * 2U);
		delay = min_t(u32, delay << 1U, POLL_DELAY_MAX_US);
	} while (nvgpu_timeout_expired(&timeout) == 0);

	return -ETIMEDOUT;
}
