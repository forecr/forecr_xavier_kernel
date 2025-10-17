/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_MODEL_20_H
#define NVGPU_PMU_MODEL_20_H

#include <nvgpu/pmu/pmuif/pg.h>
#include <nvgpu/pmu/pmuif/cmdmgmt.h>

struct gk20a;

#ifdef CONFIG_NVGPU_LS_PMU
int nvgpu_pmu_wait_fw_driver_register_ready(struct gk20a *g);
int  nvgpu_pmu_driver_register_send(struct gk20a *g,
		enum  driver_register_type register_type);
#endif

#endif /* NVGPU_PMU_MODEL_20_H */
