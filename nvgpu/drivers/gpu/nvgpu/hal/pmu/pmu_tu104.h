/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef PMU_TU104_H
#define PMU_TU104_H

struct gk20a;

bool tu104_is_pmu_supported(struct gk20a *g);
u32 tu104_pmu_falcon_base_addr(void);
u32 tu104_pmu_queue_head_r(u32 i);
u32 tu104_pmu_queue_head__size_1_v(void);
u32 tu104_pmu_queue_tail_r(u32 i);
u32 tu104_pmu_queue_tail__size_1_v(void);
u32 tu104_pmu_mutex__size_1_v(void);
void tu104_pmu_setup_apertures(struct gk20a *g);

#endif /* PMU_TU104_H */
