/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_BIOS_SW_GB20C_H
#define NVGPU_BIOS_SW_GB20C_H

struct gk20a;
struct nvgpu_bios;

s32 gb20c_bios_init(struct gk20a *g);
void nvgpu_gb20c_bios_sw_init(struct gk20a *g,
		struct nvgpu_bios *bios);

#endif /* NVGPU_BIOS_SW_GB20C_H */
