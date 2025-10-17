// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/regops.h>

#include "hal/gr/gr/gr_gk20a.h"
#include "ltc_gb10b.h"

#include <nvgpu/hw/gb10b/hw_ltc_gb10b.h>

#ifdef CONFIG_NVGPU_GRAPHICS
void gb10b_ltc_set_zbc_stencil_entry(struct gk20a *g, u32 stencil_depth,
					u32 index)
{
	nvgpu_writel(g, ltc_ltcs_ltss_dstg_zbc_index_r(),
		ltc_ltcs_ltss_dstg_zbc_index_address_f(index));

	nvgpu_writel(g,
		ltc_ltcs_ltss_dstg_zbc_stencil_clear_value_r(), stencil_depth);
}

void gb10b_ltc_set_zbc_color_entry(struct gk20a *g, u32 *color_l2, u32 index)
{
	u32 i;

	nvgpu_writel(g, ltc_ltcs_ltss_dstg_zbc_index_r(),
		ltc_ltcs_ltss_dstg_zbc_index_address_f(index));

	for (i = 0; i < ltc_ltcs_ltss_dstg_zbc_color_clear_value__size_1_v();
									i++) {
		nvgpu_writel(g,
			ltc_ltcs_ltss_dstg_zbc_color_clear_value_r(i),
			color_l2[i]);
	}
}

/*
 * Sets the ZBC depth for the passed index.
 */
void gb10b_ltc_set_zbc_depth_entry(struct gk20a *g, u32 depth_val, u32 index)
{
	nvgpu_writel(g, ltc_ltcs_ltss_dstg_zbc_index_r(),
		ltc_ltcs_ltss_dstg_zbc_index_address_f(index));

	nvgpu_writel(g,
		ltc_ltcs_ltss_dstg_zbc_depth_clear_value_r(), depth_val);
}
#endif

#ifdef CONFIG_NVGPU_DEBUGGER

bool gb10b_ltc_pri_is_ltc_addr(struct gk20a *g, u32 addr)
{
	const u32 ltc_stride = nvgpu_get_litter_value(g, GPU_LIT_LTC_STRIDE);
	u32 ltc_loc;

	if ((addr >= ltc_pltcg_base_v()) && (addr < ltc_pltcg_extent_v())) {
		return true;
	}

	if (nvgpu_safe_add_u32_return(ltc_pltcg_ltcs_base_v(), ltc_stride, &ltc_loc) == false) {
		nvgpu_err(g, "buffer overflow");
		return false;
	}
	if ((addr >= ltc_pltcg_ltcs_base_v()) && (addr < ltc_loc)) {
		/*
		 * The LTC broadcast range is disjoint from the LTC_PLCTG range
		 * from GB10B.
		 */
		return true;
	}

	return  false;
}

bool gb10b_ltc_is_pltcg_ltcs_addr(struct gk20a *g, u32 addr)
{
	const u32 ltc_stride = nvgpu_get_litter_value(g, GPU_LIT_LTC_STRIDE);
	u32 ltc_loc;

	if (nvgpu_safe_add_u32_return(ltc_pltcg_ltcs_base_v(), ltc_stride, &ltc_loc) == false) {
		nvgpu_err(g, "buffer overflow");
		return false;
	}
	return ((addr >= ltc_pltcg_ltcs_base_v()) && (addr < ltc_loc));
}

bool gb10b_ltc_is_ltcs_ltss_addr(struct gk20a *g, u32 addr)
{
	u32 ltc_shared_base = ltc_ltcs_ltss_v();
	u32 lts_stride = nvgpu_get_litter_value(g, GPU_LIT_LTS_STRIDE);
	u32 lts_loc;

	if (nvgpu_safe_add_u32_return(ltc_shared_base, lts_stride, &lts_loc) == false) {
		nvgpu_err(g, "buffer overflow");
		return false;
	}
	if (addr >= ltc_shared_base) {
		return (addr < lts_loc);
	}
	return false;
}

int gb10b_ltc_pri_shared_addr(struct gk20a *g, u32 addr, u32 *ltc_shared_addr)
{
	u32 ltc_stride = nvgpu_get_litter_value(g, GPU_LIT_LTC_STRIDE);
	u32 lts_stride = nvgpu_get_litter_value(g, GPU_LIT_LTS_STRIDE);
	u32 ltc_shared_base = ltc_ltcs_ltss_v();
	u32 ltc_addr_mask = nvgpu_safe_sub_u32(ltc_stride, 1);
	u32 lts_addr_mask = nvgpu_safe_sub_u32(lts_stride, 1);
	u32 ltc_addr = addr & ltc_addr_mask;
	u32 lts_addr = ltc_addr & lts_addr_mask;

	if (nvgpu_safe_add_u32_return(ltc_shared_base, lts_addr, ltc_shared_addr) == false) {
		nvgpu_err(g, "buffer overflow");
		return -EOVERFLOW;
	}

	return 0;
}

int gb10b_set_l2_sector_promotion(struct gk20a *g, struct nvgpu_tsg *tsg,
		u32 policy)
{
	int err = 0;
	struct nvgpu_dbg_reg_op cfg_ops[2] = {
		{
			.op = REGOP(READ_32),
			.type = REGOP(TYPE_GR_CTX),
			.offset = ltc_ltcs_ltss_tstg_cfg2_r()
		},
		{
			.op = REGOP(READ_32),
			.type = REGOP(TYPE_GR_CTX),
			.offset = ltc_ltcs_ltss_tstg_cfg3_r()
		},
	};
	u32 flags = NVGPU_REG_OP_FLAG_MODE_ALL_OR_NONE;
	u32 num_ops = 2U;
	u32 cfg2_vidmem = 0U, cfg3_sysmem = 0U;

	/*
	 * Read current value for ltc_ltcs_ltss_tstg_cfg(2,3)_r
	 */
	err = gr_gk20a_exec_ctx_ops(tsg, cfg_ops, num_ops, 0, num_ops, &flags);
	if (err != 0) {
		nvgpu_err(g, "failed to read ltcs_ltss_tstg_cfg(2,3)_r");
		goto fail;
	}
	cfg2_vidmem = cfg_ops[0].value_lo;
	cfg3_sysmem = cfg_ops[1].value_lo;

#define APPLY_SECTOR_PROMOTION_POLICY(cfg, unit, policy)				\
	do {										\
		switch (policy) {							\
		case NVGPU_L2_SECTOR_PROMOTE_FLAG_NONE:					\
			cfg = set_field(cfg,						\
				ltc_ltcs_ltss_tstg_##cfg##_##unit##_promote_m(),	\
				ltc_ltcs_ltss_tstg_##cfg##_##unit##_promote_f(		\
				ltc_ltcs_ltss_tstg_##cfg##_##unit##_promote_none_v()	\
				));							\
			break;								\
		case NVGPU_L2_SECTOR_PROMOTE_FLAG_64B:					\
			cfg = set_field(cfg,						\
				ltc_ltcs_ltss_tstg_##cfg##_##unit##_promote_m(),	\
				ltc_ltcs_ltss_tstg_##cfg##_##unit##_promote_f(		\
				ltc_ltcs_ltss_tstg_##cfg##_##unit##_promote_64b_v()	\
				));							\
			break;								\
		case NVGPU_L2_SECTOR_PROMOTE_FLAG_128B:					\
			cfg = set_field(cfg,						\
				ltc_ltcs_ltss_tstg_##cfg##_##unit##_promote_m(),	\
				ltc_ltcs_ltss_tstg_##cfg##_##unit##_promote_f(		\
				ltc_ltcs_ltss_tstg_##cfg##_##unit##_promote_128b_v()	\
				));							\
			break;								\
		}									\
	} while (0)

	/*
	 * Update T1_PROMOTE and L1_PROMOTE fields of cfg2_vidmem and
	 * cfg3_sysmem.
	 */
	APPLY_SECTOR_PROMOTION_POLICY(cfg2_vidmem, t1, policy);
	APPLY_SECTOR_PROMOTION_POLICY(cfg2_vidmem, l1, policy);
	APPLY_SECTOR_PROMOTION_POLICY(cfg3_sysmem, t1, policy);
	APPLY_SECTOR_PROMOTION_POLICY(cfg3_sysmem, l1, policy);

#undef APPLY_SECTOR_PROMOTION_POLICY

	cfg_ops[0].op = REGOP(WRITE_32);
	cfg_ops[0].value_lo = cfg2_vidmem;
	cfg_ops[1].op = REGOP(WRITE_32);
	cfg_ops[1].value_lo = cfg3_sysmem;
	err = gr_gk20a_exec_ctx_ops(tsg, cfg_ops, num_ops, num_ops, 0, &flags);
	if (err != 0) {
		nvgpu_err(g, "failed to update ltcs_ltss_tstg_cfg(2,3)_r");
		goto fail;
	}

	/* Readback and verify the write */
	cfg_ops[0].op = REGOP(READ_32);
	cfg_ops[0].value_lo = 0U;
	cfg_ops[1].op = REGOP(READ_32);
	cfg_ops[1].value_lo = 0U;
	err = gr_gk20a_exec_ctx_ops(tsg, cfg_ops, num_ops, 0, num_ops, &flags);
	if (err != 0) {
		nvgpu_err(g, "failed to read ltcs_ltss_tstg_cfg(2,3)_r");
		goto fail;
	}

	if ((get_field(cfg2_vidmem,
			ltc_ltcs_ltss_tstg_cfg2_vidmem_t1_promote_m()) !=
	     get_field(cfg_ops[0].value_lo,
			ltc_ltcs_ltss_tstg_cfg2_vidmem_t1_promote_m())) ||
	    (get_field(cfg2_vidmem,
			ltc_ltcs_ltss_tstg_cfg2_vidmem_l1_promote_m()) !=
	     get_field(cfg_ops[0].value_lo,
			ltc_ltcs_ltss_tstg_cfg2_vidmem_l1_promote_m()))) {
		nvgpu_err(g, "mismatch: cfg2: wrote(0x%x) read(0x%x)",
				cfg2_vidmem, cfg_ops[0].value_lo);
		err = -EINVAL;
		goto fail;
	}

	if ((get_field(cfg3_sysmem,
			ltc_ltcs_ltss_tstg_cfg3_sysmem_t1_promote_m()) !=
	     get_field(cfg_ops[1].value_lo,
			ltc_ltcs_ltss_tstg_cfg3_sysmem_t1_promote_m())) ||
	    (get_field(cfg3_sysmem,
			ltc_ltcs_ltss_tstg_cfg3_sysmem_l1_promote_m()) !=
	     get_field(cfg_ops[1].value_lo,
			ltc_ltcs_ltss_tstg_cfg3_sysmem_l1_promote_m()))) {
		nvgpu_err(g, "mismatch: cfg3: wrote(0x%x) read(0x%x)",
				cfg3_sysmem, cfg_ops[1].value_lo);
		err = -EINVAL;
		goto fail;
	}

fail:
	return err;
}
#endif
