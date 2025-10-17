/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_ZCULL_PRIV_H
#define NVGPU_GR_ZCULL_PRIV_H

#include <nvgpu/types.h>

struct gk20a;

struct nvgpu_gr_zcull {
	struct gk20a *g;

	u32 aliquot_width;
	u32 aliquot_height;
	u32 aliquot_size;
	u32 total_aliquots;

	u32 width_align_pixels;
	u32 height_align_pixels;
	u32 pixel_squares_by_aliquots;

	u32 zcull_ctxsw_image_size;
};

#endif /* NVGPU_GR_ZCULL_PRIV_H */
