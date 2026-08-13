// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/ltc.h>
#include <nvgpu/log.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/ecc.h>
#include <nvgpu/static_analysis.h>

#include <nvgpu/hw/gp10b/hw_ltc_gp10b.h>

#include "ltc_gm20b.h"
#include "ltc_gp10b.h"

void gp10b_ltc_init_fs_state(struct gk20a *g)
{
	gm20b_ltc_init_fs_state(g);

	gk20a_writel(g, ltc_ltca_g_axi_pctrl_r(),
			ltc_ltca_g_axi_pctrl_user_sid_f(g->ltc_streamid));

}

int gp10b_lts_ecc_init(struct gk20a *g)
{
	int err = 0;

	err = NVGPU_ECC_COUNTER_INIT_PER_LTS(ecc_sec_count);
	if (err != 0) {
		goto init_lts_err;
	}
	err = NVGPU_ECC_COUNTER_INIT_PER_LTS(ecc_ded_count);

init_lts_err:
	if (err != 0) {
		nvgpu_err(g, "ecc counter allocate failed, err=%d", err);
		nvgpu_ltc_ecc_free(g);
	}

	return err;
}
