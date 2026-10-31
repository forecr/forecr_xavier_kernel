/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FBP_PRIV_H
#define NVGPU_FBP_PRIV_H

struct nvgpu_fbp {
	u32 num_fbps;
	u32 max_fbps_count;
	u32 fbp_en_mask;
	u32 *fbp_l2_en_mask;

	/**
	 * Array to hold the mapping of fbp logical index to physical index
	 * with physical index as array index and logical index as value.
	 */
	u32 *fbp_physical_id_map;
};

#endif /* NVGPU_FBP_PRIV_H */
