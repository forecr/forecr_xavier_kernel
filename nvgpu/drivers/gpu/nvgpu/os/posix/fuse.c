// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/fuse.h>

#include <os/posix/os_posix.h>

#include <nvgpu/posix/io.h>
#include <nvgpu/posix/soc_fuse.h>

#ifdef CONFIG_NVGPU_NON_FUSA
int nvgpu_tegra_get_gpu_speedo_id(struct gk20a *g, int *id)
{
	return 0;
}

int nvgpu_tegra_fuse_read_reserved_calib(struct gk20a *g, u32 *val)
{
	struct nvgpu_os_posix *p = nvgpu_os_posix_from_gk20a(g);

	if (p->callbacks == NULL || p->callbacks->tegra_fuse_readl == NULL) {
		return -ENODEV;
	}

	return p->callbacks->tegra_fuse_readl(FUSE_RESERVED_CALIB0_0, val);
}
#endif

int nvgpu_tegra_fuse_read_gcplex_config_fuse(struct gk20a *g, u32 *val)
{
	struct nvgpu_os_posix *p = nvgpu_os_posix_from_gk20a(g);

	if (p->callbacks == NULL || p->callbacks->tegra_fuse_readl == NULL) {
		/*
		 * Generally for nvgpu, if priv_sec is enabled, we are expecting
		 * WPR to be enabled and auto fetching of VPR to _not_ be
		 * disabled (in other words VPR autofetch to be enabled - cause
		 * that's not confusing at all).
		 */
		*val = GCPLEX_CONFIG_WPR_ENABLED_MASK;

		return 0;
	}

	return p->callbacks->tegra_fuse_readl(FUSE_GCPLEX_CONFIG_FUSE_0, val);
}

#ifdef CONFIG_NVGPU_NON_FUSA
int nvgpu_tegra_fuse_read_per_device_identifier(struct gk20a *g, u64 *pdi)
{
	(void)g;
	*pdi = 0;

	return 0;
}
#endif

#ifdef CONFIG_NVGPU_TEGRA_FUSE
/*
 * Use tegra_fuse_control_read/write() APIs for fuse offsets upto 0x100
 * Use tegra_fuse_readl/writel() APIs for fuse offsets above 0x100
 */
void nvgpu_tegra_fuse_write_bypass(struct gk20a *g, u32 val)
{
	struct nvgpu_os_posix *p = nvgpu_os_posix_from_gk20a(g);

	if (p->callbacks == NULL ||
			p->callbacks->tegra_fuse_control_write == NULL) {
		return;
	}
	p->callbacks->tegra_fuse_control_write(val, FUSE_FUSEBYPASS_0);
}

void nvgpu_tegra_fuse_write_access_sw(struct gk20a *g, u32 val)
{
	struct nvgpu_os_posix *p = nvgpu_os_posix_from_gk20a(g);

	if (p->callbacks == NULL ||
			p->callbacks->tegra_fuse_control_write == NULL) {
		return;
	}

	p->callbacks->tegra_fuse_control_write(val, FUSE_WRITE_ACCESS_SW_0);
}

void nvgpu_tegra_fuse_write_opt_gpu_tpc0_disable(struct gk20a *g, u32 val)
{
	struct nvgpu_os_posix *p = nvgpu_os_posix_from_gk20a(g);

	if (p->callbacks == NULL || p->callbacks->tegra_fuse_writel == NULL) {
		return;
	}

	p->callbacks->tegra_fuse_writel(val, FUSE_OPT_GPU_TPC0_DISABLE_0);
}

void nvgpu_tegra_fuse_write_opt_gpu_tpc1_disable(struct gk20a *g, u32 val)
{
	struct nvgpu_os_posix *p = nvgpu_os_posix_from_gk20a(g);

	if (p->callbacks == NULL || p->callbacks->tegra_fuse_writel == NULL) {
		return;
	}

	return p->callbacks->tegra_fuse_writel(val, FUSE_OPT_GPU_TPC1_DISABLE_0);
}

#endif /* CONFIG_NVGPU_TEGRA_FUSE */
