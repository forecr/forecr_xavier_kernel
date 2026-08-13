/*
 * Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
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

#include <nvs/log.h>

#include <nvgpu/utils.h>
#include <nvgpu/barrier.h>
#include <nvgpu/atomic.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/string.h>
#include <nvgpu/log.h>
#include <nvgpu/kmem.h>

#include <nvgpu/nvs.h>
#include <nvgpu/nvs-control-interface-parser.h>

static const u32 min_queue_size = 2U;

static u32 nvs_control_atomic_read(void *const address)
{
	u32 value;

	value = NV_READ_ONCE(*(u32 *)address);
	nvgpu_rmb();

	return value;
}

static void nvs_control_atomic_write(void *address, u32 value)
{
	NV_WRITE_ONCE(*(u32 *)address, value);
	nvgpu_wmb();
}

static u64 nvs_control_atomic64_read(void *const address)
{
	u64 value;

	value = NV_READ_ONCE(*(u64 *)address);
	nvgpu_rmb();

	return value;
}

static void nvs_control_atomic64_write(void *address, u64 value)
{
	NV_WRITE_ONCE(*(u64 *)address, value);
	nvgpu_wmb();
}

#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
void nvs_control_fifo_sender_write_message(struct nvs_control_fifo_sender *const sender,
		const u32 msg_number, const u32 msg_sequence_tag,
		const u64 msg_timestamp_ns)
{
	u64 updated_put_revolutions;

	struct nvs_domain_message * const write_loc = &sender->fifo[sender->put_index];

	nvgpu_mb();

	write_loc->type = msg_number;
	write_loc->sequence_tag = msg_sequence_tag;
	write_loc->timestamp_ns = msg_timestamp_ns;
	nvgpu_memcpy((u8 *)&write_loc->payload, (u8 *)sender->internal_buffer,
		NVS_DOMAIN_MESSAGE_MAX_PAYLOAD_SIZE);

	nvgpu_wmb();

	sender->put_index++;
	if (sender->put_index == sender->num_queue_entries) {
		sender->put_index = 0;
		sender->num_revolutions = nvgpu_wrapping_add_u32(sender->num_revolutions, 1U);
	}

	updated_put_revolutions = hi32_lo32_to_u64(sender->num_revolutions,
			sender->put_index);

	nvs_control_atomic64_write(&sender->control_interface->put_revolutions,
		updated_put_revolutions);
}

void nvs_control_fifo_sender_out_of_space(struct nvs_control_fifo_sender *const sender)
{
	sender->num_dropped_messages++;
	nvs_control_atomic64_write(&sender->control_interface->num_dropped_messages,
		sender->num_dropped_messages);
}

int nvs_control_fifo_sender_can_write(struct nvs_control_fifo_sender *const sender)
{
	u64 curr_put_revolution;
	u64 occupied_slots;
	u64 available_slots;
	u64 curr_get_index = nvs_control_atomic_read(&sender->control_interface->get);

	/* bound check the get index */
	if (curr_get_index >= sender->num_queue_entries) {
		nvgpu_err(sender->g, "Get is out of bounds");
		return -EINVAL;
	} else if (curr_get_index == NVS_DOMAIN_MSG_FIFO_CONTROL_GET_FLOW_CTRL_DISABLED) {
		/* space is always available */
		return 0;
	}

	/* bound check the put index */
	if (sender->put_index >= sender->num_queue_entries) {
		nvgpu_err(sender->g, "Put is out of bounds. Probable memory corruption");
		return -EBADF;
	}

	curr_put_revolution = nvs_control_atomic64_read(
			&sender->control_interface->put_revolutions);
	if (curr_put_revolution != hi32_lo32_to_u64(sender->num_revolutions,
			sender->put_index)) {
		nvgpu_err(sender->g, "Put index has changed since our last index");
		return -EINVAL;
	}

	if (sender->put_index >= curr_get_index) {
		occupied_slots = sender->put_index - curr_get_index;
	} else {
		occupied_slots = sender->num_queue_entries - curr_get_index + sender->put_index;
	}

	available_slots = sender->num_queue_entries - occupied_slots - 1U;
	if (available_slots == 0) {
		return -EAGAIN;
	}

	return 0;
}

struct nvs_control_fifo_sender *nvs_control_fifo_sender_initialize(
		struct gk20a *g, struct nvs_domain_msg_fifo *const ring_buffer,
		u64 buffer_size_bytes)
{
	u64 current_put_revolutions;
	struct nvs_control_fifo_sender *sender = NULL;
	const u32 lower_bound_buffer_size =
		nvgpu_safe_add_u32(nvgpu_safe_mult_u32(min_queue_size,
			sizeof(struct nvs_domain_message)),
			sizeof(struct nvs_domain_msg_fifo_control));

	if (ring_buffer == NULL) {
		nvgpu_err(g, "ring buffer is NULL");
		return NULL;
	}

	if (buffer_size_bytes < lower_bound_buffer_size) {
		nvgpu_err(g, "buffer size must be a minimum of 2 entries");
		return NULL;
	}

	sender = nvgpu_kzalloc(g, sizeof(*sender));
	if (sender == NULL) {
		return NULL;
	}

	sender->g = g;

	sender->num_queue_entries = (nvgpu_safe_sub_u64(buffer_size_bytes,
			sizeof(struct nvs_domain_msg_fifo_control))) / sizeof(
				struct nvs_domain_message);

	sender->fifo = ring_buffer->messages;
	sender->control_interface = &ring_buffer->control;

	current_put_revolutions = nvs_control_atomic64_read(
			&sender->control_interface->put_revolutions);
	sender->put_index = u64_lo32(current_put_revolutions);
	sender->num_revolutions = u64_hi32(current_put_revolutions);

	sender->num_dropped_messages = nvs_control_atomic64_read(
			&sender->control_interface->num_dropped_messages);

	if (sender->put_index >= sender->num_queue_entries) {
		nvgpu_err(g, "Put Index more than Max Queue size");
		nvgpu_kfree(g, sender);
		return NULL;
	}

	return sender;
}

void nvs_control_fifo_read_message(struct nvs_control_fifo_receiver *const receiver)
{
	struct nvs_domain_message *const read_loc = &receiver->fifo[receiver->get_index];

	nvgpu_rmb();

	/* Copy the message from the buffer */
	receiver->msg_type = read_loc->type;
	receiver->msg_sequence = read_loc->sequence_tag;
	receiver->msg_timestamp_ns = read_loc->timestamp_ns;

	memset((u8 *)&receiver->internal_buffer, 0, NVS_DOMAIN_MESSAGE_MAX_PAYLOAD_SIZE);

	nvgpu_memcpy((u8 *)&receiver->internal_buffer,
		(u8 *)&read_loc->payload, NVS_DOMAIN_MESSAGE_MAX_PAYLOAD_SIZE);

	nvgpu_mb();

	receiver->get_index++;
	if (receiver->get_index == receiver->num_queue_entries) {
		receiver->get_index = 0;
	}

	nvs_control_atomic_write(&receiver->control_interface->get, receiver->get_index);
}

int nvs_control_fifo_receiver_can_read(struct nvs_control_fifo_receiver *const receiver)
{
	u32 put;
	u64 curr_put_revolution;

	curr_put_revolution = nvs_control_atomic64_read(
			&receiver->control_interface->put_revolutions);
	put = u64_lo32(curr_put_revolution);

	if (put == receiver->get_index) {
		nvs_dbg(receiver->g, "No new message");
		return -EAGAIN;
	}

	return 0;
}

struct nvs_control_fifo_receiver *nvs_control_fifo_receiver_initialize(
	struct gk20a *g, struct nvs_domain_msg_fifo *const fifo,
	u64 buffer_size_bytes)
{
	struct nvs_control_fifo_receiver *receiver = NULL;
	u64 num_put_revolutions;

	const u32 lower_bound_buffer_size =
		nvgpu_safe_add_u32(nvgpu_safe_mult_u32(min_queue_size,
			sizeof(struct nvs_domain_message)),
			sizeof(struct nvs_domain_msg_fifo_control));

	if (fifo == NULL) {
		nvgpu_err(g, "ring buffer is NULL");
		return NULL;
	}

	if (buffer_size_bytes < lower_bound_buffer_size) {
		nvgpu_err(g, "buffer size must be a minimum of 2 entries");
		return NULL;
	}

	receiver = nvgpu_kzalloc(g, sizeof(*receiver));
	if (receiver == NULL) {
		return NULL;
	}

	receiver->g = g;

	receiver->num_queue_entries = (nvgpu_safe_sub_u64(buffer_size_bytes,
			sizeof(struct nvs_domain_msg_fifo_control))) / sizeof(
				struct nvs_domain_message);

	receiver->fifo = fifo->messages;
	receiver->control_interface = &fifo->control;

	num_put_revolutions = nvs_control_atomic64_read(
			&receiver->control_interface->put_revolutions);
	receiver->get_index = u64_lo32(num_put_revolutions);

	if (receiver->get_index >= receiver->num_queue_entries) {
		nvgpu_err(g, "Get Index more than Max Queue size");
		nvgpu_kfree(g, receiver);
		return NULL;
	}

	nvs_control_fifo_enable_flow_control(receiver->control_interface, receiver->get_index);

	return receiver;
}

void nvs_control_fifo_sender_exit(struct gk20a *g,
		struct nvs_control_fifo_sender *const sender)
{
	nvs_control_fifo_disable_flow_control(sender->control_interface);
	nvgpu_kfree(g, sender);
}

void nvs_control_fifo_receiver_exit(struct gk20a *g,
		struct nvs_control_fifo_receiver *const receiver)
{
	nvs_control_fifo_disable_flow_control(receiver->control_interface);
	nvgpu_kfree(g, receiver);
}

void nvs_control_fifo_enable_flow_control(struct nvs_domain_msg_fifo_control *control_interface,
		u32 get_index)
{
	nvs_control_atomic_write(&control_interface->get, get_index);
}

void nvs_control_fifo_disable_flow_control(struct nvs_domain_msg_fifo_control *control_interface)
{
	nvs_control_atomic_write(&control_interface->get,
		NVS_DOMAIN_MSG_FIFO_CONTROL_GET_FLOW_CTRL_DISABLED);
}
#endif