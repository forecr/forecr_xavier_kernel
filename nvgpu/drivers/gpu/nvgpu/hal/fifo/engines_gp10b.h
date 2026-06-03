/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ENGINE_GP10B_H
#define NVGPU_ENGINE_GP10B_H

#include <nvgpu/types.h>

struct nvgpu_fifo;

int gp10b_engine_init_ce_info(struct nvgpu_fifo *f);

#endif /* NVGPU_ENGINE_GP10B_H */
