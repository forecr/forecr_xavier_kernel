/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#include <nvgpu/gk20a.h>
#include <nvgpu/gr/gr_falcon.h>
#include <nvgpu/io.h>
#include <nvgpu/debug.h>
#include <nvgpu/power_features/pg.h>
#include <nvgpu/soc.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/gr/gr_utils.h>

#include "gr_falcon_gb10b.h"
#include "common/gr/gr_falcon_priv.h"
#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/gr/gr_falcon.h>

#include "hal/gr/falcon/gr_falcon_gm20b.h"
#include "hal/gr/falcon/gr_falcon_ga100.h"

#include <nvgpu/hw/gb10b/hw_gr_gb10b.h>

#define GR_FECS_POLL_INTERVAL	5U /* usec */

#define FECS_ARB_CMD_TIMEOUT_MAX_US 40U
#define FECS_ARB_CMD_TIMEOUT_DEFAULT_US 2U
#define CTXSW_MEM_SCRUBBING_TIMEOUT_MAX_US 1000U
#define CTXSW_MEM_SCRUBBING_TIMEOUT_DEFAULT_US 10U

#define CTXSW_WDT_DEFAULT_VALUE 0x7FFFFFFFU
#define CTXSW_INTR0 BIT32(0)
#define CTXSW_INTR1 BIT32(1)

#ifdef CONFIG_NVGPU_GR_FALCON_NON_SECURE_BOOT
void gb10b_gr_falcon_gpccs_dmemc_write(struct gk20a *g, u32 port, u32 offs,
	u32 blk, u32 ainc)
{
	nvgpu_writel(g, gr_gpccs_dmemc_r(port),
			gr_gpccs_dmemc_offs_f(offs) |
			gr_gpccs_dmemc_blk_f(blk) |
			gr_gpccs_dmemc_aincw_f(ainc));
}

void gb10b_gr_falcon_gpccs_imemc_write(struct gk20a *g, u32 port, u32 offs,
	u32 blk, u32 ainc)
{
	nvgpu_writel(g, gr_gpccs_imemc_r(port),
			gr_gpccs_imemc_offs_f(offs) |
			gr_gpccs_imemc_blk_f(blk) |
			gr_gpccs_imemc_aincw_f(ainc));
}

void gb10b_gr_falcon_load_gpccs_dmem(struct gk20a *g,
			const u32 *ucode_u32_data, u32 ucode_u32_size)
{
	u32 i;
	u32 checksum = 0;

	/* enable access for gpccs dmem */
	g->ops.gr.falcon.gpccs_dmemc_write(g, 0U, 0U, 0U, 1U);

	for (i = 0; i < ucode_u32_size; i++) {
		nvgpu_writel(g, gr_gpccs_dmemd_r(0), ucode_u32_data[i]);
		checksum = nvgpu_gr_checksum_u32(checksum, ucode_u32_data[i]);
	}
	nvgpu_log_info(g, "gpccs dmem checksum: 0x%x", checksum);
}

void gb10b_gr_falcon_load_fecs_imem(struct gk20a *g,
			const u32 *ucode_u32_data, u32 ucode_u32_size)
{
	u32 cfg, fecs_imem_size;
	u32 tag, i, pad_start, pad_end;
	u32 checksum = 0;

	/* set access for fecs imem */
	g->ops.gr.falcon.fecs_imemc_write(g, 0U, 0U, 0U, 1U);

	cfg = nvgpu_readl(g, gr_fecs_cfg_r());
	fecs_imem_size = gr_fecs_cfg_imem_sz_v(cfg);

	/* Setup the tags for the instruction memory. */
	tag = 0;
	nvgpu_writel(g, gr_fecs_imemt_r(0), gr_fecs_imemt_tag_f(tag));

	for (i = 0; i < ucode_u32_size; i++) {
		if ((i != 0U) && ((i % (256U/sizeof(u32))) == 0U)) {
			tag = nvgpu_safe_add_u32(tag, 1U);
			nvgpu_writel(g, gr_fecs_imemt_r(0),
				      gr_fecs_imemt_tag_f(tag));
		}
		nvgpu_writel(g, gr_fecs_imemd_r(0), ucode_u32_data[i]);
		checksum = nvgpu_gr_checksum_u32(checksum, ucode_u32_data[i]);
	}

	pad_start = nvgpu_safe_mult_u32(i, 4U);
	pad_end = nvgpu_safe_add_u32(pad_start, nvgpu_safe_add_u32(
			nvgpu_safe_sub_u32(256U, (pad_start % 256U)), 256U));
	for (i = pad_start;
	     (i < nvgpu_safe_mult_u32(fecs_imem_size, 256U)) && i < pad_end;
	     i += 4U) {
		if ((i != 0U) && ((i % 256U) == 0U)) {
			tag = nvgpu_safe_add_u32(tag, 1U);
			nvgpu_writel(g, gr_fecs_imemt_r(0),
				      gr_fecs_imemt_tag_f(tag));
		}
		nvgpu_writel(g, gr_fecs_imemd_r(0), 0);
	}
	nvgpu_log_info(g, "fecs imem checksum: 0x%x", checksum);
}

void gb10b_gr_falcon_load_fecs_dmem(struct gk20a *g,
			const u32 *ucode_u32_data, u32 ucode_u32_size)
{
	u32 i;
	u32 checksum = 0;

	/* set access for fecs dmem */
	g->ops.gr.falcon.fecs_dmemc_write(g, 0U, 0U, 0U, 0U, 1U);

	for (i = 0; i < ucode_u32_size; i++) {
		nvgpu_writel(g, gr_fecs_dmemd_r(0), ucode_u32_data[i]);
		checksum = nvgpu_gr_checksum_u32(checksum, ucode_u32_data[i]);
	}
	nvgpu_log_info(g, "fecs dmem checksum: 0x%x", checksum);
}

void gb10b_gr_falcon_start_ucode(struct gk20a *g)
{
	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gr, " ");

	g->ops.gr.falcon.fecs_ctxsw_clear_mailbox(g, 0U, (~U32(0U)));

	/*
	 * When GPCCS and FECS are booted in NS mode, clear mailbox 1 to skip
	 * the security level check in ucode.
	 */
	g->ops.gr.falcon.fecs_ctxsw_clear_mailbox(g, 1U, (~U32(0U)));

	nvgpu_writel(g, gr_gpccs_dmactl_r(), gr_gpccs_dmactl_require_ctx_f(0U));
	nvgpu_writel(g, gr_fecs_dmactl_r(), gr_fecs_dmactl_require_ctx_f(0U));

	nvgpu_writel(g, gr_gpccs_cpuctl_r(), gr_gpccs_cpuctl_startcpu_f(1U));
	nvgpu_writel(g, gr_fecs_cpuctl_r(), gr_fecs_cpuctl_startcpu_f(1U));

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_gr, "done");
}

void gb10b_gr_falcon_load_gpccs_imem(struct gk20a *g,
			const u32 *ucode_u32_data, u32 ucode_u32_size)
{
	u32 cfg, gpccs_imem_size;
	u32 tag, i, pad_start, pad_end;
	u32 checksum = 0;

	/* enable access for gpccs imem */
	g->ops.gr.falcon.gpccs_imemc_write(g, 0U, 0U, 0U, 1U);

	cfg = nvgpu_readl(g, gr_gpc0_cfg_r());
	gpccs_imem_size = gr_gpc0_cfg_imem_sz_v(cfg);

	/* Setup the tags for the instruction memory. */
	tag = 0;
	nvgpu_writel(g, gr_gpccs_imemt_r(0), gr_gpccs_imemt_tag_f(tag));

	for (i = 0; i < ucode_u32_size; i++) {
		if ((i != 0U) && ((i % (256U/sizeof(u32))) == 0U)) {
			tag = nvgpu_safe_add_u32(tag, 1U);
			nvgpu_writel(g, gr_gpccs_imemt_r(0),
					gr_gpccs_imemt_tag_f(tag));
		}
		nvgpu_writel(g, gr_gpccs_imemd_r(0), ucode_u32_data[i]);
		checksum = nvgpu_gr_checksum_u32(checksum, ucode_u32_data[i]);
	}

	pad_start = nvgpu_safe_mult_u32(i, 4U);
	pad_end = nvgpu_safe_add_u32(pad_start, nvgpu_safe_add_u32(
			nvgpu_safe_sub_u32(256U, (pad_start % 256U)), 256U));
	for (i = pad_start;
		(i < nvgpu_safe_mult_u32(gpccs_imem_size, 256U)) &&
						(i < pad_end); i += 4U) {
		if ((i != 0U) && ((i % 256U) == 0U)) {
			tag = nvgpu_safe_add_u32(tag, 1U);
			nvgpu_writel(g, gr_gpccs_imemt_r(0),
					gr_gpccs_imemt_tag_f(tag));
		}
		nvgpu_writel(g, gr_gpccs_imemd_r(0), 0);
	}

	nvgpu_log_info(g, "gpccs imem checksum: 0x%x", checksum);
}

void gb10b_gr_falcon_fecs_imemc_write(struct gk20a *g, u32 port, u32 offs,
	u32 blk, u32 ainc)
{
	nvgpu_writel(g, gr_fecs_imemc_r(port),
			gr_fecs_imemc_offs_f(offs) |
			gr_fecs_imemc_blk_f(blk) |
			gr_fecs_imemc_aincw_f(ainc));
}
#endif

#ifdef CONFIG_NVGPU_SIM
void gb10b_gr_falcon_configure_fmodel(struct gk20a *g)
{
	nvgpu_log_fn(g, " ");

	nvgpu_writel(g, gr_fecs_ctxsw_mailbox_r(7),
		gr_fecs_ctxsw_mailbox_value_f(0xc0de7777U));
	nvgpu_writel(g, gr_gpccs_ctxsw_mailbox_r(7),
		gr_gpccs_ctxsw_mailbox_value_f(0xc0de7777U));

}
#endif

int gb10b_gr_falcon_ctrl_ctxsw(struct gk20a *g, u32 fecs_method,
		u32 data, u32 *ret_val)
{
	struct nvgpu_fecs_method_op op = {
		.mailbox = { .id = 0U, .data = 0U, .ret = NULL,
			     .clr = ~U32(0U), .ok = 0U, .fail = 0U},
		.method.data = 0U,
		.cond.ok = GR_IS_UCODE_OP_NOT_EQUAL,
		.cond.fail = GR_IS_UCODE_OP_SKIP,
	};
	u32 flags = 0U;
	int ret;

	nvgpu_log_info(g, "fecs method %d data 0x%x ret_val %p",
				fecs_method, data, ret_val);

	switch (fecs_method) {
#ifdef CONFIG_NVGPU_MIG
	case NVGPU_GR_FALCON_METHOD_SET_SMC_LTS_MASK_INDEX:
		op.method.addr =
			gr_fecs_method_push_adr_set_smc_lts_mask_index_v();
		op.method.data = data;
		op.cond.ok = GR_IS_UCODE_OP_SKIP;
		op.mailbox.ok = gr_fecs_ctxsw_mailbox_value_pass_v();
		flags |= NVGPU_GR_FALCON_SUBMIT_METHOD_F_LOCKED;

		ret = gm20b_gr_falcon_submit_fecs_method_op(g, op, flags, fecs_method);
		break;
	case NVGPU_GR_FALCON_METHOD_ASSIGN_SMC_LTS_MASK:
		op.method.addr =
			gr_fecs_method_push_adr_assign_smc_lts_mask_v();
		op.method.data = data;
		op.cond.ok = GR_IS_UCODE_OP_SKIP;
		op.mailbox.ok = gr_fecs_ctxsw_mailbox_value_pass_v();
		flags |= NVGPU_GR_FALCON_SUBMIT_METHOD_F_LOCKED;

		ret = gm20b_gr_falcon_submit_fecs_method_op(g, op, flags, fecs_method);
		break;
#endif
#ifdef CONFIG_NVGPU_POWER_PG
	case NVGPU_GR_FALCON_METHOD_REGLIST_BIND_INSTANCE:
		op.method.addr =
			gr_fecs_method_push_adr_set_reglist_bind_instance_v();
		op.method.data = data;
		op.mailbox.data = 0U;
		op.mailbox.id = 0U;
		op.mailbox.ok = 1U;
		op.cond.ok = GR_IS_UCODE_OP_EQUAL;
		ret = gm20b_gr_falcon_submit_fecs_method_op(g, op, flags,
				fecs_method);
	        break;

	case NVGPU_GR_FALCON_METHOD_REGLIST_BIND_INSTANCE_HI:
		op.method.addr =
			gr_fecs_method_push_adr_set_reglist_bind_instance_hi_v();
		op.method.data = data;
		op.mailbox.data = 0U;
		op.mailbox.id = 0U;
		op.mailbox.ok = 1U;
		op.cond.ok = GR_IS_UCODE_OP_EQUAL;
		ret = gm20b_gr_falcon_submit_fecs_method_op(g, op, flags,
				fecs_method);
	        break;

	case NVGPU_GR_FALCON_METHOD_REGLIST_SET_VIRTUAL_ADDRESS:
		op.method.addr =
			gr_fecs_method_push_adr_set_reglist_virtual_address_v(),
		op.method.data = data;
		op.mailbox.data = 0U;
		op.mailbox.id = 0U;
		op.mailbox.ok = 1U;
		op.cond.ok = GR_IS_UCODE_OP_EQUAL;
		ret = gm20b_gr_falcon_submit_fecs_method_op(g, op, flags,
				fecs_method);
		break;

	case NVGPU_GR_FALCON_METHOD_REGLIST_SET_VIRTUAL_ADDRESS_HI:
		op.method.addr =
			gr_fecs_method_push_adr_set_reglist_virtual_address_hi_v(),
		op.method.data = data;
		op.mailbox.data = 0U;
		op.mailbox.id = 0U;
		op.mailbox.ok = 1U;
		op.cond.ok = GR_IS_UCODE_OP_EQUAL;
		ret = gm20b_gr_falcon_submit_fecs_method_op(g, op, flags,
				fecs_method);
		break;
#endif
	case NVGPU_PRI_FECS_METHOD_PUSH_ADR_BUILD_PRI_ADDRESS_MAP:
		op.method.addr = gr_fecs_method_push_adr_push_adr_build_pri_address_map_v();
		op.method.data = data;
		op.mailbox.data = 0U;
		op.mailbox.id = 0U;
		op.mailbox.ok = gr_fecs_method_push_adr_push_adr_build_pri_address_map_v();
		op.mailbox.fail = gr_fecs_ctxsw_mailbox_value_fail_v();
		op.cond.ok = GR_IS_UCODE_OP_EQUAL;
		if (nvgpu_platform_is_silicon(g)) {
			op.cond.fail = GR_IS_UCODE_OP_EQUAL;
		} else {
			op.cond.fail = GR_IS_UCODE_OP_AND;
		}
		ret = gm20b_gr_falcon_submit_fecs_method_op(g, op, flags,
				fecs_method);
		break;

	case NVGPU_GR_FALCON_METHOD_SET_WATCHDOG_TIMEOUT:
		op.method.addr =
			gr_fecs_method_push_adr_set_watchdog_timeout_f();
		op.method.data = data;
		if (nvgpu_platform_is_silicon(g)) {
			op.cond.ok = GR_IS_UCODE_OP_EQUAL;
			op.mailbox.ok =
				gr_fecs_method_push_adr_set_watchdog_timeout_f();
		} else {
			// pre-si
			op.cond.ok = GR_IS_UCODE_OP_SKIP;
			op.mailbox.ok = gr_fecs_ctxsw_mailbox_value_pass_v();
		}
		flags |= NVGPU_GR_FALCON_SUBMIT_METHOD_F_LOCKED;
		ret = gm20b_gr_falcon_submit_fecs_method_op(g, op, flags,
				fecs_method);
		break;

	case NVGPU_GR_FALCON_METHOD_ADDRESS_BIND_PTR:
		op.method.addr = gr_fecs_method_push_adr_bind_pointer_v();
		op.method.data = data;
		op.mailbox.clr = 0x30U;
		op.mailbox.ok = 0x10U;
		if (nvgpu_platform_is_silicon(g)) {
			op.mailbox.fail = 0x21U;
			op.cond.ok = GR_IS_UCODE_OP_EQUAL;
			op.cond.fail = GR_IS_UCODE_OP_EQUAL;
		} else {
			op.mailbox.fail = 0x20U;
			op.cond.ok = GR_IS_UCODE_OP_AND;
			op.cond.fail = GR_IS_UCODE_OP_AND;
		}
		flags |= NVGPU_GR_FALCON_SUBMIT_METHOD_F_SLEEP;
		ret = gm20b_gr_falcon_submit_fecs_method_op(g, op, flags,
				fecs_method);
		break;

	case NVGPU_GR_FALCON_METHOD_GOLDEN_IMAGE_SAVE:
		op.method.addr = gr_fecs_method_push_adr_wfi_golden_save_v();
		op.method.data = data;
		op.mailbox.clr = 0x3U;
		op.mailbox.ok = 0x1U;
		op.mailbox.fail = 0x2U;
		if (nvgpu_platform_is_silicon(g)) {
			op.cond.ok = GR_IS_UCODE_OP_EQUAL;
			op.cond.fail = GR_IS_UCODE_OP_EQUAL;
		} else {
			op.cond.ok = GR_IS_UCODE_OP_AND;
			op.cond.fail = GR_IS_UCODE_OP_AND;
		}
		flags |= NVGPU_GR_FALCON_SUBMIT_METHOD_F_SLEEP;
		ret = gm20b_gr_falcon_submit_fecs_method_op(g, op, flags,
				fecs_method);
		break;
	default:
		ret = ga100_gr_falcon_ctrl_ctxsw(g, fecs_method,
			data, ret_val);
		break;
	}
	return ret;
}

u32 gb10b_gr_falcon_get_fecs_current_ctx_hi_data(struct gk20a *g,
		struct nvgpu_mem *inst_block)
{
	u64 inst_blk_ptr;
	inst_blk_ptr = nvgpu_inst_block_addr(g, inst_block) >>
		g->ops.ramin.base_shift();

	return gr_fecs_current_ctx_hi_ibp_f(u64_hi32(inst_blk_ptr));
}
