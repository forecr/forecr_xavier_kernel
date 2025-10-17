/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GSP_GB10B_H
#define NVGPU_GSP_GB10B_H

void gb10b_gsp_legacy_handler(struct gk20a *g, u64 cookie);
void gb10b_gsp_enable_irq(struct gk20a *g, bool enable);
int gb10b_gsp_flcn_copy_to_emem(struct gk20a *g,
			        u32 dst, u8 *src, u32 size, u8 port);
int gb10b_gsp_flcn_copy_from_emem(struct gk20a *g,
				  u32 src, u8 *dst, u32 size, u8 port);

#endif /* NVGPU_GSP_GB10B_H */
