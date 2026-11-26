/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_RISCV_GB10B_H
#define NVGPU_RISCV_GB10B_H

#include <nvgpu/types.h>

u32 gb10b_riscv_cpuctl_offset(void);

void gb10b_riscv_dump_debug_regs(struct nvgpu_falcon *flcn,
				struct nvgpu_debug_context *o);
void gb10b_riscv_dump_trace_info(struct nvgpu_falcon *flcn,
				struct nvgpu_debug_context *o);

#endif /* NVGPU_RISCV_GB10B_H */
