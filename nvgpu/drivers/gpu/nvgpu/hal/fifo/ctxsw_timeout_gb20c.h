/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CTXSW_TIMEOUT_GB20C_H
#define NVGPU_CTXSW_TIMEOUT_GB20C_H

#include <nvgpu/types.h>

struct gk20a;

u32 gb20c_fifo_ctxsw_timeout_config(u32 rleng_id);
u32 gb20c_rleng_ctxsw_timeout_info_reg_off(u32 rleng_id);
u32 gb20c_ctxsw_timeout_config_size_1(void);

#endif /* NVGPU_CTXSW_TIMEOUT_GB20C_H */
