/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef HAL_FB_VAB_GA10B_H
#define HAL_FB_VAB_GA10B_H

struct gk20a;
struct nvgpu_vab_range_checker;

int ga10b_fb_vab_init(struct gk20a *g);
void ga10b_fb_vab_set_vab_buffer_address(struct gk20a *g, u64 buf_addr);
int ga10b_fb_vab_reserve(struct gk20a *g, u32 vab_mode, u32 num_range_checkers,
	struct nvgpu_vab_range_checker *vab_range_checker);
int ga10b_fb_vab_dump_and_clear(struct gk20a *g, u8 *user_buf,
	u64 user_buf_size);
int ga10b_fb_vab_release(struct gk20a *g);
int ga10b_fb_vab_teardown(struct gk20a *g);
void ga10b_fb_vab_recover(struct gk20a *g);

#endif /* HAL_FB_VAB_GA10B_H */
