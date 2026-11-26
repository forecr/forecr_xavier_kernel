/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef TOP_GP10B_H
#define TOP_GP10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_device;

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
int gp10b_device_info_parse_data(struct gk20a *g, u32 table_entry, u32 *inst_id,
					u32 *pri_base, u32 *fault_id);
#endif
int gp10b_get_device_info(struct gk20a *g, struct nvgpu_device *dev_info,
					u32 engine_type, u32 inst_id);
#endif
