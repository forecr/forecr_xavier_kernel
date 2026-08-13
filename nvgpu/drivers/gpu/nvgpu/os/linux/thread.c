// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2017-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/version.h>

#include <nvgpu/barrier.h>
#include <nvgpu/thread.h>
#include <nvgpu/timers.h>

static int nvgpu_thread_proxy(void *threaddata)
{
	struct nvgpu_thread *thread = threaddata;
	bool was_running;
	int ret;

	/* Ensure any initialization required for this thread is completed.
	 * The corresponding write barrier lies at the end of nvgpu_worker_init_common.
	 */
	nvgpu_smp_rmb();

	ret = thread->fn(thread->data);

	was_running = nvgpu_atomic_xchg(&thread->running, false);

	/* if the thread was not running, then nvgpu_thread_stop() was
	 * called, so just wait until we get the notification that we should
	 * stop.
	 */
	if (!was_running) {
		while (!nvgpu_thread_should_stop(thread)) {
			nvgpu_usleep_range(5000, 5100);
		}
	}
	return ret;
}

int nvgpu_thread_create(struct nvgpu_thread *thread,
		void *data,
		int (*threadfn)(void *data), const char *name)
{
	struct task_struct *task = kthread_create(nvgpu_thread_proxy,
			thread, name);
	if (IS_ERR(task))
		return PTR_ERR(task);

	thread->task = task;
	thread->fn = threadfn;
	thread->data = data;
	nvgpu_atomic_set(&thread->running, true);
	wake_up_process(task);
	return 0;
};

int nvgpu_thread_create_priority(struct nvgpu_thread *thread,
			void *data, int (*threadfn)(void *data),
			int priority, const char *name)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0)
	struct sched_param sparam = {0};
#endif

	struct task_struct *task = kthread_create(nvgpu_thread_proxy,
			thread, name);
	if (IS_ERR(task))
		return PTR_ERR(task);

	if (priority > 1) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)
		/* Higher priority tasks are run in threaded interrupt priority level */
		sched_set_fifo(task);
#else
		sparam.sched_priority = MAX_RT_PRIO / 2;
		sched_setscheduler(task, SCHED_FIFO, &sparam);
#endif
	} else {
		/* Only cares about setting a priority higher than normal,
		 * Lower than threaded interrupt priority but higher than normal.
		 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)
		sched_set_fifo_low(task);
#else
		sparam.sched_priority = 1;
		sched_setscheduler(task, SCHED_FIFO, &sparam);
#endif
	}

	thread->task = task;
	thread->fn = threadfn;
	thread->data = data;
	nvgpu_atomic_set(&thread->running, true);
	wake_up_process(task);
	return 0;
}

void nvgpu_thread_stop(struct nvgpu_thread *thread)
{
	bool was_running;

	if (thread->task) {
		was_running = nvgpu_atomic_xchg(&thread->running, false);

		if (was_running) {
			kthread_stop(thread->task);
		}
		thread->task = NULL;
	}
};

void nvgpu_thread_stop_graceful(struct nvgpu_thread *thread,
		void (*thread_stop_fn)(void *data), void *data)
{
	/*
	 * Threads waiting on wq's should have nvgpu_thread_should_stop()
	 * as one of its wakeup condition. This allows the thread to be woken
	 * up when kthread_stop() is invoked and does not require an additional
	 * callback to wakeup the sleeping thread.
	 */
	nvgpu_thread_stop(thread);
};

bool nvgpu_thread_should_stop(struct nvgpu_thread *thread)
{
	return kthread_should_stop();
};

bool nvgpu_thread_is_running(struct nvgpu_thread *thread)
{
	return nvgpu_atomic_read(&thread->running);
};

void nvgpu_thread_join(struct nvgpu_thread *thread)
{
	while (nvgpu_atomic_read(&thread->running)) {
		nvgpu_msleep(10);
	}
};
