/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef LTC_GV11B_H
#define LTC_GV11B_H

#include <nvgpu/types.h>
#include <nvgpu/nvgpu_err.h>

struct gk20a;
struct nvgpu_hw_err_inject_info;
struct nvgpu_hw_err_inject_info_desc;

int gv11b_lts_ecc_init(struct gk20a *g);

#ifdef CONFIG_NVGPU_GRAPHICS
void gv11b_ltc_set_zbc_stencil_entry(struct gk20a *g,
					  u32 stencil_depth,
					  u32 index);
#endif /* CONFIG_NVGPU_GRAPHICS */

/** @cond DOXYGEN_SHOULD_SKIP_THIS */
void gv11b_ltc_init_fs_state(struct gk20a *g);
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

#endif
