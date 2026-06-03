/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_FECS_TRACE_GB10B_H
#define NVGPU_GR_FECS_TRACE_GB10B_H

#ifdef CONFIG_NVGPU_FECS_TRACE

#include <nvgpu/types.h>

struct gk20a;

int gb10b_fecs_trace_get_read_index(struct gk20a *g);
int gb10b_fecs_trace_get_write_index(struct gk20a *g);
int gb10b_fecs_trace_set_read_index(struct gk20a *g, int index);

#endif /* CONFIG_NVGPU_FECS_TRACE */
#endif /* NVGPU_GR_FECS_TRACE_GB10B_H */
