/*
 * Copyright (c) 2019-2023, NVIDIA CORPORATION.  All rights reserved.
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

#include <nvgpu/log.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/gr/ctx.h>
#include <nvgpu/gr/ctx_mappings.h>
#include <nvgpu/gr/subctx.h>
#include <nvgpu/gr/obj_ctx.h>
#ifdef CONFIG_NVGPU_GRAPHICS
#include <nvgpu/gr/zcull.h>
#endif
#include <nvgpu/gr/setup.h>
#include <nvgpu/gr/gr_instances.h>
#include <nvgpu/channel.h>
#include <nvgpu/preempt.h>
#include <nvgpu/tsg_subctx.h>

#include "gr_priv.h"

#ifdef CONFIG_NVGPU_GRAPHICS
static int nvgpu_gr_setup_zcull(struct gk20a *g, struct nvgpu_channel *c,
				struct nvgpu_gr_ctx *gr_ctx)
{
	struct nvgpu_tsg *tsg = nvgpu_tsg_from_ch(c);
	int ret = 0;

	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&tsg->ctx_init_lock);

	ret = nvgpu_channel_disable_tsg(g, c);
	if (ret != 0) {
		nvgpu_mutex_release(&tsg->ctx_init_lock);
		nvgpu_err(g, "failed to disable channel/TSG");
		return ret;
	}

	ret = nvgpu_preempt_channel(g, c);
	if (ret != 0) {
		nvgpu_err(g, "failed to preempt channel/TSG");
		goto out;
	}

	ret = nvgpu_gr_zcull_ctx_setup(g, c->subctx, gr_ctx);
	if (ret != 0) {
		nvgpu_err(g, "failed to setup zcull");
		goto out;
	}
	/* no error at this point */
	ret = nvgpu_channel_enable_tsg(g, c);
	if (ret != 0) {
		nvgpu_err(g, "failed to re-enable channel/TSG");
	}

	nvgpu_mutex_release(&tsg->ctx_init_lock);

	return ret;

out:
	/*
	 * control reaches here if preempt failed or nvgpu_gr_zcull_ctx_setup
	 * failed. Propagate preempt failure err or err for
	 * nvgpu_gr_zcull_ctx_setup
	 */
	if (nvgpu_channel_enable_tsg(g, c) != 0) {
		/* ch might not be bound to tsg */
		nvgpu_err(g, "failed to enable channel/TSG");
	}

	nvgpu_mutex_release(&tsg->ctx_init_lock);

	return ret;
}

int nvgpu_gr_setup_bind_ctxsw_zcull(struct gk20a *g, struct nvgpu_channel *c,
			u64 zcull_va, u32 mode)
{
	struct nvgpu_tsg *tsg;
	struct nvgpu_gr_ctx *gr_ctx;

	tsg = nvgpu_tsg_from_ch(c);
	if (tsg == NULL) {
		return -EINVAL;
	}

	gr_ctx = tsg->gr_ctx;

	/*
	 * Each TSG shares same context with all the channels in the tsg
	 * and zcull cannot be set per channel. If any channel tries
	 * to add a second zcull buffer, it will be ignored.
	 * See Bug 3364302.
	 * TODO - https://jirasw.nvidia.com/browse/NVGPU-451
	 * When full subcontext(multiple VA) is supported by TSG
	 * then each channel can have separate VA address for same
	 * physical zcull buffer but then zcull va ptr cannot be stored
	 * at gr_ctx level and current design needs to be re-worked.
	 */
	if (nvgpu_gr_ctx_get_zcull_ctx_va(gr_ctx) != 0ULL) {
		nvgpu_log(g, gpu_dbg_info,
			"zcull bind is ignored for already bound ctx");
		return 0;
	}
	nvgpu_gr_ctx_set_zcull_ctx(g, gr_ctx, mode, zcull_va);

	return nvgpu_gr_setup_zcull(g, c, gr_ctx);
}
#endif

static int nvgpu_gr_setup_validate_channel_and_class(struct gk20a *g,
					struct nvgpu_channel *c, u32 class_num)
{
	int err = 0;

	/* an address space needs to have been bound at this point.*/
	if (!nvgpu_channel_as_bound(c)) {
		nvgpu_err(g,
			   "not bound to address space at time"
			   " of grctx allocation");
		return -EINVAL;
	}

	if (!g->ops.gpu_class.is_valid(class_num)) {
		nvgpu_err(g,
			   "invalid obj class 0x%x", class_num);
		err = -EINVAL;
	}

	return err;
}

int nvgpu_gr_setup_alloc_obj_ctx(struct nvgpu_channel *c, u32 class_num,
		u32 flags)
{
	struct gk20a *g = c->g;
	struct nvgpu_gr_ctx *gr_ctx;
	struct nvgpu_tsg *tsg = NULL;
	int err = 0;
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr(g);
	struct nvgpu_gr_ctx_mappings *mappings = NULL;
#ifdef CONFIG_NVGPU_FECS_TRACE
	struct nvgpu_gr_subctx *gr_subctx = NULL;
#endif

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gr,
		"GR%u: allocate object context for channel %u",
		gr->instance_id, c->chid);

	err = nvgpu_gr_setup_validate_channel_and_class(g, c, class_num);
	if (err != 0) {
		goto out;
	}

	c->obj_class = class_num;

#ifndef CONFIG_NVGPU_NON_FUSA
	/*
	 * Only compute and graphics classes need object context.
	 * Return success for valid non-compute and non-graphics classes.
	 * Invalid classes are already captured in
	 * nvgpu_gr_setup_validate_channel_and_class() function.
	 */
	if (!g->ops.gpu_class.is_valid_compute(class_num) &&
	    !g->ops.gpu_class.is_valid_gfx(class_num)) {
		return 0;
	}
#endif

	tsg = nvgpu_tsg_from_ch(c);
	if (tsg == NULL) {
		return -EINVAL;
	}

	nvgpu_mutex_acquire(&tsg->ctx_init_lock);

	g->ops.tsg.disable(tsg);

	err = nvgpu_tsg_preempt(g, tsg);
	if (err != 0) {
		nvgpu_err(g, "preempt failed %d", err);
		goto enable_tsg;
	}

	err = nvgpu_tsg_validate_class_veid_pbdma(c);
	if (err != 0) {
		nvgpu_err(g, "Invalid class/veid/pbdma config");
		goto enable_tsg;
	}

	err = nvgpu_tsg_subctx_alloc_gr_subctx(g, c);
	if (err != 0) {
		nvgpu_err(g, "failed to alloc gr subctx");
		goto enable_tsg;
	}

	err = nvgpu_tsg_subctx_setup_subctx_header(g, c);
	if (err != 0) {
		nvgpu_err(g, "failed to setup subctx header");
		goto enable_tsg;
	}

	gr_ctx = tsg->gr_ctx;

	mappings = nvgpu_gr_ctx_alloc_or_get_mappings(g, tsg, c);
	if (mappings == NULL) {
		nvgpu_err(g, "fail to allocate/get ctx mappings struct");
		goto enable_tsg;
	}

	err = nvgpu_gr_obj_ctx_alloc(g, gr->golden_image,
			gr->global_ctx_buffer, gr->gr_ctx_desc,
			gr->config, gr_ctx, c, c->subctx,
			mappings, &c->inst_block, class_num, flags,
			c->cde, c->vpr);
	if (err != 0) {
		nvgpu_err(g,
			"failed to allocate gr ctx buffer");
		goto enable_tsg;
	}

	nvgpu_gr_ctx_set_tsgid(gr_ctx, tsg->tsgid);

#ifdef CONFIG_NVGPU_FECS_TRACE
	if (g->ops.gr.fecs_trace.bind_channel && !c->vpr) {
		u32 vmid = 0U;

		if (nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
			gr_subctx = nvgpu_tsg_subctx_get_gr_subctx(c->subctx);
		}

		if (g->ops.channel.get_vmid != NULL) {
			err = g->ops.channel.get_vmid(c, &vmid);
			if (err != 0) {
				nvgpu_warn(g, "failed to get vmid from channel. chid=%u", c->chid);
			}
		}

		err = g->ops.gr.fecs_trace.bind_channel(g, &c->inst_block,
			gr_subctx, gr_ctx, mappings, tsg->tgid, vmid);
		if (err != 0) {
			nvgpu_warn(g,
				"fail to bind channel for ctxsw trace");
		}
	}
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
	if ((g->num_sys_perfmon == 0U) &&
			(g->ops.perf.get_num_hwpm_perfmon != NULL) &&
			(err == 0)) {
		g->ops.perf.get_num_hwpm_perfmon(g, &g->num_sys_perfmon,
				&g->num_fbp_perfmon, &g->num_gpc_perfmon);
		nvgpu_log(g, gpu_dbg_gr | gpu_dbg_gpu_dbg,
			"num_sys_perfmon[%u] num_fbp_perfmon[%u] "
				"num_gpc_perfmon[%u] ",
			g->num_sys_perfmon, g->num_fbp_perfmon,
			g->num_gpc_perfmon);
		nvgpu_assert((g->num_sys_perfmon != 0U) &&
			(g->num_fbp_perfmon != 0U) &&
			(g->num_gpc_perfmon != 0U));
	}
#endif

	nvgpu_gr_ctx_mark_ctx_initialized(gr_ctx);

	g->ops.tsg.enable(tsg);

	nvgpu_mutex_release(&tsg->ctx_init_lock);

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gr, "done");
	return 0;

enable_tsg:
	g->ops.tsg.enable(tsg);

	nvgpu_mutex_release(&tsg->ctx_init_lock);
out:
	/* 1. gr_ctx, patch_ctx and global ctx buffer mapping
	   can be reused so no need to release them.
	   2. golden image init and load is a one time thing so if
	   they pass, no need to undo. */
	nvgpu_err(g, "fail");
	return err;
}

void nvgpu_gr_setup_free_gr_ctx(struct gk20a *g,
				struct nvgpu_gr_ctx *gr_ctx)
{
	struct nvgpu_mem *mem;

	nvgpu_log_fn(g, " ");

	if (gr_ctx != NULL) {
		mem = nvgpu_gr_ctx_get_ctx_mem(gr_ctx, NVGPU_GR_CTX_CTX);
		if (!nvgpu_mem_is_valid(mem)) {
			return;
		}

#ifdef CONFIG_DEBUG_FS
		if ((g->ops.gr.ctxsw_prog.dump_ctxsw_stats != NULL) &&
		     nvgpu_gr_ctx_desc_dump_ctxsw_stats_on_channel_close(
					g->gr->gr_ctx_desc)) {
			g->ops.gr.ctxsw_prog.dump_ctxsw_stats(g, mem);
		}
#endif

		nvgpu_gr_ctx_free(g, gr_ctx, g->gr->global_ctx_buffer);
	}
}

void nvgpu_gr_setup_free_subctx(struct nvgpu_channel *c)
{
	nvgpu_log_fn(c->g, " ");

	if (!nvgpu_is_enabled(c->g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return;
	}

	nvgpu_gr_subctx_free(c->g, c->subctx, c->vm, true);

	nvgpu_log_fn(c->g, "done");
}

bool nvgpu_gr_setup_validate_preemption_mode(u32 *graphics_preempt_mode,
				u32 *compute_preempt_mode,
				struct nvgpu_gr_ctx *gr_ctx)
{
#ifdef CONFIG_NVGPU_GRAPHICS
	/* skip setting anything if both modes are already set */
	if ((*graphics_preempt_mode != 0U) &&
		(*graphics_preempt_mode ==
			nvgpu_gr_ctx_get_graphics_preemption_mode(gr_ctx))) {
		*graphics_preempt_mode = 0;
	}
#endif /* CONFIG_NVGPU_GRAPHICS */

	if ((*compute_preempt_mode != 0U) &&
	    (*compute_preempt_mode ==
		    nvgpu_gr_ctx_get_compute_preemption_mode(gr_ctx))) {
		*compute_preempt_mode = 0;
	}

	if ((*graphics_preempt_mode == 0U) && (*compute_preempt_mode == 0U)) {
		return false;
	}

	return true;
}

int nvgpu_gr_setup_set_preemption_mode(struct nvgpu_channel *ch,
		u32 graphics_preempt_mode, u32 compute_preempt_mode,
		u32 gr_instance_id)
{
	struct nvgpu_gr_ctx_mappings *mappings;
	struct nvgpu_gr_ctx *gr_ctx;
	struct gk20a *g = ch->g;
	struct nvgpu_tsg *tsg;
	struct nvgpu_gr *gr;
	u32 class_num;
	int err = 0;

	gr = &g->gr[gr_instance_id];

	class_num = ch->obj_class;
	if (class_num == 0U) {
		return -EINVAL;
	}

	if (!g->ops.gpu_class.is_valid(class_num)) {
		nvgpu_err(g, "invalid obj class 0x%x", class_num);
		return -EINVAL;
	}

	tsg = nvgpu_tsg_from_ch(ch);
	if (tsg == NULL) {
		return -EINVAL;
	}

	gr_ctx = tsg->gr_ctx;

	nvgpu_mutex_acquire(&tsg->ctx_init_lock);

	g->ops.tsg.disable(tsg);

	err = nvgpu_preempt_channel(g, ch);
	if (err != 0) {
		nvgpu_err(g, "failed to preempt channel/TSG");
		goto enable_ch;
	}

	if (nvgpu_gr_setup_validate_preemption_mode(&graphics_preempt_mode,
				&compute_preempt_mode, gr_ctx) == false) {
		goto enable_ch;
	}

	nvgpu_log(g, gpu_dbg_gr | gpu_dbg_sched, "chid=%d tsgid=%d pid=%d "
			"graphics_preempt_mode=%u compute_preempt_mode=%u",
			ch->chid, ch->tsgid, ch->tgid,
			graphics_preempt_mode, compute_preempt_mode);

	err = nvgpu_gr_obj_ctx_set_ctxsw_preemption_mode(g, ch, gr->config,
			gr->gr_ctx_desc, gr_ctx, class_num,
			graphics_preempt_mode, compute_preempt_mode);
	if (err != 0) {
		nvgpu_err(g, "set_ctxsw_preemption_mode failed");
		goto enable_ch;
	}

	mappings = nvgpu_gr_ctx_get_mappings(tsg, ch);
	if (mappings == NULL) {
		nvgpu_err(g, "failed to get gr_ctx mappings");
		err = -EINVAL;
		goto enable_ch;
	}

#ifdef CONFIG_NVGPU_GFXP
	err = nvgpu_gr_ctx_alloc_ctx_preemption_buffers(g,
			gr->gr_ctx_desc, gr_ctx);
	if (err != 0) {
		nvgpu_err(g, "fail to allocate ctx preemption buffers");
		goto enable_ch;
	}

	err = nvgpu_gr_ctx_mappings_map_ctx_preemption_buffers(g,
			gr_ctx, ch->subctx, mappings);
	if (err != 0) {
		nvgpu_err(g, "fail to map ctx preemption buffers");
		goto enable_ch;
	}
 #endif

	nvgpu_gr_obj_ctx_update_ctxsw_preemption_mode(g, gr->config, gr_ctx,
		ch->subctx, mappings);

	if (nvgpu_gr_obj_ctx_is_gfx_engine(g, ch->subctx)) {
		nvgpu_gr_ctx_patch_write_begin(g, gr_ctx, true);
		g->ops.gr.init.commit_global_cb_manager(g, gr->config, gr_ctx,
			true);
		nvgpu_gr_ctx_patch_write_end(g, gr_ctx, true);
	}

	g->ops.tsg.enable(tsg);

	nvgpu_mutex_release(&tsg->ctx_init_lock);

	return err;

enable_ch:
	g->ops.tsg.enable(tsg);
	nvgpu_mutex_release(&tsg->ctx_init_lock);
	return err;
}
