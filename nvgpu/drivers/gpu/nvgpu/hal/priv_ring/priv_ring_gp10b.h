/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PRIV_RING_GP10B_H
#define NVGPU_PRIV_RING_GP10B_H

struct gk20a;

void gp10b_priv_ring_isr(struct gk20a *g);
#ifdef CONFIG_NVGPU_NON_FUSA
void gp10b_priv_ring_decode_error_code(struct gk20a *g, u32 error_code);
void gp10b_priv_ring_isr_handle_0(struct gk20a *g, u32 status0);
void gp10b_priv_ring_isr_handle_1(struct gk20a *g, u32 status1);
#endif
#endif /* NVGPU_PRIV_RING_GP10B_H */
