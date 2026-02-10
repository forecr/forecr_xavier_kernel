/*
 * Copyright (c) 2011-2023, NVIDIA CORPORATION.  All rights reserved.
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
#include <nvgpu/channel.h>
#include <nvgpu/fifo.h>
#include <nvgpu/engines.h>
#include <nvgpu/device.h>
#include <nvgpu/runlist.h>
#include <nvgpu/pbdma.h>
#include <nvgpu/ptimer.h>
#include <nvgpu/bug.h>
#include <nvgpu/dma.h>
#include <nvgpu/rc.h>
#include <nvgpu/barrier.h>
#include <nvgpu/string.h>
#include <nvgpu/lock.h>
#include <nvgpu/static_analysis.h>
#ifdef CONFIG_NVGPU_LS_PMU
#include <nvgpu/pmu/mutex.h>
#endif
#include <nvgpu/nvgpu_init.h>
#ifdef CONFIG_NVGPU_GSP_SCHEDULER
#include <nvgpu/gsp_sched.h>
#endif
#ifdef CONFIG_NVS_PRESENT
#include <nvgpu/nvs.h>
#endif
void nvgpu_runlist_lock_active_runlists(struct gk20a *g)
{
	struct nvgpu_fifo *f = &g->fifo;
	struct nvgpu_runlist *runlist;
	u32 i;

	nvgpu_log_info(g, "acquire runlist_lock for active runlists");
	for (i = 0; i < g->fifo.num_runlists; i++) {
		runlist = &f->active_runlists[i];
		nvgpu_mutex_acquire(&runlist->runlist_lock);
	}
}

void nvgpu_runlist_unlock_active_runlists(struct gk20a *g)
{
	struct nvgpu_fifo *f = &g->fifo;
	struct nvgpu_runlist *runlist;
	u32 i;

	nvgpu_log_info(g, "release runlist_lock for active runlists");
	for (i = 0; i < g->fifo.num_runlists; i++) {
		runlist = &f->active_runlists[i];
		nvgpu_mutex_release(&runlist->runlist_lock);
	}
}

static u32 nvgpu_runlist_append_tsg(struct gk20a *g,
		struct nvgpu_runlist_domain *domain,
		u32 **runlist_entry,
		u32 *entries_left,
		struct nvgpu_tsg *tsg)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 runlist_entry_words = f->runlist_entry_size / (u32)sizeof(u32);
	struct nvgpu_channel *ch;
	u32 count = 0;
	u32 timeslice;
	int err;

	nvgpu_log_fn(f->g, " ");

	if (*entries_left == 0U) {
		return RUNLIST_APPEND_FAILURE;
	}

	/* add TSG entry */
	nvgpu_log_info(g, "add TSG %d to runlist", tsg->tsgid);

	/*
	 * timeslice is measured with PTIMER.
	 * On some platforms, PTIMER is lower than 1GHz.
	 */
	err = nvgpu_ptimer_scale(g, tsg->timeslice_us, &timeslice);
	if (err != 0) {
		return RUNLIST_APPEND_FAILURE;
	}

	g->ops.runlist.get_tsg_entry(tsg, *runlist_entry, timeslice);

	nvgpu_log_info(g, "tsg rl entries left %d runlist [0] %x [1] %x",
			*entries_left,
			(*runlist_entry)[0], (*runlist_entry)[1]);
	*runlist_entry += runlist_entry_words;
	count++;
	(*entries_left)--;

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);
	/* add runnable channels bound to this TSG */
	nvgpu_list_for_each_entry(ch, &tsg->ch_list,
			nvgpu_channel, ch_entry) {
		if (!nvgpu_test_bit(ch->chid,
			      domain->active_channels)) {
			continue;
		}

		if (*entries_left == 0U) {
			nvgpu_rwsem_up_read(&tsg->ch_list_lock);
			return RUNLIST_APPEND_FAILURE;
		}

		nvgpu_log_info(g, "add channel %d to runlist",
			ch->chid);
		g->ops.runlist.get_ch_entry(ch, *runlist_entry);
		nvgpu_log_info(g, "rl entries left %d runlist [0] %x [1] %x",
			*entries_left,
			(*runlist_entry)[0], (*runlist_entry)[1]);
		count = nvgpu_safe_add_u32(count, 1U);
		*runlist_entry += runlist_entry_words;
		(*entries_left)--;
	}
	nvgpu_rwsem_up_read(&tsg->ch_list_lock);

	return count;
}


static u32 nvgpu_runlist_append_prio(struct nvgpu_fifo *f,
				struct nvgpu_runlist_domain *domain,
				u32 **runlist_entry,
				u32 *entries_left,
				u32 interleave_level)
{
	u32 count = 0;
	unsigned long tsgid;

	nvgpu_log_fn(f->g, " ");

	for_each_set_bit(tsgid, domain->active_tsgs, f->num_channels) {
		struct nvgpu_tsg *tsg = nvgpu_tsg_get_from_id(f->g, (u32)tsgid);
		u32 entries;

		if (tsg->interleave_level == interleave_level) {
			entries = nvgpu_runlist_append_tsg(f->g, domain,
					runlist_entry, entries_left, tsg);
			if (entries == RUNLIST_APPEND_FAILURE) {
				return RUNLIST_APPEND_FAILURE;
			}
			count += entries;
		}
	}

	return count;
}

static u32 nvgpu_runlist_append_hi(struct nvgpu_fifo *f,
				struct nvgpu_runlist_domain *domain,
				u32 **runlist_entry,
				u32 *entries_left)
{
	nvgpu_log_fn(f->g, " ");

	/*
	 * No higher levels - this is where the "recursion" ends; just add all
	 * active TSGs at this level.
	 */
	return nvgpu_runlist_append_prio(f, domain, runlist_entry,
			entries_left,
			NVGPU_FIFO_RUNLIST_INTERLEAVE_LEVEL_HIGH);
}

static u32 nvgpu_runlist_append_med(struct nvgpu_fifo *f,
				struct nvgpu_runlist_domain *domain,
				u32 **runlist_entry,
				u32 *entries_left)
{
	u32 count = 0;
	unsigned long tsgid;

	nvgpu_log_fn(f->g, " ");

	for_each_set_bit(tsgid, domain->active_tsgs, f->num_channels) {
		struct nvgpu_tsg *tsg = nvgpu_tsg_get_from_id(f->g, (u32)tsgid);
		u32 entries;

		if (tsg->interleave_level !=
				NVGPU_FIFO_RUNLIST_INTERLEAVE_LEVEL_MEDIUM) {
			continue;
		}

		/* LEVEL_MEDIUM list starts with a LEVEL_HIGH, if any */

		entries = nvgpu_runlist_append_hi(f, domain,
				runlist_entry, entries_left);
		if (entries == RUNLIST_APPEND_FAILURE) {
			return RUNLIST_APPEND_FAILURE;
		}
		count += entries;

		entries = nvgpu_runlist_append_tsg(f->g, domain,
				runlist_entry, entries_left, tsg);
		if (entries == RUNLIST_APPEND_FAILURE) {
			return RUNLIST_APPEND_FAILURE;
		}
		count += entries;
	}

	return count;
}

static u32 nvgpu_runlist_append_low(struct nvgpu_fifo *f,
				struct nvgpu_runlist_domain *domain,
				u32 **runlist_entry,
				u32 *entries_left)
{
	u32 count = 0;
	unsigned long tsgid;

	nvgpu_log_fn(f->g, " ");

	for_each_set_bit(tsgid, domain->active_tsgs, f->num_channels) {
		struct nvgpu_tsg *tsg = nvgpu_tsg_get_from_id(f->g, (u32)tsgid);
		u32 entries;

		if (tsg->interleave_level !=
				NVGPU_FIFO_RUNLIST_INTERLEAVE_LEVEL_LOW) {
			continue;
		}

		/* The medium level starts with the highs, if any. */

		entries = nvgpu_runlist_append_med(f, domain,
				runlist_entry, entries_left);
		if (entries == RUNLIST_APPEND_FAILURE) {
			return RUNLIST_APPEND_FAILURE;
		}
		count += entries;

		entries = nvgpu_runlist_append_hi(f, domain,
				runlist_entry, entries_left);
		if (entries == RUNLIST_APPEND_FAILURE) {
			return RUNLIST_APPEND_FAILURE;
		}
		count += entries;

		entries = nvgpu_runlist_append_tsg(f->g, domain,
				runlist_entry, entries_left, tsg);
		if (entries == RUNLIST_APPEND_FAILURE) {
			return RUNLIST_APPEND_FAILURE;
		}
		count += entries;
	}

	if (count == 0U) {
		/*
		 * No transitions to fill with higher levels, so add
		 * the next level once. If that's empty too, we have only
		 * LEVEL_HIGH jobs.
		 */
		count = nvgpu_runlist_append_med(f, domain,
				runlist_entry, entries_left);
		if (count == 0U) {
			count = nvgpu_runlist_append_hi(f, domain,
					runlist_entry, entries_left);
		}
	}

	return count;
}

static u32 nvgpu_runlist_append_flat(struct nvgpu_fifo *f,
				struct nvgpu_runlist_domain *domain,
				u32 **runlist_entry,
				u32 *entries_left)
{
	u32 count = 0, entries, i;

	nvgpu_log_fn(f->g, " ");

	/* Group by priority but don't interleave. High comes first. */

	for (i = 0; i < NVGPU_FIFO_RUNLIST_INTERLEAVE_NUM_LEVELS; i++) {
		u32 level = NVGPU_FIFO_RUNLIST_INTERLEAVE_LEVEL_HIGH - i;

		entries = nvgpu_runlist_append_prio(f, domain, runlist_entry,
				entries_left, level);
		if (entries == RUNLIST_APPEND_FAILURE) {
			return RUNLIST_APPEND_FAILURE;
		}
		count += entries;
	}

	return count;
}

u32 nvgpu_runlist_construct_locked(struct nvgpu_fifo *f,
				struct nvgpu_runlist_domain *domain,
				u32 max_entries)
{
	u32 *runlist_entry_base = domain->mem->mem.cpu_va;

	/*
	 * The entry pointer and capacity counter that live on the stack here
	 * keep track of the current position and the remaining space when tsg
	 * and channel entries are ultimately appended.
	 */
	if (f->g->runlist_interleave) {
		return nvgpu_runlist_append_low(f, domain,
				&runlist_entry_base, &max_entries);
	} else {
		return nvgpu_runlist_append_flat(f, domain,
				&runlist_entry_base, &max_entries);
	}
}


static bool nvgpu_runlist_modify_active_add_channel(
					struct nvgpu_runlist_domain *domain,
					struct nvgpu_channel *ch,
					struct nvgpu_tsg *tsg)
{
	if (nvgpu_test_and_set_bit(ch->chid,
			domain->active_channels)) {
		/* was already there */
		return false;
	} else {
		/* new, and belongs to a tsg */
		nvgpu_set_bit(tsg->tsgid, domain->active_tsgs);

		return true;
	}
}

static bool nvgpu_runlist_modify_active_remove_channel(
					struct nvgpu_runlist_domain *domain,
					struct nvgpu_channel *ch,
					struct nvgpu_tsg *tsg)
{
	if (!nvgpu_test_and_clear_bit(ch->chid,
			domain->active_channels)) {
		/* wasn't there */
		return false;
	} else {
		if (tsg->num_active_channels == 1U) {
			/* was the only member of this tsg */
			nvgpu_clear_bit(tsg->tsgid,
					domain->active_tsgs);
		}
		return true;
	}
}

/*
 * When a user rl domain is updated(i.e. a channel is removed/added),
 * the shadow rl domain is also updated. A channel remove/add
 * might require tsg states to be updated i.e. tsg->num_active_channels.
 * The flag can_update_tsg_state is used to control whether
 * this function can update the tsg states.
 */
static bool nvgpu_runlist_modify_active_locked(struct gk20a *g,
					struct nvgpu_runlist_domain *domain,
					struct nvgpu_channel *ch, bool add,
					bool can_update_tsg_state)
{
	struct nvgpu_tsg *tsg = nvgpu_tsg_from_ch(ch);
	bool needs_tsg_update;

	if (tsg == NULL) {
		/*
		 * Unsupported condition, but shouldn't break anything. Warn
		 * and tell the caller that nothing has changed.
		 */
		nvgpu_warn(g, "Bare channel in runlist update");
		return false;
	}

	if (add) {
		needs_tsg_update = nvgpu_runlist_modify_active_add_channel(
				domain, ch, tsg);

		if (needs_tsg_update && can_update_tsg_state) {
			tsg->num_active_channels = nvgpu_safe_add_u32(
				tsg->num_active_channels, 1U);
		}
	} else {
		needs_tsg_update = nvgpu_runlist_modify_active_remove_channel(
				domain, ch, tsg);

		if (needs_tsg_update && can_update_tsg_state) {
			tsg->num_active_channels = nvgpu_safe_sub_u32(
					tsg->num_active_channels, 1U);
		}
	}

	return needs_tsg_update;
}

static int nvgpu_runlist_reconstruct_locked(struct gk20a *g,
					    struct nvgpu_runlist *runlist,
					    struct nvgpu_runlist_domain *domain,
					    bool add_entries)
{
	u32 num_entries;
	struct nvgpu_fifo *f = &g->fifo;

	if (!add_entries) {
		domain->mem->count = 0;
		return 0;
	}

	num_entries = nvgpu_runlist_construct_locked(f, domain,
						f->num_runlist_entries);
	if (num_entries == RUNLIST_APPEND_FAILURE) {
		return -E2BIG;
	}

	domain->mem->count = num_entries;

	rl_dbg(g, "runlist[%u] num entries: [%u]", runlist->id, domain->mem->count);

	WARN_ON(domain->mem->count > f->num_runlist_entries);

	return 0;
}

void nvgpu_runlist_swap_mem(struct gk20a *g, struct nvgpu_runlist_domain *domain)
{
	struct nvgpu_runlist_mem *mem_tmp;

	/*
	 * mem becomes the previously scheduled buffer and it can be modified once
	 * the runlist lock is released.
	 */
	rl_dbg(g, "Swapping mem for rl domain[%llu]", domain->domain_id);

	nvgpu_spinlock_acquire(&domain->lock);

	mem_tmp = domain->mem;
	domain->mem = domain->mem_hw;
	domain->mem_hw = mem_tmp;

	nvgpu_spinlock_release(&domain->lock);
}

int nvgpu_runlist_wait_pending_legacy(struct gk20a *g, struct nvgpu_runlist *rl)
{
	struct nvgpu_timeout timeout;
	u32 delay = POLL_DELAY_MIN_US;
	int ret;

	nvgpu_timeout_init_cpu_timer(g, &timeout, nvgpu_get_poll_timeout(g));

	ret = -ETIMEDOUT;
	do {
		if (g->ops.runlist.check_pending(g, rl) == 0) {
			ret = 0;
			break;
		}

		/* Avoid sleeping */
		nvgpu_usleep_range(delay, delay * 2U);
		delay = min_t(u32, delay << 1, POLL_DELAY_MAX_US);
	} while (nvgpu_timeout_expired(&timeout) == 0);

	return ret;
}

static int nvgpu_runlist_update_mem_locked(struct gk20a *g, struct nvgpu_runlist *rl,
				struct nvgpu_runlist_domain *domain,
				struct nvgpu_channel *ch, bool add, bool can_update_tsg_state)
{
	int ret = 0;
	bool add_entries;

	rl_dbg(g, "updating runlist[%u], domain[%llu], channel = [%u], op = %s",
		rl->id, domain->domain_id,
		ch == NULL ? NVGPU_INVALID_CHANNEL_ID : ch->chid,
		add ? "add" : "remove");

	if (ch != NULL) {
		bool update = nvgpu_runlist_modify_active_locked(g, domain, ch, add,
			can_update_tsg_state);

		if (!update) {
			/* no change in runlist contents */
			return 0;
		}
		/* had a channel to update, so reconstruct */
		add_entries = true;
	} else {
		/* no channel; add means update all, !add means clear all */
		add_entries = add;
	}

	ret = nvgpu_runlist_reconstruct_locked(g, rl, domain, add_entries);
	if (ret != 0) {
		return ret;
	}

	return ret;
}

int nvgpu_runlist_update_locked(struct gk20a *g, struct nvgpu_runlist *rl,
				struct nvgpu_runlist_domain *domain,
				struct nvgpu_channel *ch, bool add,
				bool wait_for_finish)
{
	bool can_update_tsg_state = false;
	int ret = 0;
	(void)wait_for_finish;
	/*
	 * Certain use-cases might not have existing user rl domains, fall
	 * back to shadow domain.
	 */
	if (domain == NULL) {
		domain = rl->shadow_rl_domain;
	}

	if (domain != rl->shadow_rl_domain) {
	/* Changes enclosed here in CONFIG_NVS_ROUND_ROBIN_SCHEDULER_DISABLE
	 * enabled for manual mode schedulers which is supported by embedded
	 * platforms currently. This change will not have any impact on l4t as
	 * l4t uses RR scheduling. Once l4t migrates to manual mode scheduling
	 * these flags can be removed which will eventually enabled the change
	 * for l4t as well.
	 */
#ifdef CONFIG_NVS_ROUND_ROBIN_SCHEDULER_DISABLE
		/* Avoid duplicate updates to the TSG state in
		 * nvgpu_runlist_modify_active_locked and suppose to be updated
		 * with shadow domain as shadow domain updates the tsg state
		 * first.
		 */
		can_update_tsg_state = true;
#endif
		ret = nvgpu_runlist_update_mem_locked(g, rl,
			rl->shadow_rl_domain, ch, add, can_update_tsg_state);
		if (ret != 0) {
			return ret;
		}
#ifdef CONFIG_NVS_ROUND_ROBIN_SCHEDULER_DISABLE
		nvgpu_runlist_swap_mem(g, rl->shadow_rl_domain);
#if defined(CONFIG_NVGPU_GSP_SCHEDULER) && defined(CONFIG_NVS_PRESENT)
	if (nvgpu_is_enabled(g, (u32)(NVGPU_SUPPORT_GSP_SCHED))) {
		if ((nvgpu_gsp_is_ready(g) == true)
			&& (nvgpu_nvs_gsp_usr_domain_present(g) == true)) {
			ret = nvgpu_gps_sched_update_runlist(g, rl->shadow_rl_domain, rl);
		}
	}
#endif
#endif
	}

	ret = nvgpu_runlist_update_mem_locked(g, rl, domain, ch, add,
		!can_update_tsg_state);
	if (ret != 0) {
		return ret;
	}

	nvgpu_runlist_swap_mem(g, domain);
	/* runlist update */
#if defined(CONFIG_NVGPU_GSP_SCHEDULER) && defined(CONFIG_NVS_PRESENT)
	if (nvgpu_is_enabled(g, (u32)(NVGPU_SUPPORT_GSP_SCHED))) {
		if ((nvgpu_gsp_is_ready(g) == true)
			&& (nvgpu_nvs_gsp_usr_domain_present(g) == true)) {
			ret = nvgpu_gps_sched_update_runlist(g, domain, rl);
		}
	}
#endif

	return ret;
}

#ifdef CONFIG_NVGPU_CHANNEL_TSG_SCHEDULING
/* trigger host to expire current timeslice and reschedule runlist from front */
int nvgpu_runlist_reschedule(struct nvgpu_channel *ch, bool preempt_next,
		bool wait_preempt)
{
	struct gk20a *g = ch->g;
	struct nvgpu_runlist *runlist;
#ifdef CONFIG_NVGPU_LS_PMU
	u32 token = PMU_INVALID_MUTEX_OWNER_ID;
	int mutex_ret = 0;
#endif
	int ret = 0;

	(void)wait_preempt;
	(void)preempt_next;

	runlist = ch->runlist;
	if (nvgpu_mutex_tryacquire(&runlist->runlist_lock) == 0) {
		return -EBUSY;
	}

#ifdef CONFIG_NVGPU_LS_PMU
	mutex_ret = nvgpu_pmu_lock_acquire(
		g, g->pmu, PMU_MUTEX_ID_FIFO, &token);
#endif

#if defined(CONFIG_NVS_PRESENT)
	/*
	 * This path(CONFIG_KMD_SCHEDULING_WORKER_THREAD) contains the CPU based
	 * Manual mode scheduler. With GSP enabled, this will be no longer required
	 * and can be disabled.
	 */
#if defined(CONFIG_KMD_SCHEDULING_WORKER_THREAD)
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_KMD_SCHEDULING_WORKER_THREAD)) {
		ret = g->nvs_worker_submit(g, runlist, runlist->domain, wait_preempt);
		if (ret == 1) {
			ret = 0;
		} else if (ret != 0) {
			goto done;
		}
	} else {
		ret = nvgpu_rl_domain_sync_submit(g, runlist, runlist->domain, wait_preempt);
	}
#endif /* CONFIG_KMD_SCHEDULING_WORKER_THREAD */
	/*
	 * The else path is for some platforms that doesn't itself support
	 * NVS. They take the traditional submit path used before NVS.
	 */
#else
	ret = nvgpu_rl_domain_sync_submit(g, runlist, runlist->domain, wait_preempt);
	if (ret != 0) {
		nvgpu_err(g, "failed to submit runlist domain [%llu]", runlist->domain->domain_id);
		goto done;
	}
#endif

	/* Acquiring runlist lock above guarantees that the current
	 * domain won't be switched.
	 */
	if (preempt_next) {
		if (g->ops.runlist.reschedule_preempt_next_locked(ch,
				wait_preempt) != 0) {
			nvgpu_err(g, "reschedule preempt next failed");
		}
	}

	if (nvgpu_runlist_wait_pending_legacy(g, runlist) != 0) {
		nvgpu_err(g, "wait pending failed for runlist %u",
				runlist->id);
	}

done:
#ifdef CONFIG_NVGPU_LS_PMU
	if (mutex_ret == 0) {
		if (nvgpu_pmu_lock_release(g, g->pmu,
				PMU_MUTEX_ID_FIFO, &token) != 0) {
			nvgpu_err(g, "failed to release PMU lock");
		}
	}
#endif
	nvgpu_mutex_release(&runlist->runlist_lock);

	return ret;
}
#endif

/* add/remove a channel from runlist
   special cases below: runlist->active_channels will NOT be changed.
   (ch == NULL && !add) means remove all active channels from runlist.
   (ch == NULL &&  add) means restore all active channels on runlist. */
static int nvgpu_runlist_do_update(struct gk20a *g, struct nvgpu_runlist *rl,
				   struct nvgpu_runlist_domain *domain,
				   struct nvgpu_channel *ch,
				   bool add, bool wait_for_finish)
{
	int ret = 0;

#ifdef CONFIG_NVGPU_LS_PMU
	u32 token = PMU_INVALID_MUTEX_OWNER_ID;
	int mutex_ret = 0;
#endif

	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&rl->runlist_lock);
#ifdef CONFIG_NVGPU_LS_PMU
	mutex_ret = nvgpu_pmu_lock_acquire(g, g->pmu,
		PMU_MUTEX_ID_FIFO, &token);
#endif
	ret = nvgpu_runlist_update_locked(g, rl, domain, ch, add, wait_for_finish);
	if (ret == 0) {
#if defined(CONFIG_NVS_PRESENT)
		ret = nvgpu_rl_domain_sync_submit(g, rl, rl->domain, wait_for_finish);
		/*
		* This path(CONFIG_KMD_SCHEDULING_WORKER_THREAD) contains the CPU based
		* Manual mode scheduler. With GSP enabled, this will be no longer required
		* and can be disabled.
		*/
#if defined(CONFIG_KMD_SCHEDULING_WORKER_THREAD)
		if (nvgpu_is_enabled(g, NVGPU_SUPPORT_KMD_SCHEDULING_WORKER_THREAD)) {
			if ((domain != NULL) && (domain->domain_id != SHADOW_DOMAIN_ID)) {
				domain->remove = !add;
				rl_dbg(g, "domain-id %llu is_remove %d",
						domain->domain_id, domain->remove);
			}

			ret = g->nvs_worker_submit(g, rl, domain, wait_for_finish);
			/* Deferred Update */
			if (ret == 1) {
				ret = 0;
			}
		} else {
			ret = nvgpu_rl_domain_sync_submit(g, rl, domain, wait_for_finish);
		}
#endif /* CONFIG_KMD_SCHEDULING_WORKER_THREAD */
		/*
		* The else path is for some platforms that doesn't itself support
		* NVS. They take the traditional submit path used before NVS.
		*/
#else
		ret = nvgpu_rl_domain_sync_submit(g, rl, domain, wait_for_finish);
#endif
	}

#ifdef CONFIG_NVGPU_LS_PMU
	if (mutex_ret == 0) {
		if (nvgpu_pmu_lock_release(g, g->pmu,
				PMU_MUTEX_ID_FIFO, &token) != 0) {
			nvgpu_err(g, "failed to release PMU lock");
		}
	}
#endif
	nvgpu_mutex_release(&rl->runlist_lock);

	if (ret == -ETIMEDOUT) {
		nvgpu_rc_runlist_update(g, rl->id);
	}

	return ret;
}

static void nvgpu_runlist_domain_actual_submit(struct gk20a *g, struct nvgpu_runlist *rl)
{
	rl_dbg(g, "Runlist[%u]: submitting domain[%llu]",
		rl->id, rl->domain->domain_id);

	/* Here, its essential to synchronize between hw_submit
	 * and domain mem swaps.
	 */
	nvgpu_spinlock_acquire(&rl->domain->lock);
	nvgpu_runlist_hw_submit(g, rl);
	nvgpu_spinlock_release(&rl->domain->lock);
}

/*
 * This is expected to be called only when device is powered on.
 */
static void runlist_submit_powered(struct gk20a *g, struct nvgpu_runlist *runlist,
		struct nvgpu_runlist_domain *next_domain)
{
	runlist->domain = next_domain;

	rl_dbg(g, "Runlist[%u]: switching to domain %llu",
		runlist->id, next_domain->domain_id);

	nvgpu_runlist_domain_actual_submit(g, runlist);
}

int nvgpu_rl_domain_sync_submit(struct gk20a *g, struct nvgpu_runlist *runlist,
		struct nvgpu_runlist_domain *next_domain, bool wait_for_finish)
{
	int err = 0;
#ifdef CONFIG_NVS_PRESENT
	if (nvgpu_nvs_gsp_usr_domain_present(g) == false) {
#endif
		/* schedule runlists for only for shadow domains */
		if (next_domain == NULL) {
			next_domain = runlist->shadow_rl_domain;
		}

		runlist_submit_powered(g, runlist, next_domain);
		if (wait_for_finish) {
			err = nvgpu_runlist_wait_pending_legacy(g, runlist);
			if (err != 0) {
				nvgpu_err(g, "runlist %d update timeout", runlist->id);
				/* trigger runlist update timeout recovery */
				return err;
			}
		}
#ifdef CONFIG_NVS_PRESENT
	}
#endif

	return err;
}

#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
int nvgpu_runlist_tick(struct gk20a *g, struct nvgpu_runlist_domain **rl_domain,
	u64 preempt_grace_ns)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 i;
	int ret = 0;
	int err = -ETIMEDOUT;
	u64 start_time = nvgpu_safe_cast_s64_to_u64(nvgpu_current_time_ns());
	u64 current_time = start_time;

	rl_dbg(g, "domain tick");

	for (i = 0U; i < f->num_runlists; i++) {
		struct nvgpu_runlist *runlist;

		runlist = &f->active_runlists[i];
		runlist_submit_powered(g, runlist, rl_domain[i]);

		do {
			ret = g->ops.runlist.check_pending(g, runlist);
			if (ret == 0) {
				err = 0;
				break;
			}
			current_time = nvgpu_safe_cast_s64_to_u64(nvgpu_current_time_ns());
		} while ((preempt_grace_ns == 0ULL)
			|| (nvgpu_safe_sub_u64(current_time, start_time) <= preempt_grace_ns));
	}

	rl_dbg(g, "%llu - %llu", nvgpu_safe_sub_u64(current_time, start_time), preempt_grace_ns);

	return err;
}
#endif

int nvgpu_runlist_update(struct gk20a *g, struct nvgpu_runlist *rl,
			 struct nvgpu_channel *ch,
			 bool add, bool wait_for_finish)
{
	struct nvgpu_tsg *tsg = NULL;

	nvgpu_assert(ch != NULL);

	tsg = nvgpu_tsg_from_ch(ch);
	if (tsg == NULL) {
		return -EINVAL;
	}

	return nvgpu_runlist_do_update(g, rl, tsg->rl_domain, ch, add, wait_for_finish);
}

int nvgpu_runlist_reload(struct gk20a *g, struct nvgpu_runlist *rl,
			 struct nvgpu_runlist_domain *domain,
			 bool add, bool wait_for_finish)
{
	return nvgpu_runlist_do_update(g, rl, domain, NULL, add, wait_for_finish);
}

int nvgpu_runlist_reload_ids(struct gk20a *g, u32 runlist_ids, bool add)
{
	struct nvgpu_fifo *f = &g->fifo;
	int ret = -EINVAL;
	unsigned long runlist_id = 0;
	int errcode;
	unsigned long ulong_runlist_ids = (unsigned long)runlist_ids;

	if (g == NULL) {
		goto end;
	}

	ret = 0;
	for_each_set_bit(runlist_id, &ulong_runlist_ids, 32U) {
		/* Capture the last failure error code */
		errcode = g->ops.runlist.reload(g,
						f->runlists[runlist_id],
						f->runlists[runlist_id]->domain,
						add, true);
		if (errcode != 0) {
			nvgpu_err(g,
				"failed to update_runlist %lu %d",
				runlist_id, errcode);
			ret = errcode;
		}
	}
end:
	return ret;
}

const char *nvgpu_runlist_interleave_level_name(u32 interleave_level)
{
	const char *ret_string = NULL;

	switch (interleave_level) {
	case NVGPU_FIFO_RUNLIST_INTERLEAVE_LEVEL_LOW:
		ret_string = "LOW";
		break;

	case NVGPU_FIFO_RUNLIST_INTERLEAVE_LEVEL_MEDIUM:
		ret_string = "MEDIUM";
		break;

	case NVGPU_FIFO_RUNLIST_INTERLEAVE_LEVEL_HIGH:
		ret_string = "HIGH";
		break;

	default:
		ret_string = "?";
		break;
	}

	return ret_string;
}

void nvgpu_runlist_set_state(struct gk20a *g, u32 runlists_mask,
		u32 runlist_state)
{
#ifdef CONFIG_NVGPU_LS_PMU
	u32 token = PMU_INVALID_MUTEX_OWNER_ID;
	int mutex_ret = 0;
#endif
	nvgpu_log(g, gpu_dbg_info, "runlist mask = 0x%08x state = 0x%08x",
			runlists_mask, runlist_state);

#ifdef CONFIG_NVGPU_LS_PMU
	mutex_ret = nvgpu_pmu_lock_acquire(g, g->pmu,
		PMU_MUTEX_ID_FIFO, &token);
#endif
	g->ops.runlist.write_state(g, runlists_mask, runlist_state);
#ifdef CONFIG_NVGPU_LS_PMU
	if (mutex_ret == 0) {
		if (nvgpu_pmu_lock_release(g, g->pmu,
				PMU_MUTEX_ID_FIFO, &token) != 0) {
			nvgpu_err(g, "failed to release PMU lock");
		}
	}
#endif
}

static void free_rl_mem(struct gk20a *g, struct nvgpu_runlist_mem *mem)
{
	nvgpu_dma_free(g, &mem->mem);
	nvgpu_kfree(g, mem);
}

static void nvgpu_runlist_domain_unlink_locked(struct nvgpu_runlist_domain *domain)
{
	/* added in nvgpu_runlist_domain_alloc() */
	nvgpu_list_del(&domain->domains_list);
}

void nvgpu_runlist_domain_free(struct gk20a *g,
		struct nvgpu_runlist_domain *domain)
{
	free_rl_mem(g, domain->mem);
	domain->mem = NULL;
	free_rl_mem(g, domain->mem_hw);
	domain->mem_hw = NULL;
	nvgpu_kfree(g, domain->active_channels);
	domain->active_channels = NULL;
	nvgpu_kfree(g, domain->active_tsgs);
	domain->active_tsgs = NULL;

	nvgpu_kfree(g, domain);
}

void nvgpu_runlist_unlink_domain(struct nvgpu_runlist *runlist,
		struct nvgpu_runlist_domain *domain)
{
	nvgpu_mutex_acquire(&runlist->runlist_lock);
	nvgpu_runlist_domain_unlink_locked(domain);
	nvgpu_mutex_release(&runlist->runlist_lock);
}

void nvgpu_runlist_cleanup_sw(struct gk20a *g)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 i;
	struct nvgpu_runlist *runlist;

	if ((f->runlists == NULL) || (f->active_runlists == NULL)) {
		return;
	}

	g = f->g;

	for (i = 0; i < f->num_runlists; i++) {
		runlist = &f->active_runlists[i];

		while (!nvgpu_list_empty(&runlist->user_rl_domains)) {
			struct nvgpu_runlist_domain *domain;

			domain = nvgpu_list_first_entry(&runlist->user_rl_domains,
						     nvgpu_runlist_domain,
						     domains_list);

			nvgpu_runlist_domain_unlink_locked(domain);
		}
		/* this isn't an owning pointer, just reset */
		runlist->domain = NULL;

		nvgpu_runlist_domain_free(g, runlist->shadow_rl_domain);
		runlist->shadow_rl_domain = NULL;

		nvgpu_mutex_destroy(&runlist->runlist_lock);
		f->runlists[runlist->id] = NULL;
	}

	nvgpu_kfree(g, f->active_runlists);
	f->active_runlists = NULL;
	f->num_runlists = 0;
	nvgpu_kfree(g, f->runlists);
	f->runlists = NULL;
	f->max_runlists = 0;
}

static void nvgpu_runlist_init_engine_info(struct gk20a *g,
		struct nvgpu_runlist *runlist,
		const struct nvgpu_device *dev)
{
	u32 i = 0U;

	/*
	 * Bail out on pre-ga10b platforms.
	 */
	if (g->ops.runlist.get_engine_id_from_rleng_id == NULL) {
		return;
	}

	/*
	 * runlist_pri_base, chram_bar0_offset and pbdma_info
	 * will get over-written with same info, if multiple engines
	 * are present on same runlist. Required optimization will be
	 * done as part of JIRA NVGPU-4980
	 */
	runlist->runlist_pri_base =
			dev->rl_pri_base;
	runlist->chram_bar0_offset =
		g->ops.runlist.get_chram_bar0_offset(g, dev->rl_pri_base);

	nvgpu_log(g, gpu_dbg_info, "runlist[%d]: runlist_pri_base 0x%x",
		runlist->id, runlist->runlist_pri_base);
	nvgpu_log(g, gpu_dbg_info, "runlist[%d]: chram_bar0_offset 0x%x",
		runlist->id, runlist->chram_bar0_offset);

	runlist->pbdma_info = &dev->pbdma_info;
	for  (i = 0U; i < PBDMA_PER_RUNLIST_SIZE; i++) {
		nvgpu_log(g, gpu_dbg_info,
			"runlist[%d]: pbdma_id[%d] %d pbdma_pri_base[%d] 0x%x",
			runlist->id, i,
			runlist->pbdma_info->pbdma_id[i], i,
			runlist->pbdma_info->pbdma_pri_base[i]);
	}

	runlist->rl_dev_list[dev->rleng_id] = dev;
}

static u32 nvgpu_runlist_get_pbdma_mask(struct gk20a *g,
					struct nvgpu_runlist *runlist)
{
	u32 pbdma_mask = 0U;
	u32 i;
	u32 pbdma_id;

	(void)g;

	nvgpu_assert(runlist != NULL);

	for ( i = 0U; i < PBDMA_PER_RUNLIST_SIZE; i++) {
		pbdma_id = runlist->pbdma_info->pbdma_id[i];
		if (pbdma_id != NVGPU_INVALID_PBDMA_ID)
			pbdma_mask |= BIT32(pbdma_id);
	}
	return pbdma_mask;
}

void nvgpu_runlist_init_enginfo(struct gk20a *g, struct nvgpu_fifo *f)
{
	struct nvgpu_runlist *runlist;
	const struct nvgpu_device *dev;
	u32 i, j;

	nvgpu_log_fn(g, " ");

	if (nvgpu_is_legacy_vgpu(g)) {
		return;
	}

	for (i = 0; i < f->num_runlists; i++) {
		runlist = &f->active_runlists[i];

		nvgpu_log(g, gpu_dbg_info, "Configuring runlist %u (%u)", runlist->id, i);

		for (j = 0; j < f->num_engines; j++) {
			dev = f->active_engines[j];

			if (dev->runlist_id == runlist->id) {
				runlist->eng_bitmask |= BIT32(dev->engine_id);
				/*
				 * Populate additional runlist fields on
				 * Ampere+ chips.
				 */
				nvgpu_runlist_init_engine_info(g, runlist, dev);
			}
		}

		/*
		 * The PBDMA mask per runlist is probed differently on
		 * PreAmpere vs Ampere+ chips.
		 *
		 * Use legacy probing if g->ops.fifo.find_pbdma_for_runlist is
		 * assigned, else switch to new probe function
		 * nvgpu_runlist_get_pbdma_mask.
		 */
		if (g->ops.fifo.find_pbdma_for_runlist != NULL) {
			(void) g->ops.fifo.find_pbdma_for_runlist(g,
						runlist->id,
						&runlist->pbdma_bitmask);
		}
		else {
			if (!nvgpu_is_vf(g)) {
				runlist->pbdma_bitmask =
					nvgpu_runlist_get_pbdma_mask(g, runlist);
			}
		}
		nvgpu_log(g, gpu_dbg_info, "  Active engine bitmask: 0x%x", runlist->eng_bitmask);
		nvgpu_log(g, gpu_dbg_info, "          PBDMA bitmask: 0x%x", runlist->pbdma_bitmask);
	}

	nvgpu_log_fn(g, "done");
}

static struct nvgpu_runlist_mem *init_rl_mem(struct gk20a *g, u32 runlist_size)
{
	struct nvgpu_runlist_mem *mem = nvgpu_kzalloc(g, sizeof(*mem));
	int err;

	if (mem == NULL) {
		return NULL;
	}

	err = nvgpu_dma_alloc_flags_sys(g,
			nvgpu_is_legacy_vgpu(g) ?
			  0ULL : NVGPU_DMA_PHYSICALLY_ADDRESSED,
			runlist_size,
			&mem->mem);
	if (err != 0) {
		nvgpu_kfree(g, mem);
		mem = NULL;
	}

	return mem;
}

void nvgpu_runlist_link_domain(struct nvgpu_runlist *runlist,
		struct nvgpu_runlist_domain *domain)
{
	nvgpu_mutex_acquire(&runlist->runlist_lock);
	/* deleted in nvgpu_runlist_domain_unlink_locked() */
	nvgpu_list_add_tail(&domain->domains_list, &runlist->user_rl_domains);
	nvgpu_mutex_release(&runlist->runlist_lock);
}

struct nvgpu_runlist_domain *nvgpu_runlist_domain_alloc(struct gk20a *g,
		u64 domain_id)
{
	struct nvgpu_runlist_domain *domain = nvgpu_kzalloc(g, sizeof(*domain));
	struct nvgpu_fifo *f = &g->fifo;
	size_t runlist_size = (size_t)f->runlist_entry_size *
				(size_t)f->num_runlist_entries;

	if (domain == NULL) {
		return NULL;
	}

	domain->domain_id = domain_id;

	domain->mem = init_rl_mem(g, (u32)runlist_size);
	if (domain->mem == NULL) {
		goto free_domain;
	}

	domain->mem_hw = init_rl_mem(g, (u32)runlist_size);
	if (domain->mem_hw == NULL) {
		goto free_mem;
	}

	domain->active_channels =
		nvgpu_kzalloc(g, DIV_ROUND_UP(f->num_channels,
					      BITS_PER_BYTE));
	if (domain->active_channels == NULL) {
		goto free_mem_hw;
	}

	domain->active_tsgs =
		nvgpu_kzalloc(g, DIV_ROUND_UP(f->num_channels,
					      BITS_PER_BYTE));
	if (domain->active_tsgs == NULL) {
		goto free_active_channels;
	}

	nvgpu_spinlock_init(&domain->lock);

	return domain;
free_active_channels:
	nvgpu_kfree(g, domain->active_channels);
free_mem_hw:
	free_rl_mem(g, domain->mem_hw);
free_mem:
	free_rl_mem(g, domain->mem);
free_domain:
	nvgpu_kfree(g, domain);
	return NULL;
}

struct nvgpu_runlist_domain *nvgpu_rl_domain_get(struct gk20a *g, u32 runlist_id,
						 u64 domain_id)
{
	struct nvgpu_fifo *f = &g->fifo;
	struct nvgpu_runlist *runlist = f->runlists[runlist_id];
	struct nvgpu_runlist_domain *domain;

	nvgpu_list_for_each_entry(domain, &runlist->user_rl_domains, nvgpu_runlist_domain,
				  domains_list) {
		if (domain->domain_id == domain_id) {
			return domain;
		}
	}

	return NULL;
}

static void nvgpu_init_active_runlist_mapping(struct gk20a *g)
{
	struct nvgpu_fifo *f = &g->fifo;
	unsigned int runlist_id;
	size_t runlist_size;
	u32 i;

	rl_dbg(g, "Building active runlist map.");

	/*
	 * In most case we want to loop through active runlists only. Here
	 * we need to loop through all possible runlists, to build the mapping
	 * between runlists[runlist_id] and active_runlists[i].
	 */
	i = 0U;
	for (runlist_id = 0; runlist_id < f->max_runlists; runlist_id++) {
		struct nvgpu_runlist *runlist;

		if (!nvgpu_engine_is_valid_runlist_id(g, runlist_id)) {
			/* skip inactive runlist */
			rl_dbg(g, "  Skipping invalid runlist: %d", runlist_id);
			continue;
		}

		rl_dbg(g, "  Configuring HW runlist: %u", runlist_id);
		rl_dbg(g, "  SW runlist index to HW: %u -> %u", i, runlist_id);

		runlist = &f->active_runlists[i];
		runlist->id = runlist_id;
		f->runlists[runlist_id] = runlist;
		i = nvgpu_safe_add_u32(i, 1U);

		runlist_size = (size_t)f->runlist_entry_size *
				(size_t)f->num_runlist_entries;
		rl_dbg(g, "    RL entries: %d", f->num_runlist_entries);
		rl_dbg(g, "    RL size %zu", runlist_size);

		nvgpu_init_list_node(&runlist->user_rl_domains);
		nvgpu_mutex_init(&runlist->runlist_lock);
	}
}

static int nvgpu_runlist_alloc_shadow_rl_domain(struct gk20a *g)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 i;

	for (i = 0; i < g->fifo.num_runlists; i++) {
		struct nvgpu_runlist *runlist = &f->active_runlists[i];

		runlist->shadow_rl_domain = nvgpu_runlist_domain_alloc(g, SHADOW_DOMAIN_ID);
		if (runlist->shadow_rl_domain == NULL) {
			nvgpu_err(g, "memory allocation failed");
			/*
			 * deletion of prior domains happens in
			 * nvgpu_runlist_cleanup_sw() via the caller.
			 */
			return -ENOMEM;
		}

		rl_dbg(g, "Allocated default domain for runlist[%u]: %llu", runlist->id,
			runlist->shadow_rl_domain->domain_id);

		runlist->domain = runlist->shadow_rl_domain;
	}
	return 0;
}

int nvgpu_runlist_setup_sw(struct gk20a *g)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 num_runlists = 0U;
	unsigned int runlist_id;
	int err = 0;

	rl_dbg(g, "Initializing Runlists");

	nvgpu_spinlock_init(&f->runlist_submit_lock);

	f->runlist_entry_size = g->ops.runlist.entry_size(g);
	f->num_runlist_entries = g->ops.runlist.length_max(g);
	f->max_runlists = g->ops.runlist.count_max(g);

	f->runlists = nvgpu_kzalloc(g, nvgpu_safe_mult_u64(
				sizeof(*f->runlists), f->max_runlists));
	if (f->runlists == NULL) {
		err = -ENOMEM;
		goto clean_up_runlist;
	}

	for (runlist_id = 0; runlist_id < f->max_runlists; runlist_id++) {
		if (nvgpu_engine_is_valid_runlist_id(g, runlist_id)) {
			num_runlists = nvgpu_safe_add_u32(num_runlists, 1U);
		}
	}
	f->num_runlists = num_runlists;

	f->active_runlists = nvgpu_kzalloc(g, nvgpu_safe_mult_u64(
			 sizeof(*f->active_runlists), num_runlists));
	if (f->active_runlists == NULL) {
		err = -ENOMEM;
		goto clean_up_runlist;
	}


	rl_dbg(g, "  Max runlists:    %u", f->max_runlists);
	rl_dbg(g, "  Active runlists: %u", f->num_runlists);
	rl_dbg(g, "  RL entry size:   %u bytes", f->runlist_entry_size);
	rl_dbg(g, "  Max RL entries:  %u", f->num_runlist_entries);

	nvgpu_init_active_runlist_mapping(g);

	err = nvgpu_runlist_alloc_shadow_rl_domain(g);
	if (err != 0) {
		goto clean_up_runlist;
	}

	g->ops.runlist.init_enginfo(g, f);
	return 0;

clean_up_runlist:
	nvgpu_runlist_cleanup_sw(g);
	rl_dbg(g, "fail");
	return err;
}

u32 nvgpu_runlist_get_runlists_mask(struct gk20a *g, u32 id,
	unsigned int id_type, u32 act_eng_bitmask, u32 pbdma_bitmask)
{
	u32 i, runlists_mask = 0;
	struct nvgpu_fifo *f = &g->fifo;
	struct nvgpu_runlist *runlist;

	bool bitmask_disabled = ((act_eng_bitmask == 0U) &&
				(pbdma_bitmask == 0U));

	/* engine and/or pbdma ids are known */
	if (!bitmask_disabled) {
		for (i = 0U; i < f->num_runlists; i++) {
			runlist = &f->active_runlists[i];

			if ((runlist->eng_bitmask & act_eng_bitmask) != 0U) {
				runlists_mask |= BIT32(runlist->id);
			}

			if ((runlist->pbdma_bitmask & pbdma_bitmask) != 0U) {
				runlists_mask |= BIT32(runlist->id);
			}
		}
	}

	if (id_type != ID_TYPE_UNKNOWN) {
		if (id_type == ID_TYPE_TSG) {
			runlist = f->tsg[id].runlist;
		} else {
			runlist = f->channel[id].runlist;
		}

		if (runlist == NULL) {
			/* Warning on Linux, real assert on QNX. */
			nvgpu_assert(runlist != NULL);
		} else {
			runlists_mask |= BIT32(runlist->id);
		}
	} else {
		if (bitmask_disabled) {
			nvgpu_log(g, gpu_dbg_info, "id_type_unknown, engine "
				"and pbdma ids are unknown");

			for (i = 0U; i < f->num_runlists; i++) {
				runlist = &f->active_runlists[i];

				runlists_mask |= BIT32(runlist->id);
			}
		} else {
			nvgpu_log(g, gpu_dbg_info, "id_type_unknown, engine "
				"and/or pbdma ids are known");
		}
	}

	nvgpu_log(g, gpu_dbg_info, "runlists_mask = 0x%08x", runlists_mask);
	return runlists_mask;
}

void nvgpu_runlist_unlock_runlists(struct gk20a *g, u32 runlists_mask)
{
	struct nvgpu_fifo *f = &g->fifo;
	struct nvgpu_runlist *runlist;
	u32 i;

	nvgpu_log_info(g, "release runlist_lock for runlists set in "
				"runlists_mask: 0x%08x", runlists_mask);

	for (i = 0U; i < f->num_runlists; i++) {
		runlist = &f->active_runlists[i];

		if ((BIT32(i) & runlists_mask) != 0U) {
			nvgpu_mutex_release(&runlist->runlist_lock);
		}
	}
}

s32 nvgpu_runlist_get_runlist_info(struct gk20a *g, u32 rl_index, u32 *runlist_id,
		u8 *device_id)
{
	struct nvgpu_fifo *f = &g->fifo;
	s32 err = 0;
	u32 device_id_u32 = 0;
	struct nvgpu_runlist *runlist = &f->active_runlists[rl_index];

	err = (s32)(nvgpu_runlist_get_device_id(g, runlist, &device_id_u32));
	if (err != 0) {
		nvgpu_err(g, "error in getting device ID");
		goto exit;
	}

	*device_id = nvgpu_safe_cast_u32_to_u8(device_id_u32);
	*runlist_id = runlist->id;
exit:
	return err;
}

s32 nvgpu_runlist_get_device_id(struct gk20a *g, struct nvgpu_runlist *rl, u32 *device_id)
{
	s32 err = 0;

	if (rl->rl_dev_list[0] != NULL) {
		*device_id = rl->rl_dev_list[0]->engine_id;
	} else {
		err = (s32)(-EINVAL);
		nvgpu_err(g, "Get device ID failed:");
	}

	return err;
}

u32 nvgpu_runlist_get_num_runlists(struct gk20a *g)
{
	struct nvgpu_fifo f = g->fifo;
	return f.num_runlists;
}

struct nvgpu_runlist_domain *nvgpu_runlist_get_shadow_domain(struct gk20a *g)
{
	return g->fifo.active_runlists[0].shadow_rl_domain;
}

void nvgpu_runlist_hw_submit(struct gk20a *g, struct nvgpu_runlist *runlist)
{
	u64 runlist_iova = nvgpu_mem_get_addr(g, &runlist->domain->mem_hw->mem);
	enum nvgpu_aperture aperture = runlist->domain->mem_hw->mem.aperture;
	u32 count = runlist->domain->mem_hw->count;

	g->ops.runlist.hw_submit(g, runlist->id, runlist_iova, aperture, count);
}
