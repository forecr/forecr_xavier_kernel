// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/cond.h>
#include <nvgpu/periodic_timer.h>

#ifdef CONFIG_NVGPU_FECS_TRACE
static void timer_callback(union sigval arg)
{
	struct nvgpu_periodic_timer *timer = arg.sival_ptr;

	timer->fn(timer->arg);
	nvgpu_cond_lock(&timer->cond);
	if (!timer->enabled) {
		timer->last_run_done = true;
		nvgpu_cond_broadcast_locked(&timer->cond);
	} else {
		int err;
		err = timer_settime(timer->timerid, 0, &timer->ts, NULL);
		nvgpu_assert(err == 0);
	}
	nvgpu_cond_unlock(&timer->cond);
}

int nvgpu_periodic_timer_init(struct nvgpu_periodic_timer *timer,
			void (*fn)(void *arg), void *arg)
{
	struct sigevent se = {};
	int err;

	se.sigev_notify = SIGEV_THREAD;
	se.sigev_notify_function = timer_callback;
	se.sigev_value.sival_ptr = timer;

	err = timer_create(CLOCK_MONOTONIC, &se, &timer->timerid);
	if (err == -1) {
		err = -errno;
	} else {
		timer->fn = fn;
		timer->arg = arg;
		timer->enabled = false;
		timer->last_run_done = false;
		nvgpu_cond_init(&timer->cond);
	}
	return err;
}

#define S2NS 1000000000UL
int nvgpu_periodic_timer_start(struct nvgpu_periodic_timer *timer,
				u64 interval_ns)
{
	struct itimerspec *ts = &timer->ts;
	u64 interval;
	int err;

	memset(ts, 0, sizeof(*ts));
	interval = interval_ns / S2NS;
	ts->it_value.tv_sec = (time_t)interval;
	interval = interval_ns % S2NS;
	ts->it_value.tv_nsec = (long)interval;

	timer->enabled = true;
	err = timer_settime(timer->timerid, 0, ts, NULL);
	if (err == -1) {
		err = -errno;
	}
	return err;
}

int nvgpu_periodic_timer_stop(struct nvgpu_periodic_timer *timer)
{
	struct itimerspec *ts = &timer->ts;
	struct itimerspec old_ts = {};
	int err;

	nvgpu_cond_lock(&timer->cond);
	if (!timer->enabled) {
		nvgpu_cond_unlock(&timer->cond);
		return 0;
	}
	timer->enabled = false;
	timer->last_run_done = false;
	nvgpu_cond_unlock(&timer->cond);

	/* no one will restart the single shot timer from now */

	ts->it_value.tv_sec = 0;
	ts->it_value.tv_nsec = 0;
	err = timer_settime(timer->timerid, 0, ts, &old_ts);
	if (err == -1) {
		err = -errno;
		return err;
	}

	if (old_ts.it_value.tv_sec == 0 && old_ts.it_value.tv_nsec == 0) {
		/* timer is running or prepared to run */
		err = NVGPU_COND_WAIT(&timer->cond,
				timer->last_run_done, 0U);
	}
	return err;
}

int nvgpu_periodic_timer_destroy(struct nvgpu_periodic_timer *timer)
{
	int err;

	err = nvgpu_periodic_timer_stop(timer);
	if (err != 0) {
		return err;
	}
	err = timer_delete(timer->timerid);
	if (err == 0) {
		nvgpu_cond_destroy(&timer->cond);
	} else {
		err = -errno;
	}
	return err;
}
#endif
