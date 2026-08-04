// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/gr/config.h>
#include <nvgpu/gr/gr_falcon.h>
#include <nvgpu/falcon.h>
#include <nvgpu/soc.h>

#include "gr_falcon_ga10b.h"
#include "common/gr/gr_falcon_priv.h"

#include <nvgpu/hw/ga10b/hw_gr_ga10b.h>

#define NVGPU_FECS_UCODE_IMAGE	                "fecs.bin"
#define NVGPU_GPCCS_UCODE_IMAGE	                "gpccs.bin"
#define NVGPU_FECS_ENCRYPT_DBG_UCODE_IMAGE	    "fecs_encrypt_dbg.bin"
#define NVGPU_FECS_ENCRYPT_PROD_UCODE_IMAGE	    "fecs_encrypt_prod.bin"
#define NVGPU_GPCCS_ENCRYPT_DBG_UCODE_IMAGE	    "gpccs_encrypt_dbg.bin"
#define NVGPU_GPCCS_ENCRYPT_PROD_UCODE_IMAGE	"gpccs_encrypt_prod.bin"

#define NVGPU_NULL_METHOD_DATA 0xDEADCA11U

void ga10b_gr_falcon_get_fw_name(struct gk20a *g, const char **ucode_name, u32 falcon_id)
{
	nvgpu_log_fn(g, " ");

	if (falcon_id == FALCON_ID_FECS) {
		if (nvgpu_platform_is_simulation(g)) {
			*ucode_name = NVGPU_FECS_UCODE_IMAGE;
		} else {
			if (g->ops.pmu.is_debug_mode_enabled(g)) {
				*ucode_name = NVGPU_FECS_ENCRYPT_DBG_UCODE_IMAGE;
			} else {
				*ucode_name = NVGPU_FECS_ENCRYPT_PROD_UCODE_IMAGE;
			}
		}
	} else if (falcon_id == FALCON_ID_GPCCS) {
		if (nvgpu_platform_is_simulation(g)) {
			*ucode_name = NVGPU_GPCCS_UCODE_IMAGE;
		} else {
			if (g->ops.pmu.is_debug_mode_enabled(g)) {
				*ucode_name = NVGPU_GPCCS_ENCRYPT_DBG_UCODE_IMAGE;
			} else {
				*ucode_name = NVGPU_GPCCS_ENCRYPT_PROD_UCODE_IMAGE;
			}
		}
	} else {
		/* do nothing */
	}
}

u32 ga10b_gr_falcon_get_fecs_ctxsw_mailbox_size(void)
{
	return gr_fecs_ctxsw_mailbox__size_1_v();
}

void ga10b_gr_falcon_fecs_ctxsw_clear_mailbox(struct gk20a *g,
					u32 reg_index, u32 clear_val)
{
	u32 reg_val = 0U;

	/* Clear by writing 0 to corresponding bit(s) in mailbox register */
	reg_val = nvgpu_readl(g, gr_fecs_ctxsw_mailbox_r(reg_index));
	reg_val &= ~(clear_val);
	nvgpu_writel(g, gr_fecs_ctxsw_mailbox_r(reg_index), reg_val);
}

static void ga10b_gr_falcon_fecs_dump_stats(struct gk20a *g)
{
	unsigned int i;

#ifdef CONFIG_NVGPU_FALCON_DEBUG
	nvgpu_falcon_dump_stats(&g->fecs_flcn);
#endif

	for (i = 0U; i < g->ops.gr.falcon.fecs_ctxsw_mailbox_size(); i++) {
		nvgpu_err(g, "gr_fecs_ctxsw_mailbox_r(%d): 0x%x",
			i, nvgpu_readl(g, gr_fecs_ctxsw_mailbox_r(i)));
	}

	for (i = 0U; i < gr_fecs_ctxsw_func_tracing_mailbox__size_1_v(); i++) {
		nvgpu_err(g, "gr_fecs_ctxsw_func_tracing_mailbox_r(%d): 0x%x",
			i, nvgpu_readl(g,
				gr_fecs_ctxsw_func_tracing_mailbox_r(i)));
	}
}

static void ga10b_gr_falcon_gpccs_dump_stats(struct gk20a *g)
{
	unsigned int i;
	struct nvgpu_gr_config *gr_config = nvgpu_gr_get_config_ptr(g);
	u32 gpc_count = nvgpu_gr_config_get_gpc_count(gr_config);
	u32 gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_STRIDE);
	u32 gpc = 0U, offset = 0U;

	for (gpc = 0U; gpc < gpc_count; gpc++) {
		offset = nvgpu_safe_mult_u32(gpc_stride, gpc);
		for (i = 0U; i < gr_gpccs_ctxsw_mailbox__size_1_v(); i++) {
			nvgpu_err(g,
				"gr_gpc%d_gpccs_ctxsw_mailbox_r(%d): 0x%x",
				gpc, i,
				nvgpu_readl(g, nvgpu_safe_add_u32(
					gr_gpc0_gpccs_ctxsw_mailbox_r(i),
					offset)));
		}
	}

	for (gpc = 0U; gpc < gpc_count; gpc++) {
		offset = nvgpu_safe_mult_u32(gpc_stride, gpc);
		for (i = 0U;
			i < gr_gpc0_gpccs_ctxsw_func_tracing_mailbox__size_1_v();
			i++) {
			nvgpu_err(g,
			"gr_gpc%d_gpccs_ctxsw_func_tracing_mailbox_r(%d): 0x%x",
				gpc, i,
				nvgpu_readl(g, nvgpu_safe_add_u32(
					gr_gpc0_gpccs_ctxsw_func_tracing_mailbox_r(i),
					offset)));
		}
	}
}

#ifndef CONFIG_NVGPU_HAL_NON_FUSA
void ga10b_gr_falcon_set_null_fecs_method_data(struct gk20a *g,
		struct nvgpu_fecs_method_op *op,
		u32 fecs_method)
{
	switch (fecs_method) {
		case NVGPU_GR_FALCON_METHOD_CTXSW_DISCOVER_IMAGE_SIZE:
#if defined(CONFIG_NVGPU_GRAPHICS) && defined(CONFIG_NVGPU_ZCULL)
		case NVGPU_GR_FALCON_METHOD_CTXSW_DISCOVER_ZCULL_IMAGE_SIZE:
#endif
			op->method.data = NVGPU_NULL_METHOD_DATA;
			break;
		default:
			nvgpu_log(g, gpu_dbg_gpu_dbg, "fecs method: %d", fecs_method);
			break;
	}
}
#endif

void ga10b_gr_falcon_dump_stats(struct gk20a *g)
{
	ga10b_gr_falcon_fecs_dump_stats(g);
	ga10b_gr_falcon_gpccs_dump_stats(g);
}

#ifdef CONFIG_NVGPU_GR_FALCON_NON_SECURE_BOOT
void ga10b_gr_falcon_fecs_dmemc_write(struct gk20a *g, u32 reg_offset, u32 port,
	u32 offs, u32 blk, u32 ainc)
{
	nvgpu_writel(g, nvgpu_safe_add_u32(reg_offset, gr_fecs_dmemc_r(port)),
			gr_fecs_dmemc_offs_f(offs) |
			gr_fecs_dmemc_blk_f(blk) |
			gr_fecs_dmemc_aincw_f(ainc));
}
#endif
