/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_BIOS_H
#define NVGPU_GOPS_BIOS_H

#if defined(CONFIG_NVGPU_DGPU) || defined(CONFIG_NVGPU_NEXT)
struct gops_bios {
	int (*bios_sw_init)(struct gk20a *g);
	void (*bios_sw_deinit)(struct gk20a *g,
				struct nvgpu_bios *bios);
	u32 (*get_aon_secure_scratch_reg)(struct gk20a *g, u32 i);
	bool (*wait_for_bios_init_done)(struct gk20a *g);
};
#endif

#endif /* NVGPU_GOPS_BIOS_H */
