// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/fuse.h>

#include <nvgpu/linux/soc_fuse.h>
#include <nvgpu/linux/nvmem.h>

#include <soc/tegra/fuse.h>

int nvgpu_tegra_get_gpu_speedo_id(struct gk20a *g, int *id)
{
	*id = tegra_sku_info.gpu_speedo_id;

	return 0;
}

int nvgpu_tegra_fuse_read_reserved_calib(struct gk20a *g, u32 *val)
{
#ifdef CONFIG_NVGPU_NVMEM_FUSE
	return nvgpu_tegra_nvmem_read_reserved_calib(g, val);
#else
	return tegra_fuse_readl(FUSE_RESERVED_CALIB0_0, val);
#endif
}

int nvgpu_tegra_fuse_read_gcplex_config_fuse(struct gk20a *g, u32 *val)
{
#ifdef CONFIG_NVGPU_NVMEM_FUSE
	return nvgpu_tegra_nvmem_read_gcplex_config_fuse(g, val);
#else
	return tegra_fuse_readl(FUSE_GCPLEX_CONFIG_FUSE_0, val);
#endif
}

int nvgpu_tegra_fuse_read_opt_gpc_disable(struct gk20a *g, u32 *val)
{
	return tegra_fuse_readl(FUSE_OPT_GPC_DISABLE_0, val);
}
#ifdef __KERNEL__
int nvgpu_tegra_fuse_read_opt_emc_disable(struct gk20a *g, u32 *val)
{
	return tegra_fuse_readl(FUSE_OPT_EMC_DISABLE_0, val);
}
#endif

#if defined(CONFIG_NVGPU_HAL_NON_FUSA)
int nvgpu_tegra_fuse_read_per_device_identifier(struct gk20a *g, u64 *pdi)
{
#ifdef CONFIG_NVGPU_NVMEM_FUSE
	return nvgpu_tegra_nvmem_read_per_device_identifier(g, pdi);
#else
	u32 lo = 0U;
	u32 hi = 0U;
	int err;

	err = tegra_fuse_readl(FUSE_PDI0, &lo);
	if (err)
		return err;

	err = tegra_fuse_readl(FUSE_PDI1, &hi);
	if (err)
		return err;

	*pdi = ((u64)lo) | (((u64)hi) << 32);

	return 0;
#endif
}
#endif

#ifdef CONFIG_NVGPU_TEGRA_FUSE
/*
 * Use tegra_fuse_control_read/write() APIs for fuse offsets upto 0x100
 * Use tegra_fuse_readl/writel() APIs for fuse offsets above 0x100
 */
void nvgpu_tegra_fuse_write_bypass(struct gk20a *g, u32 val)
{
	tegra_fuse_control_write(val, FUSE_FUSEBYPASS_0);
}

void nvgpu_tegra_fuse_write_access_sw(struct gk20a *g, u32 val)
{
	tegra_fuse_control_write(val, FUSE_WRITE_ACCESS_SW_0);
}

void nvgpu_tegra_fuse_write_opt_gpu_tpc0_disable(struct gk20a *g, u32 val)
{
	tegra_fuse_writel(val, FUSE_OPT_GPU_TPC0_DISABLE_0);
}

void nvgpu_tegra_fuse_write_opt_gpu_tpc1_disable(struct gk20a *g, u32 val)
{
	tegra_fuse_writel(val, FUSE_OPT_GPU_TPC1_DISABLE_0);
}

#endif /* CONFIG_NVGPU_TEGRA_FUSE */
