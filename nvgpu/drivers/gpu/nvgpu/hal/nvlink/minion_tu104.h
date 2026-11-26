/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef MINION_TU104_H
#define MINION_TU104_H

#include <nvgpu/types.h>

enum nvgpu_nvlink_minion_dlcmd;
struct gk20a;

u32 tu104_nvlink_minion_get_dlcmd_ordinal(struct gk20a *g,
					enum nvgpu_nvlink_minion_dlcmd dlcmd);
bool tu104_nvlink_minion_is_debug_mode(struct gk20a *g);
#endif /* MINION_TU104_H */
