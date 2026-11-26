/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_FECS_TRACE_GM20B_H
#define NVGPU_GR_FECS_TRACE_GM20B_H

#ifdef CONFIG_NVGPU_FECS_TRACE

#include <nvgpu/types.h>

struct gk20a;

int gm20b_fecs_trace_flush(struct gk20a *g);
int gm20b_fecs_trace_get_read_index(struct gk20a *g);
int gm20b_fecs_trace_get_write_index(struct gk20a *g);
int gm20b_fecs_trace_set_read_index(struct gk20a *g, int index);
u32 gm20b_fecs_trace_get_buffer_full_mailbox_val(void);

#endif /* CONFIG_NVGPU_FECS_TRACE */
#endif /* NVGPU_GR_FECS_TRACE_GM20B_H */
