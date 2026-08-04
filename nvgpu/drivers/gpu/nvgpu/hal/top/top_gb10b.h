/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_TOP_GB10B_H
#define NVGPU_TOP_GB10B_H

#include <nvgpu/types.h>

struct gk20a;

struct nvgpu_device *gb10b_top_parse_next_dev(struct gk20a *g, u32 *i);
u32 gb10b_get_expected_chip_info_cfg_version(void);

#endif /* NVGPU_TOP_GB10B_H */
