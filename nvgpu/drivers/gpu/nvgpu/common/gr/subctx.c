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

#include <nvgpu/gk20a.h>
#include <nvgpu/tsg_subctx.h>
#include <nvgpu/gr/subctx.h>
#include <nvgpu/gr/ctx.h>
#include <nvgpu/gr/ctx_mappings.h>
#include <nvgpu/gmmu.h>
#include <nvgpu/dma.h>
#include <nvgpu/power_features/pg.h>

#include "common/gr/subctx_priv.h"

int nvgpu_gr_subctx_setup_header(struct gk20a *g,
	struct nvgpu_gr_subctx *subctx,
	struct vm_gk20a *vm)
{
	int err = 0;

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (subctx->ctx_header.gpu_va != 0ULL) {
		return 0;
	}

	err = nvgpu_dma_alloc_sys(g,
			g->ops.gr.ctxsw_prog.hw_get_fecs_header_size(),
			&subctx->ctx_header);
	if (err != 0) {
		nvgpu_err(g, "failed to allocate sub ctx header");
		return err;
	}

	subctx->ctx_header.gpu_va = nvgpu_gmmu_map(vm,
				&subctx->ctx_header,
				0, /* not GPU-cacheable */
				gk20a_mem_flag_none, true,
				subctx->ctx_header.aperture);
	if (subctx->ctx_header.gpu_va == 0ULL) {
		nvgpu_err(g, "failed to map ctx header");
		err = -ENOMEM;
		goto err_free_ctx_header;
	}

	nvgpu_log(g, gpu_dbg_gr, "done");

	return 0;

err_free_ctx_header:
	nvgpu_dma_free(g, &subctx->ctx_header);
	return err;
}

struct nvgpu_gr_subctx *nvgpu_gr_subctx_alloc(struct gk20a *g)
{
	struct nvgpu_gr_subctx *subctx;

	nvgpu_log(g, gpu_dbg_gr, " ");

	subctx = nvgpu_kzalloc(g, sizeof(*subctx));
	if (subctx == NULL) {
		return NULL;
	}

	nvgpu_init_list_node(&subctx->gr_ctx_mappings_entry);

	nvgpu_log(g, gpu_dbg_gr, "done");

	return subctx;
}

void nvgpu_golden_ctx_gr_subctx_free(struct gk20a *g,
		struct nvgpu_gr_subctx *gr_subctx, struct vm_gk20a *vm)
{
	nvgpu_log(g, gpu_dbg_gr, " ");
	nvgpu_dma_unmap_free(vm, &gr_subctx->ctx_header);
	nvgpu_kfree(g, gr_subctx);
	nvgpu_log(g, gpu_dbg_gr, "done");
}

void nvgpu_gr_subctx_free(struct gk20a *g,
	struct nvgpu_tsg_subctx *subctx,
	struct vm_gk20a *vm,
	bool unmap)
{
	struct nvgpu_gr_subctx *gr_subctx =
			nvgpu_tsg_subctx_get_gr_subctx(subctx);

	nvgpu_log(g, gpu_dbg_gr, " ");

	if (gr_subctx == NULL) {
		return;
	}

	if (gr_subctx->mappings != NULL) {
		nvgpu_list_del(&gr_subctx->gr_ctx_mappings_entry);
		nvgpu_gr_ctx_mappings_free_subctx_mappings(subctx,
						gr_subctx->mappings, unmap);
		gr_subctx->mappings = NULL;
	}

	if (unmap) {
		nvgpu_dma_unmap_free(vm, &gr_subctx->ctx_header);
	}

	nvgpu_kfree(g, gr_subctx);

	nvgpu_log(g, gpu_dbg_gr, "done");
}

void nvgpu_gr_subctx_load_ctx_header(struct gk20a *g,
	struct nvgpu_gr_subctx *subctx,
	struct nvgpu_gr_ctx *gr_ctx,
	struct nvgpu_gr_ctx_mappings *mappings)
{
	struct nvgpu_mem *ctxheader = &subctx->ctx_header;
	u64 gpu_va;

	gpu_va = nvgpu_gr_ctx_mappings_get_ctx_va(mappings, NVGPU_GR_CTX_CTX);

#ifdef CONFIG_NVGPU_SET_FALCON_ACCESS_MAP
	/* set priv access map */
	g->ops.gr.ctxsw_prog.set_priv_access_map_addr(g, ctxheader,
		nvgpu_gr_ctx_mappings_get_global_ctx_va(mappings,
			NVGPU_GR_GLOBAL_CTX_PRIV_ACCESS_MAP_VA));
#endif

	g->ops.gr.ctxsw_prog.set_patch_addr(g, ctxheader,
		nvgpu_gr_ctx_mappings_get_ctx_va(mappings, NVGPU_GR_CTX_PATCH_CTX));

#ifdef CONFIG_NVGPU_GRAPHICS
	g->ops.gr.ctxsw_prog.set_zcull_ptr(g, ctxheader,
		nvgpu_gr_ctx_get_zcull_ctx_va(gr_ctx));
#endif

	g->ops.gr.ctxsw_prog.set_context_buffer_ptr(g, ctxheader, gpu_va);

	g->ops.gr.ctxsw_prog.set_type_per_veid_header(g, ctxheader);
}

struct nvgpu_mem *nvgpu_gr_subctx_get_ctx_header(struct nvgpu_gr_subctx *subctx)
{
	return &subctx->ctx_header;
}

struct nvgpu_gr_ctx_mappings *nvgpu_gr_subctx_get_mappings(
				struct nvgpu_gr_subctx *subctx)
{
	if (subctx == NULL) {
		return NULL;
	}

	return subctx->mappings;
}

#ifdef CONFIG_NVGPU_GRAPHICS
void nvgpu_gr_subctx_zcull_setup(struct gk20a *g, struct nvgpu_gr_subctx *subctx,
		struct nvgpu_gr_ctx *gr_ctx)
{

	nvgpu_log_fn(g, " ");

	g->ops.gr.ctxsw_prog.set_zcull_ptr(g, &subctx->ctx_header,
		nvgpu_gr_ctx_get_zcull_ctx_va(gr_ctx));
}
#endif /* CONFIG_NVGPU_GRAPHICS */

#ifdef CONFIG_NVGPU_GFXP
void nvgpu_gr_subctx_set_preemption_buffer_va(struct gk20a *g,
	struct nvgpu_gr_subctx *subctx,
	struct nvgpu_gr_ctx_mappings *veid0_mappings)
{
	u64 preempt_ctxsw_veid0_gpu_va;
	u64 preempt_ctxsw_gpu_va;
	struct nvgpu_mem *ctxheader;

	ctxheader = nvgpu_gr_subctx_get_ctx_header(subctx);

	preempt_ctxsw_gpu_va = nvgpu_gr_ctx_mappings_get_ctx_va(
					subctx->mappings,
					NVGPU_GR_CTX_PREEMPT_CTXSW);

	preempt_ctxsw_veid0_gpu_va = nvgpu_gr_ctx_mappings_get_ctx_va(
					veid0_mappings,
					NVGPU_GR_CTX_PREEMPT_CTXSW);

	g->ops.gr.ctxsw_prog.set_full_preemption_ptr(g, ctxheader,
				preempt_ctxsw_gpu_va);

	if (g->ops.gr.ctxsw_prog.set_full_preemption_ptr_veid0 != NULL) {
		g->ops.gr.ctxsw_prog.set_full_preemption_ptr_veid0(g,
			ctxheader, preempt_ctxsw_veid0_gpu_va);
	}
}

void nvgpu_gr_subctx_clear_preemption_buffer_va(struct gk20a *g,
	struct nvgpu_gr_subctx *subctx)
{
	struct nvgpu_mem *ctxheader = nvgpu_gr_subctx_get_ctx_header(subctx);

	g->ops.gr.ctxsw_prog.set_full_preemption_ptr(g, ctxheader, 0ULL);

	if (g->ops.gr.ctxsw_prog.set_full_preemption_ptr_veid0 != NULL) {
		g->ops.gr.ctxsw_prog.set_full_preemption_ptr_veid0(g,
			ctxheader, 0ULL);
	}
}
#endif /* CONFIG_NVGPU_GFXP */

#ifdef CONFIG_NVGPU_DEBUGGER
void nvgpu_gr_subctx_set_hwpm_ptr(struct gk20a *g,
	struct nvgpu_gr_subctx *subctx,
	bool set_pm_ctx_gpu_va)
{
	u64 pm_ctx_gpu_va = 0ULL;

	if (set_pm_ctx_gpu_va) {
		pm_ctx_gpu_va = nvgpu_gr_ctx_mappings_get_ctx_va(
					subctx->mappings,
					NVGPU_GR_CTX_PM_CTX);
	}

	g->ops.gr.ctxsw_prog.set_pm_ptr(g, &subctx->ctx_header,
			pm_ctx_gpu_va);
}
#endif
