/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_MMU_FAULT_TU104_H
#define NVGPU_FB_MMU_FAULT_TU104_H

#include <nvgpu/types.h>

struct gk20a;

void tu104_fb_handle_mmu_fault(struct gk20a *g);

void tu104_fb_write_mmu_fault_buffer_lo_hi(struct gk20a *g, u32 index,
	u32 addr_lo, u32 addr_hi);
u32  tu104_fb_read_mmu_fault_buffer_get(struct gk20a *g, u32 index);
void tu104_fb_write_mmu_fault_buffer_get(struct gk20a *g, u32 index,
	u32 reg_val);
u32  tu104_fb_read_mmu_fault_buffer_put(struct gk20a *g, u32 index);
u32  tu104_fb_read_mmu_fault_buffer_size(struct gk20a *g, u32 index);
void tu104_fb_write_mmu_fault_buffer_size(struct gk20a *g, u32 index,
	u32 reg_val);
void tu104_fb_read_mmu_fault_addr_lo_hi(struct gk20a *g,
	u32 *addr_lo, u32 *addr_hi);
void tu104_fb_read_mmu_fault_inst_lo_hi(struct gk20a *g,
	u32 *inst_lo, u32 *inst_hi);
u32  tu104_fb_read_mmu_fault_info(struct gk20a *g);
u32  tu104_fb_read_mmu_fault_status(struct gk20a *g);
void tu104_fb_write_mmu_fault_status(struct gk20a *g, u32 reg_val);

#ifdef CONFIG_NVGPU_REPLAYABLE_FAULT
int  tu104_fb_mmu_invalidate_replay(struct gk20a *g,
	u32 invalidate_replay_val);
#endif

#endif /* NVGPU_FB_MMU_FAULT_TU104_H */

