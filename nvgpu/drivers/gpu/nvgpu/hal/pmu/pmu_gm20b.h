/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef PMU_GM20B_H
#define PMU_GM20B_H

#include <nvgpu/types.h>

struct gk20a;

void gm20b_pmu_setup_elpg(struct gk20a *g);
void pmu_dump_security_fuses_gm20b(struct gk20a *g);
void gm20b_write_dmatrfbase(struct gk20a *g, u32 addr);
bool gm20b_pmu_is_debug_mode_en(struct gk20a *g);
void gm20b_pmu_ns_setup_apertures(struct gk20a *g);
void gm20b_pmu_setup_apertures(struct gk20a *g);
void gm20b_pmu_flcn_setup_boot_config(struct gk20a *g);
void gm20b_secured_pmu_start(struct gk20a *g);
bool gm20b_is_pmu_supported(struct gk20a *g);
void gm20b_clear_pmu_bar0_host_err_status(struct gk20a *g);
u32 gm20b_pmu_queue_head_r(u32 i);
u32 gm20b_pmu_queue_head__size_1_v(void);
u32 gm20b_pmu_queue_tail_r(u32 i);
u32 gm20b_pmu_queue_tail__size_1_v(void);
u32 gm20b_pmu_mutex__size_1_v(void);
u32 gm20b_pmu_msgq_tail_r(void);

#endif /* PMU_GM20B_H */
