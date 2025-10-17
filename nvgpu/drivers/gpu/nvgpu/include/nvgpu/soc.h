/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_SOC_H
#define NVGPU_SOC_H

#include <nvgpu/types.h>

struct gk20a;

#if defined(CONFIG_NVGPU_TEGRA_FUSE) || defined(CONFIG_TEGRA_FUSE_UPSTREAM)
/**
 * @brief Check whether running on silicon or not.
 *
 * @param g [in]	GPU super structure.
 *
 * - Extract out platform info by calling NvTegraSysGetPlatform().
 * - If info is NULL return false.
 * - If info is not "silicon" then return false. Otherwise return true.
 *
 * @return Returns true if it's silicon else return false.
 */
bool nvgpu_platform_is_silicon(struct gk20a *g);

/**
 * @brief Check whether running simulation or not.
 *
 * @param g [in]	GPU super structure.
 *
 * - Read CPU type by calling NvTegraSysGetCpuType().
 * - Read platform info by calling NvTegraSysGetPlatform().
 * - If any of above two or both are NULL return false.
 * - True if any/all of the below condition is true.
 *   - CPU type is "vdk" or
 *   - CPU type is "dsim" or
 *   - CPU type is "asim" or
 *   - platform info is "quickturn".
 * @return Returns true if it's simulation else returns false.
 */
bool nvgpu_platform_is_simulation(struct gk20a *g);

/**
 * @brief Check whether running fpga or not.
 *
 * @param g [in]	GPU super structure.
 *
 * - Get tegra Platform info using NvTegraSysGetPlatform().
 * - Return false if return info is NULL.
 * - Return true if any/all of the below condition is true.
 *   - Info is  "system_fpga" or
 *   - Info is  "unit_fpga".
 * @return Returns true if it's fpga else returns false.
 */
bool nvgpu_platform_is_fpga(struct gk20a *g);

/**
 * @brief Check whether running in virtualized environment.
 *
 * @param g [in]	GPU super structure.
 *
 * - Return true if NvHvCheckOsNative() is successful.
 *
 * @return Returns true if it's virtualized environment else returns false.
 */
bool nvgpu_is_hypervisor_mode(struct gk20a *g);

/**
 * @brief Check whether soc is t194 and revision a01.
 *
 * @param g [in]	GPU super structure.
 *
 * - Return true only if NvTegraSysGetChipId() is equal to TEGRA_CHIPID_TEGRA19
 *   and NvTegraSysGetChipRevision() is equal to TEGRA_REVISION_A01.
 * @return Returns true if soc is t194-a01 else returns false.
 */
bool nvgpu_is_soc_t194_a01(struct gk20a *g);

/**
 * @brief Do soc related init
 *
 * @param g [in]	GPU super structure.
 *
 * - Set VMID_UNINITIALIZED to r->gid.
 * - Check if nvgpu_is_hypervisor_mode is enabled if yes then
 *   - If module is not virtual then set nvgpu_hv_ipa_pa to desc->phys_addr.
 *
 * @return 0 in case of success, <0 in case of call to NvHvGetOsVmId() gets fail.
 * @retval ENODEV Get VM id fails to open device node.
 * @retval EINVAL Invalid argument
 * @retval EFAULT devctl call to nvhv device node returned failure
 */
int nvgpu_init_soc_vars(struct gk20a *g);

#else /* CONFIG_NVGPU_TEGRA_FUSE */

static inline bool nvgpu_platform_is_silicon(struct gk20a *g)
{
	return true;
}

static inline bool nvgpu_platform_is_simulation(struct gk20a *g)
{
	return false;
}

static inline bool nvgpu_platform_is_fpga(struct gk20a *g)
{
	return false;
}

static inline bool nvgpu_is_hypervisor_mode(struct gk20a *g)
{
	return false;
}

static inline bool nvgpu_is_soc_t194_a01(struct gk20a *g)
{
	return false;
}

static inline int nvgpu_init_soc_vars(struct gk20a *g)
{
	return 0;
}
#endif /* CONFIG_NVGPU_TEGRA_FUSE */

/**
 * @brief Get the physical address from the given intermediate physical address.
 *
 * @param[in]  g	Pointer to GPU structure.
 * @param[in]  ipa	Intermediate physical address.
 *
 * @return translated physical address.
 */
u64 nvgpu_get_pa_from_ipa(struct gk20a *g, u64 ipa);

#endif /* NVGPU_SOC_H */
