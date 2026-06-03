/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RAMIN_GB10B_H
#define NVGPU_RAMIN_GB10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_mem;

void gb10b_ramin_set_gr_ptr(struct gk20a *g,
		struct nvgpu_mem *inst_block, u64 gpu_va);

#endif /* NVGPU_RAMIN_GB10B_H */
