/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FIFO_CTXSW_TIMEOUT_GK20A_H
#define NVGPU_FIFO_CTXSW_TIMEOUT_GK20A_H

#include <nvgpu/types.h>

struct gk20a;

void gk20a_fifo_ctxsw_timeout_enable(struct gk20a *g, bool enable);
bool gk20a_fifo_handle_ctxsw_timeout(struct gk20a *g);

#endif /* NVGPU_FIFO_CTXSW_TIMEOUT_GK20A_H */
