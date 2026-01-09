/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMGR_PMGR_H
#define NVGPU_PMGR_PMGR_H

#include "pwrdev.h"
#include "pwrmonitor.h"
#include "pwrpolicy.h"

struct pmgr_pmupstate {
	struct pwr_devices pmgr_deviceobjs;
	struct pmgr_pwr_monitor pmgr_monitorobjs;
	struct pmgr_pwr_policy pmgr_policyobjs;
};

#endif /* NVGPU_PMGR_PMGR_H */
