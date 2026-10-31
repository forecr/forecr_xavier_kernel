/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVS_CONTROL_INTERFACE_PARSER_H
#define NVS_CONTROL_INTERFACE_PARSER_H

#ifdef CONFIG_NVS_PRESENT
#include <nvs/nvs-control-interface.h>
#include <nvs/types-internal.h>
#else
#define NVS_DOMAIN_MESSAGE_MAX_PAYLOAD_SIZE 48
#endif

#include <nvgpu/types.h>

#define NVS_DOMAIN_MSG_TYPE_CTRL_SWITCH_DOMAIN_STATUS_FAIL 1

struct gk20a;
struct nvs_domain_message;
struct nvs_domain_msg_fifo_control;
struct nvs_domain_msg_fifo;

struct nvs_control_fifo_sender {
	struct gk20a *g;
	/* pointer to the message queues */
	struct nvs_domain_message *fifo;
	/* pointer to the control interface */
	struct nvs_domain_msg_fifo_control *control_interface;
	/* internal buffer storage */
	u8 internal_buffer[NVS_DOMAIN_MESSAGE_MAX_PAYLOAD_SIZE];
	/* Below fields maintain local copies */
	u32 put_index;
	u32 num_revolutions;
	u64 num_queue_entries;
	u64 num_dropped_messages;
};

struct nvs_control_fifo_receiver {
	struct gk20a *g;
	/* pointer to the message queues */
	struct nvs_domain_message *fifo;
	/* pointer to the control interface */
	struct nvs_domain_msg_fifo_control *control_interface;
	/* internal buffer storage */
	u8 internal_buffer[NVS_DOMAIN_MESSAGE_MAX_PAYLOAD_SIZE];
	/* Below fields maintain local copies */
	u32 msg_type;
	u32 msg_sequence;
	u64 msg_timestamp_ns;

	u32 nvs_get_index;
	u64 num_queue_entries;
};
#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
void nvs_control_fifo_sender_write_message(struct nvs_control_fifo_sender *const sender,
		u32 const msg_number, u32 const msg_sequence_tag,
		u64 const msg_timestamp_ns);
void nvs_control_fifo_sender_out_of_space(struct nvs_control_fifo_sender *const sender);
int nvs_control_fifo_sender_can_write(struct nvs_control_fifo_sender * const sender);
struct nvs_control_fifo_sender *nvs_control_fifo_sender_initialize(
		struct gk20a *g, struct nvs_domain_msg_fifo *const ring_buffer,
		u64 buffer_size_bytes);

void nvs_control_fifo_read_message(struct nvs_control_fifo_receiver *const receiver);
int nvs_control_fifo_receiver_can_read(struct nvs_control_fifo_receiver *const receiver);
struct nvs_control_fifo_receiver *nvs_control_fifo_receiver_initialize(
		struct gk20a *g, struct nvs_domain_msg_fifo *const fifo,
		u64 buffer_size_bytes);

void nvs_control_fifo_receiver_exit(struct gk20a *g,
		struct nvs_control_fifo_receiver *const receiver);
void nvs_control_fifo_sender_exit(struct gk20a *g,
		struct nvs_control_fifo_sender *const sender);
void nvs_control_fifo_enable_flow_control(struct nvs_domain_msg_fifo_control *control_interface,
		u32 get_index);
void nvs_control_fifo_disable_flow_control(struct nvs_domain_msg_fifo_control *control_interface);
#endif

#endif
