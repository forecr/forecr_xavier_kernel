// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#ifndef NVGPU_GR_GB20C
#define NVGPU_GR_GB20C

#ifdef CONFIG_NVGPU_DEBUGGER

#include <nvgpu/types.h>

struct gk20a;

int gb20c_gr_set_mmu_debug_mode(struct gk20a *g, struct nvgpu_channel *ch,
				bool enable, bool enable_gcc);

#endif /* CONFIG_NVGPU_DEBUGGER */
#endif
