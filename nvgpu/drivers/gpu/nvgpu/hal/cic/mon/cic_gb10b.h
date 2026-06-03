/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef CIC_GB10B_H
#define CIC_GB10B_H

#include <nvgpu/nvgpu_err_info.h>

struct gk20a;
struct nvgpu_cic_mon;

extern struct nvgpu_err_hw_module gb10b_err_lut[];
extern u32 size_of_gb10b_lut;

int gb10b_cic_mon_init(struct gk20a *g, struct nvgpu_cic_mon *cic_mon);

#endif /* CIC_GB10B_H */
