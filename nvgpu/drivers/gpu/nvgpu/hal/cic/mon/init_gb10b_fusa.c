// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>

#include "common/cic/mon/cic_mon_priv.h"
#include "cic_gb10b.h"

int gb10b_cic_mon_init(struct gk20a *g, struct nvgpu_cic_mon *cic_mon)
{
	if (cic_mon == NULL) {
		nvgpu_err(g, "Invalid CIC reference pointer.");
		return -EINVAL;
	}

	cic_mon->err_lut = gb10b_err_lut;
	cic_mon->num_hw_modules = size_of_gb10b_lut;
	return 0;
}
