/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef GR_PRI_GA10B_H
#define GR_PRI_GA10B_H

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/static_analysis.h>

static inline u32 pri_rop_in_gpc_shared_addr(struct gk20a *g, u32 addr)
{
	u32 gpc_addr = pri_gpccs_addr_mask(g, addr);
	u32 gpc_shared_base = nvgpu_get_litter_value(g, GPU_LIT_GPC_SHARED_BASE);
	u32 rop_base = nvgpu_get_litter_value(g, GPU_LIT_ROP_IN_GPC_BASE);
	u32 rop_shared_base = nvgpu_get_litter_value(g, GPU_LIT_ROP_IN_GPC_SHARED_BASE);
	u32 rop_stride = nvgpu_get_litter_value(g, GPU_LIT_ROP_IN_GPC_STRIDE);

	return nvgpu_safe_add_u32(
			nvgpu_safe_add_u32(gpc_shared_base, rop_shared_base),
			nvgpu_safe_sub_u32(gpc_addr, rop_base) % rop_stride);
}

static inline bool pri_is_rop_in_gpc_addr_shared(struct gk20a *g, u32 addr)
{
	u32 rop_shared_base = nvgpu_get_litter_value(g, GPU_LIT_ROP_IN_GPC_SHARED_BASE);
	u32 rop_stride = nvgpu_get_litter_value(g, GPU_LIT_ROP_IN_GPC_STRIDE);

	return (addr >= rop_shared_base) &&
		(addr < nvgpu_safe_add_u32(rop_shared_base, rop_stride));
}

static inline bool pri_is_rop_in_gpc_addr(struct gk20a *g, u32 addr)
{
	u32 rop_base = nvgpu_get_litter_value(g, GPU_LIT_ROP_IN_GPC_BASE);
	u32 rop_shared_base = nvgpu_get_litter_value(g, GPU_LIT_ROP_IN_GPC_SHARED_BASE);
	u32 rop_stride = nvgpu_get_litter_value(g, GPU_LIT_ROP_IN_GPC_STRIDE);

	return (addr >= rop_base) &&
		(addr < nvgpu_safe_add_u32(rop_shared_base, rop_stride));
}

#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* GR_PRI_GA10B_H */
