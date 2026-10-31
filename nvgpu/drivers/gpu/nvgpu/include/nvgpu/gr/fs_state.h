/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_FS_STATE_H
#define NVGPU_GR_FS_STATE_H

/**
 * @file
 *
 * common.gr.fs_state unit interface
 */
struct gk20a;
struct nvgpu_gr_config;

/**
 * @brief Initialize GR engine h/w state post-floorsweeping.
 *
 * @param g [in]		Pointer to GPU driver struct.
 * @param config [in]		Pointer to GR config struct.
 *
 * This function initializes GR engine h/w state after considering
 * floorsweeping.
 *
 * It is possible that certain TPC (and hence SMs) in GPC are
 * floorswept and hence not available for any processing. In this case
 * common.gr unit is responsible to enumerate only available TPCs
 * and configure GR engine h/w registers with available GPC/TPC/SM count
 * and mapping.
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -ENOMEM if memory allocation fails for any internal data
 *         structure.
 */
int nvgpu_gr_fs_state_init(struct gk20a *g, struct nvgpu_gr_config *config);
/** @cond DOXYGEN_SHOULD_SKIP_THIS */
int nvgpu_gr_init_sm_id_early_config(struct gk20a *g, struct nvgpu_gr_config *config);
int nvgpu_gr_load_sm_id_config(struct gk20a *g, struct nvgpu_gr_config *config);
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

#endif /* NVGPU_GR_FS_STATE_H */
