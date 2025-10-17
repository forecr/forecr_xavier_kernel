/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_GA100_H
#define NVGPU_GR_GA100_H

struct gk20a;
struct nvgpu_debug_context;

int gr_ga100_dump_gr_status_regs(struct gk20a *g,
				 struct nvgpu_debug_context *o);
void gr_ga100_set_circular_buffer_size(struct gk20a *g, u32 data);
#ifdef CONFIG_NVGPU_DEBUGGER
int gr_ga100_process_context_buffer_priv_segment(struct gk20a *g,
					     enum ctxsw_addr_type addr_type,
					     u32 pri_addr,
					     u32 gpc_num, u32 num_tpcs,
					     u32 num_ppcs, u32 ppc_mask,
					     u32 *priv_offset);
#endif /* CONFIG_NVGPU_DEBUGGER */
#endif /* NVGPU_GR_GA100_H */
