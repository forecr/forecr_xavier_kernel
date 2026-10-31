/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_TSG_GB10B_H
#define NVGPU_TSG_GB10B_H

struct nvgpu_tsg;
struct nvgpu_channel;

void gb10b_tsg_bind_channel_eng_method_buffers(struct nvgpu_tsg *tsg,
		struct nvgpu_channel *ch);

#endif /* NVGPU_TSG_GB10B_H */
