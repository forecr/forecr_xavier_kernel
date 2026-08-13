/*
 * Copyright (c) 2021-2023, NVIDIA CORPORATION.  All rights reserved.
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
#include <nvs/nvs_sched.h>
#include <nvgpu/types.h>

#include <nvgpu/nvs.h>
#include <nvgpu/string.h>
#include <nvgpu/kmem.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/runlist.h>
#include <nvgpu/kref.h>

#ifdef CONFIG_NVGPU_GSP_SCHEDULER
#include <nvgpu/gsp_sched.h>
#endif

static struct nvs_sched_ops nvgpu_nvs_ops = {
	.preempt = NULL,
	.recover = NULL,
};

#ifndef NSEC_PER_MSEC
#define NSEC_PER_MSEC 1000000U
#endif

#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
/*
 * TODO: make use of worker items when
 * 1) the active domain gets modified
 *    - currently updates happen asynchronously elsewhere
 *    - either resubmit the domain or do the updates later
 * 2) recovery gets triggered
 *    - currently it just locks all affected runlists
 *    - consider pausing the scheduler logic and signaling users
 */

struct nvgpu_nvs_worker_item {
	struct gk20a *g;
	struct nvgpu_runlist *rl;
	struct nvgpu_runlist_domain *rl_domain;
	struct nvgpu_cond cond;
	bool wait_for_finish;
	bool locked;
	int status;
	struct nvgpu_ref ref;
	struct nvgpu_list_node list;
	nvgpu_atomic_t state;
};

static inline struct nvgpu_nvs_worker_item *
nvgpu_nvs_worker_item_from_worker_item(struct nvgpu_list_node *node)
{
	return (struct nvgpu_nvs_worker_item *)
	   ((uintptr_t)node - offsetof(struct nvgpu_nvs_worker_item, list));
};

static inline struct nvgpu_nvs_worker *
nvgpu_nvs_worker_from_worker(struct nvgpu_worker *worker)
{
	return (struct nvgpu_nvs_worker *)
	   ((uintptr_t)worker - offsetof(struct nvgpu_nvs_worker, worker));
};

static inline struct nvgpu_nvs_worker_item *
nvgpu_nvs_worker_item_from_ref(struct nvgpu_ref *ref_node)
{
	return (struct nvgpu_nvs_worker_item *)
	   ((uintptr_t)ref_node - offsetof(struct nvgpu_nvs_worker_item, ref));
};

static void nvgpu_nvs_worker_poll_init(struct nvgpu_worker *worker)
{
	struct nvgpu_nvs_worker *nvs_worker =
		nvgpu_nvs_worker_from_worker(worker);

	/* 100 ms is a nice arbitrary timeout for default status */
	nvs_worker->current_timeout = 100;
	nvgpu_timeout_init_cpu_timer_sw(worker->g, &nvs_worker->timeout,
		nvs_worker->current_timeout);

	nvgpu_atomic_set(&nvs_worker->nvs_sched_state, NVS_WORKER_STATE_RUNNING);

	/* Atomic Set() and Read() operations donot have implicit barriers */
	nvgpu_wmb();
	nvgpu_cond_signal(&nvs_worker->wq_request);
}

static u32 nvgpu_nvs_worker_wakeup_timeout(struct nvgpu_worker *worker)
{
	struct nvgpu_nvs_worker *nvs_worker =
		nvgpu_nvs_worker_from_worker(worker);

	return nvs_worker->current_timeout;
}

#ifndef CONFIG_NVS_ROUND_ROBIN_SCHEDULER_DISABLE
static u64 nvgpu_nvs_tick(struct gk20a *g)
{
	struct nvgpu_nvs_scheduler *sched = g->scheduler;
	struct nvgpu_nvs_domain *domain;
	struct nvs_domain *nvs_next;
	struct nvgpu_nvs_domain *nvgpu_domain_next;
	u64 timeslice;

	nvs_dbg(g, "nvs tick");

	nvgpu_mutex_acquire(&g->sched_mutex);

	domain = sched->active_domain;

	/* If active_domain == shadow_domain, then nvs_next is NULL */
	nvs_next = nvs_domain_get_next_domain(sched->sched, domain->parent);
	if (nvs_next == NULL) {
		nvs_next = sched->shadow_domain->parent;
	}

	if (nvs_next->priv == sched->shadow_domain) {
		/*
		 * This entire thread is going to be changed soon.
		 * The above check ensures that there are no other domain,
		 * besides the shadow domain. So, its safe to simply return here.
		 * Any shadow domain updates shall are taken care of during
		 * nvgpu_nvs_worker_wakeup_process_item().
		 *
		 * This is a temporary hack for legacy cases where we donot have
		 * any active domains available. This needs to be relooked at
		 * during implementation of manual mode.
		 *
		 * A better fix is to ensure this thread is suspended during Railgate.
		 */
		timeslice = nvs_next->timeslice_ns;
		nvgpu_mutex_release(&g->sched_mutex);
		return timeslice;
	}

	timeslice = nvs_next->timeslice_ns;
	nvgpu_domain_next = nvs_next->priv;

	(void)nvgpu_runlist_tick(g, nvgpu_domain_next->rl_domains, 0ULL);
	sched->active_domain = nvs_next->priv;

	nvgpu_mutex_release(&g->sched_mutex);

	return timeslice;
}

static void nvgpu_nvs_worker_multi_domain_process(struct nvgpu_worker *worker)
{
	struct gk20a *g = worker->g;
	struct nvgpu_nvs_worker *nvs_worker =
		nvgpu_nvs_worker_from_worker(worker);

	if (nvgpu_timeout_peek_expired(&nvs_worker->timeout)) {
		u64 next_timeout_ns = nvgpu_nvs_tick(g);
		u64 timeout = next_timeout_ns + NSEC_PER_MSEC - 1U;

		if (next_timeout_ns != 0U) {
			nvs_worker->current_timeout =
				nvgpu_safe_cast_u64_to_u32(timeout / NSEC_PER_MSEC);
		}

		nvgpu_timeout_init_cpu_timer_sw(g, &nvs_worker->timeout,
				nvs_worker->current_timeout);
	}
}
#endif

static void nvgpu_nvs_worker_item_release(struct nvgpu_ref *ref)
{
	struct nvgpu_nvs_worker_item *work =
			nvgpu_nvs_worker_item_from_ref(ref);
	struct gk20a *g = work->g;

	nvgpu_cond_destroy(&work->cond);
	nvgpu_kfree(g, work);
}

static void nvgpu_nvs_worker_wakeup_process_item(struct nvgpu_list_node *work_item)
{
	struct nvgpu_nvs_worker_item *work =
			nvgpu_nvs_worker_item_from_worker_item(work_item);
	struct gk20a *g = work->g;
	int ret = 0;
	struct nvgpu_nvs_scheduler *sched = g->scheduler;
	struct nvgpu_nvs_domain *nvgpu_nvs_domain;
	struct nvs_domain *nvs_domain;
	struct nvgpu_runlist *runlist = work->rl;
	struct nvgpu_runlist_domain *rl_domain = work->rl_domain;
	bool is_shadow_part_remove = false;

	nvgpu_mutex_acquire(&g->sched_mutex);

	if (rl_domain == NULL) {
		nvs_domain = sched->shadow_domain->parent;
		rl_domain = runlist->shadow_rl_domain;
	} else if (rl_domain->domain_id == SHADOW_DOMAIN_ID) {
		nvs_domain = sched->shadow_domain->parent;
	} else {
		nvgpu_nvs_domain = nvgpu_nvs_domain_by_id_locked(g, rl_domain->domain_id);
		if (nvgpu_nvs_domain == NULL) {
			nvgpu_err(g, "Unable to find domain[%llu]", rl_domain->domain_id);
			ret = -EINVAL;
			goto done;
		} else {
			nvs_domain = nvgpu_nvs_domain->parent;
		}

		if ((sched->active_domain->id == SHADOW_DOMAIN_ID) &&
				rl_domain->remove) {
			rl_domain = runlist->shadow_rl_domain;
			is_shadow_part_remove = true;
			nvs_dbg(g, "remove flag is set for domain %llu",
				rl_domain->domain_id);
		}
	}

	nvs_dbg(g, "Thread sync started");
	/*
	 * There can be two cases where sync_submit is going to be triggered.
	 * 1. Active domain is same as the domain for which work has submitted, or
	 * 2. Shadow domain is active and remove flag is set for the domain for
	 * which work is submitted.
	 */
	if ((sched->active_domain == nvs_domain->priv) || is_shadow_part_remove) {
		/* Instantly switch domain and force runlist updates */
		ret = nvgpu_rl_domain_sync_submit(g, runlist, rl_domain, work->wait_for_finish);
		nvs_dbg(g, "Active thread updated");
	} else {
		ret = 1;
	}

done:
	nvgpu_mutex_release(&g->sched_mutex);
	work->status = ret;
	nvgpu_atomic_set(&work->state, 1);

	nvgpu_smp_mb();
	/* Wakeup threads waiting on runlist submit */
	nvgpu_cond_signal(&work->cond);

	/* This reference was taken as part of nvgpu_nvs_worker_submit */
	nvgpu_ref_put(&work->ref, nvgpu_nvs_worker_item_release);
}

static int nvgpu_nvs_worker_submit(struct gk20a *g, struct nvgpu_runlist *rl,
		struct nvgpu_runlist_domain *rl_domain, bool wait_for_finish)
{
	struct nvgpu_nvs_scheduler *sched = g->scheduler;
	struct nvgpu_nvs_worker *worker = &sched->worker;
	struct nvgpu_nvs_worker_item *work;
	int ret = 0;

	if (sched == NULL) {
		return -ENODEV;
	}

	nvs_dbg(g, " ");

	work = nvgpu_kzalloc(g, sizeof(*work));
	if (work == NULL) {
		nvgpu_err(g, "Unable to allocate memory for runlist job");
		ret = -ENOMEM;
		goto free_domain;
	}

	work->g = g;
	work->rl = rl;
	work->rl_domain = rl_domain;
	nvgpu_cond_init(&work->cond);
	nvgpu_init_list_node(&work->list);
	work->wait_for_finish = wait_for_finish;
	nvgpu_atomic_set(&work->state, 0);
	nvgpu_ref_init(&work->ref);

	nvs_dbg(g, " enqueueing runlist submit");

	/* Add a barrier here to ensure all reads and writes have happened before
	 * enqueuing the job in the worker thread.
	 */
	nvgpu_smp_mb();

	/* The corresponding refcount is decremented inside the wakeup_process item */
	nvgpu_ref_get(&work->ref);
	ret = nvgpu_worker_enqueue(&worker->worker, &work->list);
	if (ret != 0) {
		/* Refcount is decremented here as no additional job is enqueued */
		nvgpu_ref_put(&work->ref, nvgpu_nvs_worker_item_release);
		goto fail;
	}

	ret = NVGPU_COND_WAIT(&work->cond, nvgpu_atomic_read(&work->state) == 1, 0U);
	if (ret != 0) {
		/* refcount is not decremented here since even though this thread is
		 * unblocked, but the job could be still queued.
		 */
		nvgpu_err(g, "Runlist submit interrupted while waiting for submit");
		goto fail;
	}

	nvs_dbg(g, " ");

	ret = work->status;

fail:
	nvgpu_ref_put(&work->ref, nvgpu_nvs_worker_item_release);

free_domain:

	return ret;
}

static bool nvgpu_nvs_worker_wakeup_condition(struct nvgpu_worker *worker)
{
	struct nvgpu_nvs_worker *nvs_worker =
		nvgpu_nvs_worker_from_worker(worker);
	struct gk20a *g = worker->g;
	int nvs_worker_state;

	nvs_worker_state = nvgpu_atomic_read(&nvs_worker->nvs_sched_state);

	if (nvs_worker_state == NVS_WORKER_STATE_SHOULD_RESUME) {
		/* Set the state to running. Worker will automatically update the timeout
		 * in the subsequent if block as previous timeout is 0.
		 */
		nvgpu_atomic_set(&nvs_worker->nvs_sched_state, NVS_WORKER_STATE_RUNNING);

		/* Atomic set donot have an implicit barrier.
		 * Ensure, that value is updated before invoking signal below.
		 */
		nvgpu_wmb();
		/* Signal waiting threads about resume */
		nvgpu_cond_signal(&nvs_worker->wq_request);

		nvs_dbg(g, "nvs set for resume");
	} else if (nvs_worker_state == NVS_WORKER_STATE_SHOULD_PAUSE) {
		return true;
	}

	return false;
}

static void nvgpu_nvs_handle_pause_requests(struct nvgpu_worker *worker)
{
	struct gk20a *g = worker->g;
	struct nvgpu_nvs_worker *nvs_worker =
		nvgpu_nvs_worker_from_worker(worker);
	int nvs_worker_state = nvgpu_atomic_read(&nvs_worker->nvs_sched_state);

	if (nvs_worker_state == NVS_WORKER_STATE_SHOULD_PAUSE) {
		nvgpu_atomic_set(&nvs_worker->nvs_sched_state, NVS_WORKER_STATE_PAUSED);
		/* Set the worker->timeout to 0, to allow the worker thread to sleep infinitely. */
		nvgpu_timeout_init_cpu_timer_sw(g, &nvs_worker->timeout, 0);

		/* Atomic_Set doesn't have an implicit barrier.
		 * Ensure, that value is updated before invoking signal below.
		 */
		nvgpu_wmb();
		/* Wakeup user threads waiting for pause state */
		nvgpu_cond_signal(&nvs_worker->wq_request);

		nvs_dbg(g, "nvs set for pause");
	}
}

static void nvgpu_nvs_worker_wakeup_post_process(struct nvgpu_worker *worker)
{
	if (nvgpu_nvs_ctrl_fifo_is_enabled(worker->g)) {
		nvgpu_nvs_ctrl_fifo_scheduler_handle_requests(worker->g);
	} else {
#ifndef CONFIG_NVS_ROUND_ROBIN_SCHEDULER_DISABLE
		nvgpu_nvs_worker_multi_domain_process(worker);
#else
		(void)worker;
#endif
	}

	nvgpu_nvs_handle_pause_requests(worker);
}

static const struct nvgpu_worker_ops nvs_worker_ops = {
	.pre_process = nvgpu_nvs_worker_poll_init,
	.wakeup_condition = nvgpu_nvs_worker_wakeup_condition,
	.wakeup_timeout = nvgpu_nvs_worker_wakeup_timeout,
	.wakeup_process_item = nvgpu_nvs_worker_wakeup_process_item,
	.wakeup_post_process = nvgpu_nvs_worker_wakeup_post_process,
};

void nvgpu_nvs_worker_pause(struct gk20a *g)
{
	struct nvgpu_worker *worker = &g->scheduler->worker.worker;
	struct nvgpu_nvs_worker *nvs_worker = &g->scheduler->worker;
	int nvs_worker_state;

	if (g->scheduler == NULL)
		return;

	if (g->is_virtual) {
		return;
	}

	nvs_worker_state = nvgpu_atomic_cmpxchg(&nvs_worker->nvs_sched_state,
			NVS_WORKER_STATE_RUNNING, NVS_WORKER_STATE_SHOULD_PAUSE);

	if (nvs_worker_state == NVS_WORKER_STATE_RUNNING) {
		nvs_dbg(g, "Setting thread state to sleep.");
		/* wakeup worker forcibly. */
		nvgpu_cond_signal_interruptible(&worker->wq);

		/* Ensure signal has happened before waiting */
		nvgpu_mb();

		NVGPU_COND_WAIT(&nvs_worker->wq_request,
			nvgpu_atomic_read(
				&nvs_worker->nvs_sched_state) == NVS_WORKER_STATE_PAUSED, 0);

		nvs_dbg(g, "Thread is paused");
	} else {
		nvs_dbg(g, "Thread state is not running.");
	}
}

void nvgpu_nvs_worker_resume(struct gk20a *g)
{
	struct nvgpu_worker *worker = &g->scheduler->worker.worker;
	struct nvgpu_nvs_worker *nvs_worker = &g->scheduler->worker;
	int nvs_worker_state;

	if (g->is_virtual) {
		return;
	}

	nvs_worker_state = nvgpu_atomic_cmpxchg(&nvs_worker->nvs_sched_state,
			NVS_WORKER_STATE_PAUSED, NVS_WORKER_STATE_SHOULD_RESUME);

	if (nvs_worker_state == NVS_WORKER_STATE_PAUSED) {
		nvs_dbg(g, "Waiting for nvs thread to be resumed");

		/* wakeup worker forcibly. */
		nvgpu_cond_signal_interruptible(&worker->wq);

		/* Ensure signal has happened before waiting */
		nvgpu_mb();

		NVGPU_COND_WAIT(&nvs_worker->wq_request,
			nvgpu_atomic_read(
				&nvs_worker->nvs_sched_state) == NVS_WORKER_STATE_RUNNING, 0);

		nvs_dbg(g, "Thread resumed");
	} else {
		nvs_dbg(g, "Thread not paused");
	}
}

static int nvgpu_nvs_worker_init(struct gk20a *g)
{
	int err = 0;
	struct nvgpu_worker *worker = &g->scheduler->worker.worker;
	struct nvgpu_nvs_worker *nvs_worker = &g->scheduler->worker;

	if (nvgpu_is_legacy_vgpu(g)) {
		return 0;
	}

	nvgpu_cond_init(&nvs_worker->wq_request);
	(void)nvgpu_atomic_xchg(&nvs_worker->nvs_sched_state, NVS_WORKER_STATE_STOPPED);

	nvgpu_worker_init_name(worker, "nvgpu_nvs", g->name);

#ifdef __KERNEL__
	/* Scheduler a worker thread with RR priority of 1 for Linux.
	 * Linux uses CFS scheduling class by default for all kernel threads.
	 * CFS prioritizes threads that have executed for the least amount
	 * of time and as a result other higher priority kernel threads
	 * can get delayed.
	 * Using a RT priority of 1 for linux, ensures that this thread
	 * always executes before other regular kernel threads.
	 */
	err = nvgpu_priority_worker_init(g, worker, 1, &nvs_worker_ops);
#else
	err = nvgpu_worker_init(g, worker, &nvs_worker_ops);
#endif
	if (err == 0) {
		/* Ensure that scheduler thread is started as soon as possible to handle
		 * minimal uptime for applications.
		 */
		err = NVGPU_COND_WAIT(&nvs_worker->wq_request,
				nvgpu_atomic_read(
					&nvs_worker->nvs_sched_state) == NVS_WORKER_STATE_RUNNING,
					0);
		if (err != 0) {
			nvgpu_err(g, "Interrupted while waiting for scheduler thread");
		}

		nvs_dbg(g, "Thread started");
	}

	return err;
}

static void nvgpu_nvs_worker_deinit(struct gk20a *g)
{
	struct nvgpu_worker *worker = &g->scheduler->worker.worker;
	struct nvgpu_nvs_worker *nvs_worker = &g->scheduler->worker;

	if (g->is_virtual) {
		return;
	}

	nvgpu_worker_deinit(worker);

	nvgpu_atomic_set(&nvs_worker->nvs_sched_state, NVS_WORKER_STATE_STOPPED);
	nvgpu_cond_destroy(&nvs_worker->wq_request);

	nvs_dbg(g, "NVS worker suspended");
}
#endif /* CONFIG_KMD_SCHEDULING_WORKER_THREAD */

static struct nvgpu_nvs_domain *
	nvgpu_nvs_gen_domain(struct gk20a *g, const char *name, u64 id,
		u64 timeslice, u64 preempt_grace)
{
	struct nvgpu_fifo *f = &g->fifo;
	struct nvs_domain *nvs_dom = NULL;
	struct nvgpu_nvs_domain *nvgpu_dom = NULL;
	u32 num_runlists = f->num_runlists;

	nvs_dbg(g, "Adding new domain: %s", name);

	nvgpu_dom = nvgpu_kzalloc(g, sizeof(*nvgpu_dom));
	if (nvgpu_dom == NULL) {
		nvs_dbg(g, "failed to allocate memory for domain %s", name);
		return nvgpu_dom;
	}

	nvgpu_dom->rl_domains = nvgpu_kzalloc(g, sizeof(*nvgpu_dom->rl_domains) * num_runlists);
	if (nvgpu_dom->rl_domains == NULL) {
		nvs_dbg(g, "failed to allocate memory for domain->rl_domains");
		nvgpu_kfree(g, nvgpu_dom);
		nvgpu_dom = NULL;
		return nvgpu_dom;
	}

	nvgpu_dom->id = id;
	nvgpu_dom->ref = 1U;

	nvs_dom = nvs_domain_create(g->scheduler->sched, name,
				timeslice, preempt_grace, nvgpu_dom);

	if (nvs_dom == NULL) {
		nvs_dbg(g, "failed to create nvs domain for %s", name);
		nvgpu_kfree(g, nvgpu_dom->rl_domains);
		nvgpu_kfree(g, nvgpu_dom);
		nvgpu_dom = NULL;
		return nvgpu_dom;
	}

	nvgpu_dom->parent = nvs_dom;

	return nvgpu_dom;
}

static void nvgpu_nvs_link_shadow_rl_domains(struct gk20a *g,
		struct nvgpu_nvs_domain *nvgpu_dom)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 num_runlists = f->num_runlists;
	u32 i;

	for (i = 0U; i < num_runlists; i++) {
		struct nvgpu_runlist *runlist = &f->active_runlists[i];
		nvgpu_dom->rl_domains[i] = runlist->shadow_rl_domain;
	}
}

static int nvgpu_nvs_gen_shadow_domain(struct gk20a *g)
{
	int err = 0;
	struct nvgpu_nvs_domain *nvgpu_dom;

	if (g->scheduler->shadow_domain != NULL) {
		goto error;
	}

	nvgpu_dom = nvgpu_nvs_gen_domain(g, SHADOW_DOMAIN_NAME, U64_MAX,
		100U * NSEC_PER_MSEC, 0U);
	if (nvgpu_dom == NULL) {
		err = -ENOMEM;
		goto error;
	}

	nvgpu_nvs_link_shadow_rl_domains(g, nvgpu_dom);

	g->scheduler->shadow_domain = nvgpu_dom;

	/* Set active_domain to shadow_domain during Init */
	g->scheduler->active_domain = g->scheduler->shadow_domain;

#ifdef CONFIG_NVGPU_GSP_SCHEDULER
	if (nvgpu_is_enabled(g, (u32)(NVGPU_SUPPORT_GSP_SCHED))) {
		err = nvgpu_gsp_nvs_add_domain(g, nvgpu_dom->id);
		if (err != 0) {
			nvgpu_err(g, "add domain for shadow domain failed");
		}
	}
#endif

error:
	return err;
}

static void nvgpu_nvs_remove_shadow_domain(struct gk20a *g)
{
	struct nvgpu_nvs_scheduler *sched = g->scheduler;
	struct nvs_domain *nvs_dom;
	s32 err = 0;

	if (sched == NULL) {
		/* never powered on to init anything */
		return;
	}

	if (sched->shadow_domain == NULL) {
		return;
	}

	if (sched->shadow_domain->ref != 1U) {
		nvgpu_warn(g,
				"domain %llu is still in use during shutdown! refs: %u",
				sched->shadow_domain->id, sched->shadow_domain->ref);
		nvgpu_err(g, "%u", err);
	}

	nvs_dom = sched->shadow_domain->parent;

	nvs_domain_destroy(sched->sched, nvs_dom);

	nvgpu_kfree(g, sched->shadow_domain->rl_domains);
	sched->shadow_domain->rl_domains = NULL;
	nvgpu_kfree(g, sched->shadow_domain);
	sched->shadow_domain = NULL;
}

int nvgpu_nvs_init(struct gk20a *g)
{
	int err;

	nvgpu_mutex_init(&g->sched_mutex);

	err = nvgpu_nvs_open(g);
	if (err != 0) {
		return err;
	}

	return 0;
}

void nvgpu_nvs_remove_support(struct gk20a *g)
{
	struct nvgpu_nvs_scheduler *sched = g->scheduler;
	struct nvs_domain *nvs_dom;

	if (sched == NULL) {
		/* never powered on to init anything */
		return;
	}
#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
	nvgpu_nvs_worker_deinit(g);
#endif
	nvs_domain_for_each(sched->sched, nvs_dom) {
		struct nvgpu_nvs_domain *nvgpu_dom = nvs_dom->priv;
		if (nvgpu_dom->ref != 1U) {
			nvgpu_warn(g,
				   "domain %llu is still in use during shutdown! refs: %u",
				   nvgpu_dom->id, nvgpu_dom->ref);
		}

		/* runlist removal will clear the rl domains */
		nvgpu_kfree(g, nvgpu_dom);
	}

	nvgpu_nvs_remove_shadow_domain(g);

	nvs_sched_close(sched->sched);
	nvgpu_kfree(g, sched->sched);
	nvgpu_kfree(g, sched);
	g->scheduler = NULL;

	nvgpu_nvs_ctrl_fifo_destroy(g);

	nvgpu_mutex_destroy(&g->sched_mutex);
}

int nvgpu_nvs_open(struct gk20a *g)
{
	int err = 0;

	nvs_dbg(g, "Opening NVS node.");

	nvgpu_mutex_acquire(&g->sched_mutex);

	if (g->scheduler != NULL) {
		/* resuming from railgate */
		nvgpu_mutex_release(&g->sched_mutex);
#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
		if (nvgpu_is_enabled(g, NVGPU_SUPPORT_KMD_SCHEDULING_WORKER_THREAD)) {
			nvgpu_nvs_worker_resume(g);
		}
#endif
#ifdef CONFIG_NVGPU_GSP_SCHEDULER
	if (nvgpu_is_enabled(g, (u32)(NVGPU_SUPPORT_GSP_SCHED))) {
		err = nvgpu_gsp_nvs_add_domain(g, U64_MAX);
		if (err != 0) {
			nvgpu_err(g, "add domain for shadow domain failed");
		}
	}
#endif
		return err;
	}

	g->scheduler = nvgpu_kzalloc(g, sizeof(*g->scheduler));
	if (g->scheduler == NULL) {
		err = -ENOMEM;
		goto unlock;
	}

	nvgpu_atomic64_set(&g->scheduler->id_counter, 0);
	nvgpu_smp_wmb();

	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_NVS)) {
		g->sched_ctrl_fifo = nvgpu_nvs_ctrl_fifo_create(g);
		if (g->sched_ctrl_fifo == NULL) {
			err = -ENOMEM;
			goto unlock;
		}
	}

	/* separately allocated to keep the definition hidden from other files */
	g->scheduler->sched = nvgpu_kzalloc(g, sizeof(*g->scheduler->sched));
	if (g->scheduler->sched == NULL) {
		err = -ENOMEM;
		goto unlock;
	}

	nvs_dbg(g, "  Creating NVS scheduler.");
	err = nvs_sched_create(g->scheduler->sched, &nvgpu_nvs_ops, g);
	if (err != 0) {
		goto unlock;
	}

	/* Ensure all the previous writes are seen */
	nvgpu_wmb();

	err = nvgpu_nvs_gen_shadow_domain(g);
	if (err != 0) {
		goto unlock;
	}

	/* Ensure all the previous writes are seen */
	nvgpu_wmb();

#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_KMD_SCHEDULING_WORKER_THREAD)) {
		err = nvgpu_nvs_worker_init(g);
		if (err != 0) {
			nvgpu_nvs_remove_shadow_domain(g);
			goto unlock;
		}

		g->nvs_worker_submit = nvgpu_nvs_worker_submit;
#endif
unlock:

#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
		if (err) {
			nvs_dbg(g, "  Failed! Error code: %d", err);
			if (g->scheduler) {
				nvgpu_kfree(g, g->scheduler->sched);
				nvgpu_kfree(g, g->scheduler);
				g->scheduler = NULL;
			}
			if (g->sched_ctrl_fifo)
				nvgpu_nvs_ctrl_fifo_destroy(g);
		}
	}
#endif

#ifdef CONFIG_NVGPU_GSP_SCHEDULER
	if (err != 0) {
		if (nvgpu_is_enabled(g, (u32)(NVGPU_SUPPORT_GSP_SCHED))) {
			err = nvgpu_gsp_nvs_delete_domain(g, g->scheduler->shadow_domain->id);
			if (err != 0) {
				nvgpu_err(g, "delete domain for shadow domain failed");
			}
		}
	}
#endif

	nvgpu_mutex_release(&g->sched_mutex);

	return err;
}

/*
 * A trivial allocator for now.
 */
static u64 nvgpu_nvs_new_id(struct gk20a *g)
{
	return nvgpu_safe_cast_s64_to_u64(
		nvgpu_atomic64_inc_return(&g->scheduler->id_counter));
}

static int nvgpu_nvs_create_rl_domain_mem(struct gk20a *g,
		struct nvgpu_nvs_domain *domain)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 i, j;
	int err = 0;

	for (i = 0U; i < f->num_runlists; i++) {
		domain->rl_domains[i] = nvgpu_runlist_domain_alloc(g, domain->id);
		if (domain->rl_domains[i] == NULL) {
			err = -ENOMEM;
			break;
		}
	}

	if (err != 0) {
		for (j = 0; j != i; j++) {
			nvgpu_runlist_domain_free(g, domain->rl_domains[j]);
			domain->rl_domains[j] = NULL;
		}
	}

	return err;
}

static void nvgpu_nvs_link_rl_domains(struct gk20a *g,
		struct nvgpu_nvs_domain *domain)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 i;

	for (i = 0U; i < f->num_runlists; i++) {
		struct nvgpu_runlist *runlist;

		runlist = &f->active_runlists[i];
		nvgpu_runlist_link_domain(runlist, domain->rl_domains[i]);
	}
}

int nvgpu_nvs_add_domain(struct gk20a *g, const char *name, u64 timeslice,
			 u64 preempt_grace, struct nvgpu_nvs_domain **pdomain)
{
	int err = 0;
	struct nvs_domain *nvs_dom;
	struct nvgpu_nvs_domain *nvgpu_dom;
	struct nvgpu_nvs_scheduler *sched = g->scheduler;

	if (name == NULL || pdomain == NULL) {
		return -EINVAL;
	}

	err = nvgpu_string_validate(name);
	if (err != 0) {
		nvgpu_err(g, "Domain Name %s is Invalid.\n", name);
		return err;
	}

	nvgpu_mutex_acquire(&g->sched_mutex);

	if (nvs_domain_by_name(g->scheduler->sched, name) != NULL) {
		err = -EEXIST;
		goto unlock;
	}

	nvgpu_dom = nvgpu_nvs_gen_domain(g, name, nvgpu_nvs_new_id(g),
		timeslice, preempt_grace);
	if (nvgpu_dom == NULL) {
		err = -ENOMEM;
		goto unlock;
	}

	err = nvgpu_nvs_create_rl_domain_mem(g, nvgpu_dom);
	if (err != 0) {
		nvs_domain_destroy(sched->sched, nvgpu_dom->parent);
		nvgpu_kfree(g, nvgpu_dom->rl_domains);
		nvgpu_kfree(g, nvgpu_dom);
		goto unlock;
	}

	nvgpu_nvs_link_rl_domains(g, nvgpu_dom);

	nvs_dom = nvgpu_dom->parent;

	nvs_domain_scheduler_attach(g->scheduler->sched, nvs_dom);

	nvgpu_dom->parent = nvs_dom;

#ifdef CONFIG_NVGPU_GSP_SCHEDULER
	if (nvgpu_is_enabled(g, (u32)(NVGPU_SUPPORT_GSP_SCHED))) {
		err = nvgpu_gsp_nvs_add_domain(g, nvgpu_dom->id);
		if (err != 0) {
			nvgpu_err(g, "sending domain info to gsp failed");
			goto unlock;
		}
	}
#endif

	*pdomain = nvgpu_dom;
unlock:
	nvgpu_mutex_release(&g->sched_mutex);

	return err;
}

struct nvgpu_nvs_domain *
nvgpu_nvs_domain_by_id_locked(struct gk20a *g, u64 domain_id)
{
	struct nvgpu_nvs_scheduler *sched = g->scheduler;
	struct nvs_domain *nvs_dom;

	nvgpu_log(g, gpu_dbg_nvs, "lookup %llu", domain_id);

	nvs_domain_for_each(sched->sched, nvs_dom) {
		struct nvgpu_nvs_domain *nvgpu_dom = nvs_dom->priv;

		if (nvgpu_dom->id == domain_id) {
			return nvgpu_dom;
		}
	}
	
	return NULL;
}

struct nvgpu_nvs_domain *
nvgpu_nvs_get_shadow_domain_locked(struct gk20a *g)
{
	return g->scheduler->shadow_domain;
}

struct nvgpu_nvs_domain *
nvgpu_nvs_domain_by_id(struct gk20a *g, u64 domain_id)
{
	struct nvgpu_nvs_domain *dom = NULL;

	nvgpu_log(g, gpu_dbg_nvs, "lookup %llu", domain_id);

	nvgpu_mutex_acquire(&g->sched_mutex);

	dom = nvgpu_nvs_domain_by_id_locked(g, domain_id);
	if (dom == NULL) {
		goto unlock;
	}

	dom->ref++;

unlock:
	nvgpu_mutex_release(&g->sched_mutex);
	return dom;
}

struct nvgpu_nvs_domain *
nvgpu_nvs_domain_by_name(struct gk20a *g, const char *name)
{
	struct nvs_domain *nvs_dom;
	struct nvgpu_nvs_domain *dom = NULL;
	struct nvgpu_nvs_scheduler *sched = g->scheduler;

	if (name == NULL) {
		return NULL;
	}

	nvgpu_log(g, gpu_dbg_nvs, "lookup %s", name);

	nvgpu_mutex_acquire(&g->sched_mutex);

	nvs_dom = nvs_domain_by_name(sched->sched, name);
	if (nvs_dom == NULL) {
		goto unlock;
	}

	dom = nvs_dom->priv;
	dom->ref++;

unlock:
	nvgpu_mutex_release(&g->sched_mutex);
	return dom;
}

void nvgpu_nvs_domain_get(struct gk20a *g, struct nvgpu_nvs_domain *dom)
{
	if (dom == NULL) {
		return;
	}

	nvgpu_mutex_acquire(&g->sched_mutex);
	WARN_ON(dom->ref == 0U);
	dom->ref++;
	nvgpu_log(g, gpu_dbg_nvs, "domain %s: ref++ = %u",
			dom->parent->name, dom->ref);
	nvgpu_mutex_release(&g->sched_mutex);
}

void nvgpu_nvs_domain_put(struct gk20a *g, struct nvgpu_nvs_domain *dom)
{
	if (dom == NULL) {
		return;
	}

	nvgpu_mutex_acquire(&g->sched_mutex);
	dom->ref--;
	WARN_ON(dom->ref == 0U);
	nvgpu_log(g, gpu_dbg_nvs, "domain %s: ref-- = %u",
			dom->parent->name, dom->ref);
	nvgpu_mutex_release(&g->sched_mutex);
}

static void nvgpu_nvs_delete_rl_domain_mem(struct gk20a *g, struct nvgpu_nvs_domain *dom)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 i;

	for (i = 0U; i < f->num_runlists; i++) {
		nvgpu_runlist_domain_free(g, dom->rl_domains[i]);
		dom->rl_domains[i] = NULL;
	}
}

static void nvgpu_nvs_unlink_rl_domains(struct gk20a *g, struct nvgpu_nvs_domain *domain)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 i;

	for (i = 0; i < f->num_runlists; i++) {
		struct nvgpu_runlist *runlist;
		runlist = &f->active_runlists[i];

		nvgpu_runlist_unlink_domain(runlist, domain->rl_domains[i]);
	}
}

int nvgpu_nvs_del_domain(struct gk20a *g, u64 dom_id)
{
	struct nvgpu_nvs_scheduler *s = g->scheduler;
	struct nvgpu_nvs_domain *nvgpu_dom;
	struct nvs_domain *nvs_dom, *nvs_next;
	int err = 0;

	nvgpu_mutex_acquire(&g->sched_mutex);

	nvs_dbg(g, "Attempting to remove domain: %llu", dom_id);

	nvgpu_dom = nvgpu_nvs_domain_by_id_locked(g, dom_id);
	if (nvgpu_dom == NULL) {
		nvs_dbg(g, "domain %llu does not exist!", dom_id);
		err = -ENOENT;
		goto unlock;
	}

	if (nvgpu_dom->ref != 1U) {
		nvs_dbg(g, "domain %llu is still in use! refs: %u",
				dom_id, nvgpu_dom->ref);
		err = -EBUSY;
		goto unlock;
	}

	nvs_dom = nvgpu_dom->parent;

#ifdef CONFIG_NVGPU_GSP_SCHEDULER
	if (nvgpu_is_enabled(g, (u32)(NVGPU_SUPPORT_GSP_SCHED))) {
		err = nvgpu_gsp_nvs_delete_domain(g, dom_id);
		if (err != 0) {
			nvgpu_err(g, "failed to delete domain");
		}
	}
#endif
	nvgpu_nvs_unlink_rl_domains(g, nvgpu_dom);
	nvgpu_nvs_delete_rl_domain_mem(g, nvgpu_dom);
	nvgpu_dom->ref = 0U;

	if (s->active_domain == nvgpu_dom) {
		nvs_next = nvs_domain_get_next_domain(s->sched, nvs_dom);
		/* Its the only entry in the list. Set the shadow domain as the active domain */
		if (nvs_next == nvs_dom) {
			nvs_next = s->shadow_domain->parent;
		}
		s->active_domain = nvs_next->priv;
	}

	nvs_domain_unlink_and_destroy(s->sched, nvs_dom);

	nvgpu_kfree(g, nvgpu_dom->rl_domains);
	nvgpu_kfree(g, nvgpu_dom);

unlock:
	nvgpu_mutex_release(&g->sched_mutex);
	return err;
}

u32 nvgpu_nvs_domain_count(struct gk20a *g)
{
	u32 count;

	nvgpu_mutex_acquire(&g->sched_mutex);
	count = nvs_domain_count(g->scheduler->sched);
	nvgpu_mutex_release(&g->sched_mutex);

	return count;
}

const char *nvgpu_nvs_domain_get_name(struct nvgpu_nvs_domain *dom)
{
	struct nvs_domain *nvs_dom;

	if (dom == NULL) {
		return NULL;
	}

	nvs_dom = dom->parent;

	return nvs_dom->name;
}

void nvgpu_nvs_get_log(struct gk20a *g, s64 *timestamp, const char **msg)
{
	struct nvs_log_event ev;

	nvs_log_get(g->scheduler->sched, &ev);

	if (ev.event == NVS_EV_NO_EVENT) {
		*timestamp = 0;
		*msg = NULL;
		return;
	}

	*msg       = nvs_log_event_string(ev.event);
	*timestamp = ev.timestamp;
}

void nvgpu_nvs_print_domain(struct gk20a *g, struct nvgpu_nvs_domain *domain)
{
	struct nvs_domain *nvs_dom;

	if (domain == NULL) {
		return;
	}

	nvs_dom = domain->parent;

	nvs_dbg(g, "Domain %s", nvs_dom->name);
	nvs_dbg(g, "  timeslice:     %llu ns", nvs_dom->timeslice_ns);
	nvs_dbg(g, "  preempt grace: %llu ns", nvs_dom->preempt_grace_ns);
	nvs_dbg(g, "  domain ID:     %llu", domain->id);
}

#ifdef CONFIG_NVGPU_GSP_SCHEDULER
s32 nvgpu_nvs_gsp_get_runlist_domain_info(struct gk20a *g, u64 nvgpu_domain_id,
		u32 *num_entries, u64 *runlist_iova, u32 *aperture, u32 index)
{
	struct nvgpu_runlist_domain *domain;
	struct nvgpu_nvs_domain *nvgpu_domain;
	s32 err = 0;

	if (nvgpu_domain_id == (u64)(SHADOW_DOMAIN_ID)) {
		nvgpu_domain = nvgpu_nvs_get_shadow_domain_locked(g);
		if (nvgpu_domain == NULL) {
			nvgpu_err(g, "gsp nvgpu_domain is NULL");
			err = -ENXIO;
			goto exit;
		}

		domain = nvgpu_runlist_get_shadow_domain(g);
	} else {
		nvgpu_domain = nvgpu_nvs_domain_by_id_locked(g, nvgpu_domain_id);
		if (nvgpu_domain == NULL) {
			nvgpu_err(g, "gsp nvgpu_domain is NULL");
			err = -ENXIO;
			goto exit;
		}

		domain = nvgpu_domain->rl_domains[index];
	}

	if (domain == NULL) {
		nvgpu_err(g, "gsp runlist domain is NULL");
		err = -ENXIO;
		goto exit;
	}

	*runlist_iova = nvgpu_mem_get_addr(g, &domain->mem_hw->mem);
	*aperture = g->ops.runlist.get_runlist_aperture(g, &domain->mem_hw->mem);
	*num_entries = domain->mem_hw->count;
exit:
	return err;
}

s32 nvgpu_nvs_get_gsp_domain_info(struct gk20a *g, u64 nvgpu_domain_id,
		u32 *domain_id, u32 *timeslice_ns)
{
	struct nvgpu_nvs_domain *nvgpu_domain;
    s32 err = 0;

    if (nvgpu_domain_id == SHADOW_DOMAIN_ID) {
        nvgpu_domain = nvgpu_nvs_get_shadow_domain_locked(g);
    } else {
        nvgpu_domain = nvgpu_nvs_domain_by_id_locked(g, nvgpu_domain_id);
    }

    if (nvgpu_domain == NULL) {
        nvgpu_err(g, "gsp nvgpu_domain is NULL");
        err = -ENXIO;
		goto exit;
    }
	*domain_id = u64_lo32(nvgpu_domain->id);
	*timeslice_ns = nvgpu_safe_cast_u64_to_u32(
			nvgpu_domain->parent->timeslice_ns);
exit:
	return err;
}
#endif

#ifdef CONFIG_NVS_PRESENT
bool nvgpu_nvs_gsp_usr_domain_present(struct gk20a *g)
{
	bool ret = false;

	if (nvs_domain_count(g->scheduler->sched) > 0U) {
		/* for count more than 0 user domains are present */
		ret = true;
	}
	return ret;
}
#endif