/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RAMIN_GA10B_H
#define NVGPU_RAMIN_GA10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_mem;

void ga10b_ramin_init_pdb(struct gk20a *g, struct nvgpu_mem *inst_block,
		u64 pdb_addr, struct nvgpu_mem *pdb_mem);
void ga10b_ramin_set_magic_value(struct gk20a *g, struct nvgpu_mem *inst_block);

#endif /* NVGPU_RAMIN_GA10B_H */
