/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef PMU_GP10B_H
#define PMU_GP10B_H

#include <nvgpu/types.h>

struct gk20a;

bool gp10b_is_pmu_supported(struct gk20a *g);
void gp10b_pmu_setup_elpg(struct gk20a *g);
void gp10b_write_dmatrfbase(struct gk20a *g, u32 addr);
u32 gp10b_pmu_queue_head_r(u32 i);
u32 gp10b_pmu_queue_head__size_1_v(void);
u32 gp10b_pmu_queue_tail_r(u32 i);
u32 gp10b_pmu_queue_tail__size_1_v(void);
u32 gp10b_pmu_mutex__size_1_v(void);

#endif /* PMU_GP10B_H */
