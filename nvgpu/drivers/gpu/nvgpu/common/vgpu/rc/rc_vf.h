/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RC_VF_H
#define NVGPU_RC_VF_H

void vgpu_vf_fifo_recover(struct gk20a *g, u32 gfid, u32 runlist_id,
			u32 id, unsigned int id_type, unsigned int rc_type,
			bool should_defer_reset);

#endif
