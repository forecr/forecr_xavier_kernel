/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef CBC_GA100_H
#define CBC_GA100_H

#if defined(CONFIG_NVGPU_COMPRESSION) && defined(CONFIG_NVGPU_NON_FUSA)

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_cbc;

int ga100_cbc_alloc_comptags(struct gk20a *g, struct nvgpu_cbc *cbc);

#endif
#endif /* CBC_GA100_H */
