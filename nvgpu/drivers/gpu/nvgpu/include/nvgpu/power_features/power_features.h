/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */


#ifndef NVGPU_POWER_FEATURES_H
#define NVGPU_POWER_FEATURES_H

#ifdef CONFIG_NVGPU_NON_FUSA

#include <nvgpu/types.h>

struct gk20a;

int nvgpu_cg_pg_disable(struct gk20a *g);
int nvgpu_cg_pg_enable(struct gk20a *g);

#endif
#endif /*NVGPU_POWER_FEATURES_H*/
