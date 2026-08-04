/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_BIOS_SW_TU104_H
#define NVGPU_BIOS_SW_TU104_H

#define NVGPU_BIOS_DEVINIT_VERIFY_TIMEOUT_MS		1000U
#define NVGPU_BIOS_DEVINIT_VERIFY_DELAY_US		10U
#define NVGPU_BIOS_DEVINIT_VERIFY_COMPLETION_MS		1U

struct gk20a;

int tu104_bios_verify_devinit(struct gk20a *g);
int tu104_bios_init(struct gk20a *g);
void nvgpu_tu104_bios_sw_init(struct gk20a *g,
		struct nvgpu_bios *bios);

#endif /* NVGPU_BIOS_SW_TU104_H */
