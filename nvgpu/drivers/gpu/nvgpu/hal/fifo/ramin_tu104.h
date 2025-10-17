/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RAMIN_TU104_H
#define NVGPU_RAMIN_TU104_H

#include <nvgpu/types.h>

struct gk20a;

int tu104_ramin_init_pdb_cache_errata(struct gk20a *g);
void tu104_ramin_deinit_pdb_cache_errata(struct gk20a *g);

#endif /* NVGPU_RAMIN_TU104_H */
