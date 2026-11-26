/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMU_PMGR_H
#define NVGPU_PMU_PMGR_H

#include <nvgpu/types.h>

struct gk20a;

int pmgr_pwr_devices_get_current(struct gk20a *g, u32 *val);
int pmgr_pwr_devices_get_voltage(struct gk20a *g, u32 *val);
int pmgr_pwr_devices_get_power(struct gk20a *g, u32 *val);
int pmgr_domain_sw_setup(struct gk20a *g);
int pmgr_domain_pmu_setup(struct gk20a *g);
int pmgr_pmu_init_pmupstate(struct gk20a *g);
void pmgr_pmu_free_pmupstate(struct gk20a *g);


#endif /* NVGPU_PMU_PMGR_H */
