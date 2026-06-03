/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CTXSW_PROG_GA100_H
#define NVGPU_CTXSW_PROG_GA100_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_mem;

u32 ga100_ctxsw_prog_hw_get_fecs_header_size(void);

#ifdef CONFIG_NVGPU_DEBUGGER
u32 ga100_ctxsw_prog_hw_get_gpccs_header_size(void);
bool ga100_ctxsw_prog_check_main_image_header_magic(u32 *context);
bool ga100_ctxsw_prog_check_local_header_magic(u32 *context);
u32 ga100_ctxsw_prog_hw_get_pm_gpc_gnic_stride(struct gk20a *g);
#endif /* CONFIG_NVGPU_DEBUGGER */
#ifdef CONFIG_DEBUG_FS
void ga100_ctxsw_prog_dump_ctxsw_stats(struct gk20a *g,
	struct nvgpu_mem *ctx_mem);
#endif

#endif /* NVGPU_CTXSW_PROG_GA100_H */
