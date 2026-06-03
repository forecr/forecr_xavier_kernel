/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FIFO_INTR_GV100_H
#define NVGPU_FIFO_INTR_GV100_H

#include <nvgpu/types.h>
struct gk20a;

void gv100_fifo_intr_set_recover_mask(struct gk20a *g);
void gv100_fifo_intr_unset_recover_mask(struct gk20a *g);

#endif /* NVGPU_FIFO_INTR_GV100_H */
