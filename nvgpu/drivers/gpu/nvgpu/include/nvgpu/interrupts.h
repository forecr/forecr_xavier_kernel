/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_INTERRUPTS_H
#define NVGPU_INTERRUPTS_H

#include <nvgpu/types.h>

#define NVGPU_MAX_INTERRUPTS_STALL    3U
#define NVGPU_MAX_INTERRUPTS_NONSTALL 1U
#define NVGPU_MAX_INTERRUPTS          (NVGPU_MAX_INTERRUPTS_STALL + \
                                       NVGPU_MAX_INTERRUPTS_NONSTALL)

#define NVGPU_MAX_INTERRUPTS_MSI      64U

struct nvgpu_msi_cookie {
	struct gk20a *g;
	u32 id;
};

struct nvgpu_interrupts {
	bool enabled;

	u32 stall_size;
	u32 nonstall_size;

	u32 stall_lines[NVGPU_MAX_INTERRUPTS_STALL];
	u32 nonstall_line;    /* FIXME: if multiple nonstall lines are needed */

	u32 msi_size;
	u32 msi_lines[NVGPU_MAX_INTERRUPTS_MSI];
	struct nvgpu_msi_cookie msi_cookies[NVGPU_MAX_INTERRUPTS_MSI];
};

#endif /* NVGPU_INTERRUPTS_H */
