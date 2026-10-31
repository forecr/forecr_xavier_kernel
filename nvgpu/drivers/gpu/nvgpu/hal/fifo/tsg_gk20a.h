/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_TSG_GK20A_H
#define NVGPU_TSG_GK20A_H

struct nvgpu_channel;
struct nvgpu_channel_hw_state;

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
struct nvgpu_tsg;

void gk20a_tsg_enable(struct nvgpu_tsg *tsg);
#endif

int gk20a_tsg_unbind_channel_check_hw_next(struct nvgpu_channel *ch,
		struct nvgpu_channel_hw_state *hw_state);

#endif /* NVGPU_TSG_GK20A_H */
