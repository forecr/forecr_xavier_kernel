/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_MM_GK20A_H
#define HAL_MM_MM_GK20A_H

#include <nvgpu/types.h>

struct nvgpu_mem;
struct vm_gk20a;

void gk20a_mm_init_inst_block(struct nvgpu_mem *inst_block, struct vm_gk20a *vm,
			      u32 big_page_size);
int gk20a_mm_init_inst_block_core(struct nvgpu_mem *inst_block,
		struct vm_gk20a *vm, u32 big_page_size);
u64 gk20a_mm_bar1_map_userd(struct gk20a *g, struct nvgpu_mem *mem, u32 offset);

#endif
