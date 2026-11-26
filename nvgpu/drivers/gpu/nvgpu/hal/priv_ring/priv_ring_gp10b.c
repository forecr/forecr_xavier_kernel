// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/timers.h>
#include <nvgpu/enabled.h>
#include <nvgpu/io.h>
#include <nvgpu/utils.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_err.h>
#include <nvgpu/static_analysis.h>

#include <nvgpu/hw/gp10b/hw_pri_ringmaster_gp10b.h>
#include <nvgpu/hw/gp10b/hw_pri_ringstation_sys_gp10b.h>
#include <nvgpu/hw/gp10b/hw_pri_ringstation_gpc_gp10b.h>

#include "priv_ring_gp10b.h"

static const char *const error_type_badf1xyy[] = {
	"client timeout",
	"decode error",
	"client in reset",
	"client floorswept",
	"client stuck ack",
	"client expected ack",
	"fence error",
	"subid error",
	"byte access unsupported",
};

static const char *const error_type_badf2xyy[] = {
	"orphan gpc/fbp"
};

static const char *const error_type_badf3xyy[] = {
	"priv ring dead"
};

static const char *const error_type_badf5xyy[] = {
	"client error",
	"priv level violation",
	"indirect priv level violation",
	"local local ring error",
	"falcon mem access priv level violation",
	"pri route error"
};

void gp10b_priv_ring_decode_error_code(struct gk20a *g,
			u32 error_code)
{
	u32 error_type_index;

	error_type_index = (error_code & 0x00000f00U) >> 8U;
	error_code = error_code & 0xBADFf000U;

	if (error_code == 0xBADF1000U) {
		if (error_type_index <
				ARRAY_SIZE(error_type_badf1xyy)) {
			nvgpu_err(g, "%s",
				error_type_badf1xyy[error_type_index]);
		}
	} else if (error_code == 0xBADF2000U) {
		if (error_type_index <
				ARRAY_SIZE(error_type_badf2xyy)) {
			nvgpu_err(g, "%s",
				error_type_badf2xyy[error_type_index]);
		}
	} else if (error_code == 0xBADF3000U) {
		if (error_type_index <
				ARRAY_SIZE(error_type_badf3xyy)) {
			nvgpu_err(g, "%s",
				error_type_badf3xyy[error_type_index]);
		}
	} else if (error_code == 0xBADF5000U) {
		if (error_type_index <
				ARRAY_SIZE(error_type_badf5xyy)) {
			nvgpu_err(g, "%s",
				error_type_badf5xyy[error_type_index]);
		}
	} else {
		nvgpu_log_info(g, "Decoding error code 0x%x not supported.",
				error_code);
	}
}

void gp10b_priv_ring_isr_handle_0(struct gk20a *g, u32 status0)
{
	u32 error_info;
	u32 error_code;
	u32 error_adr, error_wrdat;

	if (pri_ringmaster_intr_status0_ring_start_conn_fault_v(status0) != 0U) {
		nvgpu_err(g,
			"BUG: connectivity problem on the startup sequence");
	}

	if (pri_ringmaster_intr_status0_disconnect_fault_v(status0) != 0U) {
		nvgpu_err(g, "ring disconnected");
	}

	if (pri_ringmaster_intr_status0_overflow_fault_v(status0) != 0U) {
		nvgpu_err(g, "ring overflowed");
	}

	if (pri_ringmaster_intr_status0_gbl_write_error_sys_v(status0) != 0U) {
		error_info =
			nvgpu_readl(g, pri_ringstation_sys_priv_error_info_r());
		error_code =
			nvgpu_readl(g, pri_ringstation_sys_priv_error_code_r());
		error_adr =
			nvgpu_readl(g, pri_ringstation_sys_priv_error_adr_r());
		error_wrdat =
			nvgpu_readl(g, pri_ringstation_sys_priv_error_wrdat_r());
		nvgpu_err(g, "SYS write error. ADR 0x%08x WRDAT 0x%08x "
				"INFO 0x%08x (subid 0x%08x priv level %d), "
				"CODE 0x%08x",
			error_adr,
			error_wrdat,
			error_info,
			pri_ringstation_sys_priv_error_info_subid_v(error_info),
			pri_ringstation_sys_priv_error_info_priv_level_v(error_info),
			error_code);
		if (g->ops.priv_ring.decode_error_code != NULL) {
			g->ops.priv_ring.decode_error_code(g, error_code);
		}
	}
}

void gp10b_priv_ring_isr_handle_1(struct gk20a *g, u32 status1)
{
	u32 error_info;
	u32 error_code;
	u32 error_adr, error_wrdat;
	u32 gpc;
	u32 gpc_stride, gpc_offset;

	if (status1 != 0U) {
		gpc_stride = nvgpu_get_litter_value(g, GPU_LIT_GPC_PRIV_STRIDE);
		for (gpc = 0U; gpc < g->ops.priv_ring.get_gpc_count(g); gpc++) {
			if ((status1 & BIT32(gpc)) == 0U) {
				continue;
			}
			gpc_offset = nvgpu_safe_mult_u32(gpc, gpc_stride);
			error_info = nvgpu_readl(g,
				nvgpu_safe_add_u32(
				   pri_ringstation_gpc_gpc0_priv_error_info_r(),
				   gpc_offset));
			error_code = nvgpu_readl(g,
				nvgpu_safe_add_u32(
				   pri_ringstation_gpc_gpc0_priv_error_code_r(),
				   gpc_offset));
			error_adr = nvgpu_readl(g,
				nvgpu_safe_add_u32(
				  pri_ringstation_gpc_gpc0_priv_error_adr_r(),
				  gpc_offset));
			error_wrdat = nvgpu_readl(g,
				nvgpu_safe_add_u32(
				  pri_ringstation_gpc_gpc0_priv_error_wrdat_r(),
				  gpc_offset));

			nvgpu_err(g, "GPC%u write error. ADR 0x%08x "
				"WRDAT 0x%08x "
				"INFO 0x%08x (subid 0x%08x priv level %d), "
				"CODE 0x%08x", gpc,
				error_adr,
				error_wrdat,
				error_info,
				pri_ringstation_gpc_gpc0_priv_error_info_subid_v(error_info),
				pri_ringstation_gpc_gpc0_priv_error_info_priv_level_v(error_info),
				error_code);

			if (g->ops.priv_ring.decode_error_code != NULL) {
				g->ops.priv_ring.decode_error_code(g, error_code);
			}

			status1 = status1 & (~(BIT32(gpc)));
			if (status1 == 0U) {
				break;
			}
		}
	}
}
