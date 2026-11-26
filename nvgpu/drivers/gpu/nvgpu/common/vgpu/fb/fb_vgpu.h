/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef FB_VGPU_H
#define FB_VGPU_H

#ifdef CONFIG_NVGPU_DEBUGGER

void vgpu_fb_set_mmu_debug_mode(struct gk20a *g, bool enable);

#endif

#endif
