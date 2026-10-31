/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef PTIMER_VGPU_H
#define PTIMER_VGPU_H

struct gk20a;
struct nvgpu_cpu_time_correlation_sample;

int vgpu_read_ptimer(struct gk20a *g, u64 *value);
int vgpu_get_timestamps_zipper(struct gk20a *g,
		u32 source_id, u32 count,
		struct nvgpu_cpu_time_correlation_sample *samples);

#endif
