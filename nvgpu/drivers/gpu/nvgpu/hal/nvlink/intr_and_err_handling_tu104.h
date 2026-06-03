/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef INTR_AND_ERR_HANDLING_TU104_H
#define INTR_AND_ERR_HANDLING_TU104_H

#include <nvgpu/types.h>
struct gk20a;

void tu104_nvlink_init_link_err_intr(struct gk20a *g, u32 link_id);
void tu104_nvlink_enable_link_err_intr(struct gk20a *g, u32 link_id,
							bool enable);
void tu104_nvlink_isr(struct gk20a *g);

#endif /* INTR_AND_ERR_HANDLING_TU104_H */
