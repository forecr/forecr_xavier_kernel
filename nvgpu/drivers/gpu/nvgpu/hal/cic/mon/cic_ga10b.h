/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef CIC_GA10B_H
#define CIC_GA10B_H

#include <nvgpu/nvgpu_err_info.h>

struct gk20a;
struct nvgpu_cic_mon;

extern struct nvgpu_err_hw_module ga10b_err_lut[];
extern u32 size_of_ga10b_lut;

int ga10b_cic_mon_init(struct gk20a *g, struct nvgpu_cic_mon *cic_mon);

#endif /* CIC_GV11B_H */
