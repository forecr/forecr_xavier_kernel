/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LTC_GP10B_H
#define NVGPU_LTC_GP10B_H
struct gk20a;

u64 gp10b_determine_L2_size_bytes(struct gk20a *g);
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gp10b_ltc_init_fs_state(struct gk20a *g);
int gp10b_lts_ecc_init(struct gk20a *g);
#endif

#if defined(CONFIG_NVGPU_NON_FUSA) || defined(CONFIG_NVGPU_KERNEL_MODE_SUBMIT)
void gp10b_ltc_set_enabled(struct gk20a *g, bool enabled);
#endif /* CONFIG_NVGPU_NON_FUSA or CONFIG_NVGPU_KERNEL_MODE_SUBMIT */
#endif
