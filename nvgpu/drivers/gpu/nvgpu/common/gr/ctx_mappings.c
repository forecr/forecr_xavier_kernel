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

#include <nvgpu/gk20a.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/tsg_subctx.h>
#include <nvgpu/gr/subctx.h>
#include <nvgpu/gr/global_ctx.h>
#include <nvgpu/gr/ctx.h>
#include <nvgpu/gr/obj_ctx.h>
#include <nvgpu/gr/ctx_mappings.h>
#include <nvgpu/vm.h>
#include <nvgpu/io.h>
#include <nvgpu/gmmu.h>
#include <nvgpu/dma.h>
#include <nvgpu/string.h>
#include <nvgpu/list.h>
#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/fifo.h>

#include <nvgpu/power_features/pg.h>
#include "common/gr/ctx_mappings_priv.h"
#include "common/gr/subctx_priv.h"

static inline struct nvgpu_gr_ctx_mappings *
nvgpu_gr_ctx_mappings_from_tsg_entry(struct nvgpu_list_node *node)
{
	return (struct nvgpu_gr_ctx_mappings *)
	   ((uintptr_t)node - offsetof(struct nvgpu_gr_ctx_mappings, tsg_entry));
};

static inline struct nvgpu_gr_subctx *
nvgpu_gr_subctx_from_gr_ctx_mappings_entry(struct nvgpu_list_node *node)
{
	return (struct nvgpu_gr_subctx *)
	   ((uintptr_t)node - offsetof(struct nvgpu_gr_subctx, gr_ctx_mappings_entry));
};

struct nvgpu_gr_ctx_mappings *nvgpu_gr_ctx_mappings_create(struct gk20a *g,
				struct nvgpu_tsg *tsg, struct vm_gk20a *vm)
{
	struct nvgpu_gr_ctx_mappings *mappings = NULL;

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (tsg == NULL || vm == NULL) {
		return NULL;
	}

	mappings = (struct nvgpu_gr_ctx_mappings *)
			nvgpu_kzalloc(g, sizeof(struct nvgpu_gr_ctx_mappings));
	if (mappings == NULL) {
		nvgpu_err(g, "failed to alloc mappings");
		return NULL;
	}

	nvgpu_vm_get(vm);
	mappings->tsg = tsg;
	mappings->vm = vm;

	nvgpu_log(g, gpu_dbg_gr, "done");

	return mappings;
}

void nvgpu_gr_ctx_mappings_free(struct gk20a *g,
				struct nvgpu_gr_ctx_mappings *mappings)
{
	nvgpu_log(g, gpu_dbg_gr, " ");

	nvgpu_vm_put(mappings->vm);
	nvgpu_kfree(g, mappings);

	nvgpu_log(g, gpu_dbg_gr, "done");
}

int nvgpu_gr_ctx_mappings_map_ctx_buffer(struct gk20a *g,
	struct nvgpu_gr_ctx *ctx, u32 index,
	struct nvgpu_gr_ctx_mappings *mappings)
{
	struct vm_gk20a *vm = mappings->vm;
	struct nvgpu_mem *mem;
	u32 mapping_flags;
	u64 gpu_va;

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (mappings->ctx_buffer_va[index] != 0ULL) {
		nvgpu_log_info(g, "buffer %u already mapped", index);
		return 0;
	}

	mem = nvgpu_gr_ctx_get_ctx_mem(ctx, index);
	mapping_flags = nvgpu_gr_ctx_get_ctx_mapping_flags(ctx, index);

	if (nvgpu_mem_is_valid(mem)) {
		gpu_va = nvgpu_gmmu_map(vm,
				mem,
				mapping_flags,
				gk20a_mem_flag_none, true,
				mem->aperture);
		if (gpu_va == 0ULL) {
			nvgpu_err(g, "failed to map ctx buffer %u", index);
			return -ENOMEM;
		}

		mappings->ctx_buffer_va[index] = gpu_va;

		nvgpu_log(g, gpu_dbg_gr, "buffer[%u] mapped at address 0x%llx", index, gpu_va);

#ifdef CONFIG_NVGPU_DEBUGGER
		if (index == NVGPU_GR_CTX_PM_CTX) {
			nvgpu_gr_ctx_set_pm_ctx_mapped(ctx, true);
		}
#endif
	} else {
		nvgpu_log(g, gpu_dbg_gr, "buffer not allocated");
	}

	nvgpu_log(g, gpu_dbg_gr, "done");

	return 0;
}

void nvgpu_gr_ctx_mappings_unmap_ctx_buffer(struct nvgpu_gr_ctx *ctx,
	u32 index, struct nvgpu_gr_ctx_mappings *mappings)
{
	struct vm_gk20a *vm = mappings->vm;
	struct nvgpu_mem *mem;

	mem = nvgpu_gr_ctx_get_ctx_mem(ctx, index);

	if (nvgpu_mem_is_valid(mem) &&
	    (mappings->ctx_buffer_va[index] != 0ULL)) {
		nvgpu_gmmu_unmap_addr(vm, mem, mappings->ctx_buffer_va[index]);
		mappings->ctx_buffer_va[index] = 0ULL;

#ifdef CONFIG_NVGPU_DEBUGGER
		if (index == NVGPU_GR_CTX_PM_CTX) {
			nvgpu_gr_ctx_set_pm_ctx_mapped(ctx, false);
		}
#endif
	}
}

static void nvgpu_gr_ctx_mappings_unmap_buffer_all_subctx(
		struct nvgpu_tsg *tsg, u32 index)
{
	struct nvgpu_gr_ctx_mappings *mappings = NULL;
	struct nvgpu_gr_ctx *gr_ctx = tsg->gr_ctx;

	nvgpu_assert(index < NVGPU_GR_CTX_COUNT);

	nvgpu_list_for_each_entry(mappings, &tsg->gr_ctx_mappings_list,
				  nvgpu_gr_ctx_mappings, tsg_entry) {
		nvgpu_gr_ctx_mappings_unmap_ctx_buffer(gr_ctx,
				index, mappings);
	}
}

int nvgpu_gr_ctx_mappings_map_buffer_all_subctx(
		struct nvgpu_tsg *tsg, u32 index)
{
	struct nvgpu_gr_ctx_mappings *mappings = NULL;
	struct nvgpu_gr_ctx *gr_ctx = tsg->gr_ctx;
	struct gk20a *g = tsg->g;
	int err;

	nvgpu_assert(index < NVGPU_GR_CTX_COUNT);

	nvgpu_list_for_each_entry(mappings, &tsg->gr_ctx_mappings_list,
				  nvgpu_gr_ctx_mappings, tsg_entry) {
		err = nvgpu_gr_ctx_mappings_map_ctx_buffer(g, gr_ctx,
				index, mappings);
		if (err != 0) {
			nvgpu_err(g, "gr_ctx buffer %u map failed %d", index, err);
			nvgpu_gr_ctx_mappings_unmap_buffer_all_subctx(tsg, index);
			return err;
		}

	}

	return 0;
}

static void nvgpu_gr_ctx_mappings_unmap_ctx_buffers(struct nvgpu_gr_ctx *ctx,
	struct nvgpu_tsg_subctx *subctx,
	struct nvgpu_gr_ctx_mappings *mappings)
{
	u32 buffers_count = NVGPU_GR_CTX_COUNT;
	u32 i;
#ifdef CONFIG_NVGPU_GFXP
	struct nvgpu_tsg *tsg = mappings->tsg;
	struct gk20a *g = tsg->g;
	bool is_sync_veid;
	bool gfxp_active;
#endif

	(void) subctx;

#ifdef CONFIG_NVGPU_GFXP
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		is_sync_veid = nvgpu_tsg_subctx_get_id(subctx) ==
					CHANNEL_INFO_VEID0;
		gfxp_active = (nvgpu_gr_ctx_get_graphics_preemption_mode(ctx) ==
			       NVGPU_PREEMPTION_MODE_GRAPHICS_GFXP);

		if (is_sync_veid && gfxp_active) {
			nvgpu_gr_ctx_mappings_unmap_buffer_all_subctx(tsg,
				NVGPU_GR_CTX_PREEMPT_CTXSW);
			nvgpu_tsg_subctxs_clear_preemption_buffer_va(subctx);
			nvgpu_gr_ctx_init_graphics_preemption_mode(ctx,
				NVGPU_PREEMPTION_MODE_GRAPHICS_WFI);
		}

		if (!is_sync_veid) {
			if (gfxp_active) {
				nvgpu_gr_subctx_clear_preemption_buffer_va(g,
					nvgpu_tsg_subctx_get_gr_subctx(subctx));
				buffers_count = NVGPU_GR_CTX_PREEMPT_CTXSW + 1U;
			} else {
				buffers_count = NVGPU_GR_CTX_PATCH_CTX + 1U;
			}
		}
	}
#endif

	for (i = 0; i < buffers_count; i++) {
		nvgpu_gr_ctx_mappings_unmap_ctx_buffer(ctx, i, mappings);
	}
}

static int nvgpu_gr_ctx_mappings_map_ctx_buffers(struct gk20a *g,
	struct nvgpu_gr_ctx *ctx,
	struct nvgpu_tsg_subctx *subctx,
	struct nvgpu_gr_ctx_mappings *mappings)
{
	u32 buffers_count = NVGPU_GR_CTX_COUNT;
	int err = 0;
	u32 i;
#ifdef CONFIG_NVGPU_GFXP
	struct nvgpu_tsg *tsg = mappings->tsg;
	bool is_sync_veid;
	bool gfxp_active;
#endif

	(void) subctx;

#ifdef CONFIG_NVGPU_GFXP
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		is_sync_veid = nvgpu_tsg_subctx_get_id(subctx) ==
					CHANNEL_INFO_VEID0;
		gfxp_active = (nvgpu_gr_ctx_get_graphics_preemption_mode(ctx) ==
			       NVGPU_PREEMPTION_MODE_GRAPHICS_GFXP);

		if (is_sync_veid && gfxp_active) {
			err = nvgpu_gr_ctx_mappings_map_buffer_all_subctx(tsg,
					NVGPU_GR_CTX_PREEMPT_CTXSW);
			if (err != 0) {
				nvgpu_err(g, "preempt buffer mapping failed %d",
					  err);
				nvgpu_gr_ctx_mappings_unmap_buffer_all_subctx(
					tsg, NVGPU_GR_CTX_PREEMPT_CTXSW);
				return err;
			}
		}

		/*
		 * Only NVGPU_GR_CTX_PREEMPT_CTXSW is to be mapped for
		 * all VEIDs.
		 * Don't map other preemption buffers for ASYNC VEIDs.
		 */
		if (!is_sync_veid) {
			if (gfxp_active) {
				buffers_count = NVGPU_GR_CTX_PREEMPT_CTXSW + 1U;
			} else {
				buffers_count = NVGPU_GR_CTX_PATCH_CTX + 1U;
			}
		}
	}
#endif

	for (i = 0; i < buffers_count; i++) {
		err = nvgpu_gr_ctx_mappings_map_ctx_buffer(g, ctx, i, mappings);
		if (err != 0) {
			nvgpu_err(g, "gr_ctx buffer %u map failed %d", i, err);
			nvgpu_gr_ctx_mappings_unmap_ctx_buffers(ctx,
						subctx, mappings);
			return err;
		}
	}

	return err;
}

#ifdef CONFIG_NVGPU_GFXP
static void nvgpu_gr_ctx_mappings_unmap_ctx_preemption_buffers(
	struct nvgpu_gr_ctx *ctx,
	struct nvgpu_tsg_subctx *subctx,
	struct nvgpu_gr_ctx_mappings *mappings)
{
	u32 buffers_count = NVGPU_GR_CTX_GFXP_RTVCB_CTXSW;
	struct nvgpu_tsg *tsg = mappings->tsg;
	struct gk20a *g = tsg->g;
	bool is_sync_veid;
	bool gfxp_active;
	u32 i;

	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		is_sync_veid = nvgpu_tsg_subctx_get_id(subctx) ==
					CHANNEL_INFO_VEID0;
		gfxp_active = (nvgpu_gr_ctx_get_graphics_preemption_mode(ctx) ==
			       NVGPU_PREEMPTION_MODE_GRAPHICS_GFXP);

		if (is_sync_veid && gfxp_active) {
			nvgpu_gr_ctx_mappings_unmap_buffer_all_subctx(tsg,
				NVGPU_GR_CTX_PREEMPT_CTXSW);

			nvgpu_rwsem_down_read(&tsg->ch_list_lock);
			nvgpu_tsg_subctxs_clear_preemption_buffer_va(subctx);
			nvgpu_rwsem_up_read(&tsg->ch_list_lock);

			nvgpu_gr_ctx_init_graphics_preemption_mode(ctx,
				NVGPU_PREEMPTION_MODE_GRAPHICS_WFI);
		}

		if (!is_sync_veid) {
			if (gfxp_active) {
				nvgpu_gr_subctx_clear_preemption_buffer_va(g,
					nvgpu_tsg_subctx_get_gr_subctx(subctx));
				buffers_count = NVGPU_GR_CTX_PREEMPT_CTXSW;
			} else {
				return;
			}
		}
	}

	for (i = NVGPU_GR_CTX_PREEMPT_CTXSW; i <= buffers_count; i++) {
		nvgpu_gr_ctx_mappings_unmap_ctx_buffer(ctx, i, mappings);
	}
}

int nvgpu_gr_ctx_mappings_map_ctx_preemption_buffers(struct gk20a *g,
	struct nvgpu_gr_ctx *ctx,
	struct nvgpu_tsg_subctx *subctx,
	struct nvgpu_gr_ctx_mappings *mappings)
{
	u32 buffers_count = NVGPU_GR_CTX_GFXP_RTVCB_CTXSW;
	struct nvgpu_tsg *tsg = mappings->tsg;
	bool is_sync_veid;
	bool gfxp_active;
	int err = 0;
	u32 i;

	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		is_sync_veid = nvgpu_tsg_subctx_get_id(subctx) ==
					CHANNEL_INFO_VEID0;
		gfxp_active = (nvgpu_gr_ctx_get_graphics_preemption_mode(ctx) ==
			       NVGPU_PREEMPTION_MODE_GRAPHICS_GFXP);

		if (is_sync_veid && gfxp_active) {
			err = nvgpu_gr_ctx_mappings_map_buffer_all_subctx(tsg,
					NVGPU_GR_CTX_PREEMPT_CTXSW);
			if (err != 0) {
				nvgpu_err(g, "preempt buffer mapping failed %d", err);
				nvgpu_gr_ctx_mappings_unmap_buffer_all_subctx(tsg,
					NVGPU_GR_CTX_PREEMPT_CTXSW);
				return err;
			}
		}

		if (!is_sync_veid) {
			if (gfxp_active) {
				buffers_count = NVGPU_GR_CTX_PREEMPT_CTXSW;
			} else {
				return 0;
			}
		}
	}

	for (i = NVGPU_GR_CTX_PREEMPT_CTXSW; i <= buffers_count; i++) {
		err = nvgpu_gr_ctx_mappings_map_ctx_buffer(g, ctx, i, mappings);
		if (err != 0) {
			nvgpu_err(g, "gr_ctx buffer %u map failed %d", i, err);
			nvgpu_gr_ctx_mappings_unmap_ctx_preemption_buffers(ctx,
				subctx, mappings);
			return err;
		}
	}

	nvgpu_log(g, gpu_dbg_gr, "done");

	return err;
}
#endif

static int nvgpu_gr_ctx_mappings_map_global_ctx_buffer(
	struct nvgpu_gr_global_ctx_buffer_desc *global_ctx_buffer,
	u32 va_type, u32 buffer_type, u32 buffer_vpr_type,
	bool vpr, struct nvgpu_gr_ctx_mappings *mappings)
{
	struct vm_gk20a *vm = mappings->vm;
	struct gk20a *g = vm->mm->g;
	u64 *g_bfr_va;
	u32 *g_bfr_index;
	u64 gpu_va = 0ULL;

	(void)vpr;
	(void)buffer_vpr_type;

	g_bfr_va = &mappings->global_ctx_buffer_va[0];
	g_bfr_index = &mappings->global_ctx_buffer_index[0];

	if (g_bfr_va[va_type] != 0ULL) {
		nvgpu_log_info(g, "global buffer %u already mapped", va_type);
		return 0;
	}

#ifdef CONFIG_NVGPU_VPR
	if (vpr && nvgpu_gr_global_ctx_buffer_ready(global_ctx_buffer,
					buffer_vpr_type)) {
		gpu_va = nvgpu_gr_global_ctx_buffer_map(global_ctx_buffer,
					buffer_vpr_type,
					vm, true);
		g_bfr_index[va_type] = buffer_vpr_type;
	} else {
#endif
		gpu_va = nvgpu_gr_global_ctx_buffer_map(global_ctx_buffer,
					buffer_type,
					vm, true);
		g_bfr_index[va_type] = buffer_type;
#ifdef CONFIG_NVGPU_VPR
	}
#endif
	if (gpu_va == 0ULL) {
		goto clean_up;
	}

	g_bfr_va[va_type] = gpu_va;

	return 0;

clean_up:
	return -ENOMEM;
}

void nvgpu_gr_ctx_mappings_unmap_global_ctx_buffers(
	struct nvgpu_gr_global_ctx_buffer_desc *global_ctx_buffer,
	struct nvgpu_gr_ctx_mappings *mappings)
{
	u64 *g_bfr_va = &mappings->global_ctx_buffer_va[0];
	u32 *g_bfr_index = &mappings->global_ctx_buffer_index[0];
	struct vm_gk20a *vm = mappings->vm;
	u32 i;

	for (i = 0U; i < NVGPU_GR_GLOBAL_CTX_VA_COUNT; i++) {
		if (g_bfr_va[i] != 0ULL) {
			nvgpu_gr_global_ctx_buffer_unmap(global_ctx_buffer,
				g_bfr_index[i], vm, g_bfr_va[i]);
		}
	}

	(void) memset(g_bfr_va, 0, sizeof(mappings->global_ctx_buffer_va));
	(void) memset(g_bfr_index, 0, sizeof(mappings->global_ctx_buffer_index));
}

int nvgpu_gr_ctx_mappings_map_global_ctx_buffers(struct gk20a *g,
	struct nvgpu_gr_global_ctx_buffer_desc *global_ctx_buffer,
	bool support_gfx,
	struct nvgpu_gr_ctx_mappings *mappings,
	bool vpr)
{
	int err;

	/*
	 * MIG supports only compute class.
	 * Allocate BUNDLE_CB, PAGEPOOL, ATTRIBUTE_CB and RTV_CB
	 * if 2D/3D/I2M classes(graphics) are supported.
	 */
	if (support_gfx) {
		/* Circular Buffer */
		err = nvgpu_gr_ctx_mappings_map_global_ctx_buffer(
					global_ctx_buffer,
					NVGPU_GR_GLOBAL_CTX_CIRCULAR_VA,
					NVGPU_GR_GLOBAL_CTX_CIRCULAR,
#ifdef CONFIG_NVGPU_VPR
					NVGPU_GR_GLOBAL_CTX_CIRCULAR_VPR,
#else
					NVGPU_GR_GLOBAL_CTX_CIRCULAR,
#endif
					vpr, mappings);
		if (err != 0) {
			nvgpu_err(g, "cannot map ctx circular buffer");
			goto fail;
		}

		/* Attribute Buffer */
		err = nvgpu_gr_ctx_mappings_map_global_ctx_buffer(
					global_ctx_buffer,
					NVGPU_GR_GLOBAL_CTX_ATTRIBUTE_VA,
					NVGPU_GR_GLOBAL_CTX_ATTRIBUTE,
#ifdef CONFIG_NVGPU_VPR
					NVGPU_GR_GLOBAL_CTX_ATTRIBUTE_VPR,
#else
					NVGPU_GR_GLOBAL_CTX_ATTRIBUTE,
#endif
					vpr, mappings);
		if (err != 0) {
			nvgpu_err(g, "cannot map ctx attribute buffer");
			goto fail;
		}

		/* Page Pool */
		err = nvgpu_gr_ctx_mappings_map_global_ctx_buffer(
					global_ctx_buffer,
					NVGPU_GR_GLOBAL_CTX_PAGEPOOL_VA,
					NVGPU_GR_GLOBAL_CTX_PAGEPOOL,
#ifdef CONFIG_NVGPU_VPR
					NVGPU_GR_GLOBAL_CTX_PAGEPOOL_VPR,
#else
					NVGPU_GR_GLOBAL_CTX_PAGEPOOL,
#endif
					vpr, mappings);
		if (err != 0) {
			nvgpu_err(g, "cannot map ctx pagepool buffer");
			goto fail;
		}
#ifdef CONFIG_NVGPU_GRAPHICS
		/*
		 * RTV circular buffer. Note that this is non-VPR buffer always.
		 */
		if (nvgpu_gr_global_ctx_buffer_ready(global_ctx_buffer,
				NVGPU_GR_GLOBAL_CTX_RTV_CIRCULAR_BUFFER)) {
			err  = nvgpu_gr_ctx_mappings_map_global_ctx_buffer(
					global_ctx_buffer,
					NVGPU_GR_GLOBAL_CTX_RTV_CIRCULAR_BUFFER_VA,
					NVGPU_GR_GLOBAL_CTX_RTV_CIRCULAR_BUFFER,
					NVGPU_GR_GLOBAL_CTX_RTV_CIRCULAR_BUFFER,
					false, mappings);
			if (err != 0) {
				nvgpu_err(g,
					"cannot map ctx rtv circular buffer");
				goto fail;
			}
		}
#endif
	}

	/* Priv register Access Map. Note that this is non-VPR buffer always. */
	err  = nvgpu_gr_ctx_mappings_map_global_ctx_buffer(
			global_ctx_buffer,
			NVGPU_GR_GLOBAL_CTX_PRIV_ACCESS_MAP_VA,
			NVGPU_GR_GLOBAL_CTX_PRIV_ACCESS_MAP,
			NVGPU_GR_GLOBAL_CTX_PRIV_ACCESS_MAP,
			false, mappings);
	if (err != 0) {
		nvgpu_err(g, "cannot map ctx priv access buffer");
		goto fail;
	}

#ifdef CONFIG_NVGPU_FECS_TRACE
	/* FECS trace buffer. Note that this is non-VPR buffer always. */
	if (nvgpu_is_enabled(g, NVGPU_FECS_TRACE_VA)) {
		err  = nvgpu_gr_ctx_mappings_map_global_ctx_buffer(
			global_ctx_buffer,
			NVGPU_GR_GLOBAL_CTX_FECS_TRACE_BUFFER_VA,
			NVGPU_GR_GLOBAL_CTX_FECS_TRACE_BUFFER,
			NVGPU_GR_GLOBAL_CTX_FECS_TRACE_BUFFER,
			false, mappings);
		if (err != 0) {
			nvgpu_err(g, "cannot map ctx fecs trace buffer");
			goto fail;
		}
	}
#endif

	return 0;

fail:
	nvgpu_gr_ctx_mappings_unmap_global_ctx_buffers(
		global_ctx_buffer, mappings);
	return err;
}

int nvgpu_gr_ctx_mappings_map_gr_ctx_buffers(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx, struct nvgpu_tsg_subctx *subctx,
	struct nvgpu_gr_global_ctx_buffer_desc *global_ctx_buffer,
	struct nvgpu_gr_ctx_mappings *mappings,
	bool vpr)
{
	int err;

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (gr_ctx == NULL || global_ctx_buffer == NULL ||
	    mappings == NULL) {
		nvgpu_err(g, "mappings/gr_ctx/global_ctx_buffer struct null");
		return -EINVAL;
	}

	err = nvgpu_gr_ctx_mappings_map_ctx_buffers(g, gr_ctx, subctx, mappings);
	if (err != 0) {
		nvgpu_err(g, "fail to map ctx buffers");
		return err;
	}

	err = nvgpu_gr_ctx_mappings_map_global_ctx_buffers(g,
			global_ctx_buffer,
			nvgpu_gr_obj_ctx_is_gfx_engine(g, subctx),
			mappings, vpr);
	if (err != 0) {
		nvgpu_err(g, "fail to map global ctx buffer");
		nvgpu_gr_ctx_mappings_unmap_ctx_buffers(gr_ctx, subctx, mappings);
		return err;
	}

	nvgpu_log(g, gpu_dbg_gr, "done");

	return err;
}

void nvgpu_gr_ctx_unmap_buffers(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx,
	struct nvgpu_tsg_subctx *subctx,
	struct nvgpu_gr_global_ctx_buffer_desc *global_ctx_buffer,
	struct nvgpu_gr_ctx_mappings *mappings)
{
	nvgpu_log(g, gpu_dbg_gr, " ");

	nvgpu_gr_ctx_mappings_unmap_global_ctx_buffers(global_ctx_buffer,
		mappings);

	nvgpu_gr_ctx_mappings_unmap_ctx_buffers(gr_ctx, subctx, mappings);

	nvgpu_log(g, gpu_dbg_gr, "done");
}

u64 nvgpu_gr_ctx_mappings_get_global_ctx_va(struct nvgpu_gr_ctx_mappings *mappings,
	u32 index)
{
	nvgpu_assert(index < NVGPU_GR_GLOBAL_CTX_VA_COUNT);
	return mappings->global_ctx_buffer_va[index];
}

u64 nvgpu_gr_ctx_mappings_get_ctx_va(struct nvgpu_gr_ctx_mappings *mappings,
	u32 index)
{
	nvgpu_assert(index < NVGPU_GR_CTX_COUNT);
	return mappings->ctx_buffer_va[index];
}

struct nvgpu_gr_ctx_mappings *nvgpu_gr_ctx_mappings_get_subctx_mappings(
				struct gk20a *g,
				struct nvgpu_tsg *tsg,
				struct vm_gk20a *vm)
{
	struct nvgpu_gr_ctx_mappings *mappings = NULL;

	nvgpu_log(g, gpu_dbg_gr, " ");

	nvgpu_list_for_each_entry(mappings, &tsg->gr_ctx_mappings_list,
				nvgpu_gr_ctx_mappings, tsg_entry) {
		if (mappings->vm == vm) {
			return mappings;
		}
	}

	nvgpu_log(g, gpu_dbg_gr, "done");

	return NULL;
}

struct nvgpu_gr_ctx_mappings *nvgpu_gr_ctx_mappings_create_subctx_mappings(
				struct gk20a *g,
				struct nvgpu_tsg *tsg,
				struct vm_gk20a *vm)
{
	struct nvgpu_gr_ctx_mappings *mappings = NULL;

	nvgpu_log(g, gpu_dbg_gr, " ");

	mappings = (struct nvgpu_gr_ctx_mappings *)
			nvgpu_kzalloc(g, sizeof(struct nvgpu_gr_ctx_mappings));
	if (mappings == NULL) {
		nvgpu_err(g, "failed to alloc mappings");
		return NULL;
	}

	nvgpu_vm_get(vm);
	mappings->tsg = tsg;
	mappings->vm = vm;

	nvgpu_init_list_node(&mappings->tsg_entry);
	nvgpu_init_list_node(&mappings->subctx_list);

	/* add mappings to the list in the tsg */
	nvgpu_list_add_tail(&mappings->tsg_entry,
			    &tsg->gr_ctx_mappings_list);

	nvgpu_log(g, gpu_dbg_gr, "done");

	return mappings;
}

void nvgpu_gr_ctx_mappings_add_gr_subctx(struct nvgpu_gr_ctx_mappings *mappings,
				   struct nvgpu_gr_subctx *subctx)
{
	struct nvgpu_gr_subctx *subctx_iter = NULL;
	struct nvgpu_tsg *tsg = mappings->tsg;
	struct gk20a *g = tsg->g;
	bool found = false;

	nvgpu_log(g, gpu_dbg_gr, " ");

	nvgpu_list_for_each_entry(subctx_iter, &mappings->subctx_list,
				nvgpu_gr_subctx, gr_ctx_mappings_entry) {
		if (subctx_iter == subctx) {
			found = true;
			goto out;
		}
	}

out:
	if (!found) {
		subctx->mappings = mappings;
		nvgpu_list_add_tail(&subctx->gr_ctx_mappings_entry,
				    &mappings->subctx_list);
	}

	nvgpu_log(g, gpu_dbg_gr, "done");
}

void nvgpu_gr_ctx_mappings_free_subctx_mappings(struct nvgpu_tsg_subctx *subctx,
			struct nvgpu_gr_ctx_mappings *mappings, bool unmap)
{
	struct nvgpu_tsg *tsg = mappings->tsg;
	struct nvgpu_gr_ctx *gr_ctx = tsg->gr_ctx;
	struct gk20a *g = tsg->g;
	struct nvgpu_gr_global_ctx_buffer_desc *global_ctx_buffer =
				nvgpu_gr_get_global_ctx_buffer_ptr(g);
	bool is_sync_veid;

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (nvgpu_list_empty(&mappings->subctx_list)) {
		if (unmap) {
			nvgpu_gr_ctx_unmap_buffers(g,
				gr_ctx, subctx, global_ctx_buffer, mappings);
		}

		/* remove mappings from the list in the tsg */
		nvgpu_list_del(&mappings->tsg_entry);

		nvgpu_gr_ctx_mappings_free(g, mappings);
	}

	is_sync_veid = nvgpu_tsg_subctx_get_id(subctx) == CHANNEL_INFO_VEID0;

	if (is_sync_veid) {
		nvgpu_gr_obj_ctx_set_global_ctx_buffers_patched(gr_ctx, false);
		nvgpu_gr_obj_ctx_set_preempt_buffers_patched(gr_ctx, false);
	}

	nvgpu_log(g, gpu_dbg_gr, "done");
}
