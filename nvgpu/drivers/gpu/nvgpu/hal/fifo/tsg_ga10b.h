/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_TSG_GA10B_H
#define NVGPU_TSG_GA10B_H

struct nvgpu_channel;
struct nvgpu_channel_hw_state;

int ga10b_tsg_unbind_channel_check_hw_next(struct nvgpu_channel *ch,
		struct nvgpu_channel_hw_state *hw_state);

#endif /* NVGPU_TSG_GA10B_H */
