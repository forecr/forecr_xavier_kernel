/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef MULTIMEDIA_TRACE_CTRL_H
#define MULTIMEDIA_TRACE_CTRL_H

int nvgpu_multimedia_trace_ctrl_init(struct gk20a *g);
int nvgpu_multimedia_trace_ctrl_deinit(struct gk20a *g);
int nvgpu_multimedia_trace_ctrl_enable(struct gk20a *g, u32 multimedia_id);
int nvgpu_multimedia_trace_ctrl_disable(struct gk20a *g, u32 multimedia_id);
int nvgpu_multimedia_trace_ctrl_poll(struct gk20a *g, u32 multimedia_id);
#endif /* MULTIMEDIA_TRACE_CTRL_H */
