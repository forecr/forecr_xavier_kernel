/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CYCLESTATS_H
#define NVGPU_CYCLESTATS_H

#ifdef CONFIG_NVGPU_CYCLESTATS

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_channel;

void nvgpu_cyclestats_exec(struct gk20a *g,
		struct nvgpu_channel *ch, u32 offset);

#endif /* CONFIG_NVGPU_CYCLESTATS */
#endif
