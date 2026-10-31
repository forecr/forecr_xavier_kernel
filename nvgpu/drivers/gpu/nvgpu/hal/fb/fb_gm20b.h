/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_GM20B
#define NVGPU_FB_GM20B

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_mem;

void gm20b_fb_init_hw(struct gk20a *g);
int gm20b_fb_tlb_invalidate(struct gk20a *g, struct nvgpu_mem *pdb);
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void fb_gm20b_init_fs_state(struct gk20a *g);
void gm20b_fb_set_mmu_page_size(struct gk20a *g);
#endif
u32 gm20b_fb_mmu_ctrl(struct gk20a *g);
u32 gm20b_fb_mmu_debug_ctrl(struct gk20a *g);
u32 gm20b_fb_mmu_debug_wr(struct gk20a *g);
u32 gm20b_fb_mmu_debug_rd(struct gk20a *g);
#ifdef CONFIG_NVGPU_COMPRESSION
bool gm20b_fb_set_use_full_comp_tag_line(struct gk20a *g);
u64 gm20b_fb_compression_page_size(struct gk20a *g);
unsigned int gm20b_fb_compressible_page_size(struct gk20a *g);
u64 gm20b_fb_compression_align_mask(struct gk20a *g);
#endif
#ifdef CONFIG_NVGPU_VPR
int gm20b_fb_vpr_info_fetch(struct gk20a *g);
void gm20b_fb_dump_vpr_info(struct gk20a *g);
#endif
void gm20b_fb_dump_wpr_info(struct gk20a *g);
void gm20b_fb_read_wpr_info(struct gk20a *g, u64 *wpr_base, u64 *wpr_size);
#ifdef CONFIG_NVGPU_DEBUGGER
bool gm20b_fb_debug_mode_enabled(struct gk20a *g);
void gm20b_fb_set_debug_mode(struct gk20a *g, bool enable);
void gm20b_fb_set_mmu_debug_mode(struct gk20a *g, bool enable);
#endif

#endif
