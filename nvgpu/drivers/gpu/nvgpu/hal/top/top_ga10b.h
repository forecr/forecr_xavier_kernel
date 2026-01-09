/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_TOP_GA10B_H
#define NVGPU_TOP_GA10B_H

#include <nvgpu/types.h>

struct gk20a;

#ifdef CONFIG_NVGPU_NON_FUSA
u32 ga10b_get_num_engine_type_entries(struct gk20a *g, u32 engine_type);
#endif
struct nvgpu_device *ga10b_top_parse_next_dev(struct gk20a *g, u32 *token);
u32 ga10b_get_expected_chip_info_cfg_version(void);

u32 ga10b_top_get_max_rop_per_gpc(struct gk20a *g);

#endif /* NVGPU_TOP_GA10B_H */
