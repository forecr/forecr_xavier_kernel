/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_NVENC_TU104_H
#define NVGPU_NVENC_TU104_H

#include <nvgpu/types.h>

struct gk20a;

u32 tu104_nvenc_base_addr(u32 inst_id);
void tu104_nvenc_setup_boot_config(struct gk20a *g, u32 inst_id);
void tu104_nvenc_halt_engine(struct gk20a *g, u32 inst_id);
void tu104_nvenc_interface_enable(struct gk20a *g, u32 inst_id);

#endif /* NVGPU_NVENC_TU104_H */
