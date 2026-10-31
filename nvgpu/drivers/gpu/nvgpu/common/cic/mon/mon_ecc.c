// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/nvgpu_err_info.h>
#include <nvgpu/cic_mon.h>

#include "cic_mon_priv.h"

void nvgpu_report_ecc_err(struct gk20a *g, u32 hw_unit, u32 inst,
		u32 err_id, u64 err_addr, u64 err_count)
{
	int err = 0;
	struct nvgpu_err_desc *err_desc = NULL;
	struct nvgpu_err_msg err_pkt;

	if (g->ops.cic_mon.report_err == NULL) {
		cic_dbg(g, "CIC does not support reporting error "
			       "to safety services");
		return;
	}

	err = nvgpu_cic_mon_get_err_desc(g, hw_unit, err_id, &err_desc);
	if (err != 0) {
		nvgpu_err(g, "Failed to get err_desc for "
			       "err_id (%u) for hw module (%u)",
				err_id, hw_unit);
		goto handle_report_failure;
	}

	nvgpu_init_ecc_err_msg(&err_pkt);
	err_pkt.hw_unit_id = hw_unit;
	err_pkt.err_id = err_desc->error_id;
	err_pkt.is_critical = err_desc->is_critical;
	err_pkt.err_info.ecc_info.header.sub_unit_id = inst;
	err_pkt.err_info.ecc_info.header.address = err_addr;
	err_pkt.err_info.ecc_info.err_cnt = err_count;
	err_pkt.err_desc = err_desc;
	err_pkt.err_size = nvgpu_safe_cast_u64_to_u8(
			sizeof(err_pkt.err_info.ecc_info));

handle_report_failure:
	if (err != 0) {
		nvgpu_sw_quiesce(g);
	}
}

void nvgpu_inject_ecc_swerror(struct gk20a *g, u32 hw_unit, u32 err_index,
		u32 inst)
{
	u64 err_addr, err_count;

	err_addr = (u64)ERR_INJECT_TEST_PATTERN;
	err_count = (u64)ERR_INJECT_TEST_PATTERN;

	nvgpu_report_ecc_err(g, hw_unit, inst, err_index, err_addr, err_count);
}
