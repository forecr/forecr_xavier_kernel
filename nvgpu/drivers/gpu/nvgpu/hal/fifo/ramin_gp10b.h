/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef RAMIN_GP10B_H
#define RAMIN_GP10B_H

struct gk20a;
struct nvgpu_mem;

void gp10b_ramin_init_pdb(struct gk20a *g, struct nvgpu_mem *inst_block,
		u64 pdb_addr, struct nvgpu_mem *pdb_mem);

#endif
