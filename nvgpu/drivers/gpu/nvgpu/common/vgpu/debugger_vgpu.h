/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_DBG_VGPU_H
#define NVGPU_DBG_VGPU_H

struct dbg_session_gk20a;
struct nvgpu_dbg_reg_op;
struct gk20a;
struct nvgpu_channel;

int vgpu_dbg_set_powergate(struct dbg_session_gk20a *dbg_s,
			bool disable_powergate);

#endif /* NVGPU_DBG_VGPU_H */
