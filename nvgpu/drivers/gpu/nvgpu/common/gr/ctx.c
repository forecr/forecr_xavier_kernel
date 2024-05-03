/*
 * Copyright (c) 2019-2022, NVIDIA CORPORATION.  All rights reserved.
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
#include <nvgpu/gr/global_ctx.h>
#include <nvgpu/gr/ctx.h>
#include <nvgpu/gr/ctx_mappings.h>
#include <nvgpu/vm.h>
#include <nvgpu/io.h>
#include <nvgpu/gmmu.h>
#include <nvgpu/dma.h>
#include <nvgpu/string.h>
#include <nvgpu/tsg_subctx.h>

#include <nvgpu/power_features/pg.h>
#include "common/gr/ctx_priv.h"

struct nvgpu_gr_ctx_desc *
nvgpu_gr_ctx_desc_alloc(struct gk20a *g)
{
	struct nvgpu_gr_ctx_desc *desc = nvgpu_kzalloc(g, sizeof(*desc));
	return desc;
}

void nvgpu_gr_ctx_desc_free(struct gk20a *g,
	struct nvgpu_gr_ctx_desc *desc)
{
	nvgpu_kfree(g, desc);
}

void nvgpu_gr_ctx_set_size(struct nvgpu_gr_ctx_desc *gr_ctx_desc,
	u32 index, u32 size)
{
	nvgpu_assert(index < NVGPU_GR_CTX_COUNT);
	gr_ctx_desc->size[index] = size;
}

u32 nvgpu_gr_ctx_get_size(struct nvgpu_gr_ctx_desc *gr_ctx_desc,
	u32 index)
{
	nvgpu_assert(index < NVGPU_GR_CTX_COUNT);
	return gr_ctx_desc->size[index];
}

struct nvgpu_gr_ctx *nvgpu_alloc_gr_ctx_struct(struct gk20a *g)
{
	return nvgpu_kzalloc(g, sizeof(struct nvgpu_gr_ctx));
}

void nvgpu_free_gr_ctx_struct(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx)
{
	nvgpu_kfree(g, gr_ctx);
}

void nvgpu_gr_ctx_free_ctx_buffers(struct gk20a *g,
	struct nvgpu_gr_ctx *ctx)
{
	u32 i;

	nvgpu_log(g, gpu_dbg_gr, " ");

	for (i = 0; i < NVGPU_GR_CTX_COUNT; i++) {
		if (nvgpu_mem_is_valid(&ctx->mem[i])) {
			nvgpu_dma_free(g, &ctx->mem[i]);
		}
	}

	nvgpu_log(g, gpu_dbg_gr, "done");
}

int nvgpu_gr_ctx_alloc_ctx_buffers(struct gk20a *g,
	struct nvgpu_gr_ctx_desc *desc,
	struct nvgpu_gr_ctx *ctx)
{
	int err = 0;
	u32 i;

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (desc->size[NVGPU_GR_CTX_CTX] == 0U) {
		nvgpu_err(g, "context buffer size not set");
		return -EINVAL;
	}

	for (i = 0; i < NVGPU_GR_CTX_COUNT; i++) {

#ifdef CONFIG_NVGPU_GFXP
		/**
		 * Skip allocating the gfxp preemption buffers if GFXP mode is
		 * not set in the gr ctx.
		 */
		if ((i >= NVGPU_GR_CTX_PREEMPT_CTXSW) &&
		    (i <= NVGPU_GR_CTX_GFXP_RTVCB_CTXSW) &&
		    (nvgpu_gr_ctx_get_graphics_preemption_mode(ctx) !=
		     NVGPU_PREEMPTION_MODE_GRAPHICS_GFXP)) {
			continue;
		}
#endif

		if (desc->size[i] != 0U && !nvgpu_mem_is_valid(&ctx->mem[i])) {
			err = nvgpu_dma_alloc_sys(g, desc->size[i],
				&ctx->mem[i]);
			if (err != 0) {
				nvgpu_err(g, "ctx buffer %u alloc failed", i);
				nvgpu_gr_ctx_free_ctx_buffers(g, ctx);
				return err;
			}

			nvgpu_log(g, gpu_dbg_gr, "ctx buffer %u allocated", i);
		}
	}

	if (!nvgpu_gr_ctx_get_ctx_initialized(ctx)) {
		ctx->ctx_id_valid = false;
	}

	nvgpu_log(g, gpu_dbg_gr, "done");

	return err;
}

void nvgpu_gr_ctx_init_ctx_buffers_mapping_flags(struct gk20a *g,
	struct nvgpu_gr_ctx *ctx)
{
	u32 i;

	nvgpu_log(g, gpu_dbg_gr, " ");

	/**
	 * Map all ctx buffers as cacheable except GR CTX and
	 * PATCH CTX buffers.
	 */
	for (i = 0; i < NVGPU_GR_CTX_COUNT; i++) {
		ctx->mapping_flags[i] = NVGPU_VM_MAP_CACHEABLE;
	}

	ctx->mapping_flags[NVGPU_GR_CTX_CTX] = 0U;
	ctx->mapping_flags[NVGPU_GR_CTX_PATCH_CTX] = 0U;

	nvgpu_log(g, gpu_dbg_gr, "done");
}

#ifdef CONFIG_NVGPU_GFXP
static void nvgpu_gr_ctx_free_ctx_preemption_buffers(struct gk20a *g,
	struct nvgpu_gr_ctx *ctx)
{
	u32 i;

	nvgpu_log_fn(g, " ");

	for (i = NVGPU_GR_CTX_PREEMPT_CTXSW;
			i <= NVGPU_GR_CTX_GFXP_RTVCB_CTXSW; i++) {
		if (nvgpu_mem_is_valid(&ctx->mem[i])) {
			nvgpu_dma_free(g, &ctx->mem[i]);
		}
	}

	nvgpu_log_fn(g, "done");
}

int nvgpu_gr_ctx_alloc_ctx_preemption_buffers(struct gk20a *g,
	struct nvgpu_gr_ctx_desc *desc,
	struct nvgpu_gr_ctx *ctx)
{
	int err = 0;
	u32 i;

	nvgpu_log(g, gpu_dbg_gr, " ");

	/**
	 * Skip allocating the gfxp preemption buffers if GFXP mode is
	 * not set in the gr ctx.
	 */
	if (nvgpu_gr_ctx_get_graphics_preemption_mode(ctx) !=
	    NVGPU_PREEMPTION_MODE_GRAPHICS_GFXP) {
		nvgpu_log(g, gpu_dbg_gr, "GFXP mode not set. Skip preemption "
					 "buffers allocation");
		return 0;
	}

	for (i = NVGPU_GR_CTX_PREEMPT_CTXSW;
			i <= NVGPU_GR_CTX_GFXP_RTVCB_CTXSW; i++) {

		if (desc->size[i] != 0U && !nvgpu_mem_is_valid(&ctx->mem[i])) {
			err = nvgpu_dma_alloc_sys(g, desc->size[i],
				&ctx->mem[i]);
			if (err != 0) {
				nvgpu_err(g, "ctx preemption buffer %u alloc failed", i);
				nvgpu_gr_ctx_free_ctx_preemption_buffers(g, ctx);
				return err;
			}

			nvgpu_log(g, gpu_dbg_gr, "ctx preemption buffer %u allocated", i);
		}
	}

	nvgpu_log(g, gpu_dbg_gr, "done");

	return err;
}
#endif

void nvgpu_gr_ctx_free(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx,
	struct nvgpu_gr_global_ctx_buffer_desc *global_ctx_buffer)
{
	struct nvgpu_tsg *tsg;

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (gr_ctx != NULL) {
		tsg = nvgpu_tsg_get_from_id(g, gr_ctx->tsgid);

		nvgpu_mutex_acquire(&tsg->ctx_init_lock);

		if (nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
			nvgpu_assert(nvgpu_list_empty(&tsg->ch_list));
			nvgpu_assert(nvgpu_list_empty(&tsg->subctx_list));
			nvgpu_assert(nvgpu_list_empty(&tsg->gr_ctx_mappings_list));
		} else {
			if (gr_ctx->mappings != NULL) {
				nvgpu_gr_ctx_unmap_buffers(g,
					gr_ctx, NULL, global_ctx_buffer,
					gr_ctx->mappings);

				nvgpu_gr_ctx_free_mappings(g, gr_ctx);
			}
		}

		nvgpu_gr_ctx_set_patch_ctx_data_count(gr_ctx, 0);

		nvgpu_gr_ctx_free_ctx_buffers(g, gr_ctx);

		(void) memset(gr_ctx, 0, sizeof(*gr_ctx));

		nvgpu_mutex_release(&tsg->ctx_init_lock);
	}

	nvgpu_log(g, gpu_dbg_gr, "done");
}

struct nvgpu_gr_ctx_mappings *nvgpu_gr_ctx_alloc_or_get_mappings(struct gk20a *g,
				struct nvgpu_tsg *tsg, struct nvgpu_channel *ch)
{
	struct nvgpu_gr_ctx_mappings *mappings = NULL;
	struct nvgpu_gr_ctx *gr_ctx = tsg->gr_ctx;
	struct vm_gk20a *vm = ch->vm;

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return nvgpu_tsg_subctx_alloc_or_get_mappings(g, tsg, ch);
	}

	mappings = gr_ctx->mappings;
	if (mappings != NULL) {
		return mappings;
	}

	mappings = nvgpu_gr_ctx_mappings_create(g, tsg, vm);
	if (mappings == NULL) {
		nvgpu_err(g, "failed to allocate gr_ctx mappings");
		return mappings;
	}

	gr_ctx->mappings = mappings;

	nvgpu_log(g, gpu_dbg_gr, "done");

	return mappings;
}

void nvgpu_gr_ctx_free_mappings(struct gk20a *g,
				struct nvgpu_gr_ctx *gr_ctx)
{
	nvgpu_log(g, gpu_dbg_gr, " ");

	if (gr_ctx->mappings == NULL) {
		return;
	}

	nvgpu_gr_ctx_mappings_free(g, gr_ctx->mappings);
	gr_ctx->mappings = NULL;

	nvgpu_log(g, gpu_dbg_gr, "done");
}

struct nvgpu_gr_ctx_mappings *nvgpu_gr_ctx_get_mappings(struct nvgpu_tsg *tsg,
					struct nvgpu_channel *ch)
{
	struct gk20a *g = tsg->g;

	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		return nvgpu_gr_ctx_mappings_get_subctx_mappings(g, tsg, ch->vm);
	}

	return tsg->gr_ctx->mappings;
}

void nvgpu_gr_ctx_set_patch_ctx_data_count(struct nvgpu_gr_ctx *gr_ctx,
	u32 data_count)
{
	gr_ctx->patch_ctx.data_count = data_count;
}

struct nvgpu_mem *nvgpu_gr_ctx_get_ctx_mem(struct nvgpu_gr_ctx *gr_ctx,
	u32 index)
{
	nvgpu_assert(index < NVGPU_GR_CTX_COUNT);
	return &gr_ctx->mem[index];
}

u32 nvgpu_gr_ctx_get_ctx_mapping_flags(struct nvgpu_gr_ctx *gr_ctx, u32 index)
{
	nvgpu_assert(index < NVGPU_GR_CTX_COUNT);
	return gr_ctx->mapping_flags[index];
}

#ifdef CONFIG_NVGPU_SM_DIVERSITY
void nvgpu_gr_ctx_set_sm_diversity_config(struct nvgpu_gr_ctx *gr_ctx,
	u32 sm_diversity_config)
{
	gr_ctx->sm_diversity_config = sm_diversity_config;
}

u32 nvgpu_gr_ctx_get_sm_diversity_config(struct nvgpu_gr_ctx *gr_ctx)
{
	return gr_ctx->sm_diversity_config;
}
#endif

/* load saved fresh copy of gloden image into channel gr_ctx */
void nvgpu_gr_ctx_load_golden_ctx_image(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx,
	struct nvgpu_gr_ctx_mappings *mappings,
	struct nvgpu_gr_global_ctx_local_golden_image *local_golden_image,
	bool cde)
{
	struct nvgpu_mem *mem;
#ifdef CONFIG_NVGPU_DEBUGGER
	u64 virt_addr = 0;
#endif

	(void)cde;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gr, " ");

	mem = &gr_ctx->mem[NVGPU_GR_CTX_CTX];

	nvgpu_gr_global_ctx_load_local_golden_image(g,
		local_golden_image, mem);

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	g->ops.gr.ctxsw_prog.init_ctxsw_hdr_data(g, mem);
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
	if ((g->ops.gr.ctxsw_prog.set_cde_enabled != NULL) && cde) {
		g->ops.gr.ctxsw_prog.set_cde_enabled(g, mem);
	}
#endif

#ifdef CONFIG_NVGPU_SET_FALCON_ACCESS_MAP
	/* set priv access map */
	g->ops.gr.ctxsw_prog.set_priv_access_map_config_mode(g, mem,
		g->allow_all);
	g->ops.gr.ctxsw_prog.set_priv_access_map_addr(g, mem,
		nvgpu_gr_ctx_mappings_get_global_ctx_va(mappings,
			NVGPU_GR_GLOBAL_CTX_PRIV_ACCESS_MAP_VA));
#endif

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
	/* disable verif features */
	g->ops.gr.ctxsw_prog.disable_verif_features(g, mem);
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
	if (g->ops.gr.ctxsw_prog.set_pmu_options_boost_clock_frequencies !=
			NULL) {
		g->ops.gr.ctxsw_prog.set_pmu_options_boost_clock_frequencies(g,
			mem, nvgpu_safe_cast_bool_to_u32(gr_ctx->boosted_ctx));
	}
#endif

	nvgpu_log(g, gpu_dbg_info | gpu_dbg_gr, "write patch count = %d",
			gr_ctx->patch_ctx.data_count);
	g->ops.gr.ctxsw_prog.set_patch_count(g, mem,
		gr_ctx->patch_ctx.data_count);
	g->ops.gr.ctxsw_prog.set_patch_addr(g, mem,
		nvgpu_gr_ctx_mappings_get_ctx_va(mappings,
			NVGPU_GR_CTX_PATCH_CTX));

#ifdef CONFIG_NVGPU_DEBUGGER
	/* PM ctxt switch is off by default */
	gr_ctx->pm_ctx.pm_mode =
		g->ops.gr.ctxsw_prog.hw_get_pm_mode_no_ctxsw();
	virt_addr = 0;

	g->ops.gr.ctxsw_prog.set_pm_mode(g, mem, gr_ctx->pm_ctx.pm_mode);
	g->ops.gr.ctxsw_prog.set_pm_ptr(g, mem, virt_addr);
#endif

	nvgpu_log(g, gpu_dbg_gr, "done");
}

/*
 * Context state can be written directly, or "patched" at times. So that code
 * can be used in either situation it is written using a series of
 * _ctx_patch_write(..., patch) statements. However any necessary map overhead
 * should be minimized; thus, bundle the sequence of these writes together, and
 * set them up and close with _ctx_patch_write_begin/_ctx_patch_write_end.
 */
void nvgpu_gr_ctx_patch_write_begin(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx,
	bool update_patch_count)
{
	struct nvgpu_mem *mem = &gr_ctx->mem[NVGPU_GR_CTX_CTX];

	if (update_patch_count) {
		/* reset patch count if ucode has already processed it */
		gr_ctx->patch_ctx.data_count =
			g->ops.gr.ctxsw_prog.get_patch_count(g, mem);
		nvgpu_log(g, gpu_dbg_info, "patch count reset to %d",
					gr_ctx->patch_ctx.data_count);
	}
}

void nvgpu_gr_ctx_patch_write_end(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx,
	bool update_patch_count)
{
	struct nvgpu_mem *mem = &gr_ctx->mem[NVGPU_GR_CTX_CTX];

	/* Write context count to context image if it is mapped */
	if (update_patch_count) {
		g->ops.gr.ctxsw_prog.set_patch_count(g, mem,
			     gr_ctx->patch_ctx.data_count);
		nvgpu_log(g, gpu_dbg_info, "write patch count %d",
			gr_ctx->patch_ctx.data_count);
	}
}

void nvgpu_gr_ctx_patch_write(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx,
	u32 addr, u32 data, bool patch)
{
	if (patch) {
		u32 patch_slot;
		u64 patch_slot_max;
		struct nvgpu_mem *patch_ctx_mem;

		if (gr_ctx == NULL) {
			nvgpu_err(g,
				"failed to access gr_ctx[NULL] but patch true");
			return;
		}

		patch_ctx_mem = &gr_ctx->mem[NVGPU_GR_CTX_PATCH_CTX];

		patch_slot =
			nvgpu_safe_mult_u32(gr_ctx->patch_ctx.data_count,
					PATCH_CTX_SLOTS_REQUIRED_PER_ENTRY);
		patch_slot_max =
			nvgpu_safe_sub_u64(
				PATCH_CTX_ENTRIES_FROM_SIZE(
					patch_ctx_mem->size),
					PATCH_CTX_SLOTS_REQUIRED_PER_ENTRY);

		if (patch_slot > patch_slot_max) {
			nvgpu_err(g, "failed to access patch_slot %d",
				patch_slot);
			return;
		}

		nvgpu_mem_wr32(g, patch_ctx_mem, (u64)patch_slot, addr);
		nvgpu_mem_wr32(g, patch_ctx_mem, (u64)patch_slot + 1ULL, data);
		gr_ctx->patch_ctx.data_count = nvgpu_safe_add_u32(
						gr_ctx->patch_ctx.data_count, 1U);
		nvgpu_log(g, gpu_dbg_info,
			"patch addr = 0x%x data = 0x%x data_count %d",
			addr, data, gr_ctx->patch_ctx.data_count);
	} else {
		nvgpu_writel(g, addr, data);
	}
}

void nvgpu_gr_ctx_init_compute_preemption_mode(struct nvgpu_gr_ctx *gr_ctx,
	u32 compute_preempt_mode)
{
	gr_ctx->compute_preempt_mode = compute_preempt_mode;
}

u32 nvgpu_gr_ctx_get_compute_preemption_mode(struct nvgpu_gr_ctx *gr_ctx)
{
	return gr_ctx->compute_preempt_mode;
}

bool nvgpu_gr_ctx_check_valid_preemption_mode(struct gk20a *g,
		struct nvgpu_channel *ch,
		struct nvgpu_gr_ctx *gr_ctx,
		u32 graphics_preempt_mode, u32 compute_preempt_mode)
{
	u32 supported_graphics_preempt_mode = 0U;
	u32 supported_compute_preempt_mode = 0U;
#if defined(CONFIG_NVGPU_CILP) && defined(CONFIG_NVGPU_GFXP)
	int err;
#endif

	(void)ch;

	if ((graphics_preempt_mode == 0U) && (compute_preempt_mode == 0U)) {
		return false;
	}

	g->ops.gr.init.get_supported__preemption_modes(
			&supported_graphics_preempt_mode,
			&supported_compute_preempt_mode);

	if (graphics_preempt_mode != 0U) {
		if ((graphics_preempt_mode & supported_graphics_preempt_mode) == 0U) {
			return false;
		}

		/* Do not allow lower preemption modes than current ones */
		if (graphics_preempt_mode < gr_ctx->graphics_preempt_mode) {
			return false;
		}
	}

	if (compute_preempt_mode != 0U) {
		if ((compute_preempt_mode & supported_compute_preempt_mode) == 0U) {
			return false;
		}

		/* Do not allow lower preemption modes than current ones */
		if (compute_preempt_mode < gr_ctx->compute_preempt_mode) {
			return false;
		}
	}

#if defined(CONFIG_NVGPU_CILP) && defined(CONFIG_NVGPU_GFXP)
	/* Invalid combination */
	if ((graphics_preempt_mode == NVGPU_PREEMPTION_MODE_GRAPHICS_GFXP) &&
		   (compute_preempt_mode == NVGPU_PREEMPTION_MODE_COMPUTE_CILP)) {
		return false;
	}

	if (g->ops.gpu_class.is_valid_compute(ch->obj_class) &&
	    compute_preempt_mode == NVGPU_PREEMPTION_MODE_COMPUTE_CILP) {
		err = nvgpu_tsg_validate_cilp_config(ch);
		if (err != 0) {
			nvgpu_err(g, "Invalid class/veid/pbdma config. CILP not allowed.");
			return false;
		}
	}

	if (g->ops.gpu_class.is_valid_gfx(ch->obj_class)) {
		err = nvgpu_tsg_validate_cilp_config(ch);
		if (err != 0) {
			nvgpu_err(g, "Invalid class/veid/pbdma config. CILP not allowed.");
			return false;
		}
	}
#endif

	return true;
}

void nvgpu_gr_ctx_set_preemption_modes(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx)
{
	struct nvgpu_mem *mem = &gr_ctx->mem[NVGPU_GR_CTX_CTX];

#ifdef CONFIG_NVGPU_GFXP
	if (gr_ctx->graphics_preempt_mode == NVGPU_PREEMPTION_MODE_GRAPHICS_GFXP) {
		g->ops.gr.ctxsw_prog.set_graphics_preemption_mode_gfxp(g, mem);
	}
#endif

#ifdef CONFIG_NVGPU_CILP
	if (gr_ctx->compute_preempt_mode == NVGPU_PREEMPTION_MODE_COMPUTE_CILP) {
		g->ops.gr.ctxsw_prog.set_compute_preemption_mode_cilp(g, mem);
	}
#endif

	if (gr_ctx->compute_preempt_mode == NVGPU_PREEMPTION_MODE_COMPUTE_CTA) {
		g->ops.gr.ctxsw_prog.set_compute_preemption_mode_cta(g, mem);
	}
}

void nvgpu_gr_ctx_set_tsgid(struct nvgpu_gr_ctx *gr_ctx, u32 tsgid)
{
	gr_ctx->tsgid = tsgid;
}

u32 nvgpu_gr_ctx_get_tsgid(struct nvgpu_gr_ctx *gr_ctx)
{
	return gr_ctx->tsgid;
}

void nvgpu_gr_ctx_mark_ctx_initialized(struct nvgpu_gr_ctx *gr_ctx)
{
	gr_ctx->ctx_initialized = true;
}

bool nvgpu_gr_ctx_get_ctx_initialized(struct nvgpu_gr_ctx *gr_ctx)
{
	return gr_ctx->ctx_initialized;
}

#ifdef CONFIG_NVGPU_GRAPHICS
void nvgpu_gr_ctx_init_graphics_preemption_mode(struct nvgpu_gr_ctx *gr_ctx,
	u32 graphics_preempt_mode)
{
	gr_ctx->graphics_preempt_mode = graphics_preempt_mode;
}

u32 nvgpu_gr_ctx_get_graphics_preemption_mode(struct nvgpu_gr_ctx *gr_ctx)
{
	return gr_ctx->graphics_preempt_mode;
}

void nvgpu_gr_ctx_set_zcull_ctx(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx,
	u32 mode, u64 gpu_va)
{
	struct zcull_ctx_desc *zcull_ctx = &gr_ctx->zcull_ctx;

	(void)g;

	zcull_ctx->ctx_sw_mode = mode;
	zcull_ctx->gpu_va = gpu_va;
}

u64 nvgpu_gr_ctx_get_zcull_ctx_va(struct nvgpu_gr_ctx *gr_ctx)
{
	return gr_ctx->zcull_ctx.gpu_va;
}

int nvgpu_gr_ctx_init_zcull(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx)
{
	struct nvgpu_mem *mem = &gr_ctx->mem[NVGPU_GR_CTX_CTX];

	nvgpu_log(g, gpu_dbg_gr, " ");

	g->ops.gr.ctxsw_prog.set_zcull_mode_no_ctxsw(g, mem);
	g->ops.gr.ctxsw_prog.set_zcull_ptr(g, mem, 0);

	return 0;
}

int nvgpu_gr_ctx_zcull_setup(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx,
	bool set_zcull_ptr)
{
	struct nvgpu_mem *mem = &gr_ctx->mem[NVGPU_GR_CTX_CTX];

	nvgpu_log_fn(g, " ");

	if (gr_ctx->zcull_ctx.gpu_va == 0ULL &&
	    g->ops.gr.ctxsw_prog.is_zcull_mode_separate_buffer(
			gr_ctx->zcull_ctx.ctx_sw_mode)) {
		return -EINVAL;
	}

	g->ops.gr.ctxsw_prog.set_zcull(g, mem, gr_ctx->zcull_ctx.ctx_sw_mode);

	if (set_zcull_ptr) {
		g->ops.gr.ctxsw_prog.set_zcull_ptr(g, mem,
			gr_ctx->zcull_ctx.gpu_va);
	}

	return 0;
}
#endif /* CONFIG_NVGPU_GRAPHICS */

#ifdef CONFIG_NVGPU_GFXP
void nvgpu_gr_ctx_set_preemption_buffer_va(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx)
{
	struct nvgpu_gr_ctx_mappings *mappings = gr_ctx->mappings;
	struct nvgpu_mem *mem = &gr_ctx->mem[NVGPU_GR_CTX_CTX];
	u64 preempt_ctxsw_gpu_va = nvgpu_gr_ctx_mappings_get_ctx_va(mappings,
						NVGPU_GR_CTX_PREEMPT_CTXSW);

	g->ops.gr.ctxsw_prog.set_full_preemption_ptr(g, mem,
				preempt_ctxsw_gpu_va);

	if (g->ops.gr.ctxsw_prog.set_full_preemption_ptr_veid0 != NULL) {
		g->ops.gr.ctxsw_prog.set_full_preemption_ptr_veid0(g,
			mem, preempt_ctxsw_gpu_va);
	}
}

bool nvgpu_gr_ctx_desc_force_preemption_gfxp(struct nvgpu_gr_ctx_desc *gr_ctx_desc)
{
	return gr_ctx_desc->force_preemption_gfxp;
}
#endif /* CONFIG_NVGPU_GFXP */

#ifdef CONFIG_NVGPU_CILP
bool nvgpu_gr_ctx_desc_force_preemption_cilp(struct nvgpu_gr_ctx_desc *gr_ctx_desc)
{
	return gr_ctx_desc->force_preemption_cilp;
}

bool nvgpu_gr_ctx_get_cilp_preempt_pending(struct nvgpu_gr_ctx *gr_ctx)
{
	return gr_ctx->cilp_preempt_pending;
}

void nvgpu_gr_ctx_set_cilp_preempt_pending(struct nvgpu_gr_ctx *gr_ctx,
	bool cilp_preempt_pending)
{
	gr_ctx->cilp_preempt_pending = cilp_preempt_pending;
}
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
void nvgpu_gr_ctx_reset_patch_count(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx)
{
	struct nvgpu_mem *mem = &gr_ctx->mem[NVGPU_GR_CTX_CTX];
	u32 tmp;

	tmp = g->ops.gr.ctxsw_prog.get_patch_count(g, mem);
	if (tmp == 0U) {
		gr_ctx->patch_ctx.data_count = 0;
	}
}

void nvgpu_gr_ctx_set_patch_ctx(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx)
{
	struct nvgpu_gr_ctx_mappings *mappings = gr_ctx->mappings;
	struct nvgpu_mem *mem = &gr_ctx->mem[NVGPU_GR_CTX_CTX];

	g->ops.gr.ctxsw_prog.set_patch_count(g, mem,
		gr_ctx->patch_ctx.data_count);

	g->ops.gr.ctxsw_prog.set_patch_addr(g, mem,
		nvgpu_gr_ctx_mappings_get_ctx_va(mappings,
					NVGPU_GR_CTX_PATCH_CTX));
}

static int nvgpu_gr_ctx_alloc_pm_ctx(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx,
	struct nvgpu_gr_ctx_desc *gr_ctx_desc)
{
	int err;

	err = nvgpu_dma_alloc_sys(g, gr_ctx_desc->size[NVGPU_GR_CTX_PM_CTX],
			&gr_ctx->mem[NVGPU_GR_CTX_PM_CTX]);
	if (err != 0) {
		nvgpu_err(g,
			"failed to allocate pm ctx buffer");
		return err;
	}

	return 0;
}

static void nvgpu_gr_ctx_free_pm_ctx(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx)
{
	if (nvgpu_mem_is_valid(&gr_ctx->mem[NVGPU_GR_CTX_PM_CTX])) {
		nvgpu_dma_free(g, &gr_ctx->mem[NVGPU_GR_CTX_PM_CTX]);
	}

	(void)g;
}

int nvgpu_gr_ctx_alloc_map_pm_ctx(struct gk20a *g,
	struct nvgpu_tsg *tsg,
	struct nvgpu_gr_ctx_desc *gr_ctx_desc,
	struct nvgpu_gr_hwpm_map *hwpm_map)
{
	struct nvgpu_gr_ctx *gr_ctx = tsg->gr_ctx;
	struct nvgpu_gr_ctx_mappings *mappings;
	int ret;

	if (gr_ctx->pm_ctx.mapped) {
		return 0;
	}

	nvgpu_gr_ctx_set_size(gr_ctx_desc,
		NVGPU_GR_CTX_PM_CTX,
		nvgpu_gr_hwpm_map_get_size(hwpm_map));

	ret = nvgpu_gr_ctx_alloc_pm_ctx(g, gr_ctx, gr_ctx_desc);
	if (ret != 0) {
		nvgpu_err(g,
			"failed to allocate pm ctxt buffer");
		return ret;
	}

	/*
	 * Commit NVGPU_GR_CTX_PM_CTX gpu va for all subcontexts
	 * when subcontexts are enabled.
	 */
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_TSG_SUBCONTEXTS)) {
		ret = nvgpu_gr_ctx_mappings_map_buffer_all_subctx(tsg,
					NVGPU_GR_CTX_PM_CTX);
	} else {
		mappings = nvgpu_gr_ctx_get_mappings(tsg, NULL);
		if (mappings == NULL) {
			nvgpu_err(g, "gr_ctx mappings struct not allocated");
			nvgpu_gr_ctx_free_pm_ctx(g, gr_ctx);
			return -ENOMEM;
		}

		ret = nvgpu_gr_ctx_mappings_map_ctx_buffer(g, gr_ctx,
				NVGPU_GR_CTX_PM_CTX, mappings);
	}

	if (ret != 0) {
		nvgpu_err(g, "gr_ctx pm_ctx buffer map failed %d", ret);
		nvgpu_gr_ctx_free_pm_ctx(g, gr_ctx);
		return ret;
	}

	return 0;
}

void nvgpu_gr_ctx_set_pm_ctx_pm_mode(struct nvgpu_gr_ctx *gr_ctx, u32 pm_mode)
{
	gr_ctx->pm_ctx.pm_mode = pm_mode;
}

u32 nvgpu_gr_ctx_get_pm_ctx_pm_mode(struct nvgpu_gr_ctx *gr_ctx)
{
	return gr_ctx->pm_ctx.pm_mode;
}

u32 nvgpu_gr_ctx_get_ctx_id(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx)
{
	struct nvgpu_mem *mem = &gr_ctx->mem[NVGPU_GR_CTX_CTX];

	if (!gr_ctx->ctx_id_valid) {
		gr_ctx->ctx_id = g->ops.gr.ctxsw_prog.get_main_image_ctx_id(g,
					mem);
		gr_ctx->ctx_id_valid = true;
	}

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_intr, "ctx_id: 0x%x", gr_ctx->ctx_id);

	return gr_ctx->ctx_id;
}

u32 nvgpu_gr_ctx_read_ctx_id(struct nvgpu_gr_ctx *gr_ctx)
{
	return gr_ctx->ctx_id;
}

#ifdef CONFIG_NVGPU_CHANNEL_TSG_SCHEDULING
void nvgpu_gr_ctx_set_boosted_ctx(struct nvgpu_gr_ctx *gr_ctx, bool boost)
{
	gr_ctx->boosted_ctx = boost;
}

bool nvgpu_gr_ctx_get_boosted_ctx(struct nvgpu_gr_ctx *gr_ctx)
{
	return gr_ctx->boosted_ctx;
}
#endif

#ifdef CONFIG_DEBUG_FS
bool nvgpu_gr_ctx_desc_dump_ctxsw_stats_on_channel_close(
		struct nvgpu_gr_ctx_desc *gr_ctx_desc)
{
	return gr_ctx_desc->dump_ctxsw_stats_on_channel_close;
}
#endif

int nvgpu_gr_ctx_set_smpc_mode(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx,
	bool enable)
{
	struct nvgpu_mem *mem = &gr_ctx->mem[NVGPU_GR_CTX_CTX];

	if (!nvgpu_mem_is_valid(mem)) {
		nvgpu_err(g, "no graphics context allocated");
		return -EFAULT;
	}

	g->ops.gr.ctxsw_prog.set_pm_smpc_mode(g, mem, enable);

	return 0;
}

int nvgpu_gr_ctx_prepare_hwpm_mode(struct gk20a *g,
	struct nvgpu_gr_ctx *gr_ctx,
	u32 mode, bool *set_pm_ctx_gpu_va, bool *skip_update)
{
	struct nvgpu_mem *mem = &gr_ctx->mem[NVGPU_GR_CTX_CTX];
	struct pm_ctx_desc *pm_ctx = &gr_ctx->pm_ctx;
	int ret = 0;

	*set_pm_ctx_gpu_va = false;
	*skip_update = false;

	if (!nvgpu_mem_is_valid(mem)) {
		nvgpu_err(g, "no graphics context allocated");
		return -EFAULT;
	}

	if ((mode == NVGPU_GR_CTX_HWPM_CTXSW_MODE_STREAM_OUT_CTXSW) &&
	    (g->ops.gr.ctxsw_prog.hw_get_pm_mode_stream_out_ctxsw == NULL)) {
		nvgpu_err(g,
			"Mode-E hwpm context switch mode is not supported");
		return -EINVAL;
	}

	switch (mode) {
	case NVGPU_GR_CTX_HWPM_CTXSW_MODE_CTXSW:
		if (pm_ctx->pm_mode ==
		    g->ops.gr.ctxsw_prog.hw_get_pm_mode_ctxsw()) {
			*skip_update = true;
			return 0;
		}
		pm_ctx->pm_mode = g->ops.gr.ctxsw_prog.hw_get_pm_mode_ctxsw();
		*set_pm_ctx_gpu_va = true;
		break;
	case  NVGPU_GR_CTX_HWPM_CTXSW_MODE_NO_CTXSW:
		if (pm_ctx->pm_mode ==
		    g->ops.gr.ctxsw_prog.hw_get_pm_mode_no_ctxsw()) {
			*skip_update = true;
			return 0;
		}
		pm_ctx->pm_mode =
			g->ops.gr.ctxsw_prog.hw_get_pm_mode_no_ctxsw();
		*set_pm_ctx_gpu_va = false;
		break;
	case NVGPU_GR_CTX_HWPM_CTXSW_MODE_STREAM_OUT_CTXSW:
		if (pm_ctx->pm_mode ==
		    g->ops.gr.ctxsw_prog.hw_get_pm_mode_stream_out_ctxsw()) {
			*skip_update = true;
			return 0;
		}
		pm_ctx->pm_mode =
			g->ops.gr.ctxsw_prog.hw_get_pm_mode_stream_out_ctxsw();
		*set_pm_ctx_gpu_va = true;
		break;
	default:
		nvgpu_err(g, "invalid hwpm context switch mode");
		ret = -EINVAL;
		break;
	}

	return ret;
}

void nvgpu_gr_ctx_set_hwpm_pm_mode(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx)
{
	struct nvgpu_mem *mem = &gr_ctx->mem[NVGPU_GR_CTX_CTX];

	g->ops.gr.ctxsw_prog.set_pm_mode(g, mem, gr_ctx->pm_ctx.pm_mode);
}

void nvgpu_gr_ctx_set_hwpm_ptr(struct gk20a *g, struct nvgpu_gr_ctx *gr_ctx,
			       bool set_pm_ctx_gpu_va)
{
	struct nvgpu_mem *mem = &gr_ctx->mem[NVGPU_GR_CTX_CTX];
	u64 pm_ctx_gpu_va = 0ULL;

	if (set_pm_ctx_gpu_va) {
		pm_ctx_gpu_va = nvgpu_gr_ctx_mappings_get_ctx_va(
						gr_ctx->mappings,
						NVGPU_GR_CTX_PM_CTX);
	}

	g->ops.gr.ctxsw_prog.set_pm_ptr(g, mem, pm_ctx_gpu_va);
}

void nvgpu_gr_ctx_set_pm_ctx_mapped(struct nvgpu_gr_ctx *ctx, bool mapped)
{
	ctx->pm_ctx.mapped = mapped;
}
#endif /* CONFIG_NVGPU_DEBUGGER */

bool nvgpu_gr_obj_ctx_global_ctx_buffers_patched(struct nvgpu_gr_ctx *gr_ctx)
{
	return gr_ctx->global_ctx_buffers_patched;
}

void nvgpu_gr_obj_ctx_set_global_ctx_buffers_patched(
			struct nvgpu_gr_ctx *gr_ctx, bool patched)
{
	gr_ctx->global_ctx_buffers_patched = patched;
}

bool nvgpu_gr_obj_ctx_preempt_buffers_patched(struct nvgpu_gr_ctx *gr_ctx)
{
	return gr_ctx->preempt_buffers_patched;
}

void nvgpu_gr_obj_ctx_set_preempt_buffers_patched(
			struct nvgpu_gr_ctx *gr_ctx, bool patched)
{
	gr_ctx->preempt_buffers_patched = patched;
}

bool nvgpu_gr_obj_ctx_default_compute_regs_patched(struct nvgpu_gr_ctx *gr_ctx)
{
	return gr_ctx->default_compute_regs_patched;
}

void nvgpu_gr_obj_ctx_set_default_compute_regs_patched(
			struct nvgpu_gr_ctx *gr_ctx, bool patched)
{
	gr_ctx->default_compute_regs_patched = patched;
}

bool nvgpu_gr_obj_ctx_default_gfx_regs_patched(struct nvgpu_gr_ctx *gr_ctx)
{
	return gr_ctx->default_gfx_regs_patched;
}

void nvgpu_gr_obj_ctx_set_default_gfx_regs_patched(
			struct nvgpu_gr_ctx *gr_ctx, bool patched)
{
	gr_ctx->default_gfx_regs_patched = patched;
}
