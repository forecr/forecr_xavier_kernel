/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#include <nvgpu/bug.h>
#include <nvgpu/debug.h>
#include <nvgpu/kmem.h>
#include <nvgpu/log.h>
#include <nvgpu/os_sched.h>
#include <nvgpu/channel.h>
#include <nvgpu/tsg.h>
#include <nvgpu/atomic.h>
#include <nvgpu/tsg_subctx.h>
#include <nvgpu/rc.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/error_notifier.h>
#include <nvgpu/gr/config.h>
#include <nvgpu/gr/ctx.h>
#include <nvgpu/runlist.h>
#include <nvgpu/nvs.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/kmem.h>
#ifdef CONFIG_NVGPU_PROFILER
#include <nvgpu/profiler.h>
#endif
#include <nvgpu/multimedia.h>
#ifdef CONFIG_NVGPU_CHECKER
#include <nvgpu/checker_common_data_utils.h>
#endif

#include "common/gr/gr_intr_priv.h"

#define MATCH_VEID		true
#define MATCH_PBDMA_ID		true
#define DONT_MATCH_VEID		false
#define DONT_MATCH_PBDMA_ID	false

/**
 * Refer section "SCG, PBDMA, and CILP Rules" from https:p4viewer.nvidia.com/
 * /get/hw/doc/gpu/volta/volta/design/Functional_Descriptions/
 * /Volta_Subcontexts_Functional_Description.docx
 */
int nvgpu_tsg_validate_class_veid_pbdma(struct nvgpu_channel *ch)
{
	struct gk20a *g = ch->g;
	struct nvgpu_tsg *tsg = nvgpu_tsg_from_ch(ch);
	bool veidnz_pbdma0_compute_active = false;
	bool veid0_pbdmanz_compute_active = false;
	bool veid0_gfx_active = false;

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return 0;
	}

	if (tsg == NULL) {
		nvgpu_err(g, "tsgid is invalid");
		return -EINVAL;
	}

	if (g->ops.gpu_class.is_valid_gfx(ch->obj_class)) {
		if (ch->runqueue_sel != 0U) {
			nvgpu_err(g, "Can't have Graphics in non-zero'th PBDMA");
			return -EINVAL;
		}

		if (ch->subctx_id != CHANNEL_INFO_VEID0) {
			nvgpu_err(g, "Can't have Graphics in non-zero'th VEID");
			return -EINVAL;
		}
	}

	veid0_gfx_active = nvgpu_tsg_channel_type_active(tsg,
				MATCH_VEID, CHANNEL_INFO_VEID0,
				MATCH_PBDMA_ID, CHANNEL_INFO_PBDMA0,
				g->ops.gpu_class.is_valid_gfx);
	if (veid0_gfx_active) {
		if (g->ops.gpu_class.is_valid_compute(ch->obj_class)) {
			if (ch->subctx_id == CHANNEL_INFO_VEID0) {
				if (ch->runqueue_sel != 0U) {
					nvgpu_err(g, "VEID0 can't do Graphics and Async Compute");
					return -EINVAL;
				}
			} else {
				if (ch->runqueue_sel == 0U) {
					nvgpu_err(g, "Async Compute can't be mixed with Graphics on PBDMA0");
					return -EINVAL;
				}
			}
		}
	}

	veid0_pbdmanz_compute_active = nvgpu_tsg_channel_type_active(tsg,
				MATCH_VEID, CHANNEL_INFO_VEID0,
				DONT_MATCH_PBDMA_ID, CHANNEL_INFO_PBDMA0,
				g->ops.gpu_class.is_valid_compute);
	if (veid0_pbdmanz_compute_active) {
		if (g->ops.gpu_class.is_valid_gfx(ch->obj_class)) {
			nvgpu_err(g, "VEID0 can't do Graphics and Async Compute");
			return -EINVAL;
		}
	}

	veidnz_pbdma0_compute_active = nvgpu_tsg_channel_type_active(tsg,
				DONT_MATCH_VEID, CHANNEL_INFO_VEID0,
				MATCH_PBDMA_ID, CHANNEL_INFO_PBDMA0,
				g->ops.gpu_class.is_valid_compute);
	if (veidnz_pbdma0_compute_active) {
		if (g->ops.gpu_class.is_valid_gfx(ch->obj_class)) {
			nvgpu_err(g, "Async Compute can't be mixed with Graphics on PBDMA0");
			return -EINVAL;
		}
	}

	return 0;
}

int nvgpu_tsg_validate_cilp_config(struct nvgpu_channel *ch)
{
#ifdef CONFIG_NVGPU_CILP
	struct gk20a *g = ch->g;
	struct nvgpu_tsg *tsg = nvgpu_tsg_from_ch(ch);
	bool veidnz_compute_active;
	bool veid0_compute_active;
	bool veid0_gfx_active;
	bool cilp_enabled;

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return 0;
	}

	veid0_gfx_active = nvgpu_tsg_channel_type_active(tsg,
				MATCH_VEID, CHANNEL_INFO_VEID0,
				MATCH_PBDMA_ID, CHANNEL_INFO_PBDMA0,
				g->ops.gpu_class.is_valid_gfx);

	veidnz_compute_active = nvgpu_tsg_channel_type_active(tsg,
				DONT_MATCH_VEID, CHANNEL_INFO_VEID0,
				DONT_MATCH_PBDMA_ID, CHANNEL_INFO_PBDMA0,
				g->ops.gpu_class.is_valid_compute);

	if (veid0_gfx_active) {
		if (g->ops.gpu_class.is_valid_compute(ch->obj_class)) {
			if (ch->subctx_id != CHANNEL_INFO_VEID0) {
				nvgpu_assert(ch->runqueue_sel != CHANNEL_INFO_PBDMA0);
				nvgpu_err(g, "SCG, with or without Sync Compute - CILP not allowed");
				return -EINVAL;
			} else if (veidnz_compute_active) {
				nvgpu_err(g, "SCG, with or without Sync Compute - CILP not allowed");
				return -EINVAL;
			}
		}
	}

	cilp_enabled = (nvgpu_gr_ctx_get_compute_preemption_mode(tsg->gr_ctx) ==
			NVGPU_PREEMPTION_MODE_COMPUTE_CILP);

	if (!cilp_enabled) {
		nvgpu_log(g, gpu_dbg_gr, "CILP not enabled currently.");
		return 0;
	}

	if (veidnz_compute_active) {
		if (g->ops.gpu_class.is_valid_gfx(ch->obj_class)) {
			nvgpu_err(g, "SCG without Sync Compute - CILP not allowed");
			return -EINVAL;
		}
	}

	veid0_compute_active = nvgpu_tsg_channel_type_active(tsg,
				MATCH_VEID, CHANNEL_INFO_VEID0,
				MATCH_PBDMA_ID, CHANNEL_INFO_PBDMA0,
				g->ops.gpu_class.is_valid_compute);
	if (veid0_compute_active && veidnz_compute_active) {
		if (g->ops.gpu_class.is_valid_gfx(ch->obj_class)) {
			nvgpu_err(g, "SCG with Sync Compute - CILP not allowed");
			return -EINVAL;
		}
	}
#else
	(void)ch;
#endif

	return 0;
}

void nvgpu_tsg_disable(struct nvgpu_tsg *tsg)
{
	struct gk20a *g = tsg->g;

	nvgpu_log_fn(g, " ");

	nvgpu_rwsem_down_write(&tsg->ch_list_lock);
	tsg->disable_count = nvgpu_safe_add_u32(tsg->disable_count, 1U);
	if (tsg->disable_count == 1U) {
		nvgpu_log_info(tsg->g, "disabling tsg: %u, disable_count: %u",
			       tsg->tsgid, tsg->disable_count);
		g->ops.tsg.disable(tsg);
	} else {
		nvgpu_log_info(tsg->g, "tsg: %u already disabled, disable_count: %u",
			       tsg->tsgid, tsg->disable_count);
	}
	nvgpu_rwsem_up_write(&tsg->ch_list_lock);

	nvgpu_log_fn(g, "done");
}

void nvgpu_tsg_enable(struct nvgpu_tsg *tsg)
{
	struct gk20a *g = tsg->g;

	nvgpu_log_fn(g, " ");

	nvgpu_rwsem_down_write(&tsg->ch_list_lock);
	if (tsg->disable_count == 0U) {
		nvgpu_log_info(g, "tsg: %u already enabled, disable_count: %u",
			       tsg->tsgid, tsg->disable_count);
		goto tsg_already_enabled;
	}
	tsg->disable_count = nvgpu_safe_sub_u32(tsg->disable_count, 1U);
	if (tsg->disable_count == 0U) {
		nvgpu_log_info(g, "enabling tsg: %u, disable_count: %u",
			       tsg->tsgid, tsg->disable_count);
		g->ops.tsg.enable(tsg);
	} else {
		nvgpu_log_info(g, "skipped enabling tsg: %u, disable_count: %u",
			       tsg->tsgid, tsg->disable_count);
	}

tsg_already_enabled:
	nvgpu_rwsem_up_write(&tsg->ch_list_lock);

	nvgpu_log_fn(g, "done");
}

void nvgpu_tsg_enable_for_cilp_cmpl(struct nvgpu_tsg *tsg)
{
	struct gk20a *g = tsg->g;

	nvgpu_log_fn(g, " ");

	/*
	 * CILP preemption completion causes TSG to be disabled once.
	 *
	 * With TSG sharing, as many processes sharing TSG can get
	 * CILP preemption completion event and can call TSG_ENABLE/
	 * RESUME_ALL_CONTEXTS. Hence, once invoked, reset the flag.
	 */
	if (nvgpu_atomic_cmpxchg(&tsg->cilp_save_cmpl, 1, 0) != 0) {
		nvgpu_log_info(g, "enabling tsg: %u once for cilp save completion",
			       tsg->tsgid);
		nvgpu_tsg_enable(tsg);
	}

	nvgpu_log_fn(g, "done");
}

struct nvgpu_tsg *nvgpu_tsg_check_and_get_from_id(struct gk20a *g, u32 tsgid)
{
	struct nvgpu_fifo *f = &g->fifo;

	if (tsgid >= f->num_channels) {
		return NULL;
	}

	return nvgpu_tsg_get_from_id(g, tsgid);
}


struct nvgpu_tsg *nvgpu_tsg_get_from_id(struct gk20a *g, u32 tsgid)
{
	struct nvgpu_fifo *f = &g->fifo;

	return &f->tsg[tsgid];
}

/*
 * Synchronous subcontext. Subcontext of this type may hold the
 * graphics channel, and multiple copy engine and compute channels.
 */
#define NVGPU_TSG_SUBCONTEXT_TYPE_SYNC               (0x0U)

/*
 * Asynchronous subcontext. Asynchronous subcontext is for compute
 * and copy engine channels only.
 */
#define NVGPU_TSG_SUBCONTEXT_TYPE_ASYNC              (0x1U)

#define MAX_SYNC_SUBCONTEXTS	1U

/**
 * @brief Allocate a synchronous VEID for a TSG.
 *
 * This function attempts to allocate a synchronous VEID (Virtual Engine ID) for
 * the specified TSG. Synchronous VEIDs are used for channels that require
 * deterministic context switching, such as graphics channels. There can only be
 * one synchronous VEID per TSG, and it is always VEID 0. If the TSG already has
 * a synchronous VEID allocated, the function reports an error and returns.
 *
 * @param [in]  g      Pointer to the GPU driver struct.
 * @param [in]  tsg    Pointer to the TSG struct for which to allocate the VEID.
 * @param [out] veid   Pointer to the variable where the allocated VEID will be stored.
 *
 * Steps:
 * -# Check if the TSG already has a synchronous VEID allocated by examining the
 *    'sync_veid' flag within the TSG structure.
 * -# If a synchronous VEID is already allocated ('sync_veid' is true), log an
 *    error message using nvgpu_err() and return with error code -#ENOSPC.
 * -# If a synchronous VEID is not already allocated ('sync_veid' is false),
 *    set the 'sync_veid' flag to true to indicate that VEID 0 is now allocated
 *    to this TSG.
 * -# Store the value 0 (indicating synchronous VEID) in the location pointed to
 *    by the 'veid' output parameter.
 * -# Return 0 to indicate successful allocation of the synchronous VEID.
 *
 * @return 0 if the synchronous VEID was successfully allocated.
 * @return -#ENOSPC if the synchronous VEID is already allocated.
 */
static int nvgpu_tsg_create_sync_subcontext(struct gk20a *g,
				struct nvgpu_tsg *tsg, u32 *veid)
{
	if (tsg->sync_veid) {
		nvgpu_err(g, "SYNC VEID not available");
		return -ENOSPC;
	}

	tsg->sync_veid = true;

	*veid = 0U;

	return 0;
}

/**
 * @brief Create an asynchronous subcontext for a TSG.
 *
 * -# Calculate the maximum number of asynchronous subcontexts by subtracting
 *    the number of synchronous subcontexts from the total maximum subcontext count.
 * -# Find the first zero bit in the asynchronous VEIDs bitmap using find_first_zero_bit(), which
 *    represents an available VEID, and cast the found index to a 32-bit unsigned integer.
 * -# Check if the found index is equal to the maximum number of asynchronous
 *    subcontexts, which would mean that no asynchronous VEID is available.
 * -# If no asynchronous VEID is available, attempt to create a synchronous
 *    subcontext by calling the function nvgpu_tsg_create_sync_subcontext().
 * -# If the attempt to create a synchronous subcontext fails, log an error
 *    message and return the error code.
 * -# If an asynchronous VEID is available, set the corresponding bit in the
 *    asynchronous VEIDs bitmap to mark it as used.
 * -# Since asynchronous VEIDs start from 1, increment the found index by the
 *    number of maximum synchronous subcontexts to get the actual VEID.
 * -# Update the VEID output parameter with the calculated VEID value.
 * -# Return success.
 *
 * @param [in]  g                 The GPU driver struct.
 * @param [in]  tsg               Pointer to TSG struct.
 * @param [in]  max_subctx_count  Maximum number of subcontexts supported.
 * @param [out] veid              Pointer to store the allocated VEID.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
static int nvgpu_tsg_create_async_subcontext(struct gk20a *g,
				struct nvgpu_tsg *tsg, u32 max_subctx_count,
				u32 *veid)
{
	u32 max_async_subcontexts = max_subctx_count - MAX_SYNC_SUBCONTEXTS;
	int err;
	u32 idx;

	idx = nvgpu_safe_cast_u64_to_u32(
			find_first_zero_bit(tsg->async_veids,
					    max_async_subcontexts));

	if (idx == max_async_subcontexts) {
		nvgpu_log_info(g, "ASYNC VEID not available");
		err = nvgpu_tsg_create_sync_subcontext(g, tsg, veid);
		if (err != 0) {
			nvgpu_err(g, "ASYNC & SYNC VEIDs not available");
			return err;
		}
	} else {
		nvgpu_set_bit(idx, tsg->async_veids);
		/* ASYNC VEIDs start from 1. */
		*veid = idx + MAX_SYNC_SUBCONTEXTS;
	}

	return 0;
}

int nvgpu_tsg_create_subcontext(struct gk20a *g, struct nvgpu_tsg *tsg,
				u32 type, struct vm_gk20a *vm,
				u32 max_subctx_count, u32 *veid)
{
	int err;

	nvgpu_mutex_acquire(&tsg->veid_alloc_lock);

	if (type == NVGPU_TSG_SUBCONTEXT_TYPE_SYNC) {
		err = nvgpu_tsg_create_sync_subcontext(g, tsg, veid);
		if (err != 0) {
			nvgpu_err(g, "Sync VEID not available");
			nvgpu_mutex_release(&tsg->veid_alloc_lock);
			return err;
		}
	}

	if (type == NVGPU_TSG_SUBCONTEXT_TYPE_ASYNC) {
		err = nvgpu_tsg_create_async_subcontext(g, tsg,
						max_subctx_count, veid);
		if (err != 0) {
			nvgpu_err(g, "Async/Sync VEID not available");
			nvgpu_mutex_release(&tsg->veid_alloc_lock);
			return err;
		}
	}

	if (tsg->subctx_vms[*veid] == NULL) {
		tsg->subctx_vms[*veid] = vm;
	}

	nvgpu_mutex_release(&tsg->veid_alloc_lock);

	nvgpu_log_info(g, "Allocated VEID %u", *veid);

	return 0;
}

void nvgpu_tsg_delete_subcontext(struct gk20a *g, struct nvgpu_tsg *tsg,
				 u32 veid)
{
	nvgpu_mutex_acquire(&tsg->veid_alloc_lock);

	if (veid == 0U) {
		if (!tsg->sync_veid) {
			nvgpu_log_info(g, "VEID 0 not allocated");
			nvgpu_mutex_release(&tsg->veid_alloc_lock);
			return;
		}

		tsg->sync_veid = false;
		tsg->subctx_vms[veid] = NULL;
	} else {
		if (!nvgpu_test_bit(veid - MAX_SYNC_SUBCONTEXTS, tsg->async_veids)) {
			nvgpu_log_info(g, "VEID %u not allocated", veid);
			nvgpu_mutex_release(&tsg->veid_alloc_lock);
			return;
		}
		nvgpu_clear_bit(veid - MAX_SYNC_SUBCONTEXTS, tsg->async_veids);
		tsg->subctx_vms[veid] = NULL;
	}

	nvgpu_mutex_release(&tsg->veid_alloc_lock);

	nvgpu_log_info(g, "Freed VEID %u", veid);
}

int nvgpu_tsg_user_delete_subcontext(struct gk20a *g, struct nvgpu_tsg *tsg,
				     u32 max_subctx_count, u32 veid)
{
	if (veid >= max_subctx_count) {
		nvgpu_err(g, "Invalid VEID specified %u", veid);
		return -EINVAL;
	}

	if (nvgpu_tsg_subctx_has_channels_bound(tsg, veid)) {
		nvgpu_err(g, "Channels should be unbound before freeing the"
			      " subcontext");
		return -EINVAL;
	}

	nvgpu_tsg_delete_subcontext(g, tsg, veid);

	return 0;
}

int nvgpu_tsg_create_sync_subcontext_internal(struct gk20a *g,
			struct nvgpu_tsg *tsg, struct nvgpu_channel *ch)
{
	u32 subctx_id = 0U;
	int err;

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return 0;
	}

	nvgpu_mutex_acquire(&tsg->veid_alloc_lock);

	subctx_id = ch->subctx_id;

	/*
	 * If this is first channel created without creating subcontext,
	 * then this channel is using subcontext with VEID 0 by default.
	 * Set subctx_vm and reserve the VEID0.
	 */
	if ((subctx_id == 0U) &&  (tsg->subctx_vms[0] == NULL)) {
		err = nvgpu_tsg_create_sync_subcontext(g, tsg, &subctx_id);
		if (err != 0) {
			nvgpu_err(g, "SYNC VEID not available");
			nvgpu_mutex_release(&tsg->veid_alloc_lock);
			return err;
		}

		tsg->subctx_vms[0] = ch->vm;
	}

	nvgpu_mutex_release(&tsg->veid_alloc_lock);

	return 0;
}

int nvgpu_tsg_validate_ch_subctx_vm(struct gk20a *g,
			struct nvgpu_tsg *tsg, struct nvgpu_channel *ch)
{
	int err = 0;

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return 0;
	}

	nvgpu_mutex_acquire(&tsg->veid_alloc_lock);

	if (ch->vm != tsg->subctx_vms[ch->subctx_id]) {
		nvgpu_err(g, "channel VM does not match with subcontext VM");
		err = -EINVAL;
	}

	nvgpu_mutex_release(&tsg->veid_alloc_lock);

	return err;
}

/*
 * API to mark channel as part of TSG
 *
 * Note that channel is not runnable when we bind it to TSG
 */
int nvgpu_tsg_bind_channel(struct nvgpu_tsg *tsg, struct nvgpu_channel *ch)
{
	struct gk20a *g = ch->g;
	u32 max_ch_per_tsg;
	int err = 0;

	nvgpu_log_fn(g, "bind tsg:%u ch:%u\n", tsg->tsgid, ch->chid);

	/* check if channel is already bound to some TSG */
	if (nvgpu_tsg_from_ch(ch) != NULL) {
		return -EINVAL;
	}

	/* cannot bind more channels than MAX channels supported per TSG */
	nvgpu_rwsem_down_read(&tsg->ch_list_lock);
	max_ch_per_tsg = g->ops.runlist.get_max_channels_per_tsg();
	if (tsg->ch_count == max_ch_per_tsg) {
		nvgpu_rwsem_up_read(&tsg->ch_list_lock);
		nvgpu_warn(g, "TSG %u trying to bind more than supported channels (%u)",
			tsg->tsgid, max_ch_per_tsg);
		return -EINVAL;
	}
	nvgpu_rwsem_up_read(&tsg->ch_list_lock);

	/* Use runqueue selector 1 for all ASYNC ids */
	if (ch->subctx_id > CHANNEL_INFO_VEID0) {
		ch->runqueue_sel = 1;
	}

	/*
	 * All the channels in a TSG must share the same runlist.
	 */
	if (tsg->runlist == NULL) {
		tsg->runlist = ch->runlist;
		if (tsg->rl_domain != NULL) {
			/*
			 * The rl domain identifier is stashed in tsg->rl_domain->domain_id
			 * when the tsg is bound to a domain, but at that point there
			 * are no channels yet to describe which runlist id should be
			 * used. Now we know.
			 */
			tsg->rl_domain = nvgpu_rl_domain_get(g, tsg->runlist->id,
				tsg->rl_domain->domain_id);
			WARN_ON(tsg->rl_domain == NULL);
		}
	} else {
		if (tsg->runlist != ch->runlist) {
			nvgpu_err(tsg->g,
				  "runlist_id mismatch ch[%d] tsg[%d]",
				  ch->runlist->id,
				  tsg->runlist->id);
			return -EINVAL;
		}
	}

	if (g->ops.tsg.bind_channel != NULL) {
		err = g->ops.tsg.bind_channel(tsg, ch);
		if (err != 0) {
			nvgpu_err(tsg->g, "fail to bind ch %u to tsg %u",
				ch->chid, tsg->tsgid);
			return err;
		}
	}

	nvgpu_rwsem_down_write(&tsg->ch_list_lock);
	/* As multimedia engines are non-VEID engines, subctx is not needed */
	if (!nvgpu_engine_is_multimedia_runlist_id(g, ch->runlist->id)) {
		err = nvgpu_tsg_subctx_bind_channel(tsg, ch);
		if (err != 0) {
			nvgpu_err(g, "Subcontext %u bind failed", ch->subctx_id);
			nvgpu_rwsem_up_write(&tsg->ch_list_lock);
			return err;
		}
	}

	nvgpu_list_add_tail(&ch->ch_entry, &tsg->ch_list);
	tsg->ch_count = nvgpu_safe_add_u32(tsg->ch_count, 1U);
	ch->tsgid = tsg->tsgid;
	/* channel is serviceable after it is bound to tsg */
	ch->unserviceable = false;
	nvgpu_rwsem_up_write(&tsg->ch_list_lock);

	if (g->ops.tsg.bind_channel_eng_method_buffers != NULL) {
		g->ops.tsg.bind_channel_eng_method_buffers(tsg, ch);
	}

	nvgpu_ref_get(&tsg->refcount);

	return err;
}

#if defined(CONFIG_NVS_PRESENT)
int nvgpu_tsg_bind_domain(struct nvgpu_tsg *tsg, struct nvgpu_nvs_domain *nnvs_domain)
{
	struct nvgpu_runlist_domain *rl_domain;
	struct gk20a *g = tsg->g;
	const char *name;

	/* Hopping channels from one domain to another is not allowed */
	if (tsg->num_active_channels != 0U) {
		return -EINVAL;
	}

	name = nvgpu_nvs_domain_get_name(nnvs_domain);

	/*
	 * The domain ptr will get updated with the right id once the runlist
	 * gets specified based on the first channel.
	 */
	rl_domain = nvgpu_rl_domain_get(g, 0, nnvs_domain->id);
	if (rl_domain == NULL) {
		nvgpu_err(g, "rl domain not found (%s) having Id[%llu]", name, nnvs_domain->id);
		/*
		 * This shouldn't happen because the nvs domain guarantees RL domains.
		 *
		 * TODO: query this via the nvs domain.
		 */
		return -ENOENT;
	}

	/* Release the default domain ref that was implicitly taken at open */
	if (tsg->nvs_domain_ptr != NULL) {
		nvgpu_nvs_domain_put(g, tsg->nvs_domain_ptr);
	}

	nvgpu_nvs_domain_get(g, nnvs_domain);
	tsg->rl_domain = rl_domain;
	tsg->nvs_domain_ptr = nnvs_domain;

	return 0;
}
#endif

/**
 * @brief Determine if a TSG has multiple channels.
 *
 * -# Acquire a read lock on the TSG's channel list to ensure a consistent view
 *    while checking the channel list.
 * -# Retrieve the first entry in the TSG's channel list.
 * -# Retrieve the last entry in the TSG's channel list.
 * -# Compare the first and last entries of the channel list. If they are not
 *    the same, it indicates that there are multiple channels in the TSG, so
 *    set the return value to true.
 * -# Release the read lock on the TSG's channel list.
 * -# Return the boolean result indicating whether the TSG has multiple channels.
 *
 * @param [in]  tsg  Pointer to the TSG struct.
 *
 * @return True if the TSG has multiple channels, false otherwise.
 */
static bool nvgpu_tsg_is_multi_channel(struct nvgpu_tsg *tsg)
{
	bool ret = false;

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);
	if (nvgpu_list_first_entry(&tsg->ch_list, nvgpu_channel,
				   ch_entry) !=
	    nvgpu_list_last_entry(&tsg->ch_list, nvgpu_channel,
				   ch_entry)) {
		ret = true;
	}
	nvgpu_rwsem_up_read(&tsg->ch_list_lock);

	return ret;
}

/**
 * @brief Unbind a channel from the TSG it is bound to.
 *
 * This function unbinds a channel from its associated TSG. It handles the
 * necessary state checks and cleanups required to safely remove a channel
 * from a TSG. If the channel or TSG is in a state that prevents safe removal,
 * the function will return an error code.
 *
 * The steps performed by the function are as follows:
 * -# Acquire a write lock on the TSG's channel list by calling nvgpu_rwsem_down_write().
 * -# Check if the channel is unserviceable by calling nvgpu_channel_check_unserviceable().
 * -# Release the write lock on the TSG's channel list by calling nvgpu_rwsem_up_write().
 * -# Disable the TSG by calling gops_tsg.disable() operation.
 * -# Preempt the TSG by calling gops_fifo.preempt_tsg() operation.
 * -# If there are multiple channels in the TSG and the TSG has not timed out,
 *    validate the hardware state of the channel by calling
 *    gops_tsg.unbind_channel_check_hw_state(). If the hardware state check returns -EAGAIN, release
 *    the acquired resources and return.
 * -# If the hardware state check fails with an error other than -EAGAIN, log the error,
 *    re-enable the TSG if it has not timed out by calling gops_tsg.enable() operation, release the
 *    acquired resources, and return the error.
 * -# Clear the channel from the runlist if the clear() operation is supported by calling
 *    gops_channel.clear().
 * -# Update the runlist by removing the channel from it by calling nvgpu_channel_update_runlist().
 *    If the update fails, log the error, re-enable the TSG if it has not timed out by calling
 *    the enable() operation provided by g->ops.tsg, release the acquired resources, and return the error.
 * -# Unbind the channel from the TSG by calling the gops_channel.unbind() operation.
 * -# Acquire a write lock on the TSG's channel list by calling nvgpu_rwsem_down_write().
 * -# Unbind the channel from any subcontexts by calling nvgpu_tsg_subctx_unbind_channel().
 * -# Remove the channel from the TSG's channel list and decrement the channel count by calling
 *    nvgpu_list_del().
 * -# Set the channel's TSG ID to an invalid value.
 * -# Disable the channel to ensure it is not scheduled by hardware by calling nvgpu_channel_disable().
 * -# Release the write lock on the TSG's channel list by calling nvgpu_rwsem_up_write().
 * -# If the TSG has not timed out, re-enable the TSG by calling the gops_tsg.enable() operation.
 * -# If supported, perform any additional channel abort cleanup by calling
 *    gops_channel.abort_clean_up()
 *
 * @param [in]  tsg        Pointer to the TSG structure.
 * @param [in]  ch         Pointer to the channel structure.
 * @param [in]  force      Flag indicating if the unbind should proceed even if the channel is busy.
 *
 * @return 0 if the channel was successfully unbound from the TSG.
 * @return Non-zero error code if the unbind operation failed.
 */
static int nvgpu_tsg_unbind_channel_common(struct nvgpu_tsg *tsg,
		struct nvgpu_channel *ch, bool force)
{
	struct gk20a *g = ch->g;
	int err;
	bool tsg_timedout;

	/* If one channel in TSG times out, we disable all channels */
	nvgpu_rwsem_down_write(&tsg->ch_list_lock);
	tsg_timedout = nvgpu_channel_check_unserviceable(ch);
	nvgpu_rwsem_up_write(&tsg->ch_list_lock);

	/* Disable TSG and examine status before unbinding channel */
	nvgpu_tsg_disable(tsg);

	err = nvgpu_tsg_preempt(g, tsg);
	if (err != 0) {
		goto fail_enable_tsg;
	}

	/*
	 * State validation is only necessary if there are multiple channels in
         * the TSG.
	 */
	if (nvgpu_tsg_is_multi_channel(tsg) && !tsg_timedout &&
	    (g->ops.tsg.unbind_channel_check_hw_state != NULL)) {
		err = g->ops.tsg.unbind_channel_check_hw_state(tsg, ch);
		if (err == -EAGAIN) {
			goto fail_enable_tsg;
		}

		if (err != 0) {
			nvgpu_err(g, "invalid hw_state for ch %u", ch->chid);
			goto fail_enable_tsg;
		}
	}

	if (g->ops.channel.clear != NULL) {
		g->ops.channel.clear(g, ch->runlist->id, ch->chid);
	}

	/* Channel should be seen as TSG channel while updating runlist */
	err = nvgpu_channel_update_runlist(ch, false);
	if (err != 0) {
		nvgpu_err(g, "update runlist failed ch:%u tsg:%u",
				ch->chid, tsg->tsgid);
		goto fail_enable_tsg;
	}

#ifdef CONFIG_NVGPU_DEBUGGER
	while (nvgpu_atomic_read(&ch->mmu_debug_mode_refcnt) > 0) {
		err = nvgpu_tsg_set_mmu_debug_mode(ch, false);
		if (err != 0) {
			nvgpu_err(g, "disable mmu debug mode failed ch:%u",
				ch->chid);
			break;
		}
	}
	while (nvgpu_atomic_read(&ch->mmu_debug_gcc_mode_refcnt) > 0) {
		err = nvgpu_tsg_set_mmu_debug_gcc_mode(ch, false);
		if (err != 0) {
			nvgpu_err(g, "disable mmu debug gcc mode failed ch:%u",
				ch->chid);
			break;
		}
	}

	while (nvgpu_atomic_read(&ch->sched_exit_wait_for_errbar_refcnt) > 0) {
		err = nvgpu_tsg_set_sched_exit_wait_for_errbar(ch, false);
		if (err != 0) {
			nvgpu_err(g, "disable implicit ERRBAR failed ch:%u",
				ch->chid);
			break;
		}
	}
#endif

	g->ops.channel.unbind(ch);

	nvgpu_rwsem_down_write(&tsg->ch_list_lock);
	if (!nvgpu_engine_is_multimedia_runlist_id(g, ch->runlist->id)) {
		nvgpu_tsg_subctx_unbind_channel(tsg, ch, force);
	}
	nvgpu_list_del(&ch->ch_entry);
	tsg->ch_count = nvgpu_safe_sub_u32(tsg->ch_count, 1U);
	ch->tsgid = NVGPU_INVALID_TSG_ID;

	/* another thread could have re-enabled the channel because it was
	 * still on the list at that time, so make sure it's truly disabled
	 */
	nvgpu_channel_disable(ch);
	nvgpu_rwsem_up_write(&tsg->ch_list_lock);

	/*
	 * Don't re-enable all channels if TSG has timed out already
	 *
	 * Note that we can skip disabling and preempting TSG too in case of
	 * time out, but we keep that to ensure TSG is kicked out
	 */
	if (!tsg_timedout) {
		nvgpu_tsg_enable(tsg);
	}

	if (g->ops.channel.abort_clean_up != NULL) {
		g->ops.channel.abort_clean_up(ch);
	}

	return 0;

fail_enable_tsg:
	if (!tsg_timedout) {
		nvgpu_tsg_enable(tsg);
	}
	return err;
}

static int wait_for_ch_idle(struct gk20a *g, struct nvgpu_tsg *tsg,
			    struct nvgpu_channel *ch)
{
	struct nvgpu_channel_hw_state hw_state;
	struct nvgpu_timeout timeout;
	u32 delay = POLL_DELAY_MIN_US;
	bool tsg_timedout;
	int ret = 0;
	int err;

	nvgpu_log_fn(g, " ");

	nvgpu_rwsem_down_write(&tsg->ch_list_lock);
	tsg_timedout = nvgpu_channel_check_unserviceable(ch);
	nvgpu_rwsem_up_write(&tsg->ch_list_lock);

	/*
	 * Need to wait for next bit to clear only if there are multiple
	 * channels in the TSG.
	 */
	if (!nvgpu_tsg_is_multi_channel(tsg) || tsg_timedout ||
	    g->ops.tsg.unbind_channel_check_hw_next == NULL) {
		return 0;
	}

	nvgpu_timeout_init_cpu_timer(g, &timeout, nvgpu_get_poll_timeout(g));

	ret = -ETIMEDOUT;

	do {
		nvgpu_rwsem_down_read(&tsg->ch_list_lock);
		g->ops.channel.read_state(g, ch->runlist->id, ch->chid, &hw_state);
		nvgpu_rwsem_up_read(&tsg->ch_list_lock);

		err = g->ops.tsg.unbind_channel_check_hw_next(ch, &hw_state);
		if ((err != -EAGAIN) || (!hw_state.busy)) {
			ret = 0;
			break;
		}

		nvgpu_usleep_range(delay, delay * 2U);
		delay = min_t(u32,
				delay << 1U, POLL_DELAY_MAX_US);
	} while (nvgpu_timeout_expired(&timeout) == 0);

	if (ret != 0) {
		/*
		 * possible causes:
		 * channel has outstanding work pending.
		 * esched has not gotten chance to update the chram for this channel.
		 */
		nvgpu_err(g, "cannot clear next bit");
	}

	nvgpu_log_fn(g, "done");

	return ret;
}

/* The caller must ensure that channel belongs to a tsg */
int nvgpu_tsg_unbind_channel(struct nvgpu_tsg *tsg, struct nvgpu_channel *ch,
			     bool force)
{
	struct gk20a *g = ch->g;
	bool unbind_ch = true;
	int err;

	nvgpu_log_fn(g, "unbind tsg:%u ch:%u\n", tsg->tsgid, ch->chid);

	/**
	 * Remove channel from TSG and re-enable rest of the channels.
	 * Since channel removal can lead to subctx removal and/or
	 * VM mappings removal, acquire ctx_init_lock.
	 */
	nvgpu_mutex_acquire(&tsg->ctx_init_lock);

	/*
	 * If the NEXT bit is set for the channel, then wait for the channel
	 * to go idle and do the tsg unbind again.
	 */
	while (unbind_ch) {
		err = nvgpu_tsg_unbind_channel_common(tsg, ch, force);
		if (err == -EAGAIN) {
			err = wait_for_ch_idle(g, tsg, ch);
			if (err != 0) {
				nvgpu_err(g, "failed to idle channel, err=%d",
					  err);
				unbind_ch = false;
			} else {
				unbind_ch = true;
			}
		} else {
			unbind_ch = false;
		}
	}

	/* Let userspace retry the unbind if HW is busy. */
	if (err == -EAGAIN && !force) {
		nvgpu_mutex_release(&tsg->ctx_init_lock);
		return -EAGAIN;
	}

	if (err != 0) {
		nvgpu_err(g, "unbind common failed, err=%d", err);
		goto fail_common;
	}

	if (g->ops.tsg.unbind_channel != NULL) {
		err = g->ops.tsg.unbind_channel(tsg, ch);
		if (err != 0) {
			/*
			 * ch already removed from TSG's list.
			 * mark error explicitly.
			 */
			(void) nvgpu_channel_mark_error(g, ch);
			nvgpu_err(g, "unbind hal failed, err=%d", err);
			nvgpu_tsg_abort(g, tsg, true);
		}
	}

	nvgpu_mutex_release(&tsg->ctx_init_lock);

	nvgpu_ref_put(&tsg->refcount, nvgpu_tsg_release);

	return err;

fail_common:
	if (g->ops.tsg.unbind_channel != NULL) {
		int unbind_err = g->ops.tsg.unbind_channel(tsg, ch);
		if (unbind_err != 0) {
			nvgpu_err(g, "unbind hal failed, err=%d", unbind_err);
		}
	}

	nvgpu_err(g, "Channel %d unbind failed, tearing down TSG %d",
		ch->chid, tsg->tsgid);

	nvgpu_tsg_abort(g, tsg, true);

	if (g->ops.channel.clear != NULL) {
		g->ops.channel.clear(g, ch->runlist->id, ch->chid);
	}

	/* If channel unbind fails, channel is still part of runlist */
	if (nvgpu_channel_update_runlist(ch, false) != 0) {
		nvgpu_err(g, "remove ch %u from runlist failed", ch->chid);
	}

#ifdef CONFIG_NVGPU_DEBUGGER
	while (nvgpu_atomic_read(&ch->mmu_debug_mode_refcnt) > 0) {
		err = nvgpu_tsg_set_mmu_debug_mode(ch, false);
		if (err != 0) {
			nvgpu_err(g, "disable mmu debug mode failed ch:%u",
				ch->chid);
			break;
		}
	}

	while (nvgpu_atomic_read(&ch->mmu_debug_gcc_mode_refcnt) > 0) {
		err = nvgpu_tsg_set_mmu_debug_gcc_mode(ch, false);
		if (err != 0) {
			nvgpu_err(g, "disable mmu debug gcc mode failed ch:%u",
				ch->chid);
			break;
		}
	}

	while (nvgpu_atomic_read(&ch->sched_exit_wait_for_errbar_refcnt) > 0) {
		err = nvgpu_tsg_set_sched_exit_wait_for_errbar(ch, false);
		if (err != 0) {
			nvgpu_err(g, "disable implicit ERRBAR failed ch:%u",
				ch->chid);
			break;
		}
	}
#endif

	g->ops.channel.unbind(ch);

	nvgpu_rwsem_down_write(&tsg->ch_list_lock);
	/* As subctx bind is not done for MM engines, skip unbind as well */
	if (!nvgpu_engine_is_multimedia_runlist_id(g, ch->runlist->id)) {
		nvgpu_tsg_subctx_unbind_channel(tsg, ch, force);
	}
	nvgpu_list_del(&ch->ch_entry);
	ch->tsgid = NVGPU_INVALID_TSG_ID;
	tsg->ch_count = nvgpu_safe_sub_u32(tsg->ch_count, 1U);
	nvgpu_rwsem_up_write(&tsg->ch_list_lock);

	nvgpu_mutex_release(&tsg->ctx_init_lock);

	nvgpu_ref_put(&tsg->refcount, nvgpu_tsg_release);

	return err;

}

int nvgpu_tsg_unbind_channel_hw_state_check(struct nvgpu_tsg *tsg,
		struct nvgpu_channel *ch)
{
	struct gk20a *g = ch->g;
	struct nvgpu_channel_hw_state hw_state;
	int err = 0;

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);
	g->ops.channel.read_state(g, ch->runlist->id, ch->chid, &hw_state);
	nvgpu_rwsem_up_read(&tsg->ch_list_lock);

	if (g->ops.tsg.unbind_channel_check_hw_next != NULL) {
		err = g->ops.tsg.unbind_channel_check_hw_next(ch, &hw_state);
	}

	if (g->ops.tsg.unbind_channel_check_ctx_reload != NULL) {
		g->ops.tsg.unbind_channel_check_ctx_reload(tsg, ch, &hw_state);
	}

	if (g->ops.tsg.unbind_channel_check_eng_faulted != NULL) {
		g->ops.tsg.unbind_channel_check_eng_faulted(tsg, ch,
				&hw_state);
	}

	return err;
}

void nvgpu_tsg_unbind_channel_ctx_reload_check(struct nvgpu_tsg *tsg,
	struct nvgpu_channel *ch,
	struct nvgpu_channel_hw_state *hw_state)
{
	struct gk20a *g = ch->g;
	struct nvgpu_channel *temp_ch;

	/* If CTX_RELOAD is set on a channel, move it to some other channel */
	if (hw_state->ctx_reload) {
		nvgpu_rwsem_down_read(&tsg->ch_list_lock);
		nvgpu_list_for_each_entry(temp_ch, &tsg->ch_list,
				nvgpu_channel, ch_entry) {
			if (temp_ch->chid != ch->chid) {
				g->ops.channel.force_ctx_reload(g,
					temp_ch->runlist->id, temp_ch->chid);
				break;
			}
		}
		nvgpu_rwsem_up_read(&tsg->ch_list_lock);
	}
}

/**
 * @brief Destroy the TSG's mutexes.
 *
 * -# Destroy the mutex used for synchronizing context initialization by calling
 *    the function nvgpu_mutex_destroy() with the 'ctx_init_lock' mutex of the TSG.
 * -# Destroy the mutex used for VEID allocation synchronization by calling
 *    the function nvgpu_mutex_destroy() with the 'veid_alloc_lock' mutex of the TSG.
 *
 * @param [in]  tsg  Pointer to the TSG struct.
 */
static void nvgpu_tsg_destroy(struct nvgpu_tsg *tsg)
{
#ifdef CONFIG_NVGPU_CHANNEL_TSG_CONTROL
	nvgpu_mutex_destroy(&tsg->event_id_list_lock);
#endif
#ifdef CONFIG_NVGPU_TSG_SHARING
	nvgpu_mutex_destroy(&tsg->tsg_share_lock);
#endif
	nvgpu_mutex_destroy(&tsg->ctx_init_lock);
	nvgpu_mutex_destroy(&tsg->eng_ctx_lock);
	nvgpu_mutex_destroy(&tsg->veid_alloc_lock);
}

#ifdef CONFIG_NVGPU_CHANNEL_TSG_CONTROL
/* force reset tsg that the channel is bound to */
int nvgpu_tsg_force_reset_ch(struct nvgpu_channel *ch,
				u32 err_code, bool verbose)
{
	struct gk20a *g = ch->g;

	struct nvgpu_tsg *tsg = nvgpu_tsg_from_ch(ch);

	if (tsg != NULL) {
		/* gfid uses 0 since it's not called on sr-iov gpu server */
		nvgpu_tsg_set_error_notifier(g, 0U, tsg->tsgid, err_code);
		nvgpu_rc_tsg_and_related_engines(g, tsg, verbose,
			RC_TYPE_FORCE_RESET);
	} else {
		nvgpu_err(g, "chid: %d is not bound to tsg", ch->chid);
	}

	return 0;
}
#endif

void nvgpu_tsg_cleanup_sw(struct gk20a *g)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 tsgid;

	for (tsgid = 0; tsgid < f->num_channels; tsgid++) {
		struct nvgpu_tsg *tsg = &f->tsg[tsgid];

		nvgpu_tsg_destroy(tsg);
	}

	nvgpu_vfree(g, f->tsg);
	f->tsg = NULL;
	nvgpu_mutex_destroy(&f->tsg_inuse_mutex);
}

/**
 * @brief Initialize TSG support structures and synchronization primitives.
 *
 * -# Assign the TSG structure from the global FIFO's TSG array using the provided TSG ID.
 * -# Set the 'in_use' flag of the TSG to false, indicating that the TSG is not currently in use.
 * -# Set the 'tsgid' of the TSG to the provided TSG ID.
 * -# Set the 'abortable' flag of the TSG to true, indicating that the TSG can be aborted.
 * -# Initialize the channel list head for the TSG by calling nvgpu_init_list_node().
 * -# Initialize the subcontext list head for the TSG by calling nvgpu_init_list_node().
 * -# Initialize the graphics context mappings list head for the TSG by calling
 *    nvgpu_init_list_node().
 * -# Initialize the read-write semaphore for the TSG's channel list by calling nvgpu_rwsem_init().
 * -# Initialize the mutex for synchronizing context initialization by calling nvgpu_mutex_init()
 *    with the 'ctx_init_lock' mutex of the TSG.
 * -# Initialize the mutex for VEID allocation synchronization by calling nvgpu_mutex_init() with
 *    the 'veid_alloc_lock' mutex of the TSG.
 *
 * @param [in]  g      Pointer to the GPU driver struct.
 * @param [in]  tsgid  TSG identifier.
 */
static void nvgpu_tsg_init_support(struct gk20a *g, u32 tsgid)
{
	struct nvgpu_tsg *tsg = NULL;

	tsg = &g->fifo.tsg[tsgid];

	tsg->in_use = false;
	tsg->tsgid = tsgid;
	tsg->abortable = true;

	nvgpu_init_list_node(&tsg->ch_list);
	nvgpu_init_list_node(&tsg->subctx_list);
	nvgpu_init_list_node(&tsg->gr_ctx_mappings_list);
	nvgpu_rwsem_init(&tsg->ch_list_lock);
	nvgpu_mutex_init(&tsg->ctx_init_lock);
	nvgpu_mutex_init(&tsg->eng_ctx_lock);
	nvgpu_mutex_init(&tsg->veid_alloc_lock);

#ifdef CONFIG_NVGPU_CHANNEL_TSG_CONTROL
	nvgpu_init_list_node(&tsg->event_id_list);
	nvgpu_mutex_init(&tsg->event_id_list_lock);
#endif
#ifdef CONFIG_NVGPU_TSG_SHARING
	nvgpu_init_list_node(&tsg->ctrl_devices_list);
	nvgpu_mutex_init(&tsg->tsg_share_lock);
#endif
}

int nvgpu_tsg_setup_sw(struct gk20a *g)
{
	struct nvgpu_fifo *f = &g->fifo;
	u32 tsgid;
	int err;

	nvgpu_mutex_init(&f->tsg_inuse_mutex);

	f->tsg = nvgpu_vzalloc(g, f->num_channels * sizeof(*f->tsg));
	if (f->tsg == NULL) {
		nvgpu_err(g, "no mem for tsgs");
		err = -ENOMEM;
		goto clean_up_mutex;
	}

	for (tsgid = 0; tsgid < f->num_channels; tsgid++) {
		nvgpu_tsg_init_support(g, tsgid);
	}

	return 0;

clean_up_mutex:
	nvgpu_mutex_destroy(&f->tsg_inuse_mutex);
	return err;
}

void nvgpu_tsg_set_unserviceable(struct gk20a *g, u32 gfid, u32 tsgid)
{
	struct nvgpu_tsg *tsg = NULL;
	struct nvgpu_channel *ch = NULL;

	if (g->ops.tsg.set_unserviceable != NULL) {
		g->ops.tsg.set_unserviceable(g, gfid, tsgid);
		return;
	}

	tsg = nvgpu_tsg_check_and_get_from_id(g, tsgid);
	if (tsg == NULL) {
		nvgpu_err(g, "invalid tsgid %u", tsgid);
		return;
	}

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);
	nvgpu_list_for_each_entry(ch, &tsg->ch_list, nvgpu_channel, ch_entry) {
		if (nvgpu_channel_get(ch) != NULL) {
			nvgpu_channel_set_unserviceable(ch);
			nvgpu_channel_put(ch);
		}
	}
	nvgpu_rwsem_up_read(&tsg->ch_list_lock);
}

void nvgpu_tsg_wakeup_wqs(struct gk20a *g,
				struct nvgpu_tsg *tsg)
{
	struct nvgpu_channel *ch = NULL;

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);
	nvgpu_list_for_each_entry(ch, &tsg->ch_list, nvgpu_channel, ch_entry) {
		if (nvgpu_channel_get(ch) != NULL) {
			nvgpu_channel_wakeup_wqs(g, ch);
			nvgpu_channel_put(ch);
		}
	}
	nvgpu_rwsem_up_read(&tsg->ch_list_lock);
}

bool nvgpu_tsg_mark_error(struct gk20a *g,
		struct nvgpu_tsg *tsg)
{
	struct nvgpu_channel *ch = NULL;
	bool verbose = false;

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);
	nvgpu_list_for_each_entry(ch, &tsg->ch_list, nvgpu_channel, ch_entry) {
		if (nvgpu_channel_get(ch) != NULL) {
			if (nvgpu_channel_mark_error(g, ch)) {
				verbose = true;
			}
			nvgpu_channel_put(ch);
		}
	}
	nvgpu_rwsem_up_read(&tsg->ch_list_lock);

	return verbose;

}

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
void nvgpu_tsg_set_ctxsw_timeout_accumulated_ms(struct nvgpu_tsg *tsg, u32 ms)
{
	struct nvgpu_channel *ch = NULL;

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);
	nvgpu_list_for_each_entry(ch, &tsg->ch_list, nvgpu_channel, ch_entry) {
		if (nvgpu_channel_get(ch) != NULL) {
			ch->ctxsw_timeout_accumulated_ms = ms;
			nvgpu_channel_put(ch);
		}
	}
	nvgpu_rwsem_up_read(&tsg->ch_list_lock);
}

bool nvgpu_tsg_ctxsw_timeout_debug_dump_state(struct nvgpu_tsg *tsg)
{
	struct nvgpu_channel *ch = NULL;
	bool verbose = false;

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);
	nvgpu_list_for_each_entry(ch, &tsg->ch_list, nvgpu_channel, ch_entry) {
		if (nvgpu_channel_get(ch) != NULL) {
			if (ch->ctxsw_timeout_debug_dump) {
				verbose = true;
			}
			nvgpu_channel_put(ch);
		}
	}
	nvgpu_rwsem_up_read(&tsg->ch_list_lock);

	return verbose;
}
#endif

void nvgpu_tsg_set_error_notifier(struct gk20a *g, u32 gfid, u32 tsgid,
		u32 error_notifier)
{
	struct nvgpu_tsg *tsg = NULL;
	struct nvgpu_channel *ch = NULL;

	if (g->ops.tsg.set_error_notifier != NULL) {
		g->ops.tsg.set_error_notifier(g, gfid, tsgid, error_notifier);
		return;
	}

	if (error_notifier >= NVGPU_ERR_NOTIFIER_INVAL) {
		return;
	}

	tsg = nvgpu_tsg_check_and_get_from_id(g, tsgid);
	if (tsg == NULL) {
		nvgpu_err(g, "invalid tsgid %u", tsgid);
		return;
	}

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);
	nvgpu_list_for_each_entry(ch, &tsg->ch_list, nvgpu_channel, ch_entry) {
		if (nvgpu_channel_get(ch) != NULL) {
			nvgpu_channel_set_error_notifier(g, ch, error_notifier);
			nvgpu_channel_put(ch);
		}
	}
	nvgpu_rwsem_up_read(&tsg->ch_list_lock);
}

void nvgpu_tsg_set_ctx_mmu_error(struct gk20a *g, u32 gfid, u32 tsgid)
{
	nvgpu_err(g, "TSG %d generated a mmu fault", tsgid);

	nvgpu_tsg_set_error_notifier(g, gfid, tsgid, /* mmu fault on VF for SR-IOV */
		NVGPU_ERR_NOTIFIER_FIFO_ERROR_MMU_ERR_FLT);
}

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
bool nvgpu_tsg_check_ctxsw_timeout(struct gk20a *g, u32 tsgid,
		bool *debug_dump, u32 *ms)
{
	struct nvgpu_tsg *tsg;
	struct nvgpu_channel *ch;
	bool recover = false;
	bool progress = false;

	tsg = nvgpu_tsg_check_and_get_from_id(g, tsgid);
	if (tsg == NULL) {
		nvgpu_err(g, "invalid tsgid %u", tsgid);
		return false;
	}

	*debug_dump = false;
	*ms = g->ctxsw_timeout_period_ms;

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);

	/* check if there was some progress on any of the TSG channels.
	 * fifo recovery is needed if at least one channel reached the
	 * maximum timeout without progress (update in gpfifo pointers).
	 */
	nvgpu_list_for_each_entry(ch, &tsg->ch_list, nvgpu_channel, ch_entry) {
		if (nvgpu_channel_get(ch) != NULL) {
			recover = nvgpu_channel_update_and_check_ctxsw_timeout(ch,
					*ms, &progress);
			if (progress || recover) {
				break;
			}
			nvgpu_channel_put(ch);
		}
	}

	if (recover) {
		/*
		 * if one channel is presumed dead (no progress for too long),
		 * then fifo recovery is needed. we can't really figure out
		 * which channel caused the problem, so set ctxsw timeout error
		 * notifier for all channels.
		 */
		*ms = ch->ctxsw_timeout_accumulated_ms;
		nvgpu_channel_put(ch);
		*debug_dump = nvgpu_tsg_ctxsw_timeout_debug_dump_state(tsg);

	} else {
		/*
		 * if at least one channel in the TSG made some progress, reset
		 * ctxsw_timeout_accumulated_ms for all channels in the TSG. In
		 * particular, this resets ctxsw_timeout_accumulated_ms timeout
		 * for channels that already completed their work.
		 */
		if (progress) {
			nvgpu_log_info(g, "progress on tsg=%d ch=%d",
					tsg->tsgid, ch->chid);
			nvgpu_channel_put(ch);
			*ms = g->ctxsw_timeout_period_ms;
			nvgpu_tsg_set_ctxsw_timeout_accumulated_ms(tsg, *ms);
		}
	}

	/* if we could not detect progress on any of the channel, but none
	 * of them has reached the timeout, there is nothing more to do:
	 * ctxsw_timeout_accumulated_ms has been updated for all of them.
	 */
	nvgpu_rwsem_up_read(&tsg->ch_list_lock);
	return recover;
}
#endif

#ifdef CONFIG_NVGPU_CHANNEL_TSG_SCHEDULING
int nvgpu_tsg_set_interleave(struct nvgpu_tsg *tsg, u32 level)
{
	struct gk20a *g = tsg->g;
	int ret;

	nvgpu_log(g, gpu_dbg_sched,
			"tsgid=%u interleave=%u", tsg->tsgid, level);

	nvgpu_speculation_barrier();

	if ((level != NVGPU_FIFO_RUNLIST_INTERLEAVE_LEVEL_LOW) &&
	    (level != NVGPU_FIFO_RUNLIST_INTERLEAVE_LEVEL_MEDIUM) &&
	    (level != NVGPU_FIFO_RUNLIST_INTERLEAVE_LEVEL_HIGH)) {
		return -EINVAL;
	}

	if (g->ops.tsg.set_interleave != NULL) {
		ret = g->ops.tsg.set_interleave(tsg, level);
		if (ret != 0) {
			nvgpu_err(g,
				"set interleave failed tsgid=%u", tsg->tsgid);
			return ret;
		}
	}

	tsg->interleave_level = level;

	/* TSG may not be bound yet */
	if (tsg->runlist == NULL) {
		return 0;
	}

	return g->ops.runlist.reload(g, tsg->runlist, tsg->rl_domain, true, true);
}

int nvgpu_tsg_set_timeslice(struct nvgpu_tsg *tsg, u32 timeslice_us)
{
	struct gk20a *g = tsg->g;

	nvgpu_log(g, gpu_dbg_sched, "tsgid=%u timeslice=%u us",
			tsg->tsgid, timeslice_us);

	if (timeslice_us < g->tsg_timeslice_min_us ||
		timeslice_us > g->tsg_timeslice_max_us) {
		return -EINVAL;
	}

	tsg->timeslice_us = timeslice_us;

	/* TSG may not be bound yet */
	if (tsg->runlist == NULL) {
		return 0;
	}

	return g->ops.runlist.reload(g, tsg->runlist, tsg->rl_domain, true, true);
}

u32 nvgpu_tsg_get_timeslice(struct nvgpu_tsg *tsg)
{
	return tsg->timeslice_us;
}

int nvgpu_tsg_set_long_timeslice(struct nvgpu_tsg *tsg, u32 timeslice_us)
{
	struct gk20a *g = tsg->g;

	nvgpu_log(g, gpu_dbg_sched, "tsgid=%u timeslice=%u us",
			tsg->tsgid, timeslice_us);

	if (timeslice_us < g->tsg_timeslice_min_us ||
			timeslice_us > g->tsg_dbg_timeslice_max_us) {
		return -EINVAL;
	}

	tsg->timeslice_us = timeslice_us;

	/* TSG may not be bound yet */
	if (tsg->runlist == NULL) {
		return 0;
	}

	return g->ops.runlist.reload(g, tsg->runlist, tsg->rl_domain, true, true);
}
#endif

u32 nvgpu_tsg_default_timeslice_us(struct gk20a *g)
{
	(void)g;
	return NVGPU_TSG_TIMESLICE_DEFAULT_US;
}

/**
 * @brief Release a TSG back to the pool of available TSGs.
 *
 * -# Acquire the mutex lock for the TSG in-use array to ensure exclusive access
 *    by calling 'nvgpu_mutex_acquire()'.
 * -# Set the 'in_use' flag of the specified TSG to false, indicating that it is
 *    no longer in use and available for allocation.
 * -# Release the mutex lock for the TSG in-use array to allow other operations
 *    by calling 'nvgpu_mutex_release()'.
 *
 * @param [in]  f    Pointer to the FIFO struct.
 * @param [in]  tsg  Pointer to the TSG struct to be released.
 */
static void nvgpu_tsg_release_used_tsg(struct nvgpu_fifo *f,
		struct nvgpu_tsg *tsg)
{
	nvgpu_mutex_acquire(&f->tsg_inuse_mutex);
	f->tsg[tsg->tsgid].in_use = false;
	nvgpu_mutex_release(&f->tsg_inuse_mutex);
}

/**
 * @brief Acquire an unused TSG from the pool of TSGs.
 *
 * -# Acquire the mutex lock for the TSG in-use array to ensure exclusive access
 *    by calling nvgpu_mutex_acquire().
 * -# Iterate over the array of TSGs to find an unused one:
 *    -# For each TSG, check if the 'in_use' flag is false, indicating it is not in use.
 *    -# If an unused TSG is found, set its 'in_use' flag to true, indicating it is now in use.
 *    -# Assign the pointer of the newly acquired TSG to the variable 'tsg'.
 *    -# Break the loop as we have found an available TSG.
 * -# Release the mutex lock for the TSG in-use array to allow other operations
 *    by calling nvgpu_mutex_release().
 * -# Return the pointer to the acquired TSG if one was found, or NULL otherwise.
 *
 * @param [in]  f  Pointer to the FIFO struct.
 *
 * @return Pointer to the acquired TSG struct, or NULL if no unused TSG is available.
 */
static struct nvgpu_tsg *nvgpu_tsg_acquire_unused_tsg(struct nvgpu_fifo *f)
{
	struct nvgpu_tsg *tsg = NULL;
	unsigned int tsgid;

	nvgpu_mutex_acquire(&f->tsg_inuse_mutex);
	for (tsgid = 0; tsgid < f->num_channels; tsgid++) {
		if (!f->tsg[tsgid].in_use) {
			f->tsg[tsgid].in_use = true;
			tsg = &f->tsg[tsgid];
			break;
		}
	}
	nvgpu_mutex_release(&f->tsg_inuse_mutex);

	return tsg;
}

/**
 * @brief Allocate VEID state for a TSG.
 *
 * -# Check if TSG subcontexts are supported by the GPU. If not, return success
 *    as there is no need to allocate VEID state.
 * -# Set the 'sync_veid' flag of the TSG to false, indicating that synchronous
 *    VEIDs are not yet allocated.
 * -# Retrieve the maximum subcontext count supported by the GPU by calling
 *    gops_gr_init.get_max_subctx_count.
 * -# Calculate the maximum number of asynchronous subcontexts by subtracting the
 *    number of maximum synchronous subcontexts from the total maximum subcontext count.
 * -# Allocate memory for the asynchronous VEIDs bitmap by calling nvgpu_kzalloc().
 *    The size of the allocation is determined by converting the number of bits
 *    required for 'max_async_subcontexts' into the number of longs required.
 * -# If the allocation for the asynchronous VEIDs bitmap fails, log an error and
 *    return an error code indicating out of memory.
 * -# Allocate memory for the array of pointers to subcontext VMs by calling
 *    nvgpu_kzalloc(). The size of the allocation is the number of maximum
 *    subcontexts multiplied by the size of a VM pointer.
 * -# If the allocation for the subcontext VMs array fails, log an error, free the
 *    previously allocated asynchronous VEIDs bitmap, set the pointer to NULL, and
 *    return an error code indicating out of memory.
 * -# If TSG sharing is enabled (indicated by the macro CONFIG_NVGPU_TSG_SHARING),
 *    set the 'share_token_count' of the TSG to 1, indicating that the TSG has one
 *    share token available for use.
 * -# Return success.
 *
 * @param [in]  g    Pointer to the GPU driver struct.
 * @param [in]  tsg  Pointer to the TSG struct.
 *
 * @return 0 in case of success, -ENOMEM in case of memory allocation failure.
 */
static int nvgpu_tsg_alloc_veid_state(struct gk20a *g, struct nvgpu_tsg *tsg)
{
	u32 max_async_subcontexts;
	u32 max_subctx_count;

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return 0;
	}

	tsg->sync_veid = false;

	max_subctx_count = g->ops.gr.init.get_max_subctx_count();
	max_async_subcontexts = nvgpu_safe_sub_u32(max_subctx_count, MAX_SYNC_SUBCONTEXTS);

	tsg->async_veids = nvgpu_kzalloc(g,
				BITS_TO_LONGS(max_async_subcontexts) *
				sizeof(unsigned long));
	if (tsg->async_veids == NULL) {
		nvgpu_err(g, "async veids bitmap alloc failed");
		return -ENOMEM;
	}

	tsg->subctx_vms = nvgpu_kzalloc(g,
				sizeof(struct vm_gk20a *) * max_subctx_count);
	if (tsg->subctx_vms == NULL) {
		nvgpu_err(g, "subctx vms alloc failed");
		nvgpu_kfree(g, tsg->async_veids);
		tsg->async_veids = NULL;
		return -ENOMEM;
	}

#ifdef CONFIG_NVGPU_TSG_SHARING
	tsg->share_token_count = 1U;
#endif

	return 0;
}

/**
 * @brief Free VEID state for a TSG.
 *
 * -# Check if TSG subcontexts are supported by the GPU. If not, return immediately
 *    as there is no VEID state to free.
 * -# Free the memory allocated for the subcontext VMs array by calling 'nvgpu_kfree()'.
 * -# Set the 'subctx_vms' pointer in the TSG struct to NULL.
 * -# Free the memory allocated for the asynchronous VEIDs bitmap by calling 'nvgpu_kfree()'.
 * -# Set the 'async_veids' pointer in the TSG struct to NULL.
 * -# Set the 'sync_veid' flag of the TSG to false, indicating that synchronous VEIDs
 *    are no longer allocated.
 *
 * @param [in]  g    Pointer to the GPU driver struct.
 * @param [in]  tsg  Pointer to the TSG struct.
 */
static void nvgpu_tsg_free_veid_state(struct gk20a *g, struct nvgpu_tsg *tsg)
{
	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return;
	}

	nvgpu_kfree(g, tsg->subctx_vms);
	tsg->subctx_vms = NULL;

	nvgpu_kfree(g, tsg->async_veids);
	tsg->async_veids = NULL;

	tsg->sync_veid = false;
}

int nvgpu_tsg_open_common(struct gk20a *g, struct nvgpu_tsg *tsg, pid_t pid, u32 instance_id)
{
	u32 no_of_sm = g->ops.gr.init.get_no_of_sm(g, instance_id);
	int err;

	/* we need to allocate this after g->ops.gr.init_fs_state() since
	 * we initialize gr.config->no_of_sm in this function
	 */
	if (no_of_sm == 0U) {
		nvgpu_err(g, "no_of_sm %d not set, failed allocation", no_of_sm);
		return -EINVAL;
	}

	err = nvgpu_tsg_alloc_sm_error_states_mem(g, tsg, no_of_sm);
	if (err != 0) {
		return err;
	}

	tsg->tgid = pid;
	tsg->g = g;
	tsg->num_active_channels = 0U;
	tsg->ch_count = 0U;
	tsg->disable_count = 0U;
	nvgpu_atomic_set(&tsg->cilp_save_cmpl, 0);
	tsg->gpu_instance_id = instance_id;
	tsg->is_shadow_cleared = false;
	nvgpu_ref_init(&tsg->refcount);

	tsg->interleave_level = NVGPU_FIFO_RUNLIST_INTERLEAVE_LEVEL_LOW;
	tsg->timeslice_us = g->ops.tsg.default_timeslice_us(g);
	tsg->runlist = NULL;
	tsg->rl_domain = NULL;
	tsg->nvs_domain_ptr = NULL;
#ifdef CONFIG_NVGPU_DEBUGGER
	tsg->sm_exception_mask_type = NVGPU_SM_EXCEPTION_TYPE_MASK_NONE;
#endif
	tsg->gr_ctx = nvgpu_alloc_gr_ctx_struct(g);
	if (tsg->gr_ctx == NULL) {
		err = -ENOMEM;
		goto clean_up;
	}

	if (g->ops.tsg.init_subctx_state != NULL) {
		err = g->ops.tsg.init_subctx_state(g, tsg);
		if (err != 0) {
			nvgpu_err(g, "tsg %d subctx state init failed %d",
				  tsg->tsgid, err);
			goto clean_up;
		}
	}

	err = nvgpu_tsg_alloc_veid_state(g, tsg);
	if (err != 0) {
		nvgpu_err(g, "VEID sw state alloc failed %d", err);
		goto clean_up;
	}

#ifdef CONFIG_NVGPU_SM_DIVERSITY
	nvgpu_gr_ctx_set_sm_diversity_config(tsg->gr_ctx,
		NVGPU_INVALID_SM_CONFIG_ID);
#endif

	if (g->ops.tsg.init_eng_method_buffers != NULL) {
		err = g->ops.tsg.init_eng_method_buffers(g, tsg);
		if (err != 0) {
			nvgpu_err(g, "tsg %d init eng method bufs failed %d",
				  tsg->tsgid, err);
			goto clean_up;
		}
	}

	if (g->ops.tsg.open != NULL) {
		err = g->ops.tsg.open(tsg);
		if (err != 0) {
			nvgpu_err(g, "tsg %d fifo open failed %d",
				  tsg->tsgid, err);
			goto clean_up;
		}
	}

	return 0;

clean_up:
	nvgpu_tsg_release_common(g, tsg);
	nvgpu_ref_put(&tsg->refcount, NULL);

	return err;
}

struct nvgpu_tsg *nvgpu_tsg_open(struct gk20a *g, pid_t pid, u32 instance_id)
{
	struct nvgpu_tsg *tsg;
	int err;

	tsg = nvgpu_tsg_acquire_unused_tsg(&g->fifo);
	if (tsg == NULL) {
		return NULL;
	}

	err = nvgpu_tsg_open_common(g, tsg, pid, instance_id);
	if (err != 0) {
		nvgpu_tsg_release_used_tsg(&g->fifo, tsg);
		nvgpu_err(g, "tsg %d open failed %d", tsg->tsgid, err);
		return NULL;
	}

	nvgpu_log(g, gpu_dbg_fn, "tsg opened %d\n", tsg->tsgid);

	return tsg;
}

void nvgpu_tsg_release_common(struct gk20a *g, struct nvgpu_tsg *tsg)
{
	if (g->ops.tsg.release != NULL) {
		g->ops.tsg.release(tsg);
	}

	nvgpu_tsg_free_veid_state(g, tsg);

	nvgpu_free_gr_ctx_struct(g, tsg->gr_ctx);
	tsg->gr_ctx = NULL;

	if (g->ops.tsg.deinit_subctx_state != NULL) {
		g->ops.tsg.deinit_subctx_state(g, tsg);
	}

	if (g->ops.tsg.deinit_eng_method_buffers != NULL) {
		g->ops.tsg.deinit_eng_method_buffers(g, tsg);
	}

	if (tsg->nvs_domain_ptr != NULL) {
		nvgpu_nvs_domain_put(g, tsg->nvs_domain_ptr);
		tsg->nvs_domain_ptr = NULL;
		tsg->rl_domain = NULL;
	}

	if(tsg->sm_error_states != NULL) {
		nvgpu_kfree(g, tsg->sm_error_states);
		tsg->sm_error_states = NULL;
#ifdef CONFIG_NVGPU_DEBUGGER
		nvgpu_mutex_destroy(&tsg->sm_exception_mask_lock);
#endif
	}

	if (tsg->disable_count > 0U) {
		nvgpu_log_info(tsg->g, "tsg: %u, disable count at release: %u",
			       tsg->tsgid, tsg->disable_count);
	}
}

/**
 * @brief Retrieve the TSG struct from a reference count pointer.
 *
 * -# Calculate the address of the TSG struct by subtracting the offset of the
 *    'refcount' member within the 'nvgpu_tsg' struct from the address of the
 *    reference count. This is done using pointer arithmetic and the 'offsetof()'
 *    macro which yields the offset of a member within a struct.
 * -# Cast the calculated address to a pointer of type 'struct nvgpu_tsg'.
 * -# Return the pointer to the TSG struct.
 *
 * @param [in]  ref  Pointer to the reference count struct.
 *
 * @return Pointer to the TSG struct containing the reference count.
 */
static struct nvgpu_tsg *tsg_gk20a_from_ref(struct nvgpu_ref *ref)
{
	return (struct nvgpu_tsg *)
		((uintptr_t)ref - offsetof(struct nvgpu_tsg, refcount));
}

void nvgpu_tsg_release(struct nvgpu_ref *ref)
{
	struct nvgpu_tsg *tsg = tsg_gk20a_from_ref(ref);
	struct nvgpu_gr_ctx *gr_ctx = tsg->gr_ctx;
	struct gk20a *g = tsg->g;
	int err;

	nvgpu_log(g, gpu_dbg_fn, "releasing tsg %d", tsg->tsgid);

	err = gk20a_busy(g);
	if (err != 0) {
		nvgpu_err(g, "cannot busy() err=%d!", err);
		return;
	}

#ifdef CONFIG_NVGPU_PROFILER
	if (tsg->prof != NULL) {
		nvgpu_profiler_unbind_context(tsg->prof);
	}
#endif
	if ((gr_ctx != NULL) && nvgpu_gr_ctx_get_ctx_initialized(gr_ctx)) {
		g->ops.gr.setup.free_gr_ctx(g, gr_ctx);
	}

	nvgpu_multimedia_free_all_ctx(tsg);

#ifdef CONFIG_NVGPU_CHANNEL_TSG_CONTROL
	/* unhook all events created on this TSG */
	nvgpu_mutex_acquire(&tsg->event_id_list_lock);
	while (nvgpu_list_empty(&tsg->event_id_list) == false) {
		nvgpu_list_del(tsg->event_id_list.next);
	}
	nvgpu_mutex_release(&tsg->event_id_list_lock);
#endif

	nvgpu_tsg_release_common(g, tsg);

#ifdef CONFIG_NVGPU_CHECKER
	nvgpu_checker_tsg_deinit(tsg->tsgid);
#endif

	nvgpu_log(g, gpu_dbg_fn, "tsg released %d", tsg->tsgid);

	nvgpu_tsg_release_used_tsg(&g->fifo, tsg);

	gk20a_idle(g);
}

struct nvgpu_tsg *nvgpu_tsg_from_ch(struct nvgpu_channel *ch)
{
	struct nvgpu_tsg *tsg = NULL;
	u32 tsgid = ch->tsgid;

	if (tsgid != NVGPU_INVALID_TSG_ID) {
		struct gk20a *g = ch->g;
		struct nvgpu_fifo *f = &g->fifo;

		tsg = &f->tsg[tsgid];
	} else {
		nvgpu_log(ch->g, gpu_dbg_fn, "tsgid is invalid for chid: %d",
			ch->chid);
	}
	return tsg;
}

int nvgpu_tsg_alloc_sm_error_states_mem(struct gk20a *g,
					struct nvgpu_tsg *tsg,
					u32 num_sm)
{
	if (tsg->sm_error_states != NULL) {
		return -EINVAL;
	}

	tsg->sm_error_states = nvgpu_kzalloc(g, nvgpu_safe_mult_u64(
			sizeof(struct nvgpu_tsg_sm_error_state), num_sm));
	if (tsg->sm_error_states == NULL) {
		nvgpu_err(g, "sm_error_states mem allocation failed");
		return -ENOMEM;
	}

#ifdef CONFIG_NVGPU_DEBUGGER
	nvgpu_mutex_init(&tsg->sm_exception_mask_lock);
#endif

	return 0;
}

int nvgpu_tsg_store_sm_error_state(struct gk20a *g, struct nvgpu_gr_isr_data *isr_data,
		u32 sm_id, u32 hww_global_esr, u32 hww_warp_esr, u64 hww_warp_esr_pc,
		u32 hww_global_esr_report_mask, u32 hww_warp_esr_report_mask,
		u32 hww_cga_esr, u32 instance_id)
{
	struct nvgpu_tsg *tsg = NULL;
	u32 tsgid = isr_data->tsgid;
	u32 num_of_sm = g->ops.gr.init.get_no_of_sm(g, instance_id);
	struct nvgpu_tsg_sm_error_state *sm_error_states = NULL;

	if (sm_id >= num_of_sm) {
		nvgpu_err(g, "Invalid number of SMs");
		return -EINVAL;
	}

	if (g->ops.tsg.store_sm_error_state != NULL) {
		return g->ops.tsg.store_sm_error_state(g, isr_data, sm_id,
				hww_global_esr, hww_warp_esr, hww_warp_esr_pc,
				hww_global_esr_report_mask, hww_warp_esr_report_mask);
	}

	tsg = nvgpu_tsg_check_and_get_from_id(g, tsgid);
	if (tsg == NULL) {
		nvgpu_err(g, "invalid tsgid %u", tsgid);
		return -EINVAL;
	}

	if (tsg->sm_error_states == NULL) {
		nvgpu_err(g, "invalid memory");
		return -ENOMEM;
	}

	sm_error_states = &tsg->sm_error_states[sm_id];
	sm_error_states->hww_global_esr = hww_global_esr;
	sm_error_states->hww_warp_esr = hww_warp_esr;
	sm_error_states->hww_warp_esr_pc = hww_warp_esr_pc;
	sm_error_states->hww_global_esr_report_mask = hww_global_esr_report_mask;
	sm_error_states->hww_warp_esr_report_mask = hww_warp_esr_report_mask;
	sm_error_states->hww_cga_esr = hww_cga_esr;

	return 0;
}

const struct nvgpu_tsg_sm_error_state *nvgpu_tsg_get_sm_error_state(struct nvgpu_tsg *tsg, u32 sm_id,
		u32 instance_id)
{
	struct gk20a *g = tsg->g;
	u32 num_of_sm = g->ops.gr.init.get_no_of_sm(g, instance_id);

	if (sm_id >= num_of_sm) {
		nvgpu_err(g, "Invalid number of SMs");
		return NULL;
	}

	if (tsg->sm_error_states == NULL) {
		nvgpu_err(g, "Invalid memory");
		return NULL;
	}

	return &tsg->sm_error_states[sm_id];
}

#ifdef CONFIG_NVGPU_DEBUGGER
int nvgpu_tsg_set_sm_exception_type_mask(struct nvgpu_channel *ch,
		u32 exception_mask)
{
	struct nvgpu_tsg *tsg;

	tsg = nvgpu_tsg_from_ch(ch);
	if (tsg == NULL) {
		return -EINVAL;
	}

	nvgpu_mutex_acquire(&tsg->sm_exception_mask_lock);
	tsg->sm_exception_mask_type = exception_mask;
	nvgpu_mutex_release(&tsg->sm_exception_mask_lock);

	return 0;
}
#endif

void nvgpu_tsg_abort(struct gk20a *g, struct nvgpu_tsg *tsg, bool preempt)
{
	struct nvgpu_channel *ch = NULL;

	nvgpu_log_fn(g, " ");

	WARN_ON(tsg->abortable == false);

	nvgpu_tsg_disable(tsg);

	if (preempt) {
		/*
		 * Ignore the return value below. If preempt fails, preempt_tsg
		 * operation will print the error and ctxsw timeout may trigger
		 * a recovery if needed.
		 */
		(void)nvgpu_tsg_preempt(g, tsg);
	}

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);
	nvgpu_list_for_each_entry(ch, &tsg->ch_list, nvgpu_channel, ch_entry) {
		if (nvgpu_channel_get(ch) != NULL) {
			nvgpu_channel_set_unserviceable(ch);
			if (g->ops.channel.abort_clean_up != NULL) {
				g->ops.channel.abort_clean_up(ch);
			}
			nvgpu_channel_put(ch);
		}
	}
	nvgpu_rwsem_up_read(&tsg->ch_list_lock);
}

void nvgpu_tsg_reset_faulted_eng_pbdma(struct gk20a *g, struct nvgpu_tsg *tsg)
{
	struct nvgpu_channel *ch;

	if (g->ops.channel.reset_faulted == NULL) {
		return;
	}

	if (tsg == NULL) {
		return;
	}

	nvgpu_log(g, gpu_dbg_info, "reset faulted eng and pbdma bits in ccsr");

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);
	nvgpu_list_for_each_entry(ch, &tsg->ch_list, nvgpu_channel, ch_entry) {
		g->ops.channel.reset_faulted(g, ch->runlist->id, ch->chid);
	}
	nvgpu_rwsem_up_read(&tsg->ch_list_lock);
}

#ifdef CONFIG_NVGPU_DEBUGGER
int nvgpu_tsg_set_mmu_debug_mode(struct nvgpu_channel *ch, bool enable)
{
	struct nvgpu_tsg *tsg = nvgpu_tsg_from_ch(ch);
	struct gk20a *g;
	int err = 0;

	if (tsg == NULL) {
		return -EINVAL;
	}
	g = ch->g;

	if ((g->ops.fb.set_mmu_debug_mode == NULL) &&
		(g->ops.gr.set_mmu_debug_mode == NULL)) {
		return -ENOSYS;
	}

	if (enable) {
		/* Enable the feature only if it is not already enabled. */
		if (nvgpu_atomic_read(&ch->mmu_debug_mode_refcnt) == 0) {
			nvgpu_atomic_inc(&ch->mmu_debug_mode_refcnt);
			nvgpu_atomic_inc(&tsg->mmu_debug_mode_refcnt);
			nvgpu_atomic_inc(&g->mmu_debug_mode_refcnt);
		} else {
			return 0;
		}
	} else {
		if (nvgpu_atomic_read(&ch->mmu_debug_mode_refcnt) != 0) {
			/* Need to disable only if the channel has the feature enabled. */
			nvgpu_atomic_dec(&ch->mmu_debug_mode_refcnt);

			if (nvgpu_atomic_read(&tsg->mmu_debug_mode_refcnt) != 0) {
				nvgpu_atomic_dec(&tsg->mmu_debug_mode_refcnt);
			}
			if (nvgpu_atomic_read(&g->mmu_debug_mode_refcnt) != 0) {
				nvgpu_atomic_dec(&g->mmu_debug_mode_refcnt);
			}
		} else {
			return 0;
		}
	}

	if (g->ops.gr.set_mmu_debug_mode != NULL) {
		/*
		 * enable GPC MMU debug mode if it was requested for at
		 * least one channel in the TSG
		 */
		err = g->ops.gr.set_mmu_debug_mode(g, ch,
					nvgpu_atomic_read(&tsg->mmu_debug_mode_refcnt) > 0,
					nvgpu_atomic_read(&tsg->mmu_debug_gcc_mode_refcnt) > 0);
		if (err != 0) {
			nvgpu_err(g, "set mmu debug mode failed, err=%d", err);
			return err;
		}
	}

	if (g->ops.fb.set_mmu_debug_mode != NULL) {
		/*
		 * enable FB/HS MMU debug mode if it was requested for
		 * at least one TSG
		 */
		g->ops.fb.set_mmu_debug_mode(g,
				nvgpu_atomic_read(&g->mmu_debug_mode_refcnt) > 0);
	}

	return err;
}

int nvgpu_tsg_set_mmu_debug_gcc_mode(struct nvgpu_channel *ch, bool enable)
{
	struct nvgpu_tsg *tsg = nvgpu_tsg_from_ch(ch);
	struct gk20a *g;
	int err = 0;

	if (tsg == NULL) {
		return -EINVAL;
	}
	g = ch->g;

	if (g->ops.gr.set_mmu_debug_mode == NULL) {
		return -ENOSYS;
	}

	if (enable) {
		/* Enable the feature only if it is not already enabled. */
		if (nvgpu_atomic_read(&ch->mmu_debug_gcc_mode_refcnt) == 0) {
			nvgpu_atomic_inc(&ch->mmu_debug_gcc_mode_refcnt);
			nvgpu_atomic_inc(&tsg->mmu_debug_gcc_mode_refcnt);
		} else {
			return 0;
		}
	} else {
		if (nvgpu_atomic_read(&ch->mmu_debug_gcc_mode_refcnt) != 0) {
			/* Need to disable only if the channel has the feature enabled. */
			nvgpu_atomic_dec(&ch->mmu_debug_gcc_mode_refcnt);

			if (nvgpu_atomic_read(&tsg->mmu_debug_gcc_mode_refcnt) != 0) {
				nvgpu_atomic_dec(&tsg->mmu_debug_gcc_mode_refcnt);
			}
		} else {
			return 0;
		}
	}

	/*
	 * enable GPC MMU debug mode if it was requested for at
	 * least one channel in the TSG
	 */
	err = g->ops.gr.set_mmu_debug_mode(g, ch,
				nvgpu_atomic_read(&tsg->mmu_debug_mode_refcnt) > 0,
				nvgpu_atomic_read(&tsg->mmu_debug_gcc_mode_refcnt) > 0);
	if (err != 0) {
		nvgpu_err(g, "set mmu debug gcc mode failed, err=%d", err);
	}

	return err;
}

int nvgpu_tsg_set_sched_exit_wait_for_errbar(struct nvgpu_channel *ch, bool enable)
{
	struct gk20a *g;
	int err = 0;
	struct nvgpu_tsg *tsg = nvgpu_tsg_from_ch(ch);

	if (tsg == NULL) {
		return -EINVAL;
	}
	g = ch->g;

	if (g->ops.gr.set_sched_wait_for_errbar == NULL) {
		return -ENOSYS;
	}

	if (enable) {
		nvgpu_atomic_inc(&ch->sched_exit_wait_for_errbar_refcnt);
		nvgpu_atomic_inc(&tsg->sched_exit_wait_for_errbar_refcnt);
	} else {
		if (nvgpu_atomic_read(&ch->sched_exit_wait_for_errbar_refcnt) != 0) {
			nvgpu_atomic_dec(&ch->sched_exit_wait_for_errbar_refcnt);
		}

		if (nvgpu_atomic_read(&tsg->sched_exit_wait_for_errbar_refcnt) != 0) {
			nvgpu_atomic_dec(&tsg->sched_exit_wait_for_errbar_refcnt);
		}
	}

	/*
	 * enable GPC implict ERRBAR if it was requested for at
	 * least one channel in the TSG
	 */
	err = g->ops.gr.set_sched_wait_for_errbar(g, ch,
			nvgpu_atomic_read(&tsg->sched_exit_wait_for_errbar_refcnt) > 0);
	if (err != 0) {
		nvgpu_err(g, "set implicit ERRBAR failed, err=%d", err);
		return err;
	}

	return err;
}
#endif

int nvgpu_tsg_preempt(struct gk20a *g, struct nvgpu_tsg *tsg)
{
	u32 runlist_id = tsg->runlist == NULL ? INVAL_ID : tsg->runlist->id;

	return g->ops.fifo.preempt_tsg(g, runlist_id, tsg->tsgid, 0);
}
