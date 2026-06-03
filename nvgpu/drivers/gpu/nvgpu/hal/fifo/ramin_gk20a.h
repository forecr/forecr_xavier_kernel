/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_RAMIN_GK20A_H
#define NVGPU_RAMIN_GK20A_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_mem;

u32 gk20a_ramin_base_shift(void);
u32 gk20a_ramin_alloc_size(void);

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gk20a_ramin_set_gr_ptr(struct gk20a *g,
		struct nvgpu_mem *inst_block, u64 gpu_va);
void gk20a_ramin_init_pdb(struct gk20a *g, struct nvgpu_mem *inst_block,
		u64 pdb_addr, struct nvgpu_mem *pdb_mem);
void gk20a_ramin_set_adr_limit(struct gk20a *g,
		struct nvgpu_mem *inst_block, u64 va_limit);
#endif

#endif /* NVGPU_RAMIN_GK20A_H */
