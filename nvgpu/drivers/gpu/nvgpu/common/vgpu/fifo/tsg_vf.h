/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_TSG_VF_H
#define NVGPU_TSG_VF_H

struct gk20a;
struct tegra_vf_tsg_set_unserviceable;

void vf_tsg_set_unserviceable(struct gk20a *g,
				struct tegra_vf_tsg_set_unserviceable *p);

#endif
