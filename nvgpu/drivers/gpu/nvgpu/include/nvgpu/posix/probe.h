/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_PROBE_H
#define NVGPU_POSIX_PROBE_H

/*
 * In the unit test FW the POSIX code is expecting a gv11b at the moment.
 */
#define NV_PMC_BOOT_0_ARCHITECTURE_GA100	(0x00000017 << \
						NVGPU_GPU_ARCHITECTURE_SHIFT)
#define NV_PMC_BOOT_0_ARCHITECTURE_GV110	(0x00000015 << \
						NVGPU_GPU_ARCHITECTURE_SHIFT)
#define NV_PMC_BOOT_0_IMPLEMENTATION_B		0xB

#define NV_PMC_BOOT_0_ARCHITECTURE_INVALID	(0x00000018 << \
						NVGPU_GPU_ARCHITECTURE_SHIFT)
#define NV_PMC_BOOT_0_IMPLEMENTATION_INVALID	0xD

struct gk20a;

struct gk20a *nvgpu_posix_current_device(void);
struct gk20a *nvgpu_posix_probe(void);
void nvgpu_posix_cleanup(struct gk20a *g);

#endif /* NVGPU_POSIX_PROBE_H */
