/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_SBR_H
#define NVGPU_GOPS_SBR_H

struct gops_sbr {
	int (*sbr_pub_load_and_execute)(struct gk20a *g);
};

#endif /* NVGPU_GOPS_SBR_H */
