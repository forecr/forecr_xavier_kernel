/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_MM_VGPU_H
#define NVGPU_MM_VGPU_H

struct nvgpu_mem;
struct nvgpu_channel;
struct vm_gk20a_mapping_batch;
struct gk20a_as_share;
struct vm_gk20a;
enum gk20a_mem_rw_flag;
struct nvgpu_sgt;
enum nvgpu_aperture;

int vgpu_locked_gmmu_unmap(struct vm_gk20a *vm,
				u64 vaddr,
				u64 size,
				u32 pgsz_idx,
				bool va_allocated,
				enum gk20a_mem_rw_flag rw_flag,
				bool sparse,
				struct vm_gk20a_mapping_batch *batch);
int vgpu_vm_bind_channel(struct vm_gk20a *vm,
				struct nvgpu_channel *ch);
int vgpu_mm_fb_flush(struct gk20a *g);
void vgpu_mm_l2_invalidate(struct gk20a *g);
int vgpu_mm_l2_flush(struct gk20a *g, bool invalidate);
int vgpu_mm_tlb_invalidate(struct gk20a *g, struct nvgpu_mem *pdb);
#ifdef CONFIG_NVGPU_DEBUGGER
void vgpu_mm_mmu_set_debug_mode(struct gk20a *g, bool enable);
#endif
u64 vgpu_locked_gmmu_map(struct vm_gk20a *vm,
				u64 map_offset,
				struct nvgpu_sgt *sgt,
				u64 buffer_offset,
				u64 size,
				u32 pgsz_idx,
				u8 kind_v,
				u32 ctag_offset,
				u32 flags,
				enum gk20a_mem_rw_flag rw_flag,
				bool clear_ctags,
				bool sparse,
				bool priv,
				struct vm_gk20a_mapping_batch *batch,
				enum nvgpu_aperture aperture);
int vgpu_init_mm_support(struct gk20a *g);

#endif /* NVGPU_MM_VGPU_H */
