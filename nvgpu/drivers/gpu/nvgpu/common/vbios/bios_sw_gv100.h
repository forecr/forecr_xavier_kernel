/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_BIOS_SW_GV100_H
#define NVGPU_BIOS_SW_GV100_H

struct gk20a;
struct nvgpu_bios;

void gv100_bios_preos_reload_check(struct gk20a *g);
int gv100_bios_preos_wait_for_halt(struct gk20a *g);
int gv100_bios_devinit(struct gk20a *g);
int gv100_bios_preos(struct gk20a *g);
int gv100_bios_init(struct gk20a *g);
void nvgpu_gv100_bios_sw_init(struct gk20a *g,
		struct nvgpu_bios *bios);

#endif /* NVGPU_BIOS_SW_GV100_H */
