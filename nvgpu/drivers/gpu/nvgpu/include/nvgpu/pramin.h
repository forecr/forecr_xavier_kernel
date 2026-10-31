/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PRAMIN_H
#define NVGPU_PRAMIN_H

#ifdef CONFIG_NVGPU_DGPU

#include <nvgpu/types.h>

struct gk20a;
struct mm_gk20a;
struct nvgpu_mem;


void nvgpu_pramin_rd_n(struct gk20a *g, struct nvgpu_mem *mem, u64 start,
							u64 size, void *dest);
void nvgpu_pramin_wr_n(struct gk20a *g, struct nvgpu_mem *mem, u64 start,
							u64 size, void *src);
void nvgpu_pramin_memset(struct gk20a *g, struct nvgpu_mem *mem, u64 start,
							u64 size, u32 w);

void nvgpu_init_pramin(struct mm_gk20a *mm);

#endif

#endif /* NVGPU_PRAMIN_H */
