/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LTC_GB20C_H
#define NVGPU_LTC_GB20C_H

#include <nvgpu/types.h>

#if defined(CONFIG_NVGPU_NON_FUSA)
u32 gb20c_ltcs_ltss_tstg_cfg1_active_sets(u32 reg_val);
#endif

#endif /* NVGPU_LTC_GB20C_H */
