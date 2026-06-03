/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef PTIMER_GK20A_H
#define PTIMER_GK20A_H

#include <nvgpu/types.h>

struct gk20a;

#ifdef CONFIG_NVGPU_NON_FUSA
void gk20a_ptimer_isr(struct gk20a *g);
#endif /* CONFIG_NVGPU_NON_FUSA */
#ifdef CONFIG_NVGPU_IOCTL_NON_FUSA
int gk20a_read_ptimer(struct gk20a *g, u64 *value);
#endif /* CONFIG_NVGPU_IOCTL_NON_FUSA */

#endif /* PTIMER_GK20A_H */
