/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ECC_GA10B_H
#define NVGPU_ECC_GA10B_H

#include <nvgpu/types.h>

struct gk20a;

#ifdef CONFIG_NVGPU_INJECT_HWERR
struct nvgpu_hw_err_inject_info_desc;

struct nvgpu_hw_err_inject_info_desc *
		ga10b_gr_ecc_get_mmu_err_desc(struct gk20a *g);
#endif /* CONFIG_NVGPU_INJECT_HWERR */

void ga10b_ecc_detect_enabled_units(struct gk20a *g);
int ga10b_gr_gpc_tpc_ecc_init(struct gk20a *g);
void ga10b_gr_gpc_tpc_ecc_deinit(struct gk20a *g);

#endif /* NVGPU_ECC_GA10B_H */
