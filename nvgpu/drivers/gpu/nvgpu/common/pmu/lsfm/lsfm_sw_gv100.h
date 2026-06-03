/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_LSFM_SW_GV100_H
#define NVGPU_LSFM_SW_GV100_H

int gv100_update_lspmu_cmdline_args_copy(struct gk20a *g,
	struct nvgpu_pmu *pmu);
void nvgpu_gv100_lsfm_sw_init(struct gk20a *g, struct nvgpu_pmu_lsfm *lsfm);

#endif /* NVGPU_LSFM_SW_GV100_H */
