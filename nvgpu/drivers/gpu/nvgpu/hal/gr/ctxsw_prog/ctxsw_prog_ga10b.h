/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CTXSW_PROG_GA10B_H
#define NVGPU_CTXSW_PROG_GA10B_H

#include <nvgpu/types.h>

#define BYTE_TO_DW_SHIFT	(2U)

struct gk20a;
struct nvgpu_mem;

u32 ga10b_ctxsw_prog_hw_get_fecs_header_size(void);

#ifdef CONFIG_NVGPU_DEBUGGER
u32 ga10b_ctxsw_prog_hw_get_gpccs_header_size(void);
bool ga10b_ctxsw_prog_check_main_image_header_magic(u32 *context);
bool ga10b_ctxsw_prog_check_local_header_magic(u32 *context);
u32 ga10b_ctxsw_prog_hw_get_main_header_size(void);
u32 ga10b_ctxsw_prog_hw_get_gpccs_header_stride(void);
u32 ga10b_ctxsw_prog_get_compute_sysreglist_offset(u32 *fecs_hdr);
u32 ga10b_ctxsw_prog_get_gfx_sysreglist_offset(u32 *fecs_hdr);
u32 ga10b_ctxsw_prog_get_ltsreglist_offset(u32 *fecs_hdr);
u32 ga10b_ctxsw_prog_get_compute_gpcreglist_offset(u32 *gpccs_hdr);
u32 ga10b_ctxsw_prog_get_gfx_gpcreglist_offset(u32 *gpccs_hdr) ;
u32 ga10b_ctxsw_prog_get_compute_ppcreglist_offset(u32 *gpccs_hdr);
u32 ga10b_ctxsw_prog_get_gfx_ppcreglist_offset(u32 *gpccs_hdr);
u32 ga10b_ctxsw_prog_get_compute_tpcreglist_offset(u32 *gpccs_hdr, u32 tpc_num);
u32 ga10b_ctxsw_prog_get_gfx_tpcreglist_offset(u32 *gpccs_hdr, u32 tpc_num);
u32 ga10b_ctxsw_prog_get_compute_etpcreglist_offset(u32 *gpccs_hdr);
u32 ga10b_ctxsw_prog_get_gfx_etpcreglist_offset(u32 *gpccs_hdr);
u32 ga10b_ctxsw_prog_get_tpc_segment_pri_layout(struct gk20a *g, u32 *main_hdr);
#endif /* CONFIG_NVGPU_DEBUGGER */
#ifdef CONFIG_DEBUG_FS
void ga10b_ctxsw_prog_dump_ctxsw_stats(struct gk20a *g,
	struct nvgpu_mem *ctx_mem);
#endif

#endif /* NVGPU_CTXSW_PROG_GA10B_H */
