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

#include <nvs/log.h>
#include <nvgpu/nvs.h>
#include <nvgpu/lock.h>
#include <nvgpu/kmem.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/list.h>
#include <nvgpu/dma.h>
#include <nvgpu/runlist.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/enabled.h>
#if defined (CONFIG_NVS_PRESENT) && defined (CONFIG_NVGPU_GSP_SCHEDULER)
#include <nvgpu/gsp_sched.h>
#endif

/**
 * @brief A structure for managing all the list of control-fifo users
 *
 * Globally manage set of users i.e. exclusive and non-exclusive users.
 * Initially, all users upon creation are characterised as non-exclusive.
 *
 * Priviledged users with Write permissions can request for exclusive
 * access. A user can be converted from non-exclusive to exclusive
 * only if meets the above criteria and there exists no other exclusive
 * user. Only one exclusive user can exist at a time. An exclusive user
 * can be reset back into non-exclusive user.
 *
 * Maintain a list of non-exclusive users and only one exclusive user.
 *
 * Add a fast lock for accessing/modification of the users.
 */
struct nvgpu_nvs_domain_ctrl_fifo_users {
	/* Flag to reserve exclusive user */
	bool reserved_exclusive_rw_user;
	/* Store the single Read/Write User */
	struct nvgpu_list_node exclusive_user;
	/* Store multiple Read-Only events subscriber e.g. debugger etc. */
	struct nvgpu_list_node list_non_exclusive_user;
	/* Active users available */
	u32 usage_counter;

	struct nvgpu_spinlock user_lock;
};

/**
 * @brief A structure for managing the queues for control-fifo.
 *
 * This structure contains the Send/Receive and Event Queues
 * for managing the manual mode scheduling.
 *
 * A coarse grained lock is also defined for access control of the queue.
 */
struct nvgpu_nvs_domain_ctrl_fifo_queues {
	/**
	 * Send indicates a buffer having data(PUT) written by a userspace client
	 * and queried by the scheduler(GET).
	 */
	struct nvgpu_nvs_ctrl_queue send;

	/**
	 * Receive indicates a buffer having data(PUT) written by scheduler
	 * and queried by the userspace client(GET).
	 */
	struct nvgpu_nvs_ctrl_queue receive;

	/**
	 * Event indicates a buffer that is subscribed to by userspace clients to
	 * receive events. This buffer is Read-Only for the users and only scheduler can
	 * write to it.
	 */
	struct nvgpu_nvs_ctrl_queue event;

#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
	/*
	 * This contains the APIs required for reading the send queue.
	 */
	struct nvs_control_fifo_receiver *send_queue_receiver;

	/*
	 * This contains the APIs required for writing the receive queue.
	 */
	struct nvs_control_fifo_sender *receiver_queue_sender;
#endif

	/**
	 * Global mutex for coarse grained access control
	 * of all Queues for all UMD interfaces. e.g. IOCTL/devctls
	 * and mmap calls.
	 */
	struct nvgpu_mutex queue_lock;
};

/**
 * @brief A master structure for Control-Fifo
 *
 * A global structure for Control-Fifo per GPU isntance.
 * All the Users and Queues are centrally managed from here.
 *
 * In addition, any capabilities for the scheduler are
 * also managed here.
 */
struct nvgpu_nvs_domain_ctrl_fifo {
	/**
	 * Instance of global struct gk20a;
	 */
	struct gk20a *g;
	/**
	 * A placeholder for holding users of control-fifo.
	 *
	 */
	struct nvgpu_nvs_domain_ctrl_fifo_users users;
	/**
	 * A placeholder for storing the queues of control-fifo.
	 *
	 */
	struct nvgpu_nvs_domain_ctrl_fifo_queues queues;
	/**
	 * Store capabilities of control-fifo
	 *
	 */
	struct nvs_domain_ctrl_fifo_capabilities capabilities;
};

void nvgpu_nvs_ctrl_fifo_reset_exclusive_user(
		struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl, struct nvs_domain_ctrl_fifo_user *user)
{
	if (sched_ctrl == NULL || user == NULL) {
		return;
	}

	nvgpu_spinlock_acquire(&sched_ctrl->users.user_lock);
	nvgpu_list_del(&user->sched_ctrl_list);
	nvgpu_list_add_tail(&user->sched_ctrl_list, &sched_ctrl->users.list_non_exclusive_user);
	nvgpu_spinlock_release(&sched_ctrl->users.user_lock);
}

int nvgpu_nvs_ctrl_fifo_reserve_exclusive_user(
		struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl, struct nvs_domain_ctrl_fifo_user *user)
{
	int ret = 0;

	if (sched_ctrl == NULL || user == NULL) {
		return -ENODEV;
	}

	if (!user->has_write_access) {
		return -EPERM;
	}

	nvgpu_spinlock_acquire(&sched_ctrl->users.user_lock);

	if (nvgpu_list_empty(&sched_ctrl->users.exclusive_user)) {
		nvgpu_list_del(&user->sched_ctrl_list);
		nvgpu_list_add_tail(&user->sched_ctrl_list, &sched_ctrl->users.exclusive_user);
	} else {
		ret = -EBUSY;
	}

	nvgpu_spinlock_release(&sched_ctrl->users.user_lock);

	return ret;
}

bool nvgpu_nvs_ctrl_fifo_user_exists(struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl,
	int pid, bool rw)
{
	bool user_exists = false;
	struct nvs_domain_ctrl_fifo_user *user;

	if (sched_ctrl == NULL) {
		return false;
	}

	(void)rw;
	nvgpu_spinlock_acquire(&sched_ctrl->users.user_lock);

	nvgpu_list_for_each_entry(user, &sched_ctrl->users.list_non_exclusive_user,
			nvs_domain_ctrl_fifo_user, sched_ctrl_list) {
		if (user->pid == pid) {
			user_exists = true;
			break;
		}
	}

	if (!user_exists) {
		if (!nvgpu_list_empty(&sched_ctrl->users.exclusive_user)) {
			user = nvgpu_list_first_entry(&sched_ctrl->users.exclusive_user,
					nvs_domain_ctrl_fifo_user, sched_ctrl_list);
			if (user->pid == pid) {
				user_exists = true;
			}
		}
	}

	nvgpu_spinlock_release(&sched_ctrl->users.user_lock);

	return user_exists;
}

bool nvgpu_nvs_ctrl_fifo_is_exclusive_user(struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl,
    struct nvs_domain_ctrl_fifo_user *user)
{
	bool result = false;
	struct nvs_domain_ctrl_fifo_user *exclusive_user = NULL;

	if (sched_ctrl == NULL || user == NULL) {
		return false;
	}

	nvgpu_spinlock_acquire(&sched_ctrl->users.user_lock);

	if (!nvgpu_list_empty(&sched_ctrl->users.exclusive_user)) {
		exclusive_user = nvgpu_list_first_entry(&sched_ctrl->users.exclusive_user,
					nvs_domain_ctrl_fifo_user, sched_ctrl_list);

		if (exclusive_user == user) {
			result = true;
		}
	}

	nvgpu_spinlock_release(&sched_ctrl->users.user_lock);

	return result;
}

void nvgpu_nvs_ctrl_fifo_add_user(struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl,
	struct nvs_domain_ctrl_fifo_user *user)
{

	if (sched_ctrl == NULL || user == NULL) {
		return;
	}

	nvgpu_spinlock_acquire(&sched_ctrl->users.user_lock);

	nvgpu_list_add(&user->sched_ctrl_list, &sched_ctrl->users.list_non_exclusive_user);

	sched_ctrl->users.usage_counter++;

	nvgpu_spinlock_release(&sched_ctrl->users.user_lock);
}

bool nvgpu_nvs_ctrl_fifo_user_is_active(struct nvs_domain_ctrl_fifo_user *user)
{
	if (user == NULL) {
		return false;
	}

	return user->active_used_queues != 0;
}

void nvgpu_nvs_ctrl_fifo_remove_user(struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl,
	struct nvs_domain_ctrl_fifo_user *user)
{

	if (sched_ctrl == NULL || user == NULL) {
		return;
	}

	nvgpu_spinlock_acquire(&sched_ctrl->users.user_lock);

	nvgpu_list_del(&user->sched_ctrl_list);

	sched_ctrl->users.usage_counter--;

	nvgpu_spinlock_release(&sched_ctrl->users.user_lock);
}

struct nvgpu_nvs_domain_ctrl_fifo *nvgpu_nvs_ctrl_fifo_create(struct gk20a *g)
{
	struct nvgpu_nvs_domain_ctrl_fifo *sched = nvgpu_kzalloc(g, sizeof(*sched));

	if (sched == NULL) {
		return NULL;
	}
	sched->g = g;
	sched->capabilities.scheduler_implementation_hw = NVGPU_NVS_DOMAIN_SCHED_KMD;

	nvgpu_spinlock_init(&sched->users.user_lock);
	nvgpu_mutex_init(&sched->queues.queue_lock);
	nvgpu_init_list_node(&sched->users.exclusive_user);
	nvgpu_init_list_node(&sched->users.list_non_exclusive_user);

	return sched;
}

#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
void nvgpu_nvs_domain_ctrl_fifo_set_receiver(struct gk20a *g,
		struct nvs_control_fifo_receiver *receiver)
{
	struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl = g->sched_ctrl_fifo;

	if (sched_ctrl == NULL) {
		return;
	}

	sched_ctrl->queues.send_queue_receiver = receiver;

	nvgpu_smp_wmb();
}

void nvgpu_nvs_domain_ctrl_fifo_set_sender(struct gk20a *g,
		struct nvs_control_fifo_sender *sender)
{
	struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl = g->sched_ctrl_fifo;

	if (sched_ctrl == NULL) {
		return;
	}

	sched_ctrl->queues.receiver_queue_sender = sender;

	nvgpu_smp_wmb();
}

struct nvs_control_fifo_receiver *nvgpu_nvs_domain_ctrl_fifo_get_receiver(struct gk20a *g)
{
	struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl = g->sched_ctrl_fifo;

	if (sched_ctrl == NULL) {
		return NULL;
	}

	nvgpu_smp_rmb();

	return sched_ctrl->queues.send_queue_receiver;
}

struct nvs_control_fifo_sender *nvgpu_nvs_domain_ctrl_fifo_get_sender(struct gk20a *g)
{
	struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl = g->sched_ctrl_fifo;

	if (sched_ctrl == NULL) {
		return NULL;
	}

	nvgpu_smp_rmb();

	return sched_ctrl->queues.receiver_queue_sender;
}
#endif

bool nvgpu_nvs_ctrl_fifo_is_enabled(struct gk20a *g)
{
	struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl = g->sched_ctrl_fifo;

	if (sched_ctrl == NULL) {
		return false;
	}

	return nvgpu_nvs_ctrl_fifo_is_busy(sched_ctrl);
}

void nvgpu_nvs_ctrl_fifo_idle(struct gk20a *g)
{
	struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl = g->sched_ctrl_fifo;

	if (sched_ctrl == NULL) {
		return;
	}

	if (nvgpu_nvs_ctrl_fifo_is_busy(sched_ctrl)) {
		gk20a_idle(g);
	}
}

void nvgpu_nvs_ctrl_fifo_unidle(struct gk20a *g)
{
	struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl = g->sched_ctrl_fifo;
	int err;

	if (sched_ctrl == NULL) {
		return;
	}

	if (nvgpu_nvs_ctrl_fifo_is_busy(sched_ctrl)) {
		err = gk20a_busy(g);
		if (err != 0) {
			nvgpu_err(g, "cannot busy() again!");
		}
	}
}

bool nvgpu_nvs_ctrl_fifo_is_busy(struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl)
{
	bool ret = 0;

	if (sched_ctrl == NULL) {
		return false;
	}

	nvgpu_spinlock_acquire(&sched_ctrl->users.user_lock);
	ret = (sched_ctrl->users.usage_counter != 0);
	nvgpu_spinlock_release(&sched_ctrl->users.user_lock);

	return ret;
}

void nvgpu_nvs_ctrl_fifo_destroy(struct gk20a *g)
{
	struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl = g->sched_ctrl_fifo;

	if (sched_ctrl == NULL) {
		return;
	}

	nvgpu_assert(!nvgpu_nvs_ctrl_fifo_is_busy(sched_ctrl));

	nvgpu_nvs_ctrl_fifo_erase_all_queues(g);

	nvgpu_kfree(g, sched_ctrl);
	g->sched_ctrl_fifo = NULL;
}

struct nvgpu_nvs_ctrl_queue *nvgpu_nvs_ctrl_fifo_get_queue(
		struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl,
		enum nvgpu_nvs_ctrl_queue_num queue_num,
		enum nvgpu_nvs_ctrl_queue_direction queue_direction,
		u8 *mask)
{
	struct nvgpu_nvs_ctrl_queue *queue = NULL;

	if (sched_ctrl == NULL) {
		return NULL;
	}

	if (mask == NULL) {
		return NULL;
	}

	if (queue_num == NVGPU_NVS_NUM_CONTROL) {
		if (queue_direction == NVGPU_NVS_DIR_CLIENT_TO_SCHEDULER) {
			queue = &sched_ctrl->queues.send;
			*mask = NVGPU_NVS_CTRL_FIFO_QUEUE_EXCLUSIVE_CLIENT_WRITE;
		} else if (queue_direction == NVGPU_NVS_DIR_SCHEDULER_TO_CLIENT) {
			queue = &sched_ctrl->queues.receive;
			*mask = NVGPU_NVS_CTRL_FIFO_QUEUE_EXCLUSIVE_CLIENT_READ;
		}
	} else if (queue_num == NVGPU_NVS_NUM_EVENT) {
		if (queue_direction == NVGPU_NVS_DIR_SCHEDULER_TO_CLIENT) {
			queue = &sched_ctrl->queues.event;
			*mask = NVGPU_NVS_CTRL_FIFO_QUEUE_CLIENT_EVENTS_READ;
		}
	}

	return queue;
}

struct nvs_domain_ctrl_fifo_capabilities *nvgpu_nvs_ctrl_fifo_get_capabilities(
		struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl)
{
	if (sched_ctrl == NULL) {
		return NULL;
	}

	return &sched_ctrl->capabilities;
}

bool nvgpu_nvs_buffer_is_valid(struct gk20a *g, struct nvgpu_nvs_ctrl_queue *buf)
{
	(void)g;
	if (buf == NULL) {
		return false;
	}

	return buf->valid;
}

bool nvgpu_nvs_buffer_is_sendq_valid(struct gk20a *g)
{
	struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl = g->sched_ctrl_fifo;

	return nvgpu_nvs_buffer_is_valid(g, &sched_ctrl->queues.send);
}

bool nvgpu_nvs_buffer_is_receiveq_valid(struct gk20a *g)
{
	struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl = g->sched_ctrl_fifo;

	return nvgpu_nvs_buffer_is_valid(g, &sched_ctrl->queues.receive);
}

bool nvgpu_nvs_buffer_is_eventq_valid(struct gk20a *g)
{
	struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl = g->sched_ctrl_fifo;

	return nvgpu_nvs_buffer_is_valid(g, &sched_ctrl->queues.event);
}

int nvgpu_nvs_buffer_alloc(struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl,
		size_t bytes, u8 mask, struct nvgpu_nvs_ctrl_queue *buf)
{
	int err;
	struct gk20a *g;
	struct vm_gk20a *system_vm;
#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
	struct nvs_control_fifo_receiver *send_queue_receiver;
	struct nvs_control_fifo_sender *receiver_queue_sender;
#endif

	if (sched_ctrl == NULL) {
		return -ENODEV;
	}

	if (buf == NULL) {
		return -EINVAL;
	}

	g = sched_ctrl->g;
	if (nvgpu_is_enabled(g, (u32)(NVGPU_SUPPORT_GSP_VM))) {
		system_vm = g->mm.gsp.vm;
	} else {
		system_vm = g->mm.pmu.vm;
	}

	(void)memset(buf, 0, sizeof(*buf));
	buf->g = g;

	err = nvgpu_dma_alloc_map_flags_sys(system_vm,
		NVGPU_DMA_VM_USERMAP_ADDRESS, bytes, &buf->mem);
	if (err != 0) {
		nvgpu_err(g, "failed to allocate memory for dma");
		goto fail;
	}

#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_KMD_SCHEDULING_WORKER_THREAD)) {
		if (mask == NVGPU_NVS_CTRL_FIFO_QUEUE_EXCLUSIVE_CLIENT_WRITE) {
			send_queue_receiver = nvs_control_fifo_receiver_initialize(g,
				(struct nvs_domain_msg_fifo * const)buf->mem.cpu_va, bytes);
			if (send_queue_receiver == NULL) {
				goto fail;
			}
			nvgpu_nvs_domain_ctrl_fifo_set_receiver(g, send_queue_receiver);
		} else if (mask == NVGPU_NVS_CTRL_FIFO_QUEUE_EXCLUSIVE_CLIENT_READ) {
			receiver_queue_sender = nvs_control_fifo_sender_initialize(g,
				(struct nvs_domain_msg_fifo *)buf->mem.cpu_va, bytes);
			if (receiver_queue_sender == NULL) {
				goto fail;
			}
			nvgpu_nvs_domain_ctrl_fifo_set_sender(g, receiver_queue_sender);
		}
	}
#endif

	buf->valid = true;
	buf->mask = mask;

	return 0;

fail:
	(void)memset(buf, 0, sizeof(*buf));

	return err;
}

void nvgpu_nvs_buffer_free(struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl,
		struct nvgpu_nvs_ctrl_queue *buf)
{
	struct gk20a *g;
	struct vm_gk20a *system_vm;
	u8 mask;
#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
	struct nvs_control_fifo_receiver *send_queue_receiver;
	struct nvs_control_fifo_sender *receiver_queue_sender;
#endif

	if (sched_ctrl == NULL) {
		return;
	}

	if (buf == NULL) {
		return;
	}

	g = sched_ctrl->g;
	system_vm = g->mm.pmu.vm;
	mask = buf->mask;

#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_KMD_SCHEDULING_WORKER_THREAD)) {
		send_queue_receiver = nvgpu_nvs_domain_ctrl_fifo_get_receiver(g);
		receiver_queue_sender = nvgpu_nvs_domain_ctrl_fifo_get_sender(g);

		if (mask == NVGPU_NVS_CTRL_FIFO_QUEUE_EXCLUSIVE_CLIENT_WRITE) {
			nvgpu_nvs_domain_ctrl_fifo_set_receiver(g, NULL);
			if (send_queue_receiver != NULL) {
				nvs_control_fifo_receiver_exit(g, send_queue_receiver);
			}
		} else if (mask == NVGPU_NVS_CTRL_FIFO_QUEUE_EXCLUSIVE_CLIENT_READ) {
			nvgpu_nvs_domain_ctrl_fifo_set_sender(g, NULL);
			if (receiver_queue_sender != NULL) {
				nvs_control_fifo_sender_exit(g, receiver_queue_sender);
			}
		}
	}
#endif

	if (nvgpu_mem_is_valid(&buf->mem)) {
#if defined (CONFIG_NVS_PRESENT) && defined (CONFIG_NVGPU_GSP_SCHEDULER)
	if (nvgpu_is_enabled(g, (u32)(NVGPU_SUPPORT_GSP_SCHED))) {
		nvgpu_gsp_sched_erase_ctrl_fifo(g, buf->queue_direction);
	}
#endif
		nvgpu_dma_unmap_free(system_vm, &buf->mem);
	}

	/* Sets buf->valid as false */
	(void)memset(buf, 0, sizeof(*buf));
	(void)mask;
}

void nvgpu_nvs_ctrl_fifo_lock_queues(struct gk20a *g)
{
	struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl = g->sched_ctrl_fifo;
	nvgpu_mutex_acquire(&sched_ctrl->queues.queue_lock);
}

void nvgpu_nvs_ctrl_fifo_unlock_queues(struct gk20a *g)
{
	struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl = g->sched_ctrl_fifo;
	nvgpu_mutex_release(&sched_ctrl->queues.queue_lock);
}

bool nvgpu_nvs_ctrl_fifo_queue_has_subscribed_users(struct nvgpu_nvs_ctrl_queue *queue)
{
	if (queue == NULL) {
		return false;
	}

	return queue->ref != 0;
}

void nvgpu_nvs_ctrl_fifo_user_subscribe_queue(struct nvs_domain_ctrl_fifo_user *user,
		struct nvgpu_nvs_ctrl_queue *queue)
{
	if (queue == NULL || user == NULL) {
		return;
	}

	user->active_used_queues |= queue->mask;
	queue->ref++;
}
void nvgpu_nvs_ctrl_fifo_user_unsubscribe_queue(struct nvs_domain_ctrl_fifo_user *user,
		struct nvgpu_nvs_ctrl_queue *queue)
{
	if (queue == NULL || user == NULL) {
		return;
	}

	user->active_used_queues &= ~((u32)queue->mask);
	queue->ref--;
}
bool nvgpu_nvs_ctrl_fifo_user_is_subscribed_to_queue(struct nvs_domain_ctrl_fifo_user *user,
		struct nvgpu_nvs_ctrl_queue *queue)
{
	if (queue == NULL || user == NULL) {
		return false;
	}

	return (user->active_used_queues & queue->mask);
}

void nvgpu_nvs_ctrl_fifo_erase_all_queues(struct gk20a *g)
{
	struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl = g->sched_ctrl_fifo;

	nvgpu_nvs_ctrl_fifo_lock_queues(g);

	if (nvgpu_nvs_buffer_is_valid(g, &sched_ctrl->queues.send)) {
		nvgpu_nvs_ctrl_fifo_erase_queue_locked(g, &sched_ctrl->queues.send);
	}

	if (nvgpu_nvs_buffer_is_valid(g, &sched_ctrl->queues.receive)) {
		nvgpu_nvs_ctrl_fifo_erase_queue_locked(g, &sched_ctrl->queues.receive);
	}

	if (nvgpu_nvs_buffer_is_valid(g, &sched_ctrl->queues.event)) {
		nvgpu_nvs_ctrl_fifo_erase_queue_locked(g, &sched_ctrl->queues.event);
	}

	nvgpu_nvs_ctrl_fifo_unlock_queues(g);
}

void nvgpu_nvs_ctrl_fifo_erase_queue_locked(struct gk20a *g, struct nvgpu_nvs_ctrl_queue *queue)
{
	if (queue != NULL && queue->free != NULL) {
		queue->free(g, queue);
	}
}

#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
static int nvgpu_nvs_ctrl_fifo_scheduler_process_caps_request(struct gk20a *g,
		struct nvs_control_fifo_receiver * const send_queue_receiver,
		struct nvs_control_fifo_sender * const receiver_queue_sender)
{
	int result;

	struct nvs_domain_msg_ctrl_get_caps_req *request =
		(struct nvs_domain_msg_ctrl_get_caps_req *)send_queue_receiver->internal_buffer;
	struct nvs_domain_msg_ctrl_get_caps_resp *response =
		(struct nvs_domain_msg_ctrl_get_caps_resp *)receiver_queue_sender->internal_buffer;

	(void)g;

	if (request->client_version_major == NVS_DOMAIN_SCHED_VERSION_MAJOR) {
		result = 0;
		response->client_version_status = NVS_DOMAIN_MSG_CTRL_GET_CAPS_RESP_CLIENT_VERSION_STATUS_OK;
	} else {
		result = 1;
		response->client_version_status =
			NVS_DOMAIN_MSG_CTRL_GET_CAPS_RESP_CLIENT_VERSION_STATUS_FAILED;
	}

		response->sched_version_major = NVS_DOMAIN_SCHED_VERSION_MAJOR;
		response->sched_version_minor = NVS_DOMAIN_SCHED_VERSION_MINOR;
		response->sched_version_patch = NVS_DOMAIN_SCHED_VERSION_PATCH;

	return result;
}

static int nvgpu_nvs_ctrl_fifo_scheduler_process_erroneous_request(struct gk20a *g,
		struct nvs_control_fifo_receiver * const send_queue_receiver,
		struct nvs_control_fifo_sender * const receiver_queue_sender)
{
	struct nvs_domain_msg_ctrl_error_resp *error_response =
		(struct nvs_domain_msg_ctrl_error_resp *)receiver_queue_sender->internal_buffer;

	(void)g;
	(void)send_queue_receiver;
	error_response->error_code = NVS_DOMAIN_MSG_CTRL_ERROR_UNHANDLED_MESSAGE;

	return 0;
}

static int nvgpu_nvs_ctrl_fifo_scheduler_process_switch_request(struct gk20a *g,
		struct nvs_control_fifo_receiver * const send_queue_receiver,
		struct nvs_control_fifo_sender * const receiver_queue_sender)
{
	int result = 0;
	struct nvs_domain *nvs_domain = NULL;
	struct nvgpu_nvs_domain *nvgpu_nvs_domain = NULL;
	struct nvgpu_nvs_scheduler *sched = g->scheduler;
	s64 start_time, end_time;
	s64 duration;
	u64 domain_id;

	struct nvs_domain_msg_ctrl_switch_domain_req *request =
		(struct nvs_domain_msg_ctrl_switch_domain_req *)send_queue_receiver->internal_buffer;
	struct nvs_domain_msg_ctrl_switch_domain_resp *response =
		(struct nvs_domain_msg_ctrl_switch_domain_resp *)receiver_queue_sender->internal_buffer;

	domain_id = request->domain_id;

	if (domain_id == NVS_DOMAIN_CTRL_DOMAIN_ID_ALL) {
		nvgpu_nvs_domain = sched->shadow_domain;
	} else {
		nvgpu_nvs_domain = nvgpu_nvs_domain_by_id_locked(g, domain_id);
	}

	if (nvgpu_nvs_domain == NULL) {
		nvgpu_err(g, "Unable to find domain[%llu]", domain_id);
		result = -EINVAL;
		response->status = NVS_DOMAIN_MSG_TYPE_CTRL_SWITCH_DOMAIN_STATUS_FAIL;
	} else {
		nvs_domain = (struct nvs_domain *)nvgpu_nvs_domain->parent;

		start_time = nvgpu_current_time_ns();
		result = nvgpu_runlist_tick(g, nvgpu_nvs_domain->rl_domains, nvs_domain->preempt_grace_ns);
		end_time = nvgpu_current_time_ns();

		if (result == 0) {
			/* Change active domain here. Any runlist pending writes depend upon this.*/
			sched->active_domain = nvgpu_nvs_domain;
			duration = nvgpu_safe_sub_s64(end_time, start_time);
			response->status = NVS_DOMAIN_MSG_TYPE_CTRL_SWITCH_DOMAIN_STATUS_SUCCESS;
			response->switch_ns = nvgpu_safe_cast_s64_to_u64(duration);
		} else {
			response->status = NVS_DOMAIN_MSG_TYPE_CTRL_SWITCH_DOMAIN_STATUS_FAIL;
		}
	}

	return result;
}

static int nvgpu_nvs_ctrl_fifo_scheduler_process_receiver(struct gk20a *g,
		struct nvs_control_fifo_receiver * const send_queue_receiver,
		struct nvs_control_fifo_sender * const receiver_queue_sender)
{
	int result = 0;

	if ((receiver_queue_sender == NULL) || (send_queue_receiver == NULL)) {
		return -1;
	}

	(void)memset(receiver_queue_sender->internal_buffer, 0,
		NVS_DOMAIN_MESSAGE_MAX_PAYLOAD_SIZE);

	if (send_queue_receiver->msg_type == NVS_DOMAIN_MSG_TYPE_CTRL_GET_CAPS_INFO) {
		result = nvgpu_nvs_ctrl_fifo_scheduler_process_caps_request(g,
			send_queue_receiver, receiver_queue_sender);
	} else if (send_queue_receiver->msg_type == NVS_DOMAIN_MSG_TYPE_CTRL_SWITCH_DOMAIN) {
		result = nvgpu_nvs_ctrl_fifo_scheduler_process_switch_request(g,
			send_queue_receiver, receiver_queue_sender);
	} else {
		result = nvgpu_nvs_ctrl_fifo_scheduler_process_erroneous_request(g,
			send_queue_receiver, receiver_queue_sender);
		send_queue_receiver->msg_type = NVS_DOMAIN_MSG_TYPE_CTRL_ERROR;
	}

	return result;
}

int nvgpu_nvs_ctrl_fifo_scheduler_handle_requests(struct gk20a *g)
{
	int ret = 0;
	struct nvs_control_fifo_receiver * const send_queue_receiver =
		nvgpu_nvs_domain_ctrl_fifo_get_receiver(g);
	struct nvs_control_fifo_sender * const receiver_queue_sender =
		nvgpu_nvs_domain_ctrl_fifo_get_sender(g);

	/* Take a lock here to ensure, the queues are not messed with anywhere
	 * as long as the queue read is in progress.
	 */

	nvgpu_mutex_acquire(&g->sched_mutex);

	if (send_queue_receiver == NULL) {
		nvgpu_mutex_release(&g->sched_mutex);
		return 0;
	}

	if (nvs_control_fifo_receiver_can_read(send_queue_receiver) == 0) {
		nvs_control_fifo_read_message(send_queue_receiver);
		ret = nvgpu_nvs_ctrl_fifo_scheduler_process_receiver(g, send_queue_receiver,
			receiver_queue_sender);

		if (ret != 0) {
			nvgpu_err(g, "error occured when reading from the SEND control-queue");
		}

		if (receiver_queue_sender != NULL) {
			ret = nvs_control_fifo_sender_can_write(receiver_queue_sender);
			if (ret == -EAGAIN) {
				nvs_control_fifo_sender_out_of_space(receiver_queue_sender);
				nvgpu_err(g, "error occured due to lack of space for RECEIVE control-queue");
			} else {
				nvs_control_fifo_sender_write_message(receiver_queue_sender,
					send_queue_receiver->msg_type, send_queue_receiver->msg_sequence,
						nvgpu_safe_cast_s64_to_u64(nvgpu_current_time_ns()));
				nvs_dbg(g, " ");
			}
		}
	}
	/*
	 * Release the lock here.
	 */
	nvgpu_mutex_release(&g->sched_mutex);

	return ret;
}
#endif /* CONFIG_KMD_SCHEDULING_WORKER_THREAD */

