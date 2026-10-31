/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LTC_INTR_GV11B
#define NVGPU_LTC_INTR_GV11B

#include <nvgpu/types.h>

struct gk20a;

#ifdef CONFIG_NVGPU_NON_FUSA
void gv11b_ltc_intr_configure(struct gk20a *g);
int gv11b_ltc_intr_isr(struct gk20a *g, u32 ltc);
void gv11b_ltc_intr_en_illegal_compstat(struct gk20a *g, bool enable);

void gv11b_ltc_intr_init_counters(struct gk20a *g,
			u32 uncorrected_delta, u32 uncorrected_overflow,
			u32 offset);
void gv11b_ltc_intr_handle_rstg_ecc_interrupts(struct gk20a *g,
			u32 ltc, u32 slice, u32 ecc_status, u32 ecc_addr,
			u32 uncorrected_delta);
void gv11b_ltc_intr_handle_tstg_ecc_interrupts(struct gk20a *g,
			u32 ltc, u32 slice, u32 ecc_status, u32 ecc_addr,
			u32 uncorrected_delta);
void gv11b_ltc_intr_handle_dstg_ecc_interrupts(struct gk20a *g,
			u32 ltc, u32 slice, u32 ecc_status, u32 ecc_addr,
			u32 uncorrected_delta);

#endif
#endif
