/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

int vgpu_fb_vab_reserve(struct gk20a *g, u32 vab_mode, u32 num_range_checkers,
			struct nvgpu_vab_range_checker *vab_range_checker);
int vgpu_fb_vab_dump_and_clear(struct gk20a *g, u8 *user_buf,
				u64 user_buf_size);
int vgpu_fb_vab_release(struct gk20a *g);
