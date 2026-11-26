/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_GB10B_H
#define NVGPU_FB_GB10B_H

#define ALIGN_HI32(x)	nvgpu_safe_sub_u32(32U, (x))

struct gk20a;
struct nvgpu_cbc;

void gb10b_fb_init_fs_state(struct gk20a *g);
void gb10b_fb_init_hw(struct gk20a *g);
int gb10b_fb_set_atomic_mode(struct gk20a *g);
int gb10b_fb_serial_ats_ctrl(struct gk20a *g, bool enable);
u32 gb10b_fb_mmu_ctrl(struct gk20a *g);
u32 gb10b_fb_mmu_debug_ctrl(struct gk20a *g);
u32 gb10b_fb_mmu_debug_wr(struct gk20a *g);
u32 gb10b_fb_mmu_debug_rd(struct gk20a *g);
u32 gb10b_fb_get_num_active_ltcs(struct gk20a *g);
void gb10b_fb_read_wpr_info(struct gk20a *g, u64 *wpr_base, u64 *wpr_size);
#if defined(CONFIG_NVGPU_COMPRESSION)
u64 gb10b_fb_compression_page_size(struct gk20a *g);
u32 gb10b_fb_compressible_page_size(struct gk20a *g);
u64 gb10b_fb_compression_align_mask(struct gk20a *g);
#endif

#ifdef CONFIG_NVGPU_DEBUGGER
bool gb10b_fb_debug_mode_enabled(struct gk20a *g);
void gb10b_fb_set_mmu_debug_mode(struct gk20a *g, bool enable);
#endif

#ifdef CONFIG_NVGPU_MIG
int gb10b_fb_config_veid_smc_map(struct gk20a *g, bool enable);
void gb10b_fb_set_smc_veid_table_mask(struct gk20a *g, u32 veid_enable_mask);
void gb10b_fb_configure_vpr_mmu_mig(struct gk20a *g, bool swizzid_check_enable, u32 gfx_swizzid);
void gb10b_fb_set_swizzid(struct gk20a *g, u32 mmu_fault_id, u32 swizz_id);
#endif

int gb10b_fb_set_fbp_mask(struct gk20a *g, u32 num_gpu_instances);
#endif /* NVGPU_FB_GB10B_H */
