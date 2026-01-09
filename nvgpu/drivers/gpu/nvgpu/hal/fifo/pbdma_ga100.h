/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PBDMA_GA100_H
#define NVGPU_PBDMA_GA100_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_device;

u32 ga100_pbdma_set_clear_intr_offsets(struct gk20a *g,
			u32 set_clear_size);
u32 ga100_pbdma_get_fc_target(const struct nvgpu_device *dev);
void ga100_pbdma_force_ce_split(struct gk20a *g);
u32 ga100_pbdma_read_data(struct gk20a *g, u32 pbdma_id);
u32 ga100_pbdma_get_num_of_pbdmas(void);

#endif /* NVGPU_PBDMA_GA100_H */
