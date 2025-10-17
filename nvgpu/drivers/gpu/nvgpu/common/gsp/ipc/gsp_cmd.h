/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GSP_CMD_IF_H
#define NVGPU_GSP_CMD_IF_H

/**
 * @file
 * @page gsp_communication gsp_communication
 *
 * Overview
 * ========
 * The communication between nvgpu-rm and safety-scheduler happens through certain
 * commands and message that is managed by this unit. Command Management deals with
 * communication between nvgpu-rm and safety-scheduler. There are 2 queues namely
 * command queue and message queue. GSP communication is splitted into four parts to
 * establish the communication.
 * - gsp_cmd
 * - gsp_msg
 * - gsp_queue
 * - gsp_seq
 *
 * GSP command:
 * ================
 * nvgpu-rm writes update to command queue which generates a HW interrupt to
 * safety-scheduler
 *
 * External APIs
 * ============================
 * - @ref gsp_unit_id_is_valid
 * - @ref nvgpu_gsp_cmd_post
 * - @ref nvgpu_gsp_get_last_cmd_id
 *
 * Internal APIs
 * ============================
 * - @ref gsp_validate_cmd
 * - @ref gsp_write_cmd
 *
 * GSP Message:
 * ================
 * After processing the command safety-scheduler writes to message queue which
 * generates SW interrupt to nvgpu-rm.
 *
 * External APIs
 * ============================
 * - @ref nvgpu_gsp_process_message
 * - @ref nvgpu_gsp_wait_message_cond
 *
 * Internal APIs
 * ============================
 * - @ref gsp_response_handle
 * - @ref gsp_read_message
 * - @ref gsp_process_init_msg
 *
 * GSP Queue:
 * ================
 * - QUEUE_HEAD(i) and QUEUE_TAIL(i) registers point to the head and tail
 *   location of the CMDQ(i) in EMEM. Each has its PLM register for register
 *   access protection. It's the GSP NVRISCV CPU as the message receiver to
 *   consume the message body from the CMDQ(i) between QUEUE_HEAD(i) and
 *   QUEUE_TAIL(i), and update QUEUE_TAIL(i) to match QUEUE_HEAD(i) at the end.
 * - MSGQ_HEAD(i) and MSGQ_TAIL(i) registers point to the head and tail location
 *   of MSGQ(i) in EMEM. Each has its PLM register for register access protection.
 *   It's the NVGPU driver as the message receiver to consume the message body from
 *   the MSGQ(i) between MSGQ_HEAD(i) and MSGQ_TAIL(i), and update MSGQ_TAIL(i) to
 *   match MSGQ_HEAD(i) at the end.
 *
 * External APIs
 * ============================
 * - @ref nvgpu_gsp_queues_init
 * - @ref nvgpu_gsp_queues_free
 * - @ref nvgpu_gsp_queue_get_size
 * - @ref nvgpu_gsp_queue_push
 * - @ref nvgpu_gsp_queue_is_empty
 * - @ref nvgpu_gsp_queue_read
 * - @ref nvgpu_gsp_queue_rewind
 *
 * Internal APIs
 * ============================
 * - @ref gsp_queue_init
 * - @ref gsp_queue_free
 *
 * GSP Sequence:
 * ================
 * GSP Sequence allows to send multiple commands to GSP. GSP processes those
 * commands as per the sequence and respond to nvgpu-rm. Seq-id is embedded in the
 * nvgpu-rm handler message which helps to associate request with response.
 *
 * External APIs
 * ============================
 * - @ref nvgpu_gsp_sequences_init
 * - @ref nvgpu_gsp_sequences_free
 * - @ref nvgpu_gsp_seq_acquire
 * - @ref gsp_seq_release
 * - @ref nvgpu_gsp_seq_response_handle
 * - @ref nvgpu_gsp_seq_get_id
 * - @ref nvgpu_gsp_seq_set_state
 *
 * Internal APIs
 * ============================
 * - @ref gsp_sequences_init
 *
 * Following cmds can be sent to GSP.
 *  - @ref NV_GSP_UNIT_REWIND
 *  - @ref NV_GSP_UNIT_NULL
 *  - @ref NV_GSP_UNIT_INIT
 *  - @ref NV_GSP_UNIT_DEVICES_INFO
 *  - @ref NV_GSP_UNIT_DOMAIN_SUBMIT
 *  - @ref NV_GSP_UNIT_DOMAIN_ADD
 *  - @ref NV_GSP_UNIT_DOMAIN_DELETE
 *  - @ref NV_GSP_UNIT_DOMAIN_UPDATE
 *  - @ref NV_GSP_UNIT_RUNLIST_UPDATE
 *  - @ref NV_GSP_UNIT_START_SCHEDULER
 *  - @ref NV_GSP_UNIT_STOP_SCHEDULER
 *  - @ref NV_GSP_UNIT_QUERY_NO_OF_DOMAINS
 *  - @ref NV_GSP_UNIT_QUERY_ACTIVE_DOMAIN
 *  - @ref NV_GSP_UNIT_CONTROL_INFO_SEND
 *  - @ref NV_GSP_UNIT_BIND_CTX_REG
 *  - @ref NV_GSP_UNIT_CONTROL_FIFO_ERASE
 *  - @ref NV_GSP_UNIT_END
 *
 * For more details about communication refer GSP CMD/MSG communication.
 */
#include <nvgpu/types.h>
#include <nvgpu/gsp_sched.h>
#include "common/gsp_scheduler/gsp_runlist.h"
#include "common/gsp_scheduler/gsp_scheduler.h"
#include "common/gsp_scheduler/gsp_vm_scheduler.h"
#include "gsp_seq.h"
#include "common/gsp_scheduler/gsp_ctrl_fifo.h"

struct gk20a;

#define GSP_NV_MSGQ_LOG_ID				1U

#define NV_GSP_UNIT_REWIND				NV_FLCN_UNIT_ID_REWIND
#define NV_GSP_UNIT_NULL				0x01U
#define NV_GSP_UNIT_INIT				0x02U
#define NV_GSP_UNIT_DEVICES_INFO		0x03U
#define NV_GSP_UNIT_DOMAIN_SUBMIT		0x04U
#define NV_GSP_UNIT_DOMAIN_ADD			0x05U
#define NV_GSP_UNIT_DOMAIN_DELETE		0x06U
#define NV_GSP_UNIT_DOMAIN_UPDATE		0x07U
#define NV_GSP_UNIT_RUNLIST_UPDATE		0x08U
#define NV_GSP_UNIT_START_SCHEDULER		0x09U
#define NV_GSP_UNIT_STOP_SCHEDULER		0x0AU
#define NV_GSP_UNIT_QUERY_NO_OF_DOMAINS	0x0BU
#define NV_GSP_UNIT_QUERY_ACTIVE_DOMAIN	0X0CU
#define NV_GSP_UNIT_CONTROL_INFO_SEND	0X0DU
#define NV_GSP_UNIT_BIND_CTX_REG		0X0EU
#define NV_GSP_UNIT_CONTROL_FIFO_ERASE	0X0FU
#define NV_GSP_UNIT_END					0x14U

#define GSP_MSG_HDR_SIZE	U32(sizeof(struct gsp_hdr))
#define GSP_CMD_HDR_SIZE	U32(sizeof(struct gsp_hdr))

struct gsp_hdr {
	u8 unit_id;
	u8 size;
	u8 ctrl_flags;
	u8 seq_id;
};

struct nvgpu_gsp_acr_cmd_lock_wpr {
	u8 cmd_type;
	u32 wpr_address_lo;
	u32 wpr_address_hi;
};

struct nvgpu_gsp_acr_cmd_bootstrap_engine {
	u8 cmd_type;
	u32 engine_id;
	u32 engine_instance;
	u32 engine_index_mask;
	u32 intr_ctrl_0;
	u32 intr_ctrl_1;
	u32 boot_arg_lo;
	u32 boot_arg_hi;
};

struct nvgpu_gsp_acr_cmd_unlock_wpr {
	u8 cmd_type;
};

struct nvgpu_gsp_acr_cmd_shutdown {
	u8 cmd_type;
};

union nvgpu_gsp_acr_cmd {
	u8 cmd_type;
	struct nvgpu_gsp_acr_cmd_lock_wpr lock_wpr;
	struct nvgpu_gsp_acr_cmd_bootstrap_engine bootstrap_engine;
	struct nvgpu_gsp_acr_cmd_unlock_wpr unlock_wpr_details;
	struct nvgpu_gsp_acr_cmd_shutdown shutdown;
};

struct nv_flcn_cmd_gsp {
	struct gsp_hdr hdr;
	union {
		struct nvgpu_gsp_domain_id domain_id;
		struct nvgpu_gsp_device_info device;
		struct nvgpu_gsp_runlist_info runlist;
		struct nvgpu_gsp_domain_info domain;
		struct nvgpu_gsp_ctrl_fifo_info ctrl_fifo;
		struct nvgpu_gsp_ctrl_fifo_close fifo_close;
		union nvgpu_gsp_vm_sched_cmd sched;
		union nvgpu_gsp_acr_cmd acr_cmd;
		struct nvgpu_gsp_inst_bind_info nxtctx;
	} cmd;
};

bool gsp_unit_id_is_valid(u8 id);
struct nvgpu_cmd_ack_status {
	bool command_ack;
	u8 ack_status;
};

/**
 * @brief Posts a command to the GSP and optionally registers a callback for the command completion.
 *
 * The steps performed by the function are as follows:
 * -# Check if the command buffer is NULL. If it is, log an error, set the error code to -EINVAL,
 *    and go to the exit label.
 * -# Perform a sanity check on the command input by calling gsp_validate_cmd().
 *    If the command is invalid, set the error code to -EINVAL and go to the exit label.
 * -# Attempt to reserve a sequence for this command by calling nvgpu_gsp_seq_acquire().
 *    If no sequence is available, go to the exit label.
 * -# Set the sequence number in the command header to the ID of the acquired sequence
 *    by calling nvgpu_gsp_seq_get_id() with the acquired sequence.
 * -# Set the control flags to #PMU_CMD_FLAGS_STATUS in the command header to indicate
 *    the status.
 * -# Set the state of the sequence to #GSP_SEQ_STATE_USED by using nvgpu_gsp_seq_set_state().
 * -# Write the command to the GSP by calling gsp_write_cmd(). If the write fails,
 *    release the acquired sequence and go to the exit label.
 * -# Return the error code, which will be 0 if the command is successfully posted.
 *
 * @Param [in] g          Pointer to the GPU driver struct.
 * @param [in] cmd        The command to be posted to the GSP.
 * @param [in] queue_id   The ID of the queue to which the command is to be posted.
 * @param [in] callback   The callback function to be called upon command completion.
 * @param [in] cb_param   The user-defined parameter to be passed to the callback function.
 * @param [in] timeout    The timeout in milliseconds to wait for the command to complete.
 *
 * @return 0 if the command is successfully posted.
 * @return -EINVAL if the command buffer is NULL or the command is invalid.
 * @return Other non-zero error codes if reserving a sequence or writing the command fails.
 */
int nvgpu_gsp_cmd_post(struct gk20a *g, struct nv_flcn_cmd_gsp *cmd,
	u32 queue_id, gsp_callback callback, void *cb_param, u32 timeout);

#endif /* NVGPU_GSP_CMD_IF_H */
