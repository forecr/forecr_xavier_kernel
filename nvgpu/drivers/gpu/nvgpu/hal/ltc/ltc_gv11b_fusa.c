// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/static_analysis.h>
#include <nvgpu/mc.h>

#include "ltc_gv11b.h"

#include <nvgpu/hw/gv11b/hw_ltc_gv11b.h>

#include <nvgpu/utils.h>

/*
 * Sets the ZBC stencil for the passed index.
 */

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gv11b_ltc_init_fs_state(struct gk20a *g)
{
	u32 reg;
	u32 line_size = 512U;

	nvgpu_log_info(g, "initialize gv11b l2");

	g->ltc->max_ltc_count = g->ops.top.get_num_ltcs(g);
	g->ltc->ltc_count = g->ops.priv_ring.enum_ltc(g);
	nvgpu_log_info(g, "%u ltcs out of %u", g->ltc->ltc_count,
					g->ltc->max_ltc_count);

	reg = nvgpu_readl(g, ltc_ltcs_ltss_cbc_param_r());
	g->ltc->slices_per_ltc = ltc_ltcs_ltss_cbc_param_slices_per_ltc_v(reg);;
	g->ltc->cacheline_size =
		line_size << ltc_ltcs_ltss_cbc_param_cache_line_size_v(reg);
}
#endif

int gv11b_lts_ecc_init(struct gk20a *g)
{
	int err = 0;

	err = NVGPU_ECC_COUNTER_INIT_PER_LTS(ecc_sec_count);
	if (err != 0) {
		goto done;
	}

	err = NVGPU_ECC_COUNTER_INIT_PER_LTS(ecc_ded_count);
	if (err != 0) {
		goto done;
	}

	err = NVGPU_ECC_COUNTER_INIT_PER_LTS(tstg_ecc_parity_count);
	if (err != 0) {
		goto done;
	}

	err = NVGPU_ECC_COUNTER_INIT_PER_LTS(dstg_be_ecc_parity_count);
	if (err != 0) {
		goto done;
	}

done:
	if (err != 0) {
		nvgpu_err(g, "ecc counter allocate failed, err=%d", err);
		nvgpu_ltc_ecc_free(g);
	}

	return err;
}

#ifdef CONFIG_NVGPU_GRAPHICS
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
/*
 * Sets the ZBC stencil for the passed index.
 */
void gv11b_ltc_set_zbc_stencil_entry(struct gk20a *g,
					  u32 stencil_depth,
					  u32 index)
{
	nvgpu_writel(g, ltc_ltcs_ltss_dstg_zbc_index_r(),
		ltc_ltcs_ltss_dstg_zbc_index_address_f(index));

	nvgpu_writel(g, ltc_ltcs_ltss_dstg_zbc_stencil_clear_value_r(),
			stencil_depth);
}
#endif
#endif /* CONFIG_NVGPU_GRAPHICS */
