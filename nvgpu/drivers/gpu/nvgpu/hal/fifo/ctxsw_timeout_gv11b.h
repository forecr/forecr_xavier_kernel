/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CTXSW_TIMEOUT_GV11B_H
#define NVGPU_CTXSW_TIMEOUT_GV11B_H

#include <nvgpu/types.h>

struct gk20a;

void gv11b_fifo_ctxsw_timeout_enable(struct gk20a *g, bool enable);
bool gv11b_fifo_handle_ctxsw_timeout(struct gk20a *g);

#endif /* NVGPU_CTXSW_TIMEOUT_GV11B_H */
