// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/tsg.h>
#include <nvgpu/tsg_subctx.h>
#include <nvgpu/channel.h>
#include <nvgpu/enabled.h>
#include <nvgpu/kmem.h>
#include <nvgpu/list.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/gr/subctx.h>
#include <nvgpu/gr/ctx_mappings.h>

#include "tsg_subctx_priv.h"

/**
 * @brief Retrieve the TSG subcontext structure from a TSG entry node.
 *
 * This function computes the starting address of the TSG subcontext structure
 * given a pointer to a member (tsg_entry) within that structure. It uses the
 * standard offsetof macro to determine the byte offset of the tsg_entry member
 * within the nvgpu_tsg_subctx structure, and then subtracts this offset from
 * the provided node pointer to obtain the base address of the containing
 * structure.
 *
 * @param [in] node  Pointer to the tsg_entry member within the TSG subcontext
 *                   structure.
 *
 * @return Pointer to the containing nvgpu_tsg_subctx structure.
 *
 * Steps:
 * -# Calculate the byte offset of the tsg_entry member within the
 *    nvgpu_tsg_subctx structure using the offsetof() macro.
 * -# Subtract the offset from the provided node pointer to get the base address
 *    of the nvgpu_tsg_subctx structure.
 * -# Cast the computed address to a pointer of type struct nvgpu_tsg_subctx*.
 * -# Return the pointer to the nvgpu_tsg_subctx structure.
 */
static inline struct nvgpu_tsg_subctx *
nvgpu_tsg_subctx_from_tsg_entry(struct nvgpu_list_node *node)
{
	return (struct nvgpu_tsg_subctx *)
	   ((uintptr_t)node - offsetof(struct nvgpu_tsg_subctx, tsg_entry));
};

/**
 * @brief Retrieve the TSG subcontext structure with a specific subcontext ID.
 *
 * This function iterates over the list of subcontexts in a TSG and returns the
 * subcontext that matches the provided subcontext ID. If no matching subcontext
 * is found, it returns NULL.
 *
 * @param [in] tsg        Pointer to the TSG structure containing the subcontext list.
 * @param [in] subctx_id  The subcontext ID to search for.
 *
 * @return Pointer to the matching nvgpu_tsg_subctx structure, or NULL if not found.
 *
 * Steps:
 * -# Initialize a pointer to a nvgpu_tsg_subctx structure to NULL. This will
 *    be used to store the result.
 * -# Use nvgpu_list_for_each_entry() to iterate over each subcontext in the
 *    TSG's subcontext list.
 *    - The iteration uses the tsg_entry member of the nvgpu_tsg_subctx structure
 *      to traverse the list.
 * -# Inside the loop, check if the current subcontext's ID matches the provided
 *    subctx_id.
 *    - If a match is found, the current subcontext's pointer is returned.
 * -# After the loop, return NULL as no match was found.
 */
static struct nvgpu_tsg_subctx *nvgpu_tsg_subctx_from_id(struct nvgpu_tsg *tsg,
					u32 subctx_id)
{
	struct nvgpu_tsg_subctx *subctx = NULL;

	nvgpu_list_for_each_entry(subctx, &tsg->subctx_list,
				  nvgpu_tsg_subctx, tsg_entry) {
		if (subctx->subctx_id == subctx_id) {
			return subctx;
		}
	}

	return NULL;
}

bool nvgpu_tsg_subctx_has_channels_bound(struct nvgpu_tsg *tsg, u32 subctx_id)
{
	struct nvgpu_tsg_subctx *subctx = NULL;
	struct gk20a *g = tsg->g;
	bool chs_bound;

	nvgpu_log(g, gpu_dbg_gr, " ");

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);

	subctx = nvgpu_tsg_subctx_from_id(tsg, subctx_id);
	if (subctx == NULL) {
		nvgpu_log_info(g, "Subctx %u not allocated", subctx_id);
		nvgpu_rwsem_up_read(&tsg->ch_list_lock);
		return false;
	}

	chs_bound = !nvgpu_list_empty(&subctx->ch_list);

	nvgpu_rwsem_up_read(&tsg->ch_list_lock);

	return chs_bound;
}

int nvgpu_tsg_subctx_bind_channel(struct nvgpu_tsg *tsg,
				  struct nvgpu_channel *ch)
{
	struct nvgpu_tsg_subctx *subctx = NULL;
	struct gk20a *g = tsg->g;

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return 0;
	}

	subctx = nvgpu_tsg_subctx_from_id(tsg, ch->subctx_id);
	if (subctx != NULL) {
		if (subctx->vm != ch->vm) {
			nvgpu_err(g, "subctx vm mismatch");
			return -EINVAL;
		}

		goto add_ch_subctx;
	}

	nvgpu_log(g, gpu_dbg_gr, "Allocating subctx %u", ch->subctx_id);

	subctx = nvgpu_kzalloc(g, sizeof(struct nvgpu_tsg_subctx));
	if (subctx == NULL) {
		nvgpu_err(g, "Failed to allocate subctx");
		return -ENOMEM;
	}

	subctx->subctx_id = ch->subctx_id;
	subctx->tsg = tsg;
	subctx->vm = ch->vm;
	nvgpu_init_list_node(&subctx->ch_list);
	nvgpu_init_list_node(&subctx->tsg_entry);

	nvgpu_list_add_tail(&subctx->tsg_entry, &tsg->subctx_list);

add_ch_subctx:
	ch->subctx = subctx;
	nvgpu_list_add_tail(&ch->subctx_entry, &subctx->ch_list);

	nvgpu_log(g, gpu_dbg_gr, "done");

	return 0;
}

void nvgpu_tsg_subctx_unbind_channel(struct nvgpu_tsg *tsg,
				     struct nvgpu_channel *ch, bool force)
{
	struct nvgpu_tsg_subctx *subctx;
	struct gk20a *g = tsg->g;

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return;
	}

	subctx = ch->subctx;
	nvgpu_assert(subctx != NULL);

	nvgpu_list_del(&ch->subctx_entry);

	if (nvgpu_list_empty(&subctx->ch_list)) {
		if (force) {
			nvgpu_tsg_delete_subcontext(g, tsg, ch->subctx_id);
		}

		if (g->ops.tsg.remove_subctx_channel_hw != NULL) {
			g->ops.tsg.remove_subctx_channel_hw(ch);
		}

		if (g->ops.gr.setup.free_subctx != NULL) {
			g->ops.gr.setup.free_subctx(ch);
			subctx->gr_subctx = NULL;
		}

		nvgpu_list_del(&subctx->tsg_entry);
		nvgpu_kfree(tsg->g, subctx);
	}

	ch->subctx = NULL;

	nvgpu_log(g, gpu_dbg_gr, "done");
}

int nvgpu_tsg_subctx_alloc_gr_subctx(struct gk20a *g, struct nvgpu_channel *ch)
{
	struct nvgpu_tsg_subctx *subctx;

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return 0;
	}

	subctx = ch->subctx;
	if (subctx == NULL) {
		nvgpu_err(g, "channel not bound to TSG subctx");
		return -EINVAL;
	}

	if (subctx->gr_subctx == NULL) {
		subctx->gr_subctx = nvgpu_gr_subctx_alloc(g);
		if (subctx->gr_subctx == NULL) {
			nvgpu_err(g, "gr_subctx alloc failed");
			return -ENOMEM;
		}
	}

	nvgpu_log(g, gpu_dbg_gr, "done");

	return 0;
}

int nvgpu_tsg_subctx_setup_subctx_header(struct gk20a *g,
					 struct nvgpu_channel *ch)
{
	struct nvgpu_tsg_subctx *subctx;
	int err;

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return 0;
	}

	subctx = ch->subctx;
	if ((subctx == NULL) || (subctx->gr_subctx == NULL)) {
		nvgpu_err(g, "channel not bound to TSG/GR subctx");
		return -EINVAL;
	}

	err = nvgpu_gr_subctx_setup_header(g, subctx->gr_subctx, subctx->vm);
	if (err != 0) {
		nvgpu_err(g, "gr_subctx header setup failed %d", err);
		return err;
	}

	nvgpu_log(g, gpu_dbg_gr, "done");

	return 0;
}

struct nvgpu_gr_subctx *nvgpu_tsg_subctx_get_gr_subctx(
				struct nvgpu_tsg_subctx *subctx)
{
	return subctx->gr_subctx;
}

u32 nvgpu_tsg_subctx_get_id(struct nvgpu_tsg_subctx *subctx)
{
	return subctx->subctx_id;
}

void nvgpu_tsg_subctx_set_replayable(struct nvgpu_tsg_subctx *subctx,
				     bool replayable)
{
	subctx->replayable = replayable;
}

bool nvgpu_tsg_subctx_get_replayable(struct nvgpu_tsg_subctx *subctx)
{
	return subctx->replayable;
}

struct vm_gk20a *nvgpu_tsg_subctx_get_vm(struct nvgpu_tsg_subctx *subctx)
{
	return subctx->vm;
}

struct nvgpu_gr_ctx_mappings *nvgpu_tsg_subctx_alloc_or_get_mappings(
				struct gk20a *g,
				struct nvgpu_tsg *tsg,
				struct nvgpu_channel *ch)
{
	struct nvgpu_gr_ctx_mappings *mappings = NULL;
	struct nvgpu_gr_subctx *gr_subctx = NULL;
	struct vm_gk20a *vm = ch->vm;

	nvgpu_log(g, gpu_dbg_gr, " ");

	nvgpu_assert(ch->subctx != NULL);
	nvgpu_assert(ch->subctx->vm == vm);

	mappings = nvgpu_gr_ctx_mappings_get_subctx_mappings(g, tsg, vm);
	if (mappings != NULL) {
		goto add_gr_subctx;
	}

	mappings = nvgpu_gr_ctx_mappings_create_subctx_mappings(g, tsg, vm);
	if (mappings == NULL) {
		nvgpu_err(g, "failed to allocate gr_ctx mappings");
		return NULL;
	}

add_gr_subctx:
	gr_subctx = nvgpu_tsg_subctx_get_gr_subctx(ch->subctx);
	nvgpu_assert(gr_subctx != NULL);

	nvgpu_gr_ctx_mappings_add_gr_subctx(mappings, gr_subctx);

	nvgpu_log(g, gpu_dbg_gr, "done");

	return mappings;
}

#ifdef CONFIG_NVGPU_GFXP
static struct nvgpu_gr_ctx_mappings *nvgpu_tsg_subctx_get_veid0_mappings(
				struct gk20a *g,
				struct nvgpu_tsg *tsg)
{
	struct nvgpu_gr_ctx_mappings *mappings = NULL;
	struct nvgpu_tsg_subctx *subctx = NULL;

	subctx = nvgpu_tsg_subctx_from_id(tsg, CHANNEL_INFO_VEID0);
	if (subctx == NULL) {
		nvgpu_log(g, gpu_dbg_gr, "VEID0 subctx not available");
		return NULL;
	}

	mappings = nvgpu_gr_subctx_get_mappings(subctx->gr_subctx);
	if (mappings == NULL) {
		nvgpu_log(g, gpu_dbg_gr, "VEID0 mappings not available");
		return NULL;
	}

	return mappings;
}

void nvgpu_tsg_subctxs_set_preemption_buffer_va(
			struct nvgpu_tsg_subctx *tsg_subctx)
{
	struct nvgpu_gr_ctx_mappings *veid0_mappings;
	struct nvgpu_tsg_subctx *subctx = NULL;
	struct nvgpu_tsg *tsg = tsg_subctx->tsg;
	struct gk20a *g = tsg->g;

	nvgpu_log(g, gpu_dbg_gr, " ");

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);

	veid0_mappings = nvgpu_tsg_subctx_get_veid0_mappings(g, tsg);
	if (veid0_mappings == NULL) {
		nvgpu_rwsem_up_read(&tsg->ch_list_lock);
		return;
	}

	nvgpu_list_for_each_entry(subctx, &tsg->subctx_list,
				  nvgpu_tsg_subctx, tsg_entry) {
		if (subctx->gr_subctx != NULL) {
			nvgpu_gr_subctx_set_preemption_buffer_va(g,
				subctx->gr_subctx, veid0_mappings);
		}
	}

	nvgpu_rwsem_up_read(&tsg->ch_list_lock);

	nvgpu_log(g, gpu_dbg_gr, "done");
}

void nvgpu_tsg_subctxs_clear_preemption_buffer_va(
			struct nvgpu_tsg_subctx *tsg_subctx)
{
	struct nvgpu_tsg_subctx *subctx = NULL;
	struct nvgpu_tsg *tsg = tsg_subctx->tsg;
	struct gk20a *g = tsg->g;

	nvgpu_log(g, gpu_dbg_gr, " ");

	nvgpu_list_for_each_entry(subctx, &tsg->subctx_list,
				  nvgpu_tsg_subctx, tsg_entry) {
		if (subctx->gr_subctx != NULL) {
			nvgpu_gr_subctx_clear_preemption_buffer_va(g,
				subctx->gr_subctx);
		}
	}

	nvgpu_log(g, gpu_dbg_gr, "done");
}
#endif /* CONFIG_NVGPU_GFXP */

#ifdef CONFIG_NVGPU_DEBUGGER
void nvgpu_tsg_subctxs_set_pm_buffer_va(struct nvgpu_tsg *tsg,
					bool set_pm_ctx_gpu_va)
{
	struct nvgpu_tsg_subctx *subctx = NULL;
	struct gk20a *g = tsg->g;

	nvgpu_log(g, gpu_dbg_gr, " ");

	nvgpu_rwsem_down_read(&tsg->ch_list_lock);
	nvgpu_list_for_each_entry(subctx, &tsg->subctx_list,
				  nvgpu_tsg_subctx, tsg_entry) {
		if (subctx->gr_subctx != NULL) {
			nvgpu_gr_subctx_set_hwpm_ptr(g, subctx->gr_subctx,
						     set_pm_ctx_gpu_va);
		}
	}
	nvgpu_rwsem_up_read(&tsg->ch_list_lock);

	nvgpu_log(g, gpu_dbg_gr, "done");
}
#endif /* CONFIG_NVGPU_DEBUGGER */

/**
 * @brief Retrieve the channel structure from a subcontext entry node.
 *
 * This function computes the starting address of the channel structure given a
 * pointer to a member (subctx_entry) within that structure. It uses the
 * offsetof() macro to determine the byte offset of the subctx_entry member
 * within the nvgpu_channel structure, and then subtracts this offset from the
 * provided node pointer to obtain the base address of the containing structure.
 *
 * @param [in] node  Pointer to the subctx_entry member within the channel
 *                   structure.
 *
 * @return Pointer to the containing nvgpu_channel structure.
 *
 * Steps:
 * -# Calculate the byte offset of the subctx_entry member within the
 *    nvgpu_channel structure using the offsetof() macro.
 * -# Subtract the offset from the provided node pointer to get the base address
 *    of the nvgpu_channel structure.
 * -# Cast the computed address to a pointer of type struct nvgpu_channel*.
 * -# Return the pointer to the nvgpu_channel structure.
 */
static inline struct nvgpu_channel *
nvgpu_channel_from_subctx_entry(struct nvgpu_list_node *node)
{
	return (struct nvgpu_channel *)
	   ((uintptr_t)node - offsetof(struct nvgpu_channel, subctx_entry));
};

bool nvgpu_tsg_channel_type_active(struct nvgpu_tsg *tsg,
				   bool match_subctx, u32 subctx_id,
				   bool match_pbdma, u32 pbdma_id,
				   bool (*is_valid_class)(u32 class_num))
{
	struct nvgpu_tsg_subctx *subctx = NULL;
	bool channel_active = false;
	struct gk20a *g = tsg->g;
	struct nvgpu_channel *ch;

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (is_valid_class == NULL) {
		return false;
	}

	nvgpu_rwsem_down_write(&tsg->ch_list_lock);

	nvgpu_list_for_each_entry(subctx, &tsg->subctx_list,
				nvgpu_tsg_subctx, tsg_entry) {
		if ((match_subctx && (subctx->subctx_id == subctx_id)) ||
		    (!match_subctx && (subctx->subctx_id != subctx_id))) {

			nvgpu_list_for_each_entry(ch, &subctx->ch_list,
						nvgpu_channel, subctx_entry) {
				if ((*is_valid_class)(ch->obj_class)) {

					if ((match_pbdma && (ch->runqueue_sel == pbdma_id)) ||
					    (!match_pbdma && (ch->runqueue_sel != pbdma_id))) {
						channel_active = true;
						break;
					}
				}
			}

			if (channel_active == true) {
				break;
			}
		}
	}

	nvgpu_rwsem_up_write(&tsg->ch_list_lock);

	nvgpu_log(g, gpu_dbg_gr, "done");

	return channel_active;
}
