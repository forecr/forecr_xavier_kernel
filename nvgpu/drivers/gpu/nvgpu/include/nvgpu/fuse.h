/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_FUSE_H
#define NVGPU_FUSE_H
/**
 * @file
 *
 * Interface for fuse ops.
 * @page fuse fuse
 *
 * Overview
 * ========
 *
 * The |nvgpu-rm| fuse unit provides a set of interfaces to read or write
 * various fuse registers of SoC of GPU. The fuse unit initialization part is
 * responsible for:
 *
 * - Fetching iGPU specific fuse IO region from Device Tree
 * - Mapping fuse IO region in |nvgpu-rm| address space
 * - Un-mapping fuse IO region from |nvgpu-rm| address space
 *
 * Once the fuse unit is successfully initialized, it facilitates reads and
 * writes of FUSEs. Any failure in fuse unit initialization is reported as
 * an error to the init unit.
 *
 * Static Design
 * =============
 *
 * External APIs
 * -------------
 *   + nvgpu_rmos_tegra_fuse_init()
 *   + nvgpu_rmos_tegra_fuse_deinit()
 *   + nvgpu_tegra_fuse_write_bypass()
 *   + nvgpu_tegra_fuse_write_access_sw()
 *   + nvgpu_tegra_fuse_write_opt_gpu_tpc0_disable()
 *   + nvgpu_tegra_fuse_write_opt_gpu_tpc1_disable()
 *   + nvgpu_tegra_fuse_read_gcplex_config_fuse()
 *
 * Following are the functions exposed as callbacks.
 *   + gops_fuse.check_priv_security
 *   + gops_fuse.fuse_ctrl_opt_tpc_gpc
 *   + gops_fuse.fuse_opt_priv_sec_en
 *   + gops_fuse.fuse_status_opt_fbio
 *   + gops_fuse.fuse_status_opt_fbp
 *   + gops_fuse.fuse_status_opt_tpc_gpc
 *   + gops_fuse.is_opt_ecc_enable
 *   + gops_fuse.is_opt_feature_override_disable
 *   + gops_fuse.read_gcplex_config_fuse
 *
 */

struct gk20a;

#include <nvgpu/types.h>
#include <nvgpu/errno.h>

struct nvgpu_fuse_feature_override_ecc {
	/** overide_ecc register feature */
	/** sm_lrf enable */
	bool sm_lrf_enable;
	/** sm_lrf override */
	bool sm_lrf_override;
	/** sm_l1_data enable */
	bool sm_l1_data_enable;
	/** sm_l1_data overide */
	bool sm_l1_data_override;
	/** sm_l1_tag enable */
	bool sm_l1_tag_enable;
	/** sm_l1_tag overide */
	bool sm_l1_tag_override;
	/** ltc enable */
	bool ltc_enable;
	/** ltc overide */
	bool ltc_override;
	/** dram enable */
	bool dram_enable;
	/** dram overide */
	bool dram_override;
	/** sm_cbu enable */
	bool sm_cbu_enable;
	/** sm_cbu overide */
	bool sm_cbu_override;

	/** override_ecc_1 register feature */
	/** sm_l0_icache enable */
	bool sm_l0_icache_enable;
	/** sm_l0_icache overide */
	bool sm_l0_icache_override;
	/** sm_l1_icache enable */
	bool sm_l1_icache_enable;
	/** sm_l1_icache overide */
	bool sm_l1_icache_override;
};

#define GCPLEX_CONFIG_VPR_AUTO_FETCH_DISABLE_MASK	BIT32(0)
#define GCPLEX_CONFIG_VPR_ENABLED_MASK			BIT32(1)
#define GCPLEX_CONFIG_WPR_ENABLED_MASK			BIT32(2)

#ifdef CONFIG_NVGPU_NON_FUSA
int nvgpu_tegra_get_gpu_speedo_id(struct gk20a *g, int *id);
int nvgpu_tegra_fuse_read_reserved_calib(struct gk20a *g, u32 *val);
#endif /* CONFIG_NVGPU_NON_FUSA */

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
int nvgpu_tegra_fuse_read_gcplex_config_fuse(struct gk20a *g, u32 *val);

/**
 * @brief - Reads FUSE_OPT_GPC_DISABLE_0 fuse.
 *
 * @param g [in] - GPU super structure.
 * @param val [out] - Populated with register FUSE_OPT_GPC_DISABLE_0 value.
 *
 * - Provide information about the GPU GPC floor-sweep info
 *
 * @return 0 on success or negative value on failure.
 *
 */
int nvgpu_tegra_fuse_read_opt_gpc_disable(struct gk20a *g, u32 *val);

/**
 * @brief - Reads FUSE_OPT_EMC_DISABLE_0 fuse.
 *
 * @param g [in] - GPU super structure.
 * @param val [out] - Populated with register FUSE_OPT_EMC_DISABLE_0 value.
 *
 * - Provide information about the SOC EMC floor-sweep info
 *
 * @return 0 on success or negative value on failure.
 *
 */
#ifdef __KERNEL__
int nvgpu_tegra_fuse_read_opt_emc_disable(struct gk20a *g, u32 *val);
#endif

#if defined(CONFIG_NVGPU_HAL_NON_FUSA)
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
int nvgpu_tegra_fuse_read_per_device_identifier(struct gk20a *g, u64 *pdi);
#endif

#ifdef CONFIG_NVGPU_TEGRA_FUSE
/**
 * @brief -  Write Fuse bypass register which controls fuse bypass.
 *
 * @param g [in] - GPU super structure.
 * @param val [in]- 0 : DISABLED, 1 : ENABLED
 *
 * - Write 0/1 to control the fuse bypass.
 *
 * @return none.
 */
void nvgpu_tegra_fuse_write_bypass(struct gk20a *g, u32 val);

/**
 * @brief - Enable software write access
 *
 * @param g [in] - GPU super structure.
 * @param val [in] - 0 : READWRITE, 1 : READONLY
 *
 * - Bit 0 of the register is the write control register. When set to 1,
 *   it disables writes to chip.
 *
 * @return none.
 */
void nvgpu_tegra_fuse_write_access_sw(struct gk20a *g, u32 val);

/**
 * @brief - Disable TPC0
 *
 * @param g [in] - GPU super structure.
 * @param val [in] - 1 : DISABLED, 0 : ENABLED
 *
 * - Write 1/0 to fuse tpc disable register to disable/enable the TPC0.
 *
 * @return none.
 */
void nvgpu_tegra_fuse_write_opt_gpu_tpc0_disable(struct gk20a *g, u32 val);

/**
 * @brief - Disable TPC1
 *
 * @param g [in] - GPU super structure.
 * @param val [in] - 1 : DISABLED, 0 : ENABLED
 *
 * - Write 1/0 to fuse tpc disable register to disable/enable the TPC1.
 *
 * @return none.
 */
void nvgpu_tegra_fuse_write_opt_gpu_tpc1_disable(struct gk20a *g, u32 val);

#else /* CONFIG_NVGPU_TEGRA_FUSE */

static inline void nvgpu_tegra_fuse_write_bypass(struct gk20a *g, u32 val)
{
}

static inline void nvgpu_tegra_fuse_write_access_sw(struct gk20a *g, u32 val)
{
}

static inline void nvgpu_tegra_fuse_write_opt_gpu_tpc0_disable(struct gk20a *g,
							       u32 val)
{
}

static inline void nvgpu_tegra_fuse_write_opt_gpu_tpc1_disable(struct gk20a *g,
							       u32 val)
{
}

#endif /* CONFIG_NVGPU_TEGRA_FUSE */

#endif /* NVGPU_FUSE_H */
