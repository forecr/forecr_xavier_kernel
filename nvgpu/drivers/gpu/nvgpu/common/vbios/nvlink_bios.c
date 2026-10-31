// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/nvlink_bios.h>
#include <nvgpu/string.h>
#include <nvgpu/bios.h>

int nvgpu_bios_get_nvlink_config_data(struct gk20a *g)
{
	int ret = 0;
	struct nvlink_config_data_hdr_v1 config;

	if (g->bios->nvlink_config_data_offset == 0U) {
		return -EINVAL;
	}

	nvgpu_memcpy((u8 *)&config,
		&g->bios->data[g->bios->nvlink_config_data_offset],
		sizeof(config));

	if (config.version != NVLINK_CONFIG_DATA_HDR_VER_10) {
		nvgpu_err(g, "unsupported nvlink bios version: 0x%x",
				config.version);
		return -EINVAL;
	}

	switch (config.hdr_size) {
	case NVLINK_CONFIG_DATA_HDR_12_SIZE:
		g->nvlink.ac_coupling_mask = config.ac_coupling_mask;
		g->nvlink.train_at_boot = config.train_at_boot;
		g->nvlink.link_disable_mask = config.link_disable_mask;
		g->nvlink.link_mode_mask = config.link_mode_mask;
		g->nvlink.link_refclk_mask = config.link_refclk_mask;
		break;
	case NVLINK_CONFIG_DATA_HDR_11_SIZE:
		g->nvlink.train_at_boot = config.train_at_boot;
		g->nvlink.link_disable_mask = config.link_disable_mask;
		g->nvlink.link_mode_mask = config.link_mode_mask;
		g->nvlink.link_refclk_mask = config.link_refclk_mask;
		break;
	case NVLINK_CONFIG_DATA_HDR_10_SIZE:
		g->nvlink.link_disable_mask = config.link_disable_mask;
		g->nvlink.link_mode_mask = config.link_mode_mask;
		g->nvlink.link_refclk_mask = config.link_refclk_mask;
		break;
	default:
		nvgpu_err(g, "invalid nvlink bios config size");
		ret = -EINVAL;
		break;
	}

	return ret;
}
