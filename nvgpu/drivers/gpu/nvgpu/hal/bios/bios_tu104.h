/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_BIOS_TU104_H
#define NVGPU_BIOS_TU104_H

#include <nvgpu/types.h>

struct gk20a;

u32 tu104_get_aon_secure_scratch_reg(struct gk20a *g, u32 i);

#endif /*NVGPU_BIOS_TU104_H */
