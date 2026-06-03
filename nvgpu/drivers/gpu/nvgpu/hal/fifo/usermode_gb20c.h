/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_USERMODE_GB20C_H
#define NVGPU_USERMODE_GB20C_H

#include <nvgpu/types.h>

struct gk20a;

u32 gb20c_usermode_doorbell_token(struct gk20a *g, u32 runlist_id, u32 chid);

#endif /* NVGPU_USERMODE_GB20C_H */
