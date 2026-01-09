/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef CBC_GB10B_H
#define CBC_GB10B_H

#ifdef CONFIG_NVGPU_COMPRESSION

#include <nvgpu/types.h>

enum nvgpu_cbc_op;
struct gk20a;
struct nvgpu_cbc;

int gb10b_cbc_init_support(struct gk20a *g);
void gb10b_cbc_remove_support(struct gk20a *g);

#endif
#endif
