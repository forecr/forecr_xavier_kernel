/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_MM_MM_GV11B_H
#define HAL_MM_MM_GV11B_H

#include <nvgpu/types.h>

struct nvgpu_mem;
struct vm_gk20a;

void gv11b_mm_init_inst_block(struct nvgpu_mem *inst_block, struct vm_gk20a *vm,
			      u32 big_page_size);
int gv11b_mm_init_inst_block_core(struct nvgpu_mem *inst_block,
				   struct vm_gk20a *vm,
				   u32 big_page_size);
bool gv11b_mm_is_bar1_supported(struct gk20a *g);

#endif
