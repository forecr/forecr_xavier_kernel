// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#ifndef NVGPU_TSG_GB20C_H
#define NVGPU_TSG_GB20C_H

struct nvgpu_tsg;
struct nvgpu_channel;

void gb20c_tsg_bind_channel_eng_method_buffers(struct nvgpu_tsg *tsg,
		struct nvgpu_channel *ch);

#endif /* NVGPU_TSG_GB20C_H */
