// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/linux/nvmem.h>
#include <nvgpu/log.h>

#include <linux/nvmem-consumer.h>

#include "os_linux.h"

#define NVMEM_CELL_GCPLEX_CONFIG_FUSE	"gcplex-config-fuse"
#define NVMEM_CELL_CALIBRATION		"calibration"
#define NVMEM_CELL_PDI0			"pdi0"
#define NVMEM_CELL_PDI1			"pdi1"

int nvgpu_tegra_nvmem_read_reserved_calib(struct gk20a *g, u32 *val)
{
	struct device *dev = dev_from_gk20a(g);
	int ret;

	ret = nvmem_cell_read_u32(dev, NVMEM_CELL_CALIBRATION, val);
	if (ret < 0) {
		nvgpu_err(g, "%s nvmem cell read failed %d",
			  NVMEM_CELL_CALIBRATION, ret);
		return ret;
	}

	return 0;
}

int nvgpu_tegra_nvmem_read_gcplex_config_fuse(struct gk20a *g, u32 *val)
{
	struct device *dev = dev_from_gk20a(g);
	int ret;

	ret = nvmem_cell_read_u32(dev, NVMEM_CELL_GCPLEX_CONFIG_FUSE, val);
	if (ret < 0) {
		nvgpu_err(g, "%s nvmem cell read failed %d",
			  NVMEM_CELL_GCPLEX_CONFIG_FUSE, ret);
		return ret;
	}

	return 0;
}

int nvgpu_tegra_nvmem_read_per_device_identifier(struct gk20a *g, u64 *pdi)
{
	struct device *dev = dev_from_gk20a(g);
	u32 lo = 0U;
	u32 hi = 0U;
	int err;

	err = nvmem_cell_read_u32(dev, NVMEM_CELL_PDI0, &lo);
	if (err) {
		return err;
	}

	err = nvmem_cell_read_u32(dev, NVMEM_CELL_PDI1, &hi);
	if (err) {
		return err;
	}

	*pdi = ((u64)lo) | (((u64)hi) << 32);

	return 0;
}
