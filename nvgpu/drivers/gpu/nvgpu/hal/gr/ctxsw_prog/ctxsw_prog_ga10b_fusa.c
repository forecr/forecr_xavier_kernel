// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/utils.h>

#include "ctxsw_prog_ga10b.h"

#include <nvgpu/hw/ga10b/hw_ctxsw_prog_ga10b.h>

#define CTXSWBUF_SEGMENT_BLKSIZE	(256U)

u32 ga10b_ctxsw_prog_hw_get_fecs_header_size(void)
{
	return ctxsw_prog_fecs_header_size_in_bytes_v();
}

#ifdef CONFIG_NVGPU_DEBUGGER
u32 ga10b_ctxsw_prog_hw_get_main_header_size(void)
{
	return ctxsw_prog_ctxsw_header_size_in_bytes_v();
}

u32 ga10b_ctxsw_prog_hw_get_gpccs_header_stride(void)
{
	return ctxsw_prog_gpccs_header_stride_v();
}

u32 ga10b_ctxsw_prog_get_tpc_segment_pri_layout(struct gk20a *g, u32 *main_hdr)
{
	(void)g;
	return ctxsw_prog_main_tpc_segment_pri_layout_v_v(
			main_hdr[ctxsw_prog_main_tpc_segment_pri_layout_o() >>
			BYTE_TO_DW_SHIFT]);
}

u32 ga10b_ctxsw_prog_get_compute_sysreglist_offset(u32 *fecs_hdr)
{
	return ctxsw_prog_local_sys_reglist_offset_compute_v(
		fecs_hdr[ctxsw_prog_local_sys_reglist_offset_o() >>
		BYTE_TO_DW_SHIFT]) * CTXSWBUF_SEGMENT_BLKSIZE;
}

u32 ga10b_ctxsw_prog_get_gfx_sysreglist_offset(u32 *fecs_hdr)
{
	return ctxsw_prog_local_sys_reglist_offset_graphics_v(
		fecs_hdr[ctxsw_prog_local_sys_reglist_offset_o() >>
		BYTE_TO_DW_SHIFT]) * CTXSWBUF_SEGMENT_BLKSIZE;
}

u32 ga10b_ctxsw_prog_get_ltsreglist_offset(u32 *fecs_hdr)
{
	return ctxsw_prog_local_lts_reglist_offset_v_v(
		fecs_hdr[ctxsw_prog_local_lts_reglist_offset_o() >>
		BYTE_TO_DW_SHIFT]) * CTXSWBUF_SEGMENT_BLKSIZE;
}

u32 ga10b_ctxsw_prog_get_compute_gpcreglist_offset(u32 *gpccs_hdr)
{
	return ctxsw_prog_local_gpc_reglist_offset_compute_v(
		gpccs_hdr[ctxsw_prog_local_gpc_reglist_offset_o() >>
		BYTE_TO_DW_SHIFT]) * CTXSWBUF_SEGMENT_BLKSIZE;
}

u32 ga10b_ctxsw_prog_get_gfx_gpcreglist_offset(u32 *gpccs_hdr)
{
	return ctxsw_prog_local_gpc_reglist_offset_graphics_v(
		gpccs_hdr[ctxsw_prog_local_gpc_reglist_offset_o() >>
		BYTE_TO_DW_SHIFT]) * CTXSWBUF_SEGMENT_BLKSIZE;
}

u32 ga10b_ctxsw_prog_get_compute_ppcreglist_offset(u32 *gpccs_hdr)
{
	return ctxsw_prog_local_ppc_reglist_offset_compute_v(
		gpccs_hdr[ctxsw_prog_local_ppc_reglist_offset_o() >>
		BYTE_TO_DW_SHIFT]) * CTXSWBUF_SEGMENT_BLKSIZE;
}

u32 ga10b_ctxsw_prog_get_gfx_ppcreglist_offset(u32 *gpccs_hdr)
{
	return ctxsw_prog_local_ppc_reglist_offset_graphics_v(
		gpccs_hdr[ctxsw_prog_local_ppc_reglist_offset_o() >>
		BYTE_TO_DW_SHIFT]) * CTXSWBUF_SEGMENT_BLKSIZE;
}

u32 ga10b_ctxsw_prog_get_compute_tpcreglist_offset(u32 *gpccs_hdr, u32 tpc_num)
{
	return ctxsw_prog_local_tpc_reglist_offset_compute_v(
		gpccs_hdr[ctxsw_prog_local_tpc_reglist_offset_r(tpc_num) >>
		BYTE_TO_DW_SHIFT]) * CTXSWBUF_SEGMENT_BLKSIZE;
}

u32 ga10b_ctxsw_prog_get_gfx_tpcreglist_offset(u32 *gpccs_hdr, u32 tpc_num)
{
	return ctxsw_prog_local_tpc_reglist_offset_graphics_v(
		gpccs_hdr[ctxsw_prog_local_tpc_reglist_offset_r(tpc_num) >>
		BYTE_TO_DW_SHIFT]) * CTXSWBUF_SEGMENT_BLKSIZE;
}

u32 ga10b_ctxsw_prog_get_compute_etpcreglist_offset(u32 *gpccs_hdr)
{
	return ctxsw_prog_local_ext_tpc_reglist_offset_compute_v(
		gpccs_hdr[ctxsw_prog_local_ext_tpc_reglist_offset_o() >>
		BYTE_TO_DW_SHIFT]) * CTXSWBUF_SEGMENT_BLKSIZE;
}

u32 ga10b_ctxsw_prog_get_gfx_etpcreglist_offset(u32 *gpccs_hdr)
{
	return ctxsw_prog_local_ext_tpc_reglist_offset_graphics_v(
		gpccs_hdr[ctxsw_prog_local_ext_tpc_reglist_offset_o() >>
		BYTE_TO_DW_SHIFT]) * CTXSWBUF_SEGMENT_BLKSIZE;
}
#endif /* CONFIG_NVGPU_DEBUGGER */
