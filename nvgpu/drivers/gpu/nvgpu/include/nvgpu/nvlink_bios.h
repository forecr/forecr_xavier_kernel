/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_NVLINK_BIOS_H
#define NVGPU_NVLINK_BIOS_H

#include <nvgpu/types.h>
#include <nvgpu/utils.h>

#define NVLINK_CONFIG_DATA_HDR_VER_10           0x1U
#define NVLINK_CONFIG_DATA_HDR_10_SIZE          16U
#define NVLINK_CONFIG_DATA_HDR_11_SIZE          17U
#define NVLINK_CONFIG_DATA_HDR_12_SIZE          21U

struct nvlink_config_data_hdr_v1 {
	u8 version;
	u8 hdr_size;
	u16 rsvd0;
	u32 link_disable_mask;
	u32 link_mode_mask;
	u32 link_refclk_mask;
	u8 train_at_boot;
	u32 ac_coupling_mask;
} __attribute__((packed));

struct gk20a;

int nvgpu_bios_get_nvlink_config_data(struct gk20a *g);

#endif
