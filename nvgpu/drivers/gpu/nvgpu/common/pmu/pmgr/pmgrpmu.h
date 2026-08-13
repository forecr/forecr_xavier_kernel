/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMGRPMU_H
#define NVGPU_PMGRPMU_H

#include <nvgpu/gk20a.h>

#include "pwrdev.h"
#include "pwrmonitor.h"

int pmgr_send_pmgr_tables_to_pmu(struct gk20a *g);

int pmgr_pmu_pwr_devices_query_blocking(
		struct gk20a *g,
		u32 pwr_dev_mask,
		struct nv_pmu_pmgr_pwr_devices_query_payload *ppayload);

#endif /* NVGPU_PMGRPMU_H */
