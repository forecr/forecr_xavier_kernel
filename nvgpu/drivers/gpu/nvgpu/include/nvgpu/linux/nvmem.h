/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __NVGPU_LINUX_NVMEM_H__
#define __NVGPU_LINUX_NVMEM_H__

#ifdef CONFIG_NVGPU_NVMEM_FUSE

/**
 * @file
 *
 * Interface for nvmem ops.
 */

struct gk20a;

#include <nvgpu/types.h>
#include <nvgpu/errno.h>

/**
 * @brief - Reads calibration fuse.
 *
 * @param g [in] - GPU super structure.
 * @param val [out] - Populated with calibration fuse value.
 *
 * - Provide information about the calibration fuse.
 *
 * @return 0 on success.
 *
 */
int nvgpu_tegra_nvmem_read_reserved_calib(struct gk20a *g, u32 *val);

/**
 * @brief - Reads GCPLEX_CONFIG_FUSE configuration.
 *
 * @param g [in] - GPU super structure.
 * @param val [out] - Populated with register GCPLEX_CONFIG_FUSE value.
 *
 * - Provide information about the GPU complex configuration.
 *
 * @return 0 on success.
 *
 */
int nvgpu_tegra_nvmem_read_gcplex_config_fuse(struct gk20a *g, u32 *val);

/**
 * @brief - Reads the per-device identifier fuses.
 *
 * @param g [in] - GPU super structure.
 * @param pdi [out] - Per-device identifier
 *
 * The per-device identifier fuses are FUSE_PDI0 and FUSE_PDI1.
 *
 * @return 0 on success
 */
int nvgpu_tegra_nvmem_read_per_device_identifier(struct gk20a *g, u64 *pdi);

#endif /* CONFIG_NVGPU_NVMEM_FUSE */

#endif /* __NVGPU_LINUX_NVMEM_H__ */
