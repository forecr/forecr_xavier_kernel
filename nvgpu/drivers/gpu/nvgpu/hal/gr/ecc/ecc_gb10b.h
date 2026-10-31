/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ECC_GB10B_H
#define NVGPU_ECC_GB10B_H

#include <nvgpu/nvgpu_err.h>
#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_hw_err_inject_info;
struct nvgpu_hw_err_inject_info_desc;

#ifdef CONFIG_NVGPU_INJECT_HWERR
void gb10b_gr_intr_inject_sm_ecc_error(struct gk20a *g,
		struct nvgpu_hw_err_inject_info *err, u32 error_info);
struct nvgpu_hw_err_inject_info_desc *
gb10b_gr_intr_get_sm_err_desc(struct gk20a *g);
#endif /* CONFIG_NVGPU_INJECT_HWERR */

#endif /* NVGPU_ECC_GB10B_H */
